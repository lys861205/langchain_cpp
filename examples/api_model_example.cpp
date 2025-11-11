#include <iostream>
#include <memory>
#include "../include/langchain/langchain.h"

using namespace langchain;

int main() {
    std::cout << "LangChain C++ API Model Example\n";
    std::cout << "==============================\n\n";

    // Note: You need to set your API keys as environment variables
    // For security, never hardcode API keys in your source code

    // Example with OpenAI (uncomment and set your API key to test)
    /*
    const char* openai_api_key = std::getenv("OPENAI_API_KEY");
    if (openai_api_key) {
        std::cout << "Testing OpenAI Model:\n";
        auto openai_model = std::make_shared<OpenAIModel>(String(openai_api_key), "gpt-3.5-turbo");
        openai_model->set_temperature(0.7);
        openai_model->set_max_tokens(500);

        String prompt = "What is the capital of France?";
        String response = openai_model->generate(prompt);

        std::cout << "Prompt: " << prompt << std::endl;
        std::cout << "Response: " << response << std::endl << std::endl;
    } else {
        std::cout << "OPENAI_API_KEY environment variable not set. Skipping OpenAI test.\n\n";
    }
    */

    // Example with Qwen (千问) (uncomment and set your API key to test)
    /*
    const char* qwen_api_key = std::getenv("QWEN_API_KEY");
    if (qwen_api_key) {
        std::cout << "Testing Qwen Model:\n";
        auto qwen_model = std::make_shared<QwenModel>(String(qwen_api_key), "qwen-turbo");
        qwen_model->set_temperature(0.7);
        qwen_model->set_max_tokens(500);

        String prompt = "What is artificial intelligence?";
        String response = qwen_model->generate(prompt);

        std::cout << "Prompt: " << prompt << std::endl;
        std::cout << "Response: " << response << std::endl << std::endl;
    } else {
        std::cout << "QWEN_API_KEY environment variable not set. Skipping Qwen test.\n\n";
    }
    */

    // Example with Generic Model
    std::cout << "Testing Generic Model (with mock configuration):\n";
    // This is just a demonstration - you would need to configure this for your specific API
    auto generic_model = std::make_shared<GenericModel>("your-api-key", "https://your-api-endpoint.com/v1/completions");

    // Set custom request builder
    generic_model->set_request_builder([](const String& prompt) {
        nlohmann::json request;
        request["prompt"] = prompt;
        request["max_tokens"] = 100;
        request["temperature"] = 0.7;
        return request;
    });

    // Set custom response parser
    generic_model->set_response_parser([](const nlohmann::json& response) {
        if (response.contains("choices") && !response["choices"].empty()) {
            return response["choices"][0]["text"].get<String>();
        }
        return String("No response text found");
    });

    std::cout << "Generic model configured. In a real implementation, this would connect to an actual API.\n\n";

    // Test with a mock response (since we don't have real API keys in this example)
    std::cout << "Example usage pattern demonstrated.\n";

    return 0;
}