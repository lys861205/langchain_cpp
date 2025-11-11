#include "../include/langchain/sqlite_connector.h"
#include <iostream>
#include <sstream>

namespace langchain {

SQLiteConnector::SQLiteConnector(const String& database_path)
    : SQLDatabaseConnector("sqlite:///" + database_path), db_(nullptr), database_path_(database_path) {}

SQLiteConnector::~SQLiteConnector() {
    disconnect();
}

bool SQLiteConnector::connect() {
    if (connected_) {
        return true;
    }

    int rc = sqlite3_open(database_path_.c_str(), &db_);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db_) << std::endl;
        sqlite3_close(db_);
        db_ = nullptr;
        return false;
    }

    connected_ = true;

    // Create documents table if it doesn't exist
    create_documents_table();

    return true;
}

void SQLiteConnector::disconnect() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
    connected_ = false;
}

bool SQLiteConnector::is_connected() const {
    return connected_ && db_ != nullptr;
}

std::vector<Document> SQLiteConnector::load_documents(const std::map<String, String>& options) {
    if (!is_connected()) {
        std::cerr << "Not connected to database" << std::endl;
        return {};
    }

    String table_name = "documents";
    auto it = options.find("table");
    if (it != options.end()) {
        table_name = it->second;
    }

    String query = "SELECT id, content, metadata FROM " + table_name;
    return execute_query(query);
}

bool SQLiteConnector::save_documents(const std::vector<Document>& documents, const std::map<String, String>& options) {
    if (!is_connected()) {
        std::cerr << "Not connected to database" << std::endl;
        return false;
    }

    String table_name = "documents";
    auto it = options.find("table");
    if (it != options.end()) {
        table_name = it->second;
    }

    // Begin transaction for better performance
    sqlite3_exec(db_, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);

    String sql = "INSERT OR REPLACE INTO " + table_name + " (id, content, metadata) VALUES (?, ?, ?)";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);
        return false;
    }

    bool success = true;
    for (const auto& doc : documents) {
        sqlite3_bind_text(stmt, 1, doc.id.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, doc.content.c_str(), -1, SQLITE_STATIC);

        // Convert metadata to JSON string
        String metadata_str = "{";
        bool first = true;
        for (const auto& pair : doc.metadata) {
            if (!first) metadata_str += ",";
            metadata_str += "\"" + pair.first + "\":\"" + pair.second + "\"";
            first = false;
        }
        metadata_str += "}";

        sqlite3_bind_text(stmt, 3, metadata_str.c_str(), -1, SQLITE_STATIC);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db_) << std::endl;
            success = false;
            break;
        }

        sqlite3_reset(stmt);
    }

    sqlite3_finalize(stmt);

    if (success) {
        sqlite3_exec(db_, "COMMIT", nullptr, nullptr, nullptr);
    } else {
        sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);
    }

    return success;
}

bool SQLiteConnector::delete_documents(const StringList& ids, const std::map<String, String>& options) {
    if (!is_connected()) {
        std::cerr << "Not connected to database" << std::endl;
        return false;
    }

    String table_name = "documents";
    auto it = options.find("table");
    if (it != options.end()) {
        table_name = it->second;
    }

    // Begin transaction for better performance
    sqlite3_exec(db_, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);

    String sql = "DELETE FROM " + table_name + " WHERE id = ?";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);
        return false;
    }

    bool success = true;
    for (const auto& id : ids) {
        sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_STATIC);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db_) << std::endl;
            success = false;
            break;
        }

        sqlite3_reset(stmt);
    }

    sqlite3_finalize(stmt);

    if (success) {
        sqlite3_exec(db_, "COMMIT", nullptr, nullptr, nullptr);
    } else {
        sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);
    }

    return success;
}

std::vector<Document> SQLiteConnector::query_documents(const String& query, const std::map<String, String>& options) {
    if (!is_connected()) {
        std::cerr << "Not connected to database" << std::endl;
        return {};
    }

    return execute_query(query);
}

std::vector<Document> SQLiteConnector::execute_query(const String& query) {
    if (!is_connected()) {
        std::cerr << "Not connected to database" << std::endl;
        return {};
    }

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return {};
    }

    std::vector<Document> documents;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        Document doc = row_to_document(stmt);
        documents.push_back(doc);
    }

    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to execute query: " << sqlite3_errmsg(db_) << std::endl;
    }

    sqlite3_finalize(stmt);
    return documents;
}

bool SQLiteConnector::execute_statement(const String& statement) {
    if (!is_connected()) {
        std::cerr << "Not connected to database" << std::endl;
        return false;
    }

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_, statement.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    return true;
}

std::map<String, String> SQLiteConnector::get_table_schema(const String& table_name) {
    if (!is_connected()) {
        std::cerr << "Not connected to database" << std::endl;
        return {};
    }

    String query = "PRAGMA table_info(" + table_name + ")";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return {};
    }

    std::map<String, String> schema;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        String column_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        String column_type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        schema[column_name] = column_type;
    }

    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to execute query: " << sqlite3_errmsg(db_) << std::endl;
    }

    sqlite3_finalize(stmt);
    return schema;
}

bool SQLiteConnector::create_documents_table() {
    if (!is_connected()) {
        return false;
    }

    String sql = R"(
        CREATE TABLE IF NOT EXISTS documents (
            id TEXT PRIMARY KEY,
            content TEXT,
            metadata TEXT
        )
    )";

    return execute_statement(sql);
}

Document SQLiteConnector::row_to_document(sqlite3_stmt* stmt) {
    Document doc;

    // Get ID
    const unsigned char* id_text = sqlite3_column_text(stmt, 0);
    if (id_text) {
        doc.id = reinterpret_cast<const char*>(id_text);
    }

    // Get content
    const unsigned char* content_text = sqlite3_column_text(stmt, 1);
    if (content_text) {
        doc.content = reinterpret_cast<const char*>(content_text);
    }

    // Get metadata (in a real implementation, we would parse JSON metadata)
    const unsigned char* metadata_text = sqlite3_column_text(stmt, 2);
    if (metadata_text) {
        // For simplicity, we're just storing metadata as a string
        // In a real implementation, we would parse this as JSON
        doc.metadata["raw_metadata"] = reinterpret_cast<const char*>(metadata_text);
    }

    return doc;
}

} // namespace langchain