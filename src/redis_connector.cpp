#include "../include/langchain/redis_connector.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace langchain {

RedisConnector::RedisConnector(const String& host, int port, const String& password)
    : NoSQLDatabaseConnector("redis://" + host + ":" + std::to_string(port)),
      redis_ctx_(nullptr), host_(host), port_(port), password_(password) {}

RedisConnector::~RedisConnector() {
    disconnect();
}

bool RedisConnector::connect() {
    if (connected_) {
        return true;
    }

    redis_ctx_ = redisConnect(host_.c_str(), port_);
    if (redis_ctx_ == nullptr || redis_ctx_->err) {
        if (redis_ctx_) {
            std::cerr << "Redis connection error: " << redis_ctx_->errstr << std::endl;
            redisFree(redis_ctx_);
            redis_ctx_ = nullptr;
        } else {
            std::cerr << "Redis connection error: can't allocate redis context" << std::endl;
        }
        return false;
    }

    // Authenticate if password is provided
    if (!password_.empty()) {
        redisReply* reply = (redisReply*)redisCommand(redis_ctx_, "AUTH %s", password_.c_str());
        if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
            std::cerr << "Redis authentication failed" << std::endl;
            if (reply) freeReplyObject(reply);
            redisFree(redis_ctx_);
            redis_ctx_ = nullptr;
            return false;
        }
        freeReplyObject(reply);
    }

    connected_ = true;
    return true;
}

void RedisConnector::disconnect() {
    if (redis_ctx_) {
        redisFree(redis_ctx_);
        redis_ctx_ = nullptr;
    }
    connected_ = false;
}

bool RedisConnector::is_connected() const {
    return connected_ && redis_ctx_ != nullptr;
}

std::vector<Document> RedisConnector::load_documents(const std::map<String, String>& options) {
    if (!is_connected()) {
        std::cerr << "Not connected to Redis" << std::endl;
        return {};
    }

    // Get all keys matching a pattern (default: "doc:*")
    String pattern = "doc:*";
    auto it = options.find("pattern");
    if (it != options.end()) {
        pattern = it->second;
    }

    redisReply* reply = (redisReply*)redisCommand(redis_ctx_, "KEYS %s", pattern.c_str());
    if (reply == nullptr || reply->type != REDIS_REPLY_ARRAY) {
        std::cerr << "Failed to get keys from Redis" << std::endl;
        if (reply) freeReplyObject(reply);
        return {};
    }

    std::vector<Document> documents;
    for (size_t i = 0; i < reply->elements; i++) {
        String key = reply->element[i]->str;
        Document doc = get_document(key);
        if (!doc.id.empty()) {
            documents.push_back(doc);
        }
    }

    freeReplyObject(reply);
    return documents;
}

bool RedisConnector::save_documents(const std::vector<Document>& documents, const std::map<String, String>& options) {
    if (!is_connected()) {
        std::cerr << "Not connected to Redis" << std::endl;
        return false;
    }

    bool success = true;
    for (const auto& doc : documents) {
        String key = doc.id.empty() ? "doc:" + std::to_string(std::hash<String>{}(doc.content)) : "doc:" + doc.id;
        if (!put_document(key, doc)) {
            success = false;
        }
    }

    return success;
}

bool RedisConnector::delete_documents(const StringList& ids, const std::map<String, String>& options) {
    if (!is_connected()) {
        std::cerr << "Not connected to Redis" << std::endl;
        return false;
    }

    bool success = true;
    for (const auto& id : ids) {
        String key = "doc:" + id;
        if (!delete_document(key)) {
            success = false;
        }
    }

    return success;
}

std::vector<Document> RedisConnector::query_documents(const String& query, const std::map<String, String>& options) {
    if (!is_connected()) {
        std::cerr << "Not connected to Redis" << std::endl;
        return {};
    }

    // For simplicity, we'll treat the query as a key prefix
    String pattern = query + "*";
    auto it = options.find("pattern");
    if (it != options.end()) {
        pattern = it->second;
    }

    redisReply* reply = (redisReply*)redisCommand(redis_ctx_, "KEYS %s", pattern.c_str());
    if (reply == nullptr || reply->type != REDIS_REPLY_ARRAY) {
        std::cerr << "Failed to get keys from Redis" << std::endl;
        if (reply) freeReplyObject(reply);
        return {};
    }

    std::vector<Document> documents;
    for (size_t i = 0; i < reply->elements; i++) {
        String key = reply->element[i]->str;
        Document doc = get_document(key);
        if (!doc.id.empty()) {
            documents.push_back(doc);
        }
    }

    freeReplyObject(reply);
    return documents;
}

