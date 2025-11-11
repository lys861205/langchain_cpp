#include "../include/langchain/models.h"
#include "../include/langchain/http_client.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace langchain;

int main() {
    std::cout << "Comprehensive API Model Test\n";
    std::cout << "============================\n\n";

    // Test HTTP Client first
    std::cout << "1. Testing HTTP Client...\n";
    HttpClient client;

    // Test GET request
    auto get_response = client.get("https://httpbin.org/get");
    std::cout << "GET Status: " << get_response.status_code << std::endl;

    // Test POST request
    std::string post_data = "{\"test\": \"data\"}";
    std::map<std::string, std::string> headers;
    headers["Content-Type"] = "application/json";
    auto post_response = client.post("https://httpbin.org/post", post_data, headers);
    std::cout << "POST Status: " << post_response.status_code << std::endl;

    std::cout << "\n";

    // Test API Model Base Class Features
    std::cout << "2. Testing API Model Base Class Features...\n";

    // Create a generic model to test base class features
    GenericModel model("test-key", "https://httpbin.org/status/500"); // This will return 500 error

    // Test retry configuration
    model.set_retry_params(3, 100); // 3 retries with 100ms delay
    model.add_retryable_status_code(500);

    // Test if status code is retryable
    bool is_retryable = model.is_retryable_status(500);
    std::cout << "Is 500 retryable: " << (is_retryable ? "Yes" : "No") << std::endl;

    std::cout << "\n";

    // Test OpenAI Model
    std::cout << "3. Testing OpenAI Model Features...\n";
    OpenAIModel openai_model("test-key", "gpt-3.5-turbo");
    openai_model.set_temperature(0.8);
    openai_model.set_max_tokens(150);

    // Test request preparation
    auto openai_request = openai_model.prepare_request("Test prompt");
    std::cout << "OpenAI Request Model: " << openai_request["model"] << std::endl;
    std::cout << "OpenAI Request Temperature: " << openai_request["temperature"] << std::endl;

    std::cout << "\n";

    // Test Claude Model
    std::cout << "4. Testing Claude Model Features...\n";
    ClaudeModel claude_model("test-key", "claude-3-haiku-20240307");
    claude_model.set_temperature(0.8);
    claude_model.set_max_tokens(150);
    claude_model.set_anthropic_version("2023-06-01");

    // Test request preparation
    auto claude_request = claude_model.prepare_request("Test prompt");
    std::cout << "Claude Request Model: " << claude_request["model"] << std::endl;
    std::cout << "Claude Request Temperature: " << claude_request["temperature"] << std::endl;

    std::cout << "\n";

    // Test Google Model
    std::cout << "5. Testing Google Model Features...\n";
    GoogleModel google_model("test-key", "gemini-pro");
    google_model.set_temperature(0.8);
    google_model.set_max_tokens(150);

    // Test model name setting (which updates base URL)
    google_model.set_model_name("gemini-pro");

    // Test request preparation
    auto google_request = google_model.prepare_request("Test prompt");
    std::cout << "Google Request Temperature: " << google_request["generationConfig"]["temperature"] << std::endl;
    std::cout << "Google Request Max Tokens: " << google_request["generationConfig"]["maxOutputTokens"] << std::endl;

    std::cout << "\n";

    // Test Qwen Model
    std::cout << "6. Testing Qwen Model Features...\n";
    QwenModel qwen_model("test-key", "qwen-turbo");
    qwen_model.set_temperature(0.8);
    qwen_model.set_max_tokens(150);

    // Test request preparation
    auto qwen_request = qwen_model.prepare_request("Test prompt");
    std::cout << "Qwen Request Model: " << qwen_request["model"] << std::endl;
    std::cout << "Qwen Request Temperature: " << qwen_request["parameters"]["temperature"] << std::endl;

    std::cout << "\n";

    // Test Generic Model
    std::cout << "7. Testing Generic Model Features...\n";
    GenericModel generic_model("test-key", "https://api.example.com/v1/chat");

    // Test custom headers
    generic_model.add_header("X-Custom-Header", "custom-value");
    generic_model.set_http_method("POST");

    // Test custom request builder
    generic_model.set_request_builder([](const String& prompt) {
        nlohmann::json request;
        request["custom_prompt"] = prompt;
        request["custom_param"] = "value";
        return request;
    });

    // Test custom response parser
    generic_model.set_response_parser([](const nlohmann::json& response) {
        return String("Custom parsed response");
    });

    // Test request preparation
    auto generic_request = generic_model.prepare_request("Test prompt");
    std::cout << "Generic Request Custom Param: " << generic_request["custom_param"] << std::endl;

    std::cout << "\n";

    // Test Error Handling
    std::cout << "8. Testing Error Handling...\n";

    // Test parsing error response
    nlohmann::json error_response;
    error_response["error"] = "Test error";

    // Test OpenAI error parsing
    std::string openai_error = openai_model.parse_response(error_response);
    std::cout << "OpenAI Error Parse Result: " << openai_error << std::endl;

    // Test Claude error parsing
    std::string claude_error = claude_model.parse_response(error_response);
    std::cout << "Claude Error Parse Result: " << claude_error << std::endl;

    // Test Google error parsing
    std::string google_error = google_model.parse_response(error_response);
    std::cout << "Google Error Parse Result: " << google_error << std::endl;

    // Test Qwen error parsing
    std::string qwen_error = qwen_model.parse_response(error_response);
    std::cout << "Qwen Error Parse Result: " << qwen_error << std::endl;

    std::cout << "\n";

    std::cout << "Comprehensive API Model Test Completed!\n";
    return 0;
}