#include <iostream>
#include <memory>
#include <vector>
#include "../include/langchain/langchain.h"

using namespace langchain;

int main() {
    std::cout << "LangChain C++ Agent Example\n";
    std::cout << "==========================\n\n";

    // Create an LLM
    auto llm = std::make_shared<SimpleLLM>();

    // Create tools
    auto calculator = std::make_shared<CalculatorTool>();
    auto string_tool = std::make_shared<StringTool>();
    auto search_tool = std::make_shared<SearchTool>();

    std::vector<std::shared_ptr<Tool>> tools = {calculator, string_tool, search_tool};

    // Create a simple agent
    SimpleAgent agent(llm, tools);

    // Test the agent
    std::vector<std::string> queries = {
        "What is 2 + 2?",
        "Convert 'hello world' to uppercase",
        "Search for information about artificial intelligence"
    };

    std::cout << "Testing Simple Agent:\n";
    for (const auto& query : queries) {
        std::string response = agent.execute(query);
        std::cout << "Query: " << query << std::endl;
        std::cout << "Response: " << response << std::endl << std::endl;
    }

    // Test tool calls directly
    std::cout << "Testing direct tool calls:\n";
    std::string calc_result = calculator->execute("5 * 6");
    std::cout << "Calculator result: " << calc_result << std::endl;

    string_tool->set_operation("reverse");
    std::string reverse_result = string_tool->execute("hello");
    std::cout << "Reverse result: " << reverse_result << std::endl;

    return 0;
}