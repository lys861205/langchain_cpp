#include "../include/langchain/http_client.h"

#ifdef USE_BRPC
#include <brpc/channel.h>
#include <brpc/controller.h>
#include <butil/logging.h>
#else
#include <curl/curl.h>
#endif

#include <iostream>

namespace langchain {

#ifdef USE_BRPC

class HttpClient::HttpClientImpl {
public:
    HttpClientImpl() : channel_(nullptr) {}

    bool initialize() {
        brpc::ChannelOptions options;
        options.protocol = "http";
        options.timeout_ms = 30000;  // 30 seconds
        options.max_retry = 3;

        channel_ = std::make_unique<brpc::Channel>();
        if (channel_->Init("0.0.0.0:0", &options) != 0) {
            std::cerr << "Failed to initialize brpc channel" << std::endl;
            return false;
        }
        return true;
    }

    HttpResponse send_request(const HttpRequest& request) {
        HttpResponse response;

        if (!channel_) {
            response.status_code = -1;
            response.body = "Error: HTTP client not initialized";
            return response;
        }

        brpc::Controller cntl;
        cntl.set_timeout_ms(request.timeout_seconds * 1000);

        // 设置HTTP方法
        if (request.method == "POST") {
            cntl.http_request().set_method(brpc::HTTP_METHOD_POST);
        } else {
            cntl.http_request().set_method(brpc::HTTP_METHOD_GET);
        }

        // 设置URL
        cntl.http_request().uri() = request.url;

        // 设置请求体（如果是POST）
        if (!request.body.empty()) {
            cntl.request_attachment().append(request.body);
        }

        // 设置请求头
        for (const auto& header : request.headers) {
            cntl.http_request().SetHeader(header.first, header.second);
        }

        // 发送请求
        channel_->CallMethod(nullptr, &cntl, nullptr, nullptr, nullptr);

        if (cntl.Failed()) {
            response.status_code = -1;
            response.body = "Error: " + String(cntl.ErrorText());
            return response;
        }

        response.status_code = cntl.http_response().status_code();
        response.body = cntl.response_attachment().to_string();

        return response;
    }

private:
    std::unique_ptr<brpc::Channel> channel_;
};

#else

// CURL回调函数
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t total_size = size * nmemb;
    response->append((char*)contents, total_size);
    return total_size;
}

class HttpClient::HttpClientImpl {
public:
    HttpClientImpl() : is_initialized_(false) {
        // Initialize CURL globally
        CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);
        if (res == CURLE_OK) {
            is_initialized_ = true;
        }
    }

    ~HttpClientImpl() {
        if (is_initialized_) {
            curl_global_cleanup();
        }
    }

    bool initialize() {
        // CURL全局初始化已经在构造函数中完成
        return is_initialized_;
    }

    HttpResponse send_request(const HttpRequest& request) {
        HttpResponse response;

        if (!is_initialized_) {
            response.status_code = -1;
            response.body = "Error: HTTP client not initialized";
            return response;
        }

        CURL* curl = curl_easy_init();
        if (!curl) {
            response.status_code = -1;
            response.body = "Error: Failed to initialize CURL";
            return response;
        }

        std::string response_string;

        // 设置URL
        curl_easy_setopt(curl, CURLOPT_URL, request.url.c_str());

        // 设置写回调函数
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

        // 设置超时
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, request.timeout_seconds);

        // 设置User-Agent
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "LangChain-CPP/1.0");

        // 设置重定向
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        // 设置请求方法和数据
        if (request.method == "POST") {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            if (!request.body.empty()) {
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.body.c_str());
            }
        }

        // 设置请求头
        struct curl_slist* headers = nullptr;
        for (const auto& header : request.headers) {
            std::string header_str = header.first + ": " + header.second;
            headers = curl_slist_append(headers, header_str.c_str());
        }
        if (headers) {
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        }

        // 发送请求
        CURLcode res = curl_easy_perform(curl);

        // 获取HTTP状态码
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        response.status_code = static_cast<int>(http_code);

        if (res != CURLE_OK) {
            response.status_code = -1;
            response.body = "Error: " + std::string(curl_easy_strerror(res));
        } else {
            response.body = response_string;
        }

        // 清理
        if (headers) {
            curl_slist_free_all(headers);
        }
        curl_easy_cleanup(curl);

        return response;
    }

private:
    bool is_initialized_;
};

#endif

HttpClient::HttpClient() : impl_(std::make_unique<HttpClientImpl>()) {
    initialize();
}

HttpClient::~HttpClient() = default;

bool HttpClient::initialize() {
    return impl_->initialize();
}

HttpClient::HttpResponse HttpClient::send_request(const HttpRequest& request) {
    return impl_->send_request(request);
}

HttpClient::HttpResponse HttpClient::get(const String& url,
                                        const std::map<String, String>& headers,
                                        int timeout_seconds) {
    HttpRequest request;
    request.url = url;
    request.method = "GET";
    request.headers = headers;
    request.timeout_seconds = timeout_seconds;
    return send_request(request);
}

HttpClient::HttpResponse HttpClient::post(const String& url,
                                         const String& body,
                                         const std::map<String, String>& headers,
                                         int timeout_seconds) {
    HttpRequest request;
    request.url = url;
    request.method = "POST";
    request.body = body;
    request.headers = headers;
    request.timeout_seconds = timeout_seconds;
    return send_request(request);
}

} // namespace langchain