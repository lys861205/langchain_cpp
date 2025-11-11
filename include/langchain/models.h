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

public:
    APIModel(const String& api_key, const String& base_url = "", const String& model_name = "")
        : api_key_(api_key), base_url_(base_url), model_name_(model_name), timeout_seconds_(30) {}

    virtual ~APIModel() = default;

    // Set timeout in seconds
    void set_timeout(int seconds) {
        timeout_seconds_ = seconds;
    }

    // Set model name
    void set_model_name(const String& model_name) {
        model_name_ = model_name;
    }

    // Virtual method to prepare the request JSON
    virtual nlohmann::json prepare_request(const String& prompt) = 0;

    // Virtual method to parse the response
    virtual String parse_response(const nlohmann::json& response) = 0;

    // Implementation of the generate method
    String generate(const String& prompt) override {
        // 使用HttpClient替换curl
        HttpClient http_client;

        // 准备请求
        nlohmann::json request_json = prepare_request(prompt);
        String request_body = request_json.dump();

        // 准备请求头
        std::map<std::string, std::string> headers;
        headers["Content-Type"] = "application/json";
        headers["Authorization"] = "Bearer " + api_key_;

        // 发送POST请求
        auto response = http_client.post(base_url_, request_body, headers, timeout_seconds_);

        // 检查错误
        if (response.status_code != 200) {
            return "Error: HTTP request failed - HTTP " + std::to_string(response.status_code);
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

// OpenAI model implementation
class OpenAIModel : public APIModel {
private:
    double temperature_;
    int max_tokens_;

public:
    OpenAIModel(const String& api_key, const String& model_name = "gpt-3.5-turbo")
        : APIModel(api_key, "https://api.openai.com/v1/chat/completions", model_name),
          temperature_(0.7), max_tokens_(1000) {}

    // Set temperature (0.0 to 2.0)
    void set_temperature(double temperature) {
        temperature_ = temperature;
    }

    // Set max tokens
    void set_max_tokens(int max_tokens) {
        max_tokens_ = max_tokens;
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

// Generic model for other APIs
class GenericModel : public APIModel {
private:
    std::function<nlohmann::json(const String&)> request_builder_;
    std::function<String(const nlohmann::json&)> response_parser_;

public:
    GenericModel(const String& api_key, const String& base_url, const String& model_name = "")
        : APIModel(api_key, base_url, model_name) {}

    // Set custom request builder function
    void set_request_builder(std::function<nlohmann::json(const String&)> builder) {
        request_builder_ = builder;
    }

    // Set custom response parser function
    void set_response_parser(std::function<String(const nlohmann::json&)> parser) {
        response_parser_ = parser;
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
        }
        return "Error: Unable to parse response";
    }
};

} // namespace langchain

#endif // LANGCHAIN_MODELS_H