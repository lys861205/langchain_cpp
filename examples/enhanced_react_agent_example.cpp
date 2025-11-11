#include <iostream>
#include <memory>
#include <vector>
#include "../include/langchain/langchain.h"

using namespace langchain;

int main() {
    std::cout << "LangChain C++ Enhanced ReAct Agent Example\n";
    std::cout << "==========================================\n\n";

    // Create an LLM
    auto llm = std::make_shared<SimpleLLM>();

    // Create tools
    auto calculator = std::make_shared<CalculatorTool>();
    auto string_tool = std::make_shared<StringTool>();
    auto search_tool = std::make_shared<SearchTool>();
    auto time_tool = std::make_shared<TimeTool>();

    std::vector<std::shared_ptr<Tool>> tools = {calculator, string_tool, search_tool, time_tool};

    // Create an enhanced ReAct agent
    EnhancedReActAgent agent(llm, tools);

    // Set verbose mode to see intermediate steps
    agent.set_verbose(true);

    // Test the agent with a complex query that requires multiple tools
    std::vector<std::string> queries = {
        "What is 2 + 2?",
        "What is the current time?",
        "Convert 'hello world' to uppercase",
        "Search for information about artificial intelligence",
        "Calculate 5 * 6 and then convert the result to a string with 'The answer is ' prefix"
    };

    std::cout << "Testing Enhanced ReAct Agent:\n";
    for (const auto& query : queries) {
        std::cout << "\n--- Query: " << query << " ---\n";
        std::string response = agent.execute(query);
        std::cout << "Final Answer: " << response << std::endl << std::endl;
    }

    // Test with a more complex multi-step query
    std::cout << "\n--- Complex Multi-Step Query ---\n";
    std::string complex_query = "What is the result of multiplying 12 by 15, and then converting that number to uppercase letters using the string tool?";
    std::cout << "Query: " << complex_query << std::endl;
    std::string complex_response = agent.execute(complex_query);
    std::cout << "Final Answer: " << complex_response << std::endl;

    return 0;
}