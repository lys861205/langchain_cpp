#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

int main() {
    std::cout << "Simple MCP Service Simulator\n";
    std::cout << "============================\n\n";

    // Test listing tools
    std::cout << "Testing tool listing...\n";

    // Create a sample JSON response for tools
    nlohmann::json tools_response;
    tools_response["tools"] = nlohmann::json::array();

    nlohmann::json calculator_tool;
    calculator_tool["name"] = "calculator";
    calculator_tool["description"] = "A simple calculator that can perform basic arithmetic operations";
    calculator_tool["parameters"] = nlohmann::json::object();
    calculator_tool["return_type"] = "string";
    tools_response["tools"].push_back(calculator_tool);

    nlohmann::json string_tool;
    string_tool["name"] = "string_tool";
    string_tool["description"] = "A tool for string manipulation operations";
    string_tool["parameters"] = nlohmann::json::object();
    string_tool["return_type"] = "string";
    tools_response["tools"].push_back(string_tool);

    std::string tools_json = tools_response.dump();
    std::cout << "Sample tools response: " << tools_json << std::endl;

    // Test tool execution
    std::cout << "\nTesting tool execution...\n";

    // Create a sample JSON response for tool execution
    nlohmann::json execution_response;
    execution_response["result"] = "Sample result from calculator tool";
    std::string execution_json = execution_response.dump();
    std::cout << "Sample execution response: " << execution_json << std::endl;

    std::cout << "\nMCP service simulator completed!\n";
    return 0;
}