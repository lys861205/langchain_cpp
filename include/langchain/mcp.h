#ifndef LANGCHAIN_MCP_H
#define LANGCHAIN_MCP_H

#include "core.h"
#include "http_client.h"
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <nlohmann/json.hpp>

namespace langchain {

// MCP Tool metadata
struct MCPToolMetadata {
    String name;
    String description;
    std::map<String, String> parameters;
    String return_type;
};

// MCP Tool interface
class MCPTool {
public:
    MCPToolMetadata metadata;

    MCPTool(const MCPToolMetadata& meta);
    virtual ~MCPTool() = default;

    // Execute the tool with JSON input
    virtual String execute(const String& json_input) = 0;

    // Get tool metadata
    const MCPToolMetadata& get_metadata() const;
};

// MCP Tool Manager
class MCPToolManager {
private:
    std::map<String, std::shared_ptr<MCPTool>> tools_;

public:
    MCPToolManager();
    ~MCPToolManager();

    // Register a tool
    void register_tool(const std::shared_ptr<MCPTool>& tool);

    // Get tool by name
    std::shared_ptr<MCPTool> get_tool(const String& name);

    // List all tools
    std::vector<MCPToolMetadata> list_tools();

    // Execute a tool by name
    String execute_tool(const String& name, const String& json_input);

    // Check if tool exists
    bool has_tool(const String& name);
};

// MCP-compatible wrapper for existing tools
class MCPToolWrapper : public MCPTool {
private:
    std::shared_ptr<Tool> wrapped_tool_;

public:
    MCPToolWrapper(const std::shared_ptr<Tool>& tool);

    // Execute the wrapped tool
    String execute(const String& json_input) override;
};

// MCP Service Client for connecting to external MCP services
class MCPServiceClient {
private:
    String service_url_;
    String api_key_;
    std::unique_ptr<HttpClient> http_client_;
    std::map<String, MCPToolMetadata> remote_tools_;

public:
    MCPServiceClient(const String& service_url, const String& api_key = "");
    ~MCPServiceClient();

    // Connect to the MCP service
    bool connect();

    // Disconnect from the MCP service
    void disconnect();

    // List tools available from the MCP service
    std::vector<MCPToolMetadata> list_remote_tools();

    // Get tool metadata by name
    MCPToolMetadata get_remote_tool(const String& name);

    // Execute a tool on the MCP service
    String execute_remote_tool(const String& name, const String& json_input);

    // Check if a remote tool exists
    bool has_remote_tool(const String& name);

    // Refresh the list of remote tools
    bool refresh_tools();
};

// MCP Remote Tool that wraps a tool from an MCP service
class MCPRemoteTool : public MCPTool {
private:
    std::shared_ptr<MCPServiceClient> service_client_;

public:
    MCPRemoteTool(const MCPToolMetadata& meta, std::shared_ptr<MCPServiceClient> service_client);

    // Execute the remote tool via the service client
    String execute(const String& json_input) override;
};

} // namespace langchain

#endif // LANGCHAIN_MCP_H