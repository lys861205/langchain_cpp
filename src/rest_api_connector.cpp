#include "../include/langchain/rest_api_connector.h"
#include <iostream>
#include <sstream>

namespace langchain {

RestAPIConnector::RestAPIConnector(const String& base_url, const String& api_key, const String& auth_type)
    : WebAPIConnector(base_url, api_key), http_client_(std::make_unique<HttpClient>()), auth_type_(auth_type) {}

RestAPIConnector::~RestAPIConnector() {
    disconnect();
}

bool RestAPIConnector::connect() {
    // For REST API, connection is typically stateless
    // We'll just set connected_ to true
    connected_ = true;
    return true;
}

void RestAPIConnector::disconnect() {
    connected_ = false;
}

bool RestAPIConnector::is_connected() const {
    return connected_;
}

std::vector<Document> RestAPIConnector::load_documents(const std::map<String, String>& options) {
    if (!is_connected()) {
        std::cerr << "Not connected to REST API" << std::endl;
        return {};
    }

    // Get endpoint from options (default: "/documents")
    String endpoint = "/documents";
    auto it = options.find("endpoint");
    if (it != options.end()) {
        endpoint = it->second;
    }

    // Get query parameters
    std::map<String, String> params;
    for (const auto& pair : options) {
        if (pair.first != "endpoint") {
            params[pair.first] = pair.second;
        }
    }

    return get_request(endpoint, params);
}

bool RestAPIConnector::save_documents(const std::vector<Document>& documents, const std::map<String, String>& options) {
    if (!is_connected()) {
        std::cerr << "Not connected to REST API" << std::endl;
        return false;
    }

    // Get endpoint from options (default: "/documents")
    String endpoint = "/documents";
    auto it = options.find("endpoint");
    if (it != options.end()) {
        endpoint = it->second;
    }

    // Convert documents to JSON
    nlohmann::json json_array = nlohmann::json::array();
    for (const auto& doc : documents) {
        nlohmann::json json_doc;
        json_doc["id"] = doc.id;
        json_doc["content"] = doc.content;

        // Convert metadata to JSON object
        nlohmann::json metadata_obj = nlohmann::json::object();
        for (const auto& pair : doc.metadata) {
            metadata_obj[pair.first] = pair.second;
        }
        json_doc["metadata"] = metadata_obj;

        json_array.push_back(json_doc);
    }

    String json_str = json_array.dump();

    return post_request(endpoint, json_str);
}

bool RestAPIConnector::delete_documents(const StringList& ids, const std::map<String, String>& options) {
    if (!is_connected()) {
        std::cerr << "Not connected to REST API" << std::endl;
        return false;
    }

    bool success = true;
    for (const auto& id : ids) {
        String endpoint = "/documents/" + id;
        auto it = options.find("endpoint");
        if (it != options.end()) {
            endpoint = it->second + "/" + id;
        }

        if (!delete_request(endpoint)) {
            success = false;
        }
    }

    return success;
}

std::vector<Document> RestAPIConnector::query_documents(const String& query, const std::map<String, String>& options) {
    if (!is_connected()) {
        std::cerr << "Not connected to REST API" << std::endl;
        return {};
    }

    // Get endpoint from options (default: "/documents/search")
    String endpoint = "/documents/search";
    auto it = options.find("endpoint");
    if (it != options.end()) {
        endpoint = it->second;
    }

    // Add query as parameter
    std::map<String, String> params;
    params["q"] = query;

    // Add other parameters
    for (const auto& pair : options) {
        if (pair.first != "endpoint") {
            params[pair.first] = pair.second;
        }
    }

    return get_request(endpoint, params);
}

std::vector<Document> RestAPIConnector::get_request(const String& endpoint, const std::map<String, String>& params) {
    if (!is_connected()) {
        std::cerr << "Not connected to REST API" << std::endl;
        return {};
    }

    String url = build_url(endpoint);

    // Add query parameters to URL
    if (!params.empty()) {
        url += "?";
        bool first = true;
        for (const auto& pair : params) {
            if (!first) url += "&";
            url += pair.first + "=" + pair.second;
            first = false;
        }
    }

    auto headers = prepare_headers();
    auto response = http_client_->get(url, headers);

    if (response.status_code != 200) {
        std::cerr << "GET request failed with status code: " << response.status_code << std::endl;
        return {};
    }

    try {
        auto json_response = nlohmann::json::parse(response.body);

        // Check if response is an array or single object
        if (json_response.is_array()) {
            return json_array_to_documents(json_response);
        } else {
            std::vector<Document> documents;
            documents.push_back(json_to_document(json_response));
            return documents;
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse JSON response: " << e.what() << std::endl;
        return {};
    }
}

bool RestAPIConnector::post_request(const String& endpoint, const String& data, const String& content_type) {
    if (!is_connected()) {
        std::cerr << "Not connected to REST API" << std::endl;
        return false;
    }

    String url = build_url(endpoint);
    auto headers = prepare_headers(content_type);
    auto response = http_client_->post(url, data, headers);

    return response.status_code >= 200 && response.status_code < 300;
}

bool RestAPIConnector::put_request(const String& endpoint, const String& data, const String& content_type) {
    if (!is_connected()) {
        std::cerr << "Not connected to REST API" << std::endl;
        return false;
    }

    String url = build_url(endpoint);
    auto headers = prepare_headers(content_type);

    // For simplicity, we'll use HttpClient's post method but with PUT semantics
    // In a real implementation, HttpClient would have a put method
    auto response = http_client_->post(url, data, headers);

    return response.status_code >= 200 && response.status_code < 300;
}

bool RestAPIConnector::delete_request(const String& endpoint) {
    if (!is_connected()) {
        std::cerr << "Not connected to REST API" << std::endl;
        return false;
    }

    String url = build_url(endpoint);
    auto headers = prepare_headers();

    // For simplicity, we'll use HttpClient's get method but with DELETE semantics
    // In a real implementation, HttpClient would have a delete method
    auto response = http_client_->get(url, headers);

    return response.status_code >= 200 && response.status_code < 300;
}

String RestAPIConnector::build_url(const String& endpoint) {
    // Remove trailing slash from base_url_ if present
    String base_url = base_url_;
    if (!base_url.empty() && base_url.back() == '/') {
        base_url.pop_back();
    }

    // Add leading slash to endpoint if not present
    String ep = endpoint;
    if (ep.empty() || ep[0] != '/') {
        ep = "/" + ep;
    }

    return base_url + ep;
}

std::map<String, String> RestAPIConnector::prepare_headers(const String& content_type) {
    auto headers = headers_;  // Start with base headers

    // Set content type
    if (!content_type.empty()) {
        headers["Content-Type"] = content_type;
    }

    // Set authorization header based on auth_type_
    if (!api_key_.empty()) {
        if (auth_type_ == "bearer") {
            headers["Authorization"] = "Bearer " + api_key_;
        } else if (auth_type_ == "basic") {
            // In a real implementation, this would be base64 encoded
            headers["Authorization"] = "Basic " + api_key_;
        } else if (auth_type_ == "api_key") {
            headers["X-API-Key"] = api_key_;
        }
    }

    return headers;
}

Document RestAPIConnector::json_to_document(const nlohmann::json& json_obj) {
    Document doc;

    // Extract id
    if (json_obj.contains("id") && json_obj["id"].is_string()) {
        doc.id = json_obj["id"];
    }

    // Extract content
    if (json_obj.contains("content") && json_obj["content"].is_string()) {
        doc.content = json_obj["content"];
    } else if (json_obj.contains("text") && json_obj["text"].is_string()) {
        doc.content = json_obj["text"];
    }

    // Extract metadata
    if (json_obj.contains("metadata") && json_obj["metadata"].is_object()) {
        for (auto& [key, value] : json_obj["metadata"].items()) {
            if (value.is_string()) {
                doc.metadata[key] = value;
            } else {
                doc.metadata[key] = value.dump();
            }
        }
    }

    return doc;
}

std::vector<Document> RestAPIConnector::json_array_to_documents(const nlohmann::json& json_array) {
    std::vector<Document> documents;

    for (const auto& json_obj : json_array) {
        if (json_obj.is_object()) {
            documents.push_back(json_to_document(json_obj));
        }
    }

    return documents;
}

} // namespace langchain