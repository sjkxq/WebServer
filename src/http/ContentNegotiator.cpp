#include "http/ContentNegotiator.hpp"
#include <algorithm>
#include <sstream>
#include <cctype>

using namespace std;

HttpResponse ContentNegotiator::negotiate(const HttpRequest& request, 
                                       const vector<HttpResponse>& possibleResponses) {
    if (possibleResponses.empty()) {
        throw runtime_error("No possible responses provided");
    }

    // 如果只有一个选项，直接返回
    if (possibleResponses.size() == 1) {
        return possibleResponses.front();
    }

    // 收集所有可用的内容类型、语言等
    vector<string> availableTypes, availableLangs, availableEncodings, availableCharsets;
    
    for (const auto& response : possibleResponses) {
        availableTypes.push_back(response.getHeader("Content-Type"));
        if (response.hasHeader("Content-Language")) {
            availableLangs.push_back(response.getHeader("Content-Language"));
        }
        if (response.hasHeader("Content-Encoding")) {
            availableEncodings.push_back(response.getHeader("Content-Encoding"));
        }
        if (response.hasHeader("Content-Charset")) {
            availableCharsets.push_back(response.getHeader("Content-Charset"));
        }
    }

    // 获取客户端偏好
    string acceptHeader = request.getHeader("Accept");
    string acceptLanguageHeader = request.getHeader("Accept-Language");
    string acceptEncodingHeader = request.getHeader("Accept-Encoding");
    string acceptCharsetHeader = request.getHeader("Accept-Charset");

    // 协商最佳选项
    string bestType = negotiateContentType(acceptHeader, availableTypes);
    string bestLang = negotiateLanguage(acceptLanguageHeader, availableLangs);
    string bestEncoding = negotiateEncoding(acceptEncodingHeader, availableEncodings);
    string bestCharset = negotiateCharset(acceptCharsetHeader, availableCharsets);

    // 查找匹配的响应
    for (const auto& response : possibleResponses) {
        bool typeMatch = bestType.empty() || response.getHeader("Content-Type") == bestType;
        bool langMatch = bestLang.empty() || 
                        (response.hasHeader("Content-Language") && 
                         response.getHeader("Content-Language") == bestLang);
        bool encodingMatch = bestEncoding.empty() || 
                           (response.hasHeader("Content-Encoding") && 
                            response.getHeader("Content-Encoding") == bestEncoding);
        bool charsetMatch = bestCharset.empty() || 
                          (response.hasHeader("Content-Charset") && 
                           response.getHeader("Content-Charset") == bestCharset);

        if (typeMatch && langMatch && encodingMatch && charsetMatch) {
            return response;
        }
    }

    // 没有完全匹配，返回第一个
    return possibleResponses.front();
}

vector<pair<string, float>> ContentNegotiator::parseQualityValues(const string& headerValue) {
    vector<pair<string, float>> result;
    istringstream iss(headerValue);
    string item;

    while (getline(iss, item, ',')) {
        // 去除空格
        item.erase(remove_if(item.begin(), item.end(), ::isspace), item.end());

        size_t pos = item.find(';');
        string value = item.substr(0, pos);
        float q = 1.0f; // 默认质量值

        if (pos != string::npos) {
            string qParam = item.substr(pos + 1);
            if (qParam.find("q=") == 0) {
                try {
                    q = stof(qParam.substr(2));
                } catch (...) {
                    // 忽略无效的质量值
                }
            }
        }

        result.emplace_back(value, q);
    }

    // 按质量值降序排序
    sort(result.begin(), result.end(), 
        [](const pair<string, float>& a, const pair<string, float>& b) {
            return a.second > b.second;
        });

    return result;
}

string ContentNegotiator::negotiateContentType(
    const string& acceptHeader,
    const vector<string>& availableTypes) {
    
    if (acceptHeader.empty() || availableTypes.empty()) {
        return "";
    }

    auto preferences = parseQualityValues(acceptHeader);

    for (const auto& pref : preferences) {
        const string& type = pref.first;
        float q = pref.second;

        if (q == 0) continue; // 质量值为0表示不接受

        for (const auto& available : availableTypes) {
            if (type == "*/*") {
                return available; // 返回第一个可用的
            }

            size_t slashPos = type.find('/');
            if (slashPos == string::npos) continue;

            string typeMain = type.substr(0, slashPos);
            string typeSub = type.substr(slashPos + 1);

            size_t availSlashPos = available.find('/');
            if (availSlashPos == string::npos) continue;

            string availMain = available.substr(0, availSlashPos);
            string availSub = available.substr(availSlashPos + 1);

            if (typeSub == "*" && typeMain == availMain) {
                return available; // 主类型匹配
            }

            if (type == available) {
                return available; // 精确匹配
            }
        }
    }

    return ""; // 没有匹配
}

string ContentNegotiator::negotiateLanguage(
    const string& acceptLanguageHeader,
    const vector<string>& availableLangs) {
    
    if (acceptLanguageHeader.empty() || availableLangs.empty()) {
        return "";
    }

    auto preferences = parseQualityValues(acceptLanguageHeader);

    for (const auto& pref : preferences) {
        const string& langRange = pref.first;
        float q = pref.second;

        if (q == 0) continue;

        for (const auto& available : availableLangs) {
            if (langRange == "*") {
                return available;
            }

            if (langRange == available) {
                return available;
            }

            // 检查主语言标签匹配 (en-US匹配en)
            size_t dashPos = langRange.find('-');
            if (dashPos != string::npos) {
                string mainTag = langRange.substr(0, dashPos);
                if (available.find(mainTag) == 0) {
                    return available;
                }
            }
        }
    }

    return "";
}

string ContentNegotiator::negotiateEncoding(
    const string& acceptEncodingHeader,
    const vector<string>& availableEncodings) {
    
    if (acceptEncodingHeader.empty() || availableEncodings.empty()) {
        return "";
    }

    auto preferences = parseQualityValues(acceptEncodingHeader);

    for (const auto& pref : preferences) {
        const string& encoding = pref.first;
        float q = pref.second;

        if (q == 0) continue;

        for (const auto& available : availableEncodings) {
            if (encoding == "*") {
                return available;
            }

            if (encoding == available) {
                return available;
            }
        }
    }

    return "";
}

string ContentNegotiator::negotiateCharset(
    const string& acceptCharsetHeader,
    const vector<string>& availableCharsets) {
    
    if (acceptCharsetHeader.empty() || availableCharsets.empty()) {
        return "";
    }

    auto preferences = parseQualityValues(acceptCharsetHeader);

    for (const auto& pref : preferences) {
        const string& charset = pref.first;
        float q = pref.second;

        if (q == 0) continue;

        for (const auto& available : availableCharsets) {
            if (charset == "*") {
                return available;
            }

            if (charset == available) {
                return available;
            }
        }
    }

    return "";
}