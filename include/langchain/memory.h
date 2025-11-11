#ifndef LANGCHAIN_MEMORY_H
#define LANGCHAIN_MEMORY_H

#include "core.h"
#include <deque>
#include <chrono>

namespace langchain {

// Base Memory interface
class Memory {
public:
    virtual ~Memory() = default;

    // Add a message to memory
    virtual void add_message(const String& role, const String& content) = 0;

    // Get all messages
    virtual std::vector<std::pair<String, String>> get_messages() const = 0;

    // Clear memory
    virtual void clear() = 0;

    // Get memory size
    virtual size_t size() const = 0;
};

// Short-term memory implementation
class ShortTermMemory : public Memory {
private:
    std::deque<std::pair<String, String>> messages_;
    size_t max_size_;

public:
    explicit ShortTermMemory(size_t max_size = 10);

    // Add a message to memory
    void add_message(const String& role, const String& content) override;

    // Get all messages
    std::vector<std::pair<String, String>> get_messages() const override;

    // Clear memory
    void clear() override;

    // Get memory size
    size_t size() const override;

    // Get max size
    size_t max_size() const;

    // Set max size
    void set_max_size(size_t max_size);
};

// Long-term memory implementation
class LongTermMemory : public Memory {
private:
    std::shared_ptr<VectorStore> vector_store_;
    std::deque<std::pair<String, String>> recent_messages_;
    size_t recent_memory_size_;

public:
    LongTermMemory(std::shared_ptr<VectorStore> vector_store, size_t recent_memory_size = 5);

    // Add a message to memory
    void add_message(const String& role, const String& content) override;

    // Get all messages (recent + relevant from vector store)
    std::vector<std::pair<String, String>> get_messages() const override;

    // Get recent messages only
    std::vector<std::pair<String, String>> get_recent_messages() const;

    // Search for relevant messages from long-term storage
    std::vector<std::pair<String, String>> search_relevant_messages(const String& query, int k = 3) const;

    // Clear memory
    void clear() override;

    // Get memory size
    size_t size() const override;

    // Save recent messages to long-term storage
    void save_to_long_term();
};

// Redis memory implementation
class RedisMemory : public Memory {
private:
    String host_;
    int port_;
    String password_;
    String key_prefix_;
    size_t max_size_;
    void* redis_context_;  // Opaque pointer to redis context

    bool connect();
    void disconnect();
    bool is_connected() const;

public:
    RedisMemory(const String& host = "localhost", int port = 6379,
                const String& password = "", const String& key_prefix = "langchain:",
                size_t max_size = 100);
    ~RedisMemory();

    // Add a message to memory
    void add_message(const String& role, const String& content) override;

    // Get all messages
    std::vector<std::pair<String, String>> get_messages() const override;

    // Clear memory
    void clear() override;

    // Get memory size
    size_t size() const override;

    // Get max size
    size_t max_size() const;

    // Set max size
    void set_max_size(size_t max_size);
};

// Conversation buffer memory
class ConversationBufferMemory {
private:
    std::shared_ptr<Memory> memory_;
    String human_prefix_;
    String ai_prefix_;

public:
    ConversationBufferMemory(std::shared_ptr<Memory> memory);

    // Add user message
    void add_user_message(const String& message);

    // Add AI message
    void add_ai_message(const String& message);

    // Get conversation history as string
    String get_history() const;

    // Get messages
    std::vector<std::pair<String, String>> get_messages() const;

    // Clear memory
    void clear();
};

} // namespace langchain

#endif // LANGCHAIN_MEMORY_H