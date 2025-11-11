#include "../include/langchain/memory.h"
#include "../include/langchain/core.h"
#include "../include/langchain/vectorstores.h"
#include <algorithm>
#include <sstream>

#ifdef __cplusplus
extern "C" {
#endif

#include <hiredis/hiredis.h>

#ifdef __cplusplus
}
#endif

namespace langchain {

// RedisMemory implementation
RedisMemory::RedisMemory(const String& host, int port,
                         const String& password, const String& key_prefix,
                         size_t max_size)
    : host_(host), port_(port), password_(password), key_prefix_(key_prefix),
      max_size_(max_size), redis_context_(nullptr) {
    connect();
}

RedisMemory::~RedisMemory() {
    disconnect();
}

bool RedisMemory::connect() {
    redisContext* context = redisConnect(host_.c_str(), port_);
    if (context == nullptr) {
        redis_context_ = nullptr;
        return false;
    }

    if (context->err) {
        redisFree(context);
        redis_context_ = nullptr;
        return false;
    }

    // Authenticate if password is provided
    if (!password_.empty()) {
        redisReply* reply = (redisReply*)redisCommand(context, "AUTH %s", password_.c_str());
        if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
            if (reply) {
                freeReplyObject(reply);
            }
            redisFree(context);
            redis_context_ = nullptr;
            return false;
        }
        freeReplyObject(reply);
    }

    redis_context_ = context;
    return true;
}

void RedisMemory::disconnect() {
    if (redis_context_) {
        redisFree((redisContext*)redis_context_);
        redis_context_ = nullptr;
    }
}

bool RedisMemory::is_connected() const {
    if (!redis_context_) return false;
    redisContext* context = (redisContext*)redis_context_;
    return context->err == 0;
}

void RedisMemory::add_message(const String& role, const String& content) {
    if (!is_connected()) {
        if (!connect()) {
            return;
        }
    }

    redisContext* context = (redisContext*)redis_context_;

    // Create a unique key for this message
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    String message_key = key_prefix_ + "message:" + std::to_string(now);

    // Store the message as a hash
    redisReply* reply = (redisReply*)redisCommand(context, "HMSET %s role %s content %s timestamp %lld",
                                                  message_key.c_str(), role.c_str(), content.c_str(), now);
    if (reply) {
        freeReplyObject(reply);
    }

    // Add the message key to a list for ordering
    String list_key = key_prefix_ + "messages";
    reply = (redisReply*)redisCommand(context, "LPUSH %s %s", list_key.c_str(), message_key.c_str());
    if (reply) {
        freeReplyObject(reply);
    }

    // Trim the list to max_size
    long long trim_size = static_cast<long long>(max_size_ - 1);
    reply = (redisReply*)redisCommand(context, "LTRIM %s 0 %lld", list_key.c_str(), trim_size);
    if (reply) {
        freeReplyObject(reply);
    }
}

std::vector<std::pair<String, String>> RedisMemory::get_messages() const {
    std::vector<std::pair<String, String>> messages;

    if (!is_connected()) {
        return messages;
    }

    redisContext* context = (redisContext*)redis_context_;

    // Get the list of message keys
    String list_key = key_prefix_ + "messages";
    redisReply* reply = (redisReply*)redisCommand(context, "LRANGE %s 0 -1", list_key.c_str());

    if (!reply) {
        return messages;
    }

    if (reply && reply->type == REDIS_REPLY_ARRAY) {
        // Process messages in reverse order (oldest first)
        for (int i = reply->elements - 1; i >= 0; i--) {
            if (reply->element[i]->str) {
                String message_key = reply->element[i]->str;

                // Get the message content
                redisReply* msg_reply = (redisReply*)redisCommand(context, "HMGET %s role content", message_key.c_str());
                if (msg_reply && msg_reply->type == REDIS_REPLY_ARRAY && msg_reply->elements == 2) {
                    if (msg_reply->element[0]->str && msg_reply->element[1]->str) {
                        String role = msg_reply->element[0]->str;
                        String content = msg_reply->element[1]->str;
                        messages.emplace_back(role, content);
                    }
                }
                if (msg_reply) {
                    freeReplyObject(msg_reply);
                }
            }
        }
    }

    if (reply) {
        freeReplyObject(reply);
    }

    return messages;
}

void RedisMemory::clear() {
    if (!is_connected()) {
        return;
    }

    redisContext* context = (redisContext*)redis_context_;

    // Get all message keys
    String list_key = key_prefix_ + "messages";
    redisReply* reply = (redisReply*)redisCommand(context, "LRANGE %s 0 -1", list_key.c_str());

    if (reply && reply->type == REDIS_REPLY_ARRAY) {
        // Delete each message hash
        for (size_t i = 0; i < reply->elements; i++) {
            String message_key = reply->element[i]->str;
            redisReply* del_reply = (redisReply*)redisCommand(context, "DEL %s", message_key.c_str());
            if (del_reply) {
                freeReplyObject(del_reply);
            }
        }
    }

    // Clear the list
    redisReply* clear_reply = (redisReply*)redisCommand(context, "DEL %s", list_key.c_str());
    if (clear_reply) {
        freeReplyObject(clear_reply);
    }

    if (reply) {
        freeReplyObject(reply);
    }
}

size_t RedisMemory::size() const {
    if (!is_connected()) {
        return 0;
    }

    redisContext* context = (redisContext*)redis_context_;
    String list_key = key_prefix_ + "messages";
    redisReply* reply = (redisReply*)redisCommand(context, "LLEN %s", list_key.c_str());

    size_t length = 0;
    if (reply && reply->type == REDIS_REPLY_INTEGER) {
        length = reply->integer;
    }

    if (reply) {
        freeReplyObject(reply);
    }

    return length;
}

