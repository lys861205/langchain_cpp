#ifndef LANGCHAIN_AGENTS_H
#define LANGCHAIN_AGENTS_H

#include "core.h"
#include "llms.h"
#include "tools.h"
#include "chains.h"
#include "memory.h"
#include "mcp.h"
#include <sstream>

namespace langchain {

// Simple agent that can use tools
class SimpleAgent {
private:
    std::shared_ptr<LLM> llm_;
    std::vector<std::shared_ptr<Tool>> tools_;
    String system_prompt_;
    std::shared_ptr<ConversationBufferMemory> memory_;
    std::shared_ptr<MCPToolManager> mcp_tool_manager_;

public:
    SimpleAgent(std::shared_ptr<LLM> llm, const std::vector<std::shared_ptr<Tool>>& tools);
    SimpleAgent(std::shared_ptr<LLM> llm, const std::vector<std::shared_ptr<Tool>>& tools,
                std::shared_ptr<ConversationBufferMemory> memory);
    String execute(const String& query);

private:
    String handle_tool_call(const String& tool_call);
    String create_prompt(const String& query) const;
};

// ReAct agent (Reasoning and Acting)
class ReActAgent {
private:
    std::shared_ptr<LLM> llm_;
    std::vector<std::shared_ptr<Tool>> tools_;
    int max_iterations_;
    std::shared_ptr<ConversationBufferMemory> memory_;
    std::shared_ptr<MCPToolManager> mcp_tool_manager_;

public:
    ReActAgent(std::shared_ptr<LLM> llm, const std::vector<std::shared_ptr<Tool>>& tools, int max_iterations = 5);
    ReActAgent(std::shared_ptr<LLM> llm, const std::vector<std::shared_ptr<Tool>>& tools,
               std::shared_ptr<ConversationBufferMemory> memory, int max_iterations = 5);
    String execute(const String& query);

private:
    String create_thought_prompt(const String& query, const String& thought_history, const String& action_history);
    String create_action_prompt(const String& query, const String& thought_history, const String& action_history);
    String execute_action(const String& action);
};

// Enhanced ReAct agent with better ReAct pattern support
class EnhancedReActAgent {
private:
    std::shared_ptr<LLM> llm_;
    std::vector<std::shared_ptr<Tool>> tools_;
    int max_iterations_;
    std::shared_ptr<ConversationBufferMemory> memory_;
    bool verbose_;
    std::shared_ptr<MCPToolManager> mcp_tool_manager_;

public:
    EnhancedReActAgent(std::shared_ptr<LLM> llm, const std::vector<std::shared_ptr<Tool>>& tools, int max_iterations = 5);
    EnhancedReActAgent(std::shared_ptr<LLM> llm, const std::vector<std::shared_ptr<Tool>>& tools,
                       std::shared_ptr<ConversationBufferMemory> memory, int max_iterations = 5);

    // Set verbose mode to see intermediate steps
    void set_verbose(bool verbose);

    String execute(const String& query);

private:
    String create_react_prompt(const String& query, const std::vector<std::string>& intermediate_steps);
    std::pair<String, String> parse_react_response(const String& response);
    String execute_tool(const String& tool_name, const String& tool_input);
    String format_tools_list() const;
};

} // namespace langchain

#endif // LANGCHAIN_AGENTS_H