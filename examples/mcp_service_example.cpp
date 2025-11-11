#include <iostream>
#include <memory>
#include "../include/langchain/langchain.h"
#include "../include/langchain/mcp.h"

using namespace langchain;

int main() {
    std::cout << "LangChain C++ MCP Service Access Example\n";
    std::cout << "========================================\n\n";

    // Create an MCP service client
    // Note: This example assumes there's an MCP service running at the specified URL
    // In a real scenario, you would replace this with the actual MCP service URL
    auto mcp_client = std::make_shared<MCPServiceClient>("http://localhost:8000", "your-api-key-here");

    // Connect to the MCP service
    if (mcp_client->connect()) {
        std::cout << "Connected to MCP service successfully!\n\n";

        // List remote tools
        std::cout << "Discovering remote tools...\n";
        auto remote_tools = mcp_client->list_remote_tools();

        if (!remote_tools.empty()) {
            std::cout << "Available remote tools:\n";
            for (const auto& tool : remote_tools) {
                std::cout << "- " << tool.name << ": " << tool.description << std::endl;
            }
            std::cout << std::endl;

            // Create MCP tool manager and register remote tools
            auto tool_manager = std::make_shared<MCPToolManager>();

            // Register remote tools as local MCP tools
            for (const auto& tool_meta : remote_tools) {
                auto remote_tool = std::make_shared<MCPRemoteTool>(tool_meta, mcp_client);
                tool_manager->register_tool(remote_tool);
            }

            // List all tools in the manager
            std::cout << "Tools registered in tool manager:\n";
            auto all_tools = tool_manager->list_tools();
            for (const auto& tool : all_tools) {
                std::cout << "- " << tool.name << ": " << tool.description << std::endl;
            }
            std::cout << std::endl;

            // Example of executing a remote tool (if available)
            // This is just a demonstration - in practice, you would use actual tool names
            // and appropriate JSON input for that tool
            std::cout << "Testing remote tool execution (demonstration only)...\n";
            std::string tool_name = remote_tools[0].name;
            std::cout << "Attempting to execute tool: " << tool_name << std::endl;

            // Create sample JSON input (this would need to match the actual tool's expected input)
            std::string json_input = "{\"input\": \"sample input for " + tool_name + "\"}";
            std::string result = tool_manager->execute_tool(tool_name, json_input);
            std::cout << "Tool execution result: " << result << std::endl;
        } else {
            std::cout << "No remote tools found or failed to retrieve tools.\n";
        }

        // Disconnect from the MCP service
        mcp_client->disconnect();
        std::cout << "\nDisconnected from MCP service.\n";
    } else {
        std::cout << "Failed to connect to MCP service.\n";
        std::cout << "Please ensure an MCP service is running at the specified URL.\n";
    }

    std::cout << "\nMCP service access example completed!\n";

    return 0;
}