size_t RedisMemory::max_size() const {
    return max_size_;
}

void RedisMemory::set_max_size(size_t max_size) {
    max_size_ = max_size;

    // Trim existing messages if needed
    if (is_connected()) {
        redisContext* context = (redisContext*)redis_context_;
        String list_key = key_prefix_ + "messages";
        redisReply* reply = (redisReply*)redisCommand(context, "LTRIM %s 0 %zu", list_key.c_str(), max_size_ - 1);
        if (reply) {
            freeReplyObject(reply);
        }
    }
}

// ShortTermMemory implementation
ShortTermMemory::ShortTermMemory(size_t max_size) : max_size_(max_size) {}

void ShortTermMemory::add_message(const String& role, const String& content) {
    messages_.emplace_back(role, content);
    // Remove oldest message if we exceed max size
    if (messages_.size() > max_size_) {
        messages_.pop_front();
    }
}

std::vector<std::pair<String, String>> ShortTermMemory::get_messages() const {
    return std::vector<std::pair<String, String>>(messages_.begin(), messages_.end());
}

void ShortTermMemory::clear() {
    messages_.clear();
}

size_t ShortTermMemory::size() const {
    return messages_.size();
}

size_t ShortTermMemory::max_size() const {
    return max_size_;
}

void ShortTermMemory::set_max_size(size_t max_size) {
    max_size_ = max_size;
    // Trim messages if new size is smaller
    while (messages_.size() > max_size_) {
        messages_.pop_front();
    }
}

// LongTermMemory implementation
LongTermMemory::LongTermMemory(std::shared_ptr<VectorStore> vector_store, size_t recent_memory_size)
    : vector_store_(vector_store), recent_memory_size_(recent_memory_size) {}

void LongTermMemory::add_message(const String& role, const String& content) {
    recent_messages_.emplace_back(role, content);
    // Move oldest messages to long-term storage
    if (recent_messages_.size() > recent_memory_size_) {
        auto oldest = recent_messages_.front();
        recent_messages_.pop_front();

        // Create document for long-term storage
        StringMap metadata;
        metadata["role"] = oldest.first;
        metadata["type"] = "memory";

        // Add timestamp
        auto now = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        metadata["timestamp"] = std::to_string(now);

        Document doc(oldest.first + ": " + oldest.second, metadata);
        std::vector<Document> docs = {doc};
        vector_store_->add_documents(docs);
    }
}

std::vector<std::pair<String, String>> LongTermMemory::get_messages() const {
    // Get recent messages
    auto recent = get_recent_messages();

    // Get relevant messages from long-term storage (if any)
    std::vector<std::pair<String, String>> relevant;
    if (!recent.empty()) {
        String last_message = recent.back().second;
        relevant = search_relevant_messages(last_message, 3);
    }

    // Combine recent and relevant messages
    std::vector<std::pair<String, String>> all_messages = relevant;
    all_messages.insert(all_messages.end(), recent.begin(), recent.end());

    return all_messages;
}

std::vector<std::pair<String, String>> LongTermMemory::get_recent_messages() const {
    return std::vector<std::pair<String, String>>(recent_messages_.begin(), recent_messages_.end());
}

std::vector<std::pair<String, String>> LongTermMemory::search_relevant_messages(const String& query, int k) const {
    std::vector<std::pair<String, String>> relevant_messages;

    if (vector_store_) {
        auto results = vector_store_->similarity_search_with_score(query, k);
        for (const auto& result : results) {
            const Document& doc = result.first;
            String content = doc.content;
            String role = doc.metadata.count("role") ? doc.metadata.at("role") : "unknown";
            relevant_messages.emplace_back(role, content);
        }
    }

    return relevant_messages;
}

void LongTermMemory::clear() {
    recent_messages_.clear();
    // Note: We don't clear the vector store as it might contain other data
}

size_t LongTermMemory::size() const {
    return recent_messages_.size();
}

void LongTermMemory::save_to_long_term() {
    // Move all recent messages to long-term storage
    while (!recent_messages_.empty()) {
        auto message = recent_messages_.front();
        recent_messages_.pop_front();

        // Create document for long-term storage
        StringMap metadata;
        metadata["role"] = message.first;
        metadata["type"] = "memory";

        // Add timestamp
        auto now = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        metadata["timestamp"] = std::to_string(now);

        Document doc(message.first + ": " + message.second, metadata);
        std::vector<Document> docs = {doc};
        vector_store_->add_documents(docs);
    }
}

// ConversationBufferMemory implementation
ConversationBufferMemory::ConversationBufferMemory(std::shared_ptr<Memory> memory)
    : memory_(memory), human_prefix_("Human"), ai_prefix_("AI") {}

void ConversationBufferMemory::add_user_message(const String& message) {
    memory_->add_message(human_prefix_, message);
}

void ConversationBufferMemory::add_ai_message(const String& message) {
    memory_->add_message(ai_prefix_, message);
}

String ConversationBufferMemory::get_history() const {
    std::ostringstream oss;
    auto messages = memory_->get_messages();
    for (const auto& message : messages) {
        oss << message.first << ": " << message.second << "\n";
    }
    return oss.str();
}

std::vector<std::pair<String, String>> ConversationBufferMemory::get_messages() const {
    return memory_->get_messages();
}

void ConversationBufferMemory::clear() {
    memory_->clear();
}

} // namespace langchain