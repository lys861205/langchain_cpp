#ifndef LANGCHAIN_DATA_CONNECTORS_H
#define LANGCHAIN_DATA_CONNECTORS_H

#include "core.h"
#include <memory>
#include <vector>
#include <map>

namespace langchain {

// Base class for all data connectors
class DataConnector {
public:
    virtual ~DataConnector() = default;

    // Connect to the data source
    virtual bool connect() = 0;

    // Disconnect from the data source
    virtual void disconnect() = 0;

    // Check if connected to the data source
    virtual bool is_connected() const = 0;

    // Load documents from the data source
    virtual std::vector<Document> load_documents(const std::map<String, String>& options = {}) = 0;

    // Save documents to the data source
    virtual bool save_documents(const std::vector<Document>& documents, const std::map<String, String>& options = {}) = 0;

    // Delete documents from the data source
    virtual bool delete_documents(const StringList& ids, const std::map<String, String>& options = {}) = 0;

    // Query documents from the data source
    virtual std::vector<Document> query_documents(const String& query, const std::map<String, String>& options = {}) = 0;
};

// SQL Database Connector interface
class SQLDatabaseConnector : public DataConnector {
protected:
    String connection_string_;
    bool connected_;

public:
    SQLDatabaseConnector(const String& connection_string);
    virtual ~SQLDatabaseConnector() = default;

    bool connect() override;
    void disconnect() override;
    bool is_connected() const override;

    // Execute a SQL query and return results as documents
    virtual std::vector<Document> execute_query(const String& query) = 0;

    // Execute a SQL statement (INSERT, UPDATE, DELETE)
    virtual bool execute_statement(const String& statement) = 0;

    // Get table schema information
    virtual std::map<String, String> get_table_schema(const String& table_name) = 0;
};

// NoSQL Database Connector interface
class NoSQLDatabaseConnector : public DataConnector {
protected:
    String connection_string_;
    bool connected_;

public:
    NoSQLDatabaseConnector(const String& connection_string);
    virtual ~NoSQLDatabaseConnector() = default;

    bool connect() override;
    void disconnect() override;
    bool is_connected() const override;

    // Get document by key
    virtual Document get_document(const String& key) = 0;

    // Put document by key
    virtual bool put_document(const String& key, const Document& document) = 0;

    // Delete document by key
    virtual bool delete_document(const String& key) = 0;

    // Query documents with a filter
    virtual std::vector<Document> query_documents(const std::map<String, String>& filter) = 0;
};

// Web API Connector interface
class WebAPIConnector : public DataConnector {
protected:
    String base_url_;
    String api_key_;
    std::map<String, String> headers_;
    bool connected_;

public:
    WebAPIConnector(const String& base_url, const String& api_key = "");
    virtual ~WebAPIConnector() = default;

    bool connect() override;
    void disconnect() override;
    bool is_connected() const override;

    // Set authentication headers
    void set_auth_headers(const std::map<String, String>& headers);

    // Make HTTP GET request
    virtual std::vector<Document> get_request(const String& endpoint, const std::map<String, String>& params = {}) = 0;

    // Make HTTP POST request
    virtual bool post_request(const String& endpoint, const String& data, const String& content_type = "application/json") = 0;

    // Make HTTP PUT request
    virtual bool put_request(const String& endpoint, const String& data, const String& content_type = "application/json") = 0;

    // Make HTTP DELETE request
    virtual bool delete_request(const String& endpoint) = 0;
};

// Factory class for creating data connectors
class DataConnectorFactory {
public:
    // Create SQL database connector
    static std::shared_ptr<SQLDatabaseConnector> create_sql_connector(const String& type, const String& connection_string);

    // Create NoSQL database connector
    static std::shared_ptr<NoSQLDatabaseConnector> create_nosql_connector(const String& type, const String& connection_string);

    // Create Web API connector
    static std::shared_ptr<WebAPIConnector> create_web_api_connector(const String& type, const String& base_url, const String& api_key = "");
};

} // namespace langchain

#endif // LANGCHAIN_DATA_CONNECTORS_H