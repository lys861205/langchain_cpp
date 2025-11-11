#include <iostream>
#include <memory>
#include "../include/langchain/langchain.h"

using namespace langchain;

int main() {
    std::cout << "LangChain C++ Memory Example\n";
    std::cout << "============================\n\n";

    // Create an LLM (using SimpleLLM for this example)
    auto llm = std::make_shared<SimpleLLM>();

    // Create tools
    auto calculator = std::make_shared<CalculatorTool>();
    auto string_tool = std::make_shared<StringTool>();
    std::vector<std::shared_ptr<Tool>> tools = {calculator, string_tool};

    // Create short-term memory
    auto short_term_memory = std::make_shared<ShortTermMemory>(5);
    auto conversation_memory = std::make_shared<ConversationBufferMemory>(short_term_memory);

    // Create agent with memory
    std::cout << "Creating agent with short-term memory...\n";
    auto agent = std::make_shared<SimpleAgent>(llm, tools, conversation_memory);

    // Test conversation with memory
    std::cout << "\n--- Conversation with Memory ---\n";

    // First interaction
    std::cout << "User: What is the capital of France?\n";
    String response1 = agent->execute("What is the capital of France?");
    std::cout << "AI: " << response1 << "\n\n";

    // Second interaction - should remember previous conversation
    std::cout << "User: What did I just ask you?\n";
    String response2 = agent->execute("What did I just ask you?");
    std::cout << "AI: " << response2 << "\n\n";

    // Third interaction - test with a tool
    std::cout << "User: What is 25 + 17?\n";
    String response3 = agent->execute("What is 25 + 17?");
    std::cout << "AI: " << response3 << "\n\n";

    // Fourth interaction - should remember previous calculations
    std::cout << "User: What was the result of the calculation I asked for?\n";
    String response4 = agent->execute("What was the result of the calculation I asked for?");
    std::cout << "AI: " << response4 << "\n\n";

    // Show memory contents
    std::cout << "--- Memory Contents ---\n";
    auto messages = conversation_memory->get_messages();
    for (const auto& message : messages) {
        std::cout << message.first << ": " << message.second << "\n";
    }

    // Test long-term memory with vector store
    std::cout << "\n--- Testing Long-Term Memory ---\n";

    // Create a vector store for long-term memory
    auto vector_store = std::make_shared<InMemoryVectorStore>();

    // Create long-term memory
    auto long_term_memory = std::make_shared<LongTermMemory>(vector_store, 3);
    auto long_term_conversation_memory = std::make_shared<ConversationBufferMemory>(long_term_memory);

    // Create agent with long-term memory
    std::cout << "Creating agent with long-term memory...\n";
    auto long_term_agent = std::make_shared<SimpleAgent>(llm, tools, long_term_conversation_memory);

    // Add some messages to long-term memory
    long_term_conversation_memory->add_user_message("My favorite color is blue.");
    long_term_conversation_memory->add_ai_message("That's a nice color!");
    long_term_conversation_memory->add_user_message("I like blue because it reminds me of the ocean.");
    long_term_conversation_memory->add_ai_message("That makes sense. The ocean is beautiful.");

    // Save to long-term storage
    long_term_memory->save_to_long_term();

    // Now ask a related question
    std::cout << "\nUser: What did I tell you about my favorite color?\n";
    String response5 = long_term_agent->execute("What did I tell you about my favorite color?");
    std::cout << "AI: " << response5 << "\n\n";

    // Show recent messages in long-term memory
    std::cout << "--- Recent Messages in Long-Term Memory ---\n";
    auto recent_messages = long_term_memory->get_recent_messages();
    for (const auto& message : recent_messages) {
        std::cout << message.first << ": " << message.second << "\n";
    }

    std::cout << "\nMemory example completed successfully!\n";

    return 0;
}