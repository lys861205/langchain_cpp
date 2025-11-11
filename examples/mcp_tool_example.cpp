#include <iostream>
#include <memory>
#include <vector>
#include "../include/langchain/langchain.h"
#include "../include/langchain/mcp.h"
#include <nlohmann/json.hpp>

using namespace langchain;

int main() {
    std::cout << "LangChain C++ MCP Tool Example\n";
    std::cout << "==============================\n\n";

    // Create tools
    auto calculator = std::make_shared<CalculatorTool>();
    auto string_tool = std::make_shared<StringTool>();
    auto search_tool = std::make_shared<SearchTool>();
    auto time_tool = std::make_shared<TimeTool>();

    // Create MCP tool manager
    auto mcp_manager = std::make_shared<MCPToolManager>();

    // Register tools with MCP manager
    auto calc_mcp = std::make_shared<MCPToolWrapper>(calculator);
    auto string_mcp = std::make_shared<MCPToolWrapper>(string_tool);
    auto search_mcp = std::make_shared<MCPToolWrapper>(search_tool);
    auto time_mcp = std::make_shared<MCPToolWrapper>(time_tool);

    mcp_manager->register_tool(calc_mcp);
    mcp_manager->register_tool(string_mcp);
    mcp_manager->register_tool(search_mcp);
    mcp_manager->register_tool(time_mcp);

    // List registered tools
    std::cout << "Registered MCP Tools:\n";
    auto tools = mcp_manager->list_tools();
    for (const auto& tool : tools) {
        std::cout << "- " << tool.name << ": " << tool.description << std::endl;
    }
    std::cout << std::endl;

    // Test MCP tool execution
    std::cout << "Testing MCP Tool Execution:\n";

    // Test calculator tool
    nlohmann::json calc_input;
    calc_input["input"] = "10 + 5";
    std::string calc_result = mcp_manager->execute_tool("calculator", calc_input.dump());
    std::cout << "Calculator result for '10 + 5': " << calc_result << std::endl;

    // Test string tool
    nlohmann::json string_input;
    string_input["input"] = "hello world";
    std::string string_result = mcp_manager->execute_tool("string_tool", string_input.dump());
    std::cout << "String tool result for 'hello world': " << string_result << std::endl;

    // Test time tool
    nlohmann::json time_input;
    time_input["input"] = "";
    std::string time_result = mcp_manager->execute_tool("time", time_input.dump());
    std::cout << "Time tool result: " << time_result << std::endl;

    std::cout << "\nMCP tool example completed successfully!\n";

    return 0;
}