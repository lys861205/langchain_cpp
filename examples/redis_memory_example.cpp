#include <iostream>
#include <memory>
#include "../include/langchain/langchain.h"

using namespace langchain;

int main() {
    std::cout << "LangChain C++ Redis Memory Example\n";
    std::cout << "==================================\n\n";

    // Test RedisMemory
    std::cout << "Creating RedisMemory instance...\n";
    auto redis_memory = std::make_shared<RedisMemory>("localhost", 6379, "", "langchain:test:");

    // Test adding messages
    std::cout << "--- Testing Redis Memory Operations ---\n";
    redis_memory->add_message("Human", "Hello, this is a test message!");
    redis_memory->add_message("AI", "Hello! I'm responding to your test message.");
    redis_memory->add_message("Human", "Can you remember what I said earlier?");
    redis_memory->add_message("AI", "Yes, I can access our conversation history from Redis.");

    // Test getting messages
    std::cout << "Current memory size: " << redis_memory->size() << "\n";
    std::cout << "Messages in memory:\n";
    auto messages = redis_memory->get_messages();
    for (const auto& message : messages) {
        std::cout << "  " << message.first << ": " << message.second << "\n";
    }

    // Test ConversationBufferMemory with RedisMemory
    std::cout << "\n--- Testing ConversationBufferMemory with RedisMemory ---\n";
    auto redis_conversation_memory = std::make_shared<ConversationBufferMemory>(redis_memory);

    redis_conversation_memory->add_user_message("What are the benefits of using Redis for memory storage?");
    redis_conversation_memory->add_ai_message("Redis provides fast, persistent storage that can be shared across multiple instances.");

    std::cout << "Conversation history:\n";
    std::cout << redis_conversation_memory->get_history() << "\n";

    // Test clearing memory
    std::cout << "--- Testing Memory Clear ---\n";
    std::cout << "Memory size before clear: " << redis_memory->size() << "\n";
    redis_memory->clear();
    std::cout << "Memory size after clear: " << redis_memory->size() << "\n";

    // Test max size functionality
    std::cout << "\n--- Testing Max Size Functionality ---\n";
    redis_memory->set_max_size(3);

    for (int i = 1; i <= 5; i++) {
        redis_memory->add_message("System", "Message " + std::to_string(i));
    }

    std::cout << "After adding 5 messages with max_size=3, memory size: " << redis_memory->size() << "\n";
    std::cout << "Messages in memory:\n";
    messages = redis_memory->get_messages();
    for (const auto& message : messages) {
        std::cout << "  " << message.first << ": " << message.second << "\n";
    }

    std::cout << "\nRedis memory example completed successfully!\n";

    return 0;
}