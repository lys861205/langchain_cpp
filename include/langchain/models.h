#ifndef LANGCHAIN_MODELS_H
#define LANGCHAIN_MODELS_H

#include "core.h"
#include "http_client.h"
#include <nlohmann/json.hpp>
#include <memory>
#include <iostream>

namespace langchain {

// Base class for all API-based models
class APIModel : public LLM {
protected:
    String api_key_;
    String base_url_;
    String model_name_;
    int timeout_seconds_;
    int max_retries_;
    int retry_delay_ms_;  // 基础延迟（毫秒）
    std::vector<int> retryable_status_codes_;

public:
    APIModel(const String& api_key, const String& base_url = "", const String& model_name = "")
        : api_key_(api_key), base_url_(base_url), model_name_(model_name),
          timeout_seconds_(30), max_retries_(3), retry_delay_ms_(1000) {
        // 默认可重试的状态码
        retryable_status_codes_ = {429, 500, 502, 503, 504};
    }

    virtual ~APIModel() = default;

    // Set timeout in seconds
    void set_timeout(int seconds) {
        timeout_seconds_ = seconds;
    }

    // Set model name
    virtual void set_model_name(const String& model_name) {
        model_name_ = model_name;
    }

    // Set retry parameters
    void set_retry_params(int max_retries, int retry_delay_ms = 1000) {
        max_retries_ = max_retries;
        retry_delay_ms_ = retry_delay_ms;
    }

    // Add retryable status code
    void add_retryable_status_code(int status_code) {
        retryable_status_codes_.push_back(status_code);
    }

    // Check if status code is retryable
    bool is_retryable_status(int status_code) {
        return std::find(retryable_status_codes_.begin(), retryable_status_codes_.end(), status_code)
               != retryable_status_codes_.end();
    }

    // Exponential backoff delay calculation
    int calculate_retry_delay(int attempt) {
        // 指数退避：delay = base_delay * (2 ^ attempt) + random_jitter
        int delay = retry_delay_ms_ * (1 << attempt);  // 2^attempt
        // 添加随机抖动 (±25%)
        int jitter = rand() % (delay / 2) - (delay / 4);
        return std::max(retry_delay_ms_, delay + jitter);
    }

    // Virtual method to prepare the request JSON
    virtual nlohmann::json prepare_request(const String& prompt) = 0;

    // Virtual method to parse the response
    virtual String parse_response(const nlohmann::json& response) = 0;

    // Implementation of the generate method with retry logic
    String generate(const String& prompt) override {
        HttpClient http_client;
        String last_error;

        for (int attempt = 0; attempt <= max_retries_; ++attempt) {
            try {
                // 准备请求
                nlohmann::json request_json = prepare_request(prompt);
                String request_body = request_json.dump();

                // 准备请求头
                std::map<std::string, std::string> headers;
                headers["Content-Type"] = "application/json";
                headers["Authorization"] = "Bearer " + api_key_;

                // 发送POST请求
                auto response = http_client.post(base_url_, request_body, headers, timeout_seconds_);

                // 检查是否成功
                if (response.status_code == 200) {
                    // 解析响应
                    try {
                        nlohmann::json response_json = nlohmann::json::parse(response.body);
                        return parse_response(response_json);
                    } catch (const std::exception& e) {
                        last_error = "Error: Failed to parse response - " + String(e.what()) +
                                    "\nResponse: " + response.body;
                        // 解析错误不重试
                        break;
                    }
                } else if (attempt < max_retries_ && is_retryable_status(response.status_code)) {
                    // 可重试的错误，记录错误并等待后重试
                    last_error = "HTTP " + std::to_string(response.status_code) +
                                " - " + response.body;

                    // 计算延迟时间
                    int delay_ms = calculate_retry_delay(attempt);

                    // 等待后重试
                    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
                    continue;
                } else {
                    // 不可重试的错误或已达最大重试次数
                    return "Error: HTTP request failed - HTTP " + std::to_string(response.status_code) +
                           " Response: " + response.body;
                }
            } catch (const std::exception& e) {
                last_error = "Exception: " + String(e.what());
                // 网络异常等可能可重试的错误
                if (attempt < max_retries_) {
                    int delay_ms = calculate_retry_delay(attempt);
                    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
                    continue;
                } else {
                    break;
                }
            }
        }

        return "Error: Failed to generate response after " + std::to_string(max_retries_+1) +
               " attempts. Last error: " + last_error;
    }
};

// OpenAI model implementation
class OpenAIModel : public APIModel {
private:
    double temperature_;
    int max_tokens_;
    String organization_;
    bool stream_;

public:
    OpenAIModel(const String& api_key, const String& model_name = "gpt-3.5-turbo")
        : APIModel(api_key, "https://api.openai.com/v1/chat/completions", model_name),
          temperature_(0.7), max_tokens_(1000), stream_(false) {}

