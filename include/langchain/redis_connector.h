#ifndef LANGCHAIN_REDIS_CONNECTOR_H
#define LANGCHAIN_REDIS_CONNECTOR_H

#include "data_connectors.h"
#include <hiredis/hiredis.h>
#include <memory>

namespace langchain {

// Redis Database Connector
class RedisConnector : public NoSQLDatabaseConnector {
private:
    redisContext* redis_ctx_;
    String host_;
    int port_;
    String password_;

public:
    RedisConnector(const String& host, int port = 6379, const String& password = "");
    virtual ~RedisConnector();

    // Override base class methods
    bool connect() override;
    void disconnect() override;
    bool is_connected() const override;
    std::vector<Document> load_documents(const std::map<String, String>& options = {}) override;
    bool save_documents(const std::vector<Document>& documents, const std::map<String, String>& options = {}) override;
    bool delete_documents(const StringList& ids, const std::map<String, String>& options = {}) override;
    std::vector<Document> query_documents(const String& query, const std::map<String, String>& options = {}) override;

    // Redis-specific methods
    Document get_document(const String& key) override;
    bool put_document(const String& key, const Document& document) override;
    bool delete_document(const String& key) override;
    std::vector<Document> query_documents(const std::map<String, String>& filter) override;

private:
    // Helper methods
    String document_to_json(const Document& doc);
    Document json_to_document(const String& json_str);
    String escape_json_string(const String& str);
};

} // namespace langchain

#endif // LANGCHAIN_REDIS_CONNECTOR_H