Document RedisConnector::get_document(const String& key) {
    if (!is_connected()) {
        std::cerr << "Not connected to Redis" << std::endl;
        return Document();
    }

    redisReply* reply = (redisReply*)redisCommand(redis_ctx_, "GET %s", key.c_str());
    if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
        std::cerr << "Document not found in Redis: " << key << std::endl;
        if (reply) freeReplyObject(reply);
        return Document();
    }

    if (reply->type != REDIS_REPLY_STRING) {
        std::cerr << "Invalid reply type from Redis" << std::endl;
        freeReplyObject(reply);
        return Document();
    }

    String json_str = reply->str;
    freeReplyObject(reply);

    return json_to_document(json_str);
}

bool RedisConnector::put_document(const String& key, const Document& document) {
    if (!is_connected()) {
        std::cerr << "Not connected to Redis" << std::endl;
        return false;
    }

    String json_str = document_to_json(document);
    redisReply* reply = (redisReply*)redisCommand(redis_ctx_, "SET %s %s", key.c_str(), json_str.c_str());

    if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
        std::cerr << "Failed to save document to Redis" << std::endl;
        if (reply) freeReplyObject(reply);
        return false;
    }

    freeReplyObject(reply);
    return true;
}

bool RedisConnector::delete_document(const String& key) {
    if (!is_connected()) {
        std::cerr << "Not connected to Redis" << std::endl;
        return false;
    }

    redisReply* reply = (redisReply*)redisCommand(redis_ctx_, "DEL %s", key.c_str());

    if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
        std::cerr << "Failed to delete document from Redis" << std::endl;
        if (reply) freeReplyObject(reply);
        return false;
    }

    freeReplyObject(reply);
    return true;
}

std::vector<Document> RedisConnector::query_documents(const std::map<String, String>& filter) {
    if (!is_connected()) {
        std::cerr << "Not connected to Redis" << std::endl;
        return {};
    }

    // For simplicity, we'll just return all documents
    // In a real implementation, we would implement proper filtering
    return load_documents();
}

String RedisConnector::document_to_json(const Document& doc) {
    std::ostringstream json;
    json << "{";
    json << "\"id\":\"" << escape_json_string(doc.id) << "\",";
    json << "\"content\":\"" << escape_json_string(doc.content) << "\",";
    json << "\"metadata\":{";

    bool first = true;
    for (const auto& pair : doc.metadata) {
        if (!first) json << ",";
        json << "\"" << escape_json_string(pair.first) << "\":\"" << escape_json_string(pair.second) << "\"";
        first = false;
    }
    json << "}}";

    return json.str();
}

Document RedisConnector::json_to_document(const String& json_str) {
    // This is a simplified JSON parser for demonstration purposes
    // In a real implementation, you would use a proper JSON library
    Document doc;

    // Extract id
    size_t id_pos = json_str.find("\"id\":\"");
    if (id_pos != String::npos) {
        size_t start = id_pos + 6;  // Length of "\"id\":\""
        size_t end = json_str.find("\"", start);
        if (end != String::npos) {
            doc.id = json_str.substr(start, end - start);
        }
    }

    // Extract content
    size_t content_pos = json_str.find("\"content\":\"");
    if (content_pos != String::npos) {
        size_t start = content_pos + 11;  // Length of "\"content\":\""
        size_t end = json_str.find("\"", start);
        if (end != String::npos) {
            doc.content = json_str.substr(start, end - start);
        }
    }

    return doc;
}

String RedisConnector::escape_json_string(const String& str) {
    String escaped;
    escaped.reserve(str.length() * 2);

    for (char c : str) {
        switch (c) {
            case '\"': escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\b': escaped += "\\b"; break;
            case '\f': escaped += "\\f"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default:
                if (c >= 0 && c < 32) {
                    // Escape control characters
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\u%04x", c);
                    escaped += buf;
                } else {
                    escaped += c;
                }
                break;
        }
    }

    return escaped;
}

} // namespace langchain