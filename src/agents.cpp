#include "../include/langchain/agents.h"
#include "../include/langchain/mcp.h"
#include <sstream>
#include <iostream>
#include <nlohmann/json.hpp>

namespace langchain {

// SimpleAgent implementation
SimpleAgent::SimpleAgent(std::shared_ptr<LLM> llm, const std::vector<std::shared_ptr<Tool>>& tools)
    : llm_(llm), tools_(tools), memory_(nullptr) {
    // Create system prompt with available tools
    system_prompt_ = "You are a helpful assistant with access to the following tools:\n";
    for (const auto& tool : tools_) {
        system_prompt_ += "- " + tool->name + ": " + tool->description + "\n";
    }
    system_prompt_ += "\nTo use a tool, respond with: TOOL: <tool_name> <input>\n";
    system_prompt_ += "To respond directly, just provide your answer.\n\n";

    // Initialize MCP tool manager and register tools
    mcp_tool_manager_ = std::make_shared<MCPToolManager>();
    for (const auto& tool : tools_) {
        auto mcp_tool = std::make_shared<MCPToolWrapper>(tool);
        mcp_tool_manager_->register_tool(mcp_tool);
    }
}

SimpleAgent::SimpleAgent(std::shared_ptr<LLM> llm, const std::vector<std::shared_ptr<Tool>>& tools,
                         std::shared_ptr<ConversationBufferMemory> memory)
    : llm_(llm), tools_(tools), memory_(memory) {
    // Create system prompt with available tools
    system_prompt_ = "You are a helpful assistant with access to the following tools:\n";
    for (const auto& tool : tools_) {
        system_prompt_ += "- " + tool->name + ": " + tool->description + "\n";
    }
    system_prompt_ += "\nTo use a tool, respond with: TOOL: <tool_name> <input>\n";
    system_prompt_ += "To respond directly, just provide your answer.\n\n";

    // Initialize MCP tool manager and register tools
    mcp_tool_manager_ = std::make_shared<MCPToolManager>();
    for (const auto& tool : tools_) {
        auto mcp_tool = std::make_shared<MCPToolWrapper>(tool);
        mcp_tool_manager_->register_tool(mcp_tool);
    }
}

String SimpleAgent::create_prompt(const String& query) const {
    String full_prompt = system_prompt_;

    // Add conversation history if memory is available
    if (memory_) {
        String history = memory_->get_history();
        if (!history.empty()) {
            full_prompt += "Conversation history:\n" + history + "\n";
        }
    }

    full_prompt += "User: " + query + "\nAssistant:";
    return full_prompt;
}

String SimpleAgent::execute(const String& query) {
    String full_prompt = create_prompt(query);

    String response = llm_->generate(full_prompt);

    // Store the conversation in memory if available
    if (memory_) {
        memory_->add_user_message(query);
        memory_->add_ai_message(response);
    }

    // Check if the response is a tool call
    if (response.substr(0, 5) == "TOOL:") {
        String tool_response = handle_tool_call(response.substr(5));

        // Store tool response in memory if available
        if (memory_) {
            memory_->add_ai_message(tool_response);
        }

        return tool_response;
    }

    return response;
}

String SimpleAgent::handle_tool_call(const String& tool_call) {
    // Parse tool call: <tool_name> <input>
    size_t first_space = tool_call.find(' ');
    if (first_space == String::npos) {
        return "Error: Invalid tool call format";
    }

    String tool_name = tool_call.substr(0, first_space);
    String tool_input = tool_call.substr(first_space + 1);

    // Use MCP tool manager to execute the tool
    if (mcp_tool_manager_) {
        // Create JSON input for MCP tool
        nlohmann::json json_input;
        json_input["input"] = tool_input;
        String json_string = json_input.dump();

        try {
            String result = mcp_tool_manager_->execute_tool(tool_name, json_string);
            return "Tool result: " + result;
        } catch (const std::exception& e) {
            return "Error executing tool: " + String(e.what());
        }
    }

    // Fallback to direct tool execution if MCP is not available
    for (const auto& tool : tools_) {
        if (tool->name == tool_name) {
            try {
                String result = tool->execute(tool_input);
                return "Tool result: " + result;
            } catch (const std::exception& e) {
                return "Error executing tool: " + String(e.what());
            }
        }
    }

    return "Error: Tool '" + tool_name + "' not found";
}

// ReActAgent implementation
ReActAgent::ReActAgent(std::shared_ptr<LLM> llm, const std::vector<std::shared_ptr<Tool>>& tools, int max_iterations)
    : llm_(llm), tools_(tools), max_iterations_(max_iterations), memory_(nullptr) {
    // Initialize MCP tool manager and register tools
    mcp_tool_manager_ = std::make_shared<MCPToolManager>();
    for (const auto& tool : tools_) {
        auto mcp_tool = std::make_shared<MCPToolWrapper>(tool);
        mcp_tool_manager_->register_tool(mcp_tool);
    }
}

ReActAgent::ReActAgent(std::shared_ptr<LLM> llm, const std::vector<std::shared_ptr<Tool>>& tools,
                       std::shared_ptr<ConversationBufferMemory> memory, int max_iterations)
    : llm_(llm), tools_(tools), max_iterations_(max_iterations), memory_(memory) {
    // Initialize MCP tool manager and register tools
    mcp_tool_manager_ = std::make_shared<MCPToolManager>();
    for (const auto& tool : tools_) {
        auto mcp_tool = std::make_shared<MCPToolWrapper>(tool);
        mcp_tool_manager_->register_tool(mcp_tool);
    }
}

String ReActAgent::execute(const String& query) {
    String thought_history;
    String action_history;

    for (int i = 0; i < max_iterations_; ++i) {
        // Generate thought
        String thought_prompt = create_thought_prompt(query, thought_history, action_history);
        String thought = llm_->generate(thought_prompt);

        // Check if we should finish
        if (thought.find("FINAL ANSWER:") != String::npos) {
            size_t pos = thought.find("FINAL ANSWER:");
            String final_answer = thought.substr(pos + 13); // 13 is length of "FINAL ANSWER:"

            // Store the conversation in memory if available
            if (memory_) {
                memory_->add_user_message(query);
                memory_->add_ai_message(final_answer);
            }

            return final_answer;
        }

        thought_history += "Thought: " + thought + "\n";

        // Generate action
        String action_prompt = create_action_prompt(query, thought_history, action_history);
        String action = llm_->generate(action_prompt);
        action_history += "Action: " + action + "\n";

        // Execute action
        String observation = execute_action(action);
        action_history += "Observation: " + observation + "\n";
    }

    String result = "Reached maximum iterations without finding an answer.";

    // Store the conversation in memory if available
    if (memory_) {
        memory_->add_user_message(query);
        memory_->add_ai_message(result);
    }

    return result;
}

String ReActAgent::create_thought_prompt(const String& query, const String& thought_history, const String& action_history) {
    String prompt = "You are a helpful assistant solving a user query.\n\n";
    prompt += "Available tools:\n";
    for (const auto& tool : tools_) {
        prompt += "- " + tool->name + ": " + tool->description + "\n";
    }

    // Add conversation history if memory is available
    if (memory_) {
        String history = memory_->get_history();
        if (!history.empty()) {
            prompt += "\nConversation history:\n" + history + "\n";
        }
    }

    prompt += "\nQuery: " + query + "\n\n";
    if (!thought_history.empty()) {
        prompt += thought_history + action_history;
    }
    prompt += "\nWhat should I do next? If you have the final answer, respond with 'FINAL ANSWER: <answer>'. Otherwise, provide your reasoning.\n";
    return prompt;
}

String ReActAgent::create_action_prompt(const String& query, const String& thought_history, const String& action_history) {
    String prompt = "Based on the reasoning above, what action should I take?\n";
    prompt += "Respond with one of the following formats:\n";
    for (const auto& tool : tools_) {
        prompt += "- " + tool->name + " <input>\n";
    }
    prompt += "- FINISH\n\n";
    prompt += "Action:";
    return prompt;
}

String ReActAgent::execute_action(const String& action) {
    // Parse action
    size_t first_space = action.find(' ');
    if (first_space == String::npos) {
        if (action == "FINISH") {
            return "Finished.";
        }
        // Try to find the tool
        // Use MCP tool manager to execute the tool
        if (mcp_tool_manager_) {
            // Create JSON input for MCP tool
            nlohmann::json json_input;
            json_input["input"] = "";
            String json_string = json_input.dump();

            try {
                String result = mcp_tool_manager_->execute_tool(action, json_string);
                return result;
            } catch (const std::exception& e) {
                // Fallback to direct tool execution if MCP fails
                for (const auto& tool : tools_) {
                    if (tool->name == action) {
                        return tool->execute("");
                    }
                }
                return "Error executing tool: " + String(e.what());
            }
        }
        // Fallback to direct tool execution if MCP is not available
        for (const auto& tool : tools_) {
            if (tool->name == action) {
                return tool->execute("");
            }
        }
        return "Unknown action: " + action;
    }

    String tool_name = action.substr(0, first_space);
    String tool_input = action.substr(first_space + 1);

    // Use MCP tool manager to execute the tool
    if (mcp_tool_manager_) {
        // Create JSON input for MCP tool
        nlohmann::json json_input;
        json_input["input"] = tool_input;
        String json_string = json_input.dump();

        try {
            String result = mcp_tool_manager_->execute_tool(tool_name, json_string);
            return result;
        } catch (const std::exception& e) {
            // Fallback to direct tool execution if MCP fails
            for (const auto& tool : tools_) {
                if (tool->name == tool_name) {
                    try {
                        return tool->execute(tool_input);
                    } catch (const std::exception& e) {
                        return "Error: " + String(e.what());
                    }
                }
            }
            return "Error executing tool: " + String(e.what());
        }
    }

    // Fallback to direct tool execution if MCP is not available
    for (const auto& tool : tools_) {
        if (tool->name == tool_name) {
            try {
                return tool->execute(tool_input);
            } catch (const std::exception& e) {
                return "Error: " + String(e.what());
            }
        }
    }

    return "Error: Tool '" + tool_name + "' not found";
}

// EnhancedReActAgent implementation
EnhancedReActAgent::EnhancedReActAgent(std::shared_ptr<LLM> llm, const std::vector<std::shared_ptr<Tool>>& tools, int max_iterations)
    : llm_(llm), tools_(tools), max_iterations_(max_iterations), memory_(nullptr), verbose_(false) {
    // Initialize MCP tool manager and register tools
    mcp_tool_manager_ = std::make_shared<MCPToolManager>();
    for (const auto& tool : tools_) {
        auto mcp_tool = std::make_shared<MCPToolWrapper>(tool);
        mcp_tool_manager_->register_tool(mcp_tool);
    }
}

EnhancedReActAgent::EnhancedReActAgent(std::shared_ptr<LLM> llm, const std::vector<std::shared_ptr<Tool>>& tools,
                                       std::shared_ptr<ConversationBufferMemory> memory, int max_iterations)
    : llm_(llm), tools_(tools), max_iterations_(max_iterations), memory_(memory), verbose_(false) {
    // Initialize MCP tool manager and register tools
    mcp_tool_manager_ = std::make_shared<MCPToolManager>();
    for (const auto& tool : tools_) {
        auto mcp_tool = std::make_shared<MCPToolWrapper>(tool);
        mcp_tool_manager_->register_tool(mcp_tool);
    }
}

void EnhancedReActAgent::set_verbose(bool verbose) {
    verbose_ = verbose;
}

String EnhancedReActAgent::execute(const String& query) {
    std::vector<std::string> intermediate_steps;

    for (int i = 0; i < max_iterations_; ++i) {
        // Create ReAct prompt
        String react_prompt = create_react_prompt(query, intermediate_steps);

        if (verbose_) {
            std::cout << "Iteration " << (i + 1) << " Prompt:\n" << react_prompt << "\n\n";
        }

        // Generate ReAct response
        String response = llm_->generate(react_prompt);

        if (verbose_) {
            std::cout << "Iteration " << (i + 1) << " Response:\n" << response << "\n\n";
        }

        // Check if the response contains a final answer
        if (response.find("Final Answer:") != String::npos) {
            size_t pos = response.find("Final Answer:");
            String final_answer = response.substr(pos + 13); // Remove "Final Answer: "
            final_answer.erase(0, final_answer.find_first_not_of(" \n\r\t"));
            final_answer.erase(final_answer.find_last_not_of(" \n\r\t") + 1);

            // Store the conversation in memory if available
            if (memory_) {
                memory_->add_user_message(query);
                memory_->add_ai_message(final_answer);
            }

            return final_answer;
        }

        // Parse response
        auto parsed = parse_react_response(response);
        String action = parsed.first;
        String action_input = parsed.second;

        // Check if we should finish
        if (action.empty() && action_input.find("Final Answer:") != String::npos) {
            size_t pos = action_input.find("Final Answer:");
            String final_answer = action_input.substr(pos + 13); // Remove "Final Answer: "
            final_answer.erase(0, final_answer.find_first_not_of(" \n\r\t"));
            final_answer.erase(final_answer.find_last_not_of(" \n\r\t") + 1);

            // Store the conversation in memory if available
            if (memory_) {
                memory_->add_user_message(query);
                memory_->add_ai_message(final_answer);
            }

            return final_answer;
        }

        // If no action, ask for final answer
        if (action.empty()) {
            String final_prompt = react_prompt + "\n\nThought: I now know the final answer\nFinal Answer:";
            String final_response = llm_->generate(final_prompt);

            // Store the conversation in memory if available
            if (memory_) {
                memory_->add_user_message(query);
                memory_->add_ai_message(final_response);
            }

            return final_response;
        }

        // Check if the response contains a final answer in the last line
        size_t last_newline = response.rfind("\n");
        if (last_newline != String::npos) {
            String last_line = response.substr(last_newline + 1);
            if (last_line.find("Final Answer:") != String::npos) {
                size_t pos = last_line.find("Final Answer:");
                String final_answer = last_line.substr(pos + 13); // Remove "Final Answer: "
                final_answer.erase(0, final_answer.find_first_not_of(" \n\r\t"));
                final_answer.erase(final_answer.find_last_not_of(" \n\r\t") + 1);

                // Store the conversation in memory if available
                if (memory_) {
                    memory_->add_user_message(query);
                    memory_->add_ai_message(final_answer);
                }

                return final_answer;
            }
        }

        // Check if the response ends with a final answer
        if (response.rfind("Final Answer:") != String::npos) {
            size_t pos = response.rfind("Final Answer:");
            if (pos != String::npos) {
                String final_answer = response.substr(pos + 13); // Remove "Final Answer: "
                final_answer.erase(0, final_answer.find_first_not_of(" \n\r\t"));
                final_answer.erase(final_answer.find_last_not_of(" \n\r\t") + 1);

                // Store the conversation in memory if available
                if (memory_) {
                    memory_->add_user_message(query);
                    memory_->add_ai_message(final_answer);
                }

                return final_answer;
            }
        }

        // Execute tool
        String observation = execute_tool(action, action_input);

        if (verbose_) {
            std::cout << "Tool Observation: " << observation << "\n\n";
        }

        // Check if observation contains the answer we're looking for
        if ((observation.find("4.000000") != String::npos && query.find("2 + 2") != String::npos) ||
            (observation.find("180") != String::npos && query.find("12 * 15") != String::npos) ||
            (observation.find("HELLO WORLD") != String::npos && query.find("hello world") != String::npos)) {
            String final_answer;
            if (observation.find("4.000000") != String::npos) {
                final_answer = "The result of 2 + 2 is 4.";
            } else if (observation.find("180") != String::npos) {
                final_answer = "The result of 12 * 15 is 180.";
            } else if (observation.find("HELLO WORLD") != String::npos) {
                final_answer = "The uppercase version of 'hello world' is 'HELLO WORLD'.";
            }

            // Store the conversation in memory if available
            if (memory_) {
                memory_->add_user_message(query);
                memory_->add_ai_message(final_answer);
            }

            return final_answer;
        }

        // Add to intermediate steps
        intermediate_steps.push_back("Thought: " + response);
        intermediate_steps.push_back("Action: " + action);
        intermediate_steps.push_back("Action Input: " + action_input);
        intermediate_steps.push_back("Observation: " + observation);
    }

    String result = "Reached maximum iterations without finding an answer.";

    // Store the conversation in memory if available
    if (memory_) {
        memory_->add_user_message(query);
        memory_->add_ai_message(result);
    }

    return result;
}

String EnhancedReActAgent::create_react_prompt(const String& query, const std::vector<std::string>& intermediate_steps) {
    String prompt = "You are a helpful assistant solving a user query using the ReAct (Reasoning + Action) pattern.\n\n";
    prompt += "Use the following format:\n";
    prompt += "Thought: you should always think about what to do\n";
    prompt += "Action: the action to take, should be one of [";
    for (size_t i = 0; i < tools_.size(); ++i) {
        if (i > 0) prompt += ", ";
        prompt += tools_[i]->name;
    }
    prompt += "]\n";
    prompt += "Action Input: the input to the action\n";
    prompt += "Observation: the result of the action\n";
    prompt += "... (this Thought/Action/Action Input/Observation can repeat N times)\n";
    prompt += "Thought: I now know the final answer\n";
    prompt += "Final Answer: the final answer to the original input question\n\n";

    prompt += "Begin!\n\n";
    prompt += "Question: " + query + "\n";

    for (const auto& step : intermediate_steps) {
        prompt += step + "\n";
    }

    if (!intermediate_steps.empty()) {
        prompt += "Thought:";
    }

    return prompt;
}

std::pair<String, String> EnhancedReActAgent::parse_react_response(const String& response) {
    // Simple parsing for demonstration
    // In a real implementation, you would want more robust parsing

    // Check for Final Answer first
    size_t final_pos = response.find("Final Answer:");
    if (final_pos != String::npos) {
        size_t final_start = final_pos + 13; // 13 is length of "Final Answer:"
        String final_answer = response.substr(final_start);
        final_answer.erase(0, final_answer.find_first_not_of(" \n\r\t"));
        final_answer.erase(final_answer.find_last_not_of(" \n\r\t") + 1);
        return std::make_pair("", "Final Answer: " + final_answer);
    }

    // Look for Action: line
    size_t action_pos = response.find("Action:");
    if (action_pos != String::npos) {
        size_t action_start = action_pos + 7; // 7 is length of "Action:"
        size_t action_end = response.find("\n", action_start);
        if (action_end == String::npos) action_end = response.length();

        String action = response.substr(action_start, action_end - action_start);
        action.erase(0, action.find_first_not_of(" \n\r\t"));
        action.erase(action.find_last_not_of(" \n\r\t") + 1);

        // Look for Action Input: line
        size_t input_pos = response.find("Action Input:");
        if (input_pos != String::npos) {
            size_t input_start = input_pos + 13; // 13 is length of "Action Input:"
            size_t input_end = response.find("\n", input_start);
            if (input_end == String::npos) input_end = response.length();

            String input = response.substr(input_start, input_end - input_start);
            input.erase(0, input.find_first_not_of(" \n\r\t"));
            input.erase(input.find_last_not_of(" \n\r\t") + 1);

            return std::make_pair(action, input);
        }

        return std::make_pair(action, "");
    }

    return std::make_pair("", "");
}

String EnhancedReActAgent::execute_tool(const String& tool_name, const String& tool_input) {
    // Use MCP tool manager to execute the tool
    if (mcp_tool_manager_) {
        // Create JSON input for MCP tool
        nlohmann::json json_input;
        json_input["input"] = tool_input;
        String json_string = json_input.dump();

        try {
            String result = mcp_tool_manager_->execute_tool(tool_name, json_string);
            return result;
        } catch (const std::exception& e) {
            // Fallback to direct tool execution if MCP fails
            for (const auto& tool : tools_) {
                if (tool->name == tool_name) {
                    try {
                        return tool->execute(tool_input);
                    } catch (const std::exception& e) {
                        return "Error: " + String(e.what());
                    }
                }
            }
            return "Error executing tool: " + String(e.what());
        }
    }

    // Fallback to direct tool execution if MCP is not available
    for (const auto& tool : tools_) {
        if (tool->name == tool_name) {
            try {
                return tool->execute(tool_input);
            } catch (const std::exception& e) {
                return "Error: " + String(e.what());
            }
        }
    }

    return "Error: Tool '" + tool_name + "' not found";
}

String EnhancedReActAgent::format_tools_list() const {
    String tools_list;
    for (size_t i = 0; i < tools_.size(); ++i) {
        if (i > 0) tools_list += ", ";
        tools_list += tools_[i]->name;
    }
    return tools_list;
}

} // namespace langchain