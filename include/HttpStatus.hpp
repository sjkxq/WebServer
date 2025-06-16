#ifndef HTTP_STATUS_HPP
#define HTTP_STATUS_HPP

#include <string>
#include <unordered_map>

/**
 * @brief HTTP状态码枚举类
 */
enum class HttpStatus {
    // 1xx: 信息性状态码
    CONTINUE = 100,
    SWITCHING_PROTOCOLS = 101,

    // 2xx: 成功状态码
    OK = 200,
    CREATED = 201,
    ACCEPTED = 202,
    NO_CONTENT = 204,
    PARTIAL_CONTENT = 206,

    // 3xx: 重定向状态码
    MULTIPLE_CHOICES = 300,
    MOVED_PERMANENTLY = 301,
    FOUND = 302,
    SEE_OTHER = 303,
    NOT_MODIFIED = 304,
    TEMPORARY_REDIRECT = 307,
    PERMANENT_REDIRECT = 308,

    // 4xx: 客户端错误状态码
    BAD_REQUEST = 400,
    UNAUTHORIZED = 401,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    METHOD_NOT_ALLOWED = 405,
    NOT_ACCEPTABLE = 406,
    REQUEST_TIMEOUT = 408,
    CONFLICT = 409,
    GONE = 410,
    LENGTH_REQUIRED = 411,
    PAYLOAD_TOO_LARGE = 413,
    URI_TOO_LONG = 414,
    UNSUPPORTED_MEDIA_TYPE = 415,
    RANGE_NOT_SATISFIABLE = 416,
    EXPECTATION_FAILED = 417,
    UPGRADE_REQUIRED = 426,
    TOO_MANY_REQUESTS = 429,

    // 5xx: 服务器错误状态码
    INTERNAL_SERVER_ERROR = 500,
    NOT_IMPLEMENTED = 501,
    BAD_GATEWAY = 502,
    SERVICE_UNAVAILABLE = 503,
    GATEWAY_TIMEOUT = 504,
    HTTP_VERSION_NOT_SUPPORTED = 505
};

/**
 * @brief HTTP状态码处理类
 */
class HttpStatusHandler {
public:
    /**
     * @brief 获取单例实例
     * @return HttpStatusHandler的单例实例
     */
    static HttpStatusHandler& getInstance();

    /**
     * @brief 获取状态码对应的状态消息
     * @param status HTTP状态码
     * @return 状态消息
     */
    std::string getStatusMessage(HttpStatus status) const;

    /**
     * @brief 获取状态码对应的状态消息
     * @param statusCode HTTP状态码数值
     * @return 状态消息
     */
    std::string getStatusMessage(int statusCode) const;

    /**
     * @brief 检查状态码是否为信息性状态码(1xx)
     * @param status HTTP状态码
     * @return 如果是1xx状态码返回true，否则返回false
     */
    static bool isInformational(HttpStatus status);

    /**
     * @brief 检查状态码是否为成功状态码(2xx)
     * @param status HTTP状态码
     * @return 如果是2xx状态码返回true，否则返回false
     */
    static bool isSuccessful(HttpStatus status);

    /**
     * @brief 检查状态码是否为重定向状态码(3xx)
     * @param status HTTP状态码
     * @return 如果是3xx状态码返回true，否则返回false
     */
    static bool isRedirection(HttpStatus status);

    /**
     * @brief 检查状态码是否为客户端错误状态码(4xx)
     * @param status HTTP状态码
     * @return 如果是4xx状态码返回true，否则返回false
     */
    static bool isClientError(HttpStatus status);

    /**
     * @brief 检查状态码是否为服务器错误状态码(5xx)
     * @param status HTTP状态码
     * @return 如果是5xx状态码返回true，否则返回false
     */
    static bool isServerError(HttpStatus status);

private:
    HttpStatusHandler();  // 私有构造函数
    HttpStatusHandler(const HttpStatusHandler&) = delete;  // 禁止拷贝
    HttpStatusHandler& operator=(const HttpStatusHandler&) = delete;  // 禁止赋值

    std::unordered_map<int, std::string> statusMessages_;  // 状态码到状态消息的映射
};

#endif // HTTP_STATUS_HPP