    // Set temperature (0.0 to 2.0)
    void set_temperature(double temperature) {
        temperature_ = std::max(0.0, std::min(2.0, temperature));
    }

    // Set max tokens
    void set_max_tokens(int max_tokens) {
        max_tokens_ = max_tokens;
    }

    // Set organization ID
    void set_organization(const String& organization) {
        organization_ = organization;
    }

    // Enable/disable streaming
    void set_stream(bool stream) {
        stream_ = stream;
    }

    // Prepare the request JSON for OpenAI
    nlohmann::json prepare_request(const String& prompt) override {
        nlohmann::json request;
        request["model"] = model_name_;
        request["messages"] = nlohmann::json::array({
            {{"role", "user"}, {"content", prompt}}
        });
        request["temperature"] = temperature_;
        request["max_tokens"] = max_tokens_;
        request["stream"] = stream_;
        return request;
    }

    // Parse the response from OpenAI
    String parse_response(const nlohmann::json& response) override {
        try {
            if (response.contains("choices") && !response["choices"].empty()) {
                return response["choices"][0]["message"]["content"];
            }
            return "Error: No choices in response";
        } catch (const std::exception& e) {
            return "Error: Failed to parse OpenAI response - " + String(e.what());
        }
    }

    // Override generate to support organization header
    String generate(const String& prompt) override {
        HttpClient http_client;

        // 准备请求
        nlohmann::json request_json = prepare_request(prompt);
        String request_body = request_json.dump();

        // 准备请求头
        std::map<std::string, std::string> headers;
        headers["Content-Type"] = "application/json";
        headers["Authorization"] = "Bearer " + api_key_;
        if (!organization_.empty()) {
            headers["OpenAI-Organization"] = organization_;
        }

        // 发送POST请求
        auto response = http_client.post(base_url_, request_body, headers, timeout_seconds_);

        // 检查错误
        if (response.status_code != 200) {
            return "Error: HTTP request failed - HTTP " + std::to_string(response.status_code) +
                   " Response: " + response.body;
        }

        // 解析响应
        try {
            nlohmann::json response_json = nlohmann::json::parse(response.body);
            return parse_response(response_json);
        } catch (const std::exception& e) {
            return "Error: Failed to parse response - " + String(e.what()) +
                   "\nResponse: " + response.body;
        }
    }
};

// Qwen (千问) model implementation
class QwenModel : public APIModel {
private:
    double temperature_;
    int max_tokens_;

public:
    QwenModel(const String& api_key, const String& model_name = "qwen-turbo")
        : APIModel(api_key, "https://dashscope.aliyuncs.com/api/v1/services/aigc/text-generation/generation", model_name),
          temperature_(0.7), max_tokens_(1000) {}

    // Set temperature (0.0 to 2.0)
    void set_temperature(double temperature) {
        temperature_ = temperature;
    }

    // Set max tokens
    void set_max_tokens(int max_tokens) {
        max_tokens_ = max_tokens;
    }

    // Prepare the request JSON for Qwen
    nlohmann::json prepare_request(const String& prompt) override {
        nlohmann::json request;
        request["model"] = model_name_;
        request["input"]["messages"] = nlohmann::json::array({
            {{"role", "user"}, {"content", prompt}}
        });
        request["parameters"]["temperature"] = temperature_;
        request["parameters"]["max_tokens"] = max_tokens_;
        return request;
    }

    // Parse the response from Qwen
    String parse_response(const nlohmann::json& response) override {
        try {
            if (response.contains("output") && response["output"].contains("text")) {
                return response["output"]["text"];
            }
            return "Error: No output text in response";
        } catch (const std::exception& e) {
            return "Error: Failed to parse Qwen response - " + String(e.what());
        }
    }
};

// Anthropic Claude model implementation
class ClaudeModel : public APIModel {
private:
    double temperature_;
    int max_tokens_;
    String anthropic_version_;

public:
    ClaudeModel(const String& api_key, const String& model_name = "claude-3-haiku-20240307")
        : APIModel(api_key, "https://api.anthropic.com/v1/messages", model_name),
          temperature_(0.7), max_tokens_(1000), anthropic_version_("2023-06-01") {}

    // Set temperature (0.0 to 1.0)
    void set_temperature(double temperature) {
        temperature_ = std::max(0.0, std::min(1.0, temperature));
    }

    // Set max tokens
    void set_max_tokens(int max_tokens) {
        max_tokens_ = max_tokens;
    }

    // Set anthropic version
    void set_anthropic_version(const String& version) {
        anthropic_version_ = version;
    }

    // Prepare the request JSON for Claude
    nlohmann::json prepare_request(const String& prompt) override {
        nlohmann::json request;
        request["model"] = model_name_;
        request["messages"] = nlohmann::json::array({
            {{"role", "user"}, {"content", prompt}}
        });
        request["temperature"] = temperature_;
        request["max_tokens"] = max_tokens_;
        return request;
    }

    // Parse the response from Claude
    String parse_response(const nlohmann::json& response) override {
        try {
            if (response.contains("content") && !response["content"].empty()) {
                return response["content"][0]["text"];
            }
            return "Error: No content in response";
        } catch (const std::exception& e) {
            return "Error: Failed to parse Claude response - " + String(e.what());
        }
    }

    // Override generate to use Claude-specific headers
    String generate(const String& prompt) override {
        // 使用HttpClient替换curl
        HttpClient http_client;

        // 准备请求
        nlohmann::json request_json = prepare_request(prompt);
        String request_body = request_json.dump();

        // 准备请求头 - Claude特定的头部
        std::map<std::string, std::string> headers;
        headers["Content-Type"] = "application/json";
        headers["x-api-key"] = api_key_;
        headers["anthropic-version"] = anthropic_version_;

        // 发送POST请求
        auto response = http_client.post(base_url_, request_body, headers, timeout_seconds_);

        // 检查错误
        if (response.status_code != 200) {
            return "Error: HTTP request failed - HTTP " + std::to_string(response.status_code) +
                   " Response: " + response.body;
        }

        // 解析响应
        try {
            nlohmann::json response_json = nlohmann::json::parse(response.body);
            return parse_response(response_json);
        } catch (const std::exception& e) {
            return "Error: Failed to parse response - " + String(e.what()) +
                   "\nResponse: " + response.body;
        }
    }
};

// Generic model for other APIs
class GenericModel : public APIModel {
private:
    std::function<nlohmann::json(const String&)> request_builder_;
    std::function<String(const nlohmann::json&)> response_parser_;
    std::map<String, String> custom_headers_;
    String http_method_;

public:
    GenericModel(const String& api_key, const String& base_url, const String& model_name = "")
        : APIModel(api_key, base_url, model_name), http_method_("POST") {}

    // Set custom request builder function
    void set_request_builder(std::function<nlohmann::json(const String&)> builder) {
        request_builder_ = builder;
    }

    // Set custom response parser function
    void set_response_parser(std::function<String(const nlohmann::json&)> parser) {
        response_parser_ = parser;
    }

    // Add custom header
    void add_header(const String& key, const String& value) {
        custom_headers_[key] = value;
    }

    // Set HTTP method
    void set_http_method(const String& method) {
        http_method_ = method;
    }

    // Prepare the request JSON using the custom builder
    nlohmann::json prepare_request(const String& prompt) override {
        if (request_builder_) {
            return request_builder_(prompt);
        }
        // Default implementation
        nlohmann::json request;
        request["prompt"] = prompt;
        request["model"] = model_name_;
        return request;
    }

    // Parse the response using the custom parser
    String parse_response(const nlohmann::json& response) override {
        if (response_parser_) {
            return response_parser_(response);
        }
        // Default implementation - try to extract text from common fields
        if (response.contains("text")) {
            return response["text"];
        } else if (response.contains("content")) {
            return response["content"];
        } else if (response.contains("result")) {
            return response["result"];
        } else if (response.contains("choices") && !response["choices"].empty()) {
            // OpenAI-style response
            return response["choices"][0]["message"]["content"];
        } else if (response.contains("candidates") && !response["candidates"].empty()) {
            // Google AI-style response
            return response["candidates"][0]["content"]["parts"][0]["text"];
        } else if (response.contains("content") && !response["content"].empty()) {
            // Claude-style response
            return response["content"][0]["text"];
        }
        return "Error: Unable to parse response";
    }

    // Override generate to support custom headers and HTTP methods
    String generate(const String& prompt) override {
        HttpClient http_client;

        // 准备请求
        nlohmann::json request_json = prepare_request(prompt);
        String request_body = request_json.dump();

        // 准备请求头
        std::map<std::string, std::string> headers;
        headers["Content-Type"] = "application/json";

        // 添加自定义头部
        for (const auto& header : custom_headers_) {
            headers[header.first] = header.second;
        }

        // 如果没有设置自定义Authorization头部，则使用默认的Bearer token
        if (headers.find("Authorization") == headers.end() && !api_key_.empty()) {
            headers["Authorization"] = "Bearer " + api_key_;
        }

        // 发送请求（支持不同的HTTP方法）
        HttpClient::HttpResponse response;
        if (http_method_ == "GET") {
            response = http_client.get(base_url_, headers, timeout_seconds_);
        } else {
            response = http_client.post(base_url_, request_body, headers, timeout_seconds_);
        }

        // 检查错误
        if (response.status_code >= 400) {
            return "Error: HTTP request failed - HTTP " + std::to_string(response.status_code) +
                   " Response: " + response.body;
        }

        // 解析响应
        try {
            if (response.body.empty()) {
                return "";
            }
            nlohmann::json response_json = nlohmann::json::parse(response.body);
            return parse_response(response_json);
        } catch (const std::exception& e) {
            return "Error: Failed to parse response - " + String(e.what()) +
                   "\nResponse: " + response.body;
        }
    }
};

// Google AI (Gemini) model implementation
class GoogleModel : public APIModel {
private:
    double temperature_;
    int max_tokens_;
    String api_version_;

public:
    GoogleModel(const String& api_key, const String& model_name = "gemini-pro")
        : APIModel(api_key, "", model_name),  // base_url will be set dynamically
          temperature_(0.7), max_tokens_(1000), api_version_("v1") {
        // Update base_url with the initial model name
        set_model_name(model_name);
    }

    // Set temperature (0.0 to 1.0)
    void set_temperature(double temperature) {
        temperature_ = std::max(0.0, std::min(1.0, temperature));
    }

    // Set max tokens
    void set_max_tokens(int max_tokens) {
        max_tokens_ = max_tokens;
    }

    // Override base_url to include API key and model
    void set_model_name(const String& model_name) override {
        APIModel::set_model_name(model_name);
        // Update base_url with the new model name
        base_url_ = "https://generativelanguage.googleapis.com/" + api_version_ +
                   "/models/" + model_name + ":generateContent?key=" + api_key_;
    }

    // Prepare the request JSON for Google AI
    nlohmann::json prepare_request(const String& prompt) override {
        nlohmann::json request;
        request["contents"] = nlohmann::json::array({
            {{"parts", nlohmann::json::array({{{"text", prompt}}})}}
        });
        request["generationConfig"]["temperature"] = temperature_;
        request["generationConfig"]["maxOutputTokens"] = max_tokens_;
        return request;
    }

    // Parse the response from Google AI
    String parse_response(const nlohmann::json& response) override {
        try {
            if (response.contains("candidates") && !response["candidates"].empty()) {
                return response["candidates"][0]["content"]["parts"][0]["text"];
            }
            return "Error: No candidates in response";
        } catch (const std::exception& e) {
            return "Error: Failed to parse Google AI response - " + String(e.what());
        }
    }
};

} // namespace langchain

#endif // LANGCHAIN_MODELS_H