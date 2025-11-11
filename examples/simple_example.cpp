#include <iostream>
#include <memory>
#include "../include/langchain/langchain.h"

using namespace langchain;

int main() {
    std::cout << "LangChain C++ Simple Example\n";
    std::cout << "===========================\n\n";

    // Create an LLM
    auto llm = std::make_shared<SimpleLLM>();

    // Generate a response
    std::string prompt = "Hello, how are you?";
    std::string response = llm->generate(prompt);

    std::cout << "Prompt: " << prompt << std::endl;
    std::cout << "Response: " << response << std::endl << std::endl;

    // Test with different prompts
    std::vector<std::string> prompts = {
        "What is your name?",
        "Can you help me with the weather?",
        "What time is it?",
        "Tell me about artificial intelligence."
    };

    std::cout << "Testing multiple prompts:\n";
    for (const auto& p : prompts) {
        std::string resp = llm->generate(p);
        std::cout << "Prompt: " << p << std::endl;
        std::cout << "Response: " << resp << std::endl << std::endl;
    }

    // Test async generation
    std::cout << "Testing async generation:\n";
    auto future = llm->generate_async("This is an async test");
    std::string async_response = future.get();
    std::cout << "Async Response: " << async_response << std::endl;

    return 0;
}