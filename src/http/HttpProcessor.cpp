#include "http/HttpProcessor.hpp"
#include <stdexcept>

using namespace std;

HttpResponse HttpProcessor::process(const HttpRequest& request, 
                                 const vector<HttpResponse>& possibleResponses) {
    try {
        // 如果有提供可能的响应，使用内容协商
        if (!possibleResponses.empty()) {
            return ContentNegotiator::negotiate(request, possibleResponses);
        }

        // 否则准备默认响应
        auto responses = prepareResponses(request);
        if (!responses.empty()) {
            return ContentNegotiator::negotiate(request, responses);
        }

        // 没有可用的响应，返回404
        return createErrorResponse(404, "Not Found");
    } catch (const exception& e) {
        // 处理过程中出错，返回500
        return createErrorResponse(500, "Internal Server Error: " + string(e.what()));
    }
}

vector<HttpResponse> HttpProcessor::prepareResponses(const HttpRequest& request) {
    vector<HttpResponse> responses;

    // 示例：为同一资源准备不同表示形式
    if (request.getPath() == "/example") {
        // JSON表示
        HttpResponse jsonResp;
        jsonResp.setStatusCode(200);
        jsonResp.setHeader("Content-Type", "application/json");
        jsonResp.setBody(R"({"message": "Hello in JSON"})");
        responses.push_back(jsonResp);

        // XML表示
        HttpResponse xmlResp;
        xmlResp.setStatusCode(200);
        xmlResp.setHeader("Content-Type", "application/xml");
        xmlResp.setBody("<message>Hello in XML</message>");
        responses.push_back(xmlResp);

        // HTML表示
        HttpResponse htmlResp;
        htmlResp.setStatusCode(200);
        htmlResp.setHeader("Content-Type", "text/html");
        htmlResp.setBody("<html><body><h1>Hello in HTML</h1></body></html>");
        responses.push_back(htmlResp);
    }

    return responses;
}

HttpResponse HttpProcessor::createErrorResponse(int statusCode, const string& message) {
    HttpResponse response;
    response.setStatusCode(statusCode);
    response.setHeader("Content-Type", "text/plain");
    response.setBody(message);
    return response;
}