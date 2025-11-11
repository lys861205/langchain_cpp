#ifndef LANGCHAIN_SIMPLE_CONNECTORS_H
#define LANGCHAIN_SIMPLE_CONNECTORS_H

#include "data_connectors.h"
#include <memory>

namespace langchain {

// Simple SQL Database Connector implementation
class SimpleSQLConnector : public SQLDatabaseConnector {
public:
    SimpleSQLConnector(const String& connection_string);
    virtual ~SimpleSQLConnector() = default;

    // Implement pure virtual methods
    std::vector<Document> load_documents(const std::map<String, String>& options = {}) override;
    bool save_documents(const std::vector<Document>& documents, const std::map<String, String>& options = {}) override;
    bool delete_documents(const StringList& ids, const std::map<String, String>& options = {}) override;
    std::vector<Document> query_documents(const String& query, const std::map<String, String>& options = {}) override;

    // SQL-specific methods
    std::vector<Document> execute_query(const String& query) override;
    bool execute_statement(const String& statement) override;
    std::map<String, String> get_table_schema(const String& table_name) override;
};

// Simple NoSQL Database Connector implementation
class SimpleNoSQLConnector : public NoSQLDatabaseConnector {
public:
    SimpleNoSQLConnector(const String& connection_string);
    virtual ~SimpleNoSQLConnector() = default;

    // Implement pure virtual methods
    std::vector<Document> load_documents(const std::map<String, String>& options = {}) override;
    bool save_documents(const std::vector<Document>& documents, const std::map<String, String>& options = {}) override;
    bool delete_documents(const StringList& ids, const std::map<String, String>& options = {}) override;
    std::vector<Document> query_documents(const String& query, const std::map<String, String>& options = {}) override;

    // NoSQL-specific methods
    Document get_document(const String& key) override;
    bool put_document(const String& key, const Document& document) override;
    bool delete_document(const String& key) override;
    std::vector<Document> query_documents(const std::map<String, String>& filter) override;
};

// Simple Web API Connector implementation
class SimpleWebAPIConnector : public WebAPIConnector {
public:
    SimpleWebAPIConnector(const String& base_url, const String& api_key = "");
    virtual ~SimpleWebAPIConnector() = default;

    // Implement pure virtual methods
    std::vector<Document> load_documents(const std::map<String, String>& options = {}) override;
    bool save_documents(const std::vector<Document>& documents, const std::map<String, String>& options = {}) override;
    bool delete_documents(const StringList& ids, const std::map<String, String>& options = {}) override;
    std::vector<Document> query_documents(const String& query, const std::map<String, String>& options = {}) override;

    // Web API-specific methods
    std::vector<Document> get_request(const String& endpoint, const std::map<String, String>& params = {}) override;
    bool post_request(const String& endpoint, const String& data, const String& content_type = "application/json") override;
    bool put_request(const String& endpoint, const String& data, const String& content_type = "application/json") override;
    bool delete_request(const String& endpoint) override;
};

} // namespace langchain

#endif // LANGCHAIN_SIMPLE_CONNECTORS_H