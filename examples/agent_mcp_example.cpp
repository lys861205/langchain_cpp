#include <iostream>
#include <memory>
#include <vector>
#include "../include/langchain/langchain.h"
#include "../include/langchain/mcp.h"

using namespace langchain;

int main() {
    std::cout << "LangChain C++ Agent with MCP Service Access Example\n";
    std::cout << "==================================================\n\n";

    // Create an LLM
    auto llm = std::make_shared<SimpleLLM>();

    // Create local tools
    auto calculator = std::make_shared<CalculatorTool>();
    auto string_tool = std::make_shared<StringTool>();
    auto time_tool = std::make_shared<TimeTool>();

    // Create tools vector with local tools
    std::vector<std::shared_ptr<Tool>> local_tools = {calculator, string_tool, time_tool};

    // Create an MCP service client (in a real scenario, you would connect to an actual MCP service)
    auto mcp_client = std::make_shared<MCPServiceClient>("http://localhost:8000", "your-api-key-here");

    // Create MCP tool manager
    auto mcp_tool_manager = std::make_shared<MCPToolManager>();

    // Register local tools with MCP tool manager
    for (const auto& tool : local_tools) {
        auto mcp_tool = std::make_shared<MCPToolWrapper>(tool);
        mcp_tool_manager->register_tool(mcp_tool);
    }

    // Try to connect to MCP service and register remote tools
    if (mcp_client->connect()) {
        std::cout << "Connected to MCP service. Discovering remote tools...\n";
        auto remote_tools = mcp_client->list_remote_tools();

        if (!remote_tools.empty()) {
            std::cout << "Found " << remote_tools.size() << " remote tools:\n";
            for (const auto& tool : remote_tools) {
                std::cout << "- " << tool.name << ": " << tool.description << std::endl;

                // Register remote tools as MCP tools
                auto remote_tool = std::make_shared<MCPRemoteTool>(tool, mcp_client);
                mcp_tool_manager->register_tool(remote_tool);
            }
        } else {
            std::cout << "No remote tools found.\n";
        }
        mcp_client->disconnect();
    } else {
        std::cout << "Could not connect to MCP service. Continuing with local tools only.\n";
    }

    // List all available tools
    std::cout << "\nAll available tools in MCP tool manager:\n";
    auto all_tools = mcp_tool_manager->list_tools();
    for (const auto& tool : all_tools) {
        std::cout << "- " << tool.name << ": " << tool.description << std::endl;
    }

    // Create a simple agent with MCP tool manager
    // For this example, we'll create a simple agent that can use the MCP tool manager directly
    std::cout << "\nTesting direct MCP tool execution:\n";

    // Test calculator tool
    nlohmann::json calc_input;
    calc_input["input"] = "10 + 5";
    std::string calc_result = mcp_tool_manager->execute_tool("calculator", calc_input.dump());
    std::cout << "Calculator result for '10 + 5': " << calc_result << std::endl;

    // Test string tool
    nlohmann::json string_input;
    string_input["input"] = "hello world";
    std::string string_result = mcp_tool_manager->execute_tool("string_tool", string_input.dump());
    std::cout << "String tool result for 'hello world': " << string_result << std::endl;

    // Test time tool
    nlohmann::json time_input;
    time_input["input"] = "";
    std::string time_result = mcp_tool_manager->execute_tool("time", time_input.dump());
    std::cout << "Time tool result: " << time_result << std::endl;

    std::cout << "\nAgent with MCP service access example completed successfully!\n";

    return 0;
}