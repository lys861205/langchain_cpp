#ifndef LANGCHAIN_REST_API_CONNECTOR_H
#define LANGCHAIN_REST_API_CONNECTOR_H

#include "data_connectors.h"
#include "../include/langchain/http_client.h"
#include <nlohmann/json.hpp>
#include <memory>

namespace langchain {

// REST API Connector
class RestAPIConnector : public WebAPIConnector {
private:
    std::unique_ptr<HttpClient> http_client_;
    String auth_type_;  // "bearer", "basic", "api_key", etc.

public:
    RestAPIConnector(const String& base_url, const String& api_key = "", const String& auth_type = "bearer");
    virtual ~RestAPIConnector();

    // Override base class methods
    bool connect() override;
    void disconnect() override;
    bool is_connected() const override;
    std::vector<Document> load_documents(const std::map<String, String>& options = {}) override;
    bool save_documents(const std::vector<Document>& documents, const std::map<String, String>& options = {}) override;
    bool delete_documents(const StringList& ids, const std::map<String, String>& options = {}) override;
    std::vector<Document> query_documents(const String& query, const std::map<String, String>& options = {}) override;

    // REST API-specific methods
    std::vector<Document> get_request(const String& endpoint, const std::map<String, String>& params = {}) override;
    bool post_request(const String& endpoint, const String& data, const String& content_type = "application/json") override;
    bool put_request(const String& endpoint, const String& data, const String& content_type = "application/json") override;
    bool delete_request(const String& endpoint) override;

private:
    // Helper methods
    String build_url(const String& endpoint);
    std::map<String, String> prepare_headers(const String& content_type = "application/json");
    Document json_to_document(const nlohmann::json& json_obj);
    std::vector<Document> json_array_to_documents(const nlohmann::json& json_array);
};

} // namespace langchain

#endif // LANGCHAIN_REST_API_CONNECTOR_H