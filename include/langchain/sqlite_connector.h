#ifndef LANGCHAIN_SQLITE_CONNECTOR_H
#define LANGCHAIN_SQLITE_CONNECTOR_H

#include "data_connectors.h"
#include <sqlite3.h>
#include <memory>

namespace langchain {

// SQLite Database Connector
class SQLiteConnector : public SQLDatabaseConnector {
private:
    sqlite3* db_;
    std::string database_path_;

public:
    SQLiteConnector(const String& database_path);
    virtual ~SQLiteConnector();

    // Override base class methods
    bool connect() override;
    void disconnect() override;
    bool is_connected() const override;
    std::vector<Document> load_documents(const std::map<String, String>& options = {}) override;
    bool save_documents(const std::vector<Document>& documents, const std::map<String, String>& options = {}) override;
    bool delete_documents(const StringList& ids, const std::map<String, String>& options = {}) override;
    std::vector<Document> query_documents(const String& query, const std::map<String, String>& options = {}) override;

    // SQLite-specific methods
    std::vector<Document> execute_query(const String& query) override;
    bool execute_statement(const String& statement) override;
    std::map<String, String> get_table_schema(const String& table_name) override;

private:
    // Helper methods
    bool create_documents_table();
    Document row_to_document(sqlite3_stmt* stmt);
};

} // namespace langchain

#endif // LANGCHAIN_SQLITE_CONNECTOR_H