#ifndef LANGCHAIN_HTTP_CLIENT_H
#define LANGCHAIN_HTTP_CLIENT_H

#include "core.h"
#include <string>
#include <map>
#include <memory>

namespace langchain {

class HttpClient {
public:
    struct HttpResponse {
        int status_code;
        String body;
        std::map<String, String> headers;
    };

    struct HttpRequest {
        String url;
        String method;
        String body;
        std::map<String, String> headers;
        int timeout_seconds;

        HttpRequest() : timeout_seconds(30) {}
    };

    HttpClient();
    ~HttpClient();

    // 初始化客户端
    bool initialize();

    // 发送HTTP请求
    HttpResponse send_request(const HttpRequest& request);

    // GET请求
    HttpResponse get(const String& url,
                    const std::map<String, String>& headers = {},
                    int timeout_seconds = 30);

    // POST请求
    HttpResponse post(const String& url,
                     const String& body,
                     const std::map<String, String>& headers = {},
                     int timeout_seconds = 30);

private:
    class HttpClientImpl;
    std::unique_ptr<HttpClientImpl> impl_;
};

} // namespace langchain

#endif // LANGCHAIN_HTTP_CLIENT_H