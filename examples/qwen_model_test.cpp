#include <iostream>
#include <memory>
#include <cstdlib>
#include "../include/langchain/langchain.h"

using namespace langchain;

int main() {
    std::cout << "LangChain C++ Qwen Model Test\n";
    std::cout << "=============================\n\n";

    // Get Qwen API key from environment variable
    const char* qwen_api_key = std::getenv("QWEN_API_KEY");
    if (!qwen_api_key) {
        std::cout << "QWEN_API_KEY environment variable not set.\n";
        std::cout << "Please set your Qwen API key as an environment variable to test the model.\n";
        std::cout << "Example: export QWEN_API_KEY=your_api_key_here\n\n";

        std::cout << "Showing Qwen model configuration for reference:\n";

        // Demonstrate Qwen model configuration
        auto qwen_model = std::make_shared<QwenModel>("your-api-key-here", "qwen-turbo");
        qwen_model->set_temperature(0.7);
        qwen_model->set_max_tokens(500);

        std::cout << "Model: QwenModel\n";
        std::cout << "Base URL: https://dashscope.aliyuncs.com/api/v1/services/aigc/text-generation/generation\n";
        std::cout << "Default model name: qwen-turbo\n";
        std::cout << "Temperature: 0.7\n";
        std::cout << "Max tokens: 500\n\n";

        std::cout << "To test with a real API key:\n";
        std::cout << "1. Set the QWEN_API_KEY environment variable\n";
        std::cout << "2. Uncomment the testing code below\n";
        std::cout << "3. Recompile and run the example\n\n";

        return 0;
    }

    // Test Qwen model with real API key
    std::cout << "Testing Qwen Model with real API key:\n";
    auto qwen_model = std::make_shared<QwenModel>(String(qwen_api_key), "qwen-turbo");
    qwen_model->set_temperature(0.7);
    qwen_model->set_max_tokens(500);

    // Test prompts
    std::vector<String> prompts = {
        "你好，世界！",
        "什么是人工智能？",
        "请用中文解释量子计算的基本原理。",
        "写一首关于春天的诗。"
    };

    for (size_t i = 0; i < prompts.size(); ++i) {
        std::cout << "Prompt " << (i + 1) << ": " << prompts[i] << std::endl;
        String response = qwen_model->generate(prompts[i]);
        std::cout << "Response: " << response << std::endl << std::endl;
    }

    std::cout << "Qwen model test completed successfully!\n";

    return 0;
}