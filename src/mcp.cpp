#include "../include/langchain/mcp.h"
#include "../include/langchain/http_client.h"
#include <nlohmann/json.hpp>
#include <iostream>

namespace langchain {

// MCPTool implementation
MCPTool::MCPTool(const MCPToolMetadata& meta) : metadata(meta) {}

const MCPToolMetadata& MCPTool::get_metadata() const {
    return metadata;
}

// MCPToolManager implementation
MCPToolManager::MCPToolManager() {}

MCPToolManager::~MCPToolManager() {}

void MCPToolManager::register_tool(const std::shared_ptr<MCPTool>& tool) {
    tools_[tool->get_metadata().name] = tool;
}

std::shared_ptr<MCPTool> MCPToolManager::get_tool(const String& name) {
    auto it = tools_.find(name);
    if (it != tools_.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<MCPToolMetadata> MCPToolManager::list_tools() {
    std::vector<MCPToolMetadata> result;
    for (const auto& pair : tools_) {
        result.push_back(pair.second->get_metadata());
    }
    return result;
}

String MCPToolManager::execute_tool(const String& name, const String& json_input) {
    auto tool = get_tool(name);
    if (tool) {
        try {
            return tool->execute(json_input);
        } catch (const std::exception& e) {
            return "Error executing tool: " + String(e.what());
        }
    }
    return "Error: Tool '" + name + "' not found";
}

bool MCPToolManager::has_tool(const String& name) {
    return tools_.find(name) != tools_.end();
}

// MCPToolWrapper implementation
MCPToolWrapper::MCPToolWrapper(const std::shared_ptr<Tool>& tool)
    : MCPTool({tool->name, tool->description, {}, "string"}), wrapped_tool_(tool) {}

String MCPToolWrapper::execute(const String& json_input) {
    try {
        // Parse JSON input to extract the actual input for the wrapped tool
        auto json = nlohmann::json::parse(json_input);
        String tool_input = "";

        // Extract input from JSON if available
        if (json.contains("input")) {
            tool_input = json["input"].get<String>();
        } else if (json.contains("query")) {
            tool_input = json["query"].get<String>();
        } else if (json.contains("text")) {
            tool_input = json["text"].get<String>();
        } else {
            // If no specific field, use the whole JSON as string
            tool_input = json_input;
        }

        return wrapped_tool_->execute(tool_input);
    } catch (const std::exception& e) {
        // If JSON parsing fails, treat input as plain text
        return wrapped_tool_->execute(json_input);
    }
}

// MCPServiceClient implementation

MCPServiceClient::MCPServiceClient(const String& service_url, const String& api_key)
    : service_url_(service_url), api_key_(api_key) {
    http_client_ = std::make_unique<HttpClient>();
}

MCPServiceClient::~MCPServiceClient() {
    // 不需要特殊清理
}

bool MCPServiceClient::connect() {
    return http_client_->initialize();
}

void MCPServiceClient::disconnect() {
    // 不需要特殊清理
}

std::vector<MCPToolMetadata> MCPServiceClient::list_remote_tools() {
    if (!http_client_) {
        if (!connect()) {
            return {};
        }
    }

    std::string url = service_url_ + "/tools";

    // 准备请求头
    std::map<std::string, std::string> headers;
    headers["User-Agent"] = "LangChain-CPP-MCP-Client/1.0";
    if (!api_key_.empty()) {
        headers["Authorization"] = "Bearer " + api_key_;
    }

    // 发送GET请求
    auto response = http_client_->get(url, headers, 30);

    if (response.status_code != 200) {
        std::cerr << "Error performing request: HTTP " << response.status_code << std::endl;
        return {};
    }

    try {
        auto json_response = nlohmann::json::parse(response.body);
        std::vector<MCPToolMetadata> tools;

        if (json_response.contains("tools") && json_response["tools"].is_array()) {
            for (const auto& tool_json : json_response["tools"]) {
                MCPToolMetadata meta;
                if (tool_json.contains("name")) {
                    meta.name = tool_json["name"].get<std::string>();
                }
                if (tool_json.contains("description")) {
                    meta.description = tool_json["description"].get<std::string>();
                }
                if (tool_json.contains("parameters")) {
                    for (auto& [key, value] : tool_json["parameters"].items()) {
                        meta.parameters[key] = value.get<std::string>();
                    }
                }
                if (tool_json.contains("return_type")) {
                    meta.return_type = tool_json["return_type"].get<std::string>();
                } else {
                    meta.return_type = "string";
                }
                tools.push_back(meta);
                remote_tools_[meta.name] = meta;
            }
        }

        return tools;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON response: " << e.what() << std::endl;
        return {};
    }
}

MCPToolMetadata MCPServiceClient::get_remote_tool(const String& name) {
    auto it = remote_tools_.find(name);
    if (it != remote_tools_.end()) {
        return it->second;
    }

    // If not found, try to refresh tools
    refresh_tools();
    it = remote_tools_.find(name);
    if (it != remote_tools_.end()) {
        return it->second;
    }

    return MCPToolMetadata{};
}

String MCPServiceClient::execute_remote_tool(const String& name, const String& json_input) {
    if (!http_client_) {
        if (!connect()) {
            return "Error: Failed to connect to MCP service";
        }
    }

    std::string url = service_url_ + "/tools/" + name + "/execute";

    // 准备请求头
    std::map<std::string, std::string> headers;
    headers["Content-Type"] = "application/json";
    headers["User-Agent"] = "LangChain-CPP-MCP-Client/1.0";
    if (!api_key_.empty()) {
        headers["Authorization"] = "Bearer " + api_key_;
    }

    // 发送POST请求
    auto response = http_client_->post(url, json_input, headers, 30);

    if (response.status_code != 200) {
        return "Error performing request: HTTP " + std::to_string(response.status_code);
    }

    return response.body;
}

bool MCPServiceClient::has_remote_tool(const String& name) {
    return remote_tools_.find(name) != remote_tools_.end();
}

bool MCPServiceClient::refresh_tools() {
    auto tools = list_remote_tools();
    return !tools.empty();
}

// MCPRemoteTool implementation
MCPRemoteTool::MCPRemoteTool(const MCPToolMetadata& meta, std::shared_ptr<MCPServiceClient> service_client)
    : MCPTool(meta), service_client_(service_client) {}

String MCPRemoteTool::execute(const String& json_input) {
    if (service_client_) {
        return service_client_->execute_remote_tool(metadata.name, json_input);
    }
    return "Error: MCP service client not available";
}

} // namespace langchain