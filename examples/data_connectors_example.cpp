#include <iostream>
#include <memory>
#include "../include/langchain/langchain.h"

using namespace langchain;

int main() {
    std::cout << "LangChain C++ Data Connectors Example\n";
    std::cout << "=====================================\n\n";

    // Example 1: Using SQLite Database Connector
    std::cout << "1. Creating SQLite Database Connector...\n";
    auto sqlite_connector = DataConnectorFactory::create_sql_connector("sqlite", "sqlite:///example.db");

    if (sqlite_connector->connect()) {
        std::cout << "Connected to SQLite database successfully!\n";

        // Create some sample documents
        std::vector<Document> sqlite_documents = {
            Document("This is a sample document from SQLite database", {{"source", "sqlite_db"}, {"table", "documents"}}),
            Document("Another document with different content", {{"source", "sqlite_db"}, {"table", "documents"}})
        };

        // Save documents (in a real implementation, this would save to the database)
        bool saved = sqlite_connector->save_documents(sqlite_documents);
        std::cout << "Documents saved to SQLite database: " << (saved ? "Yes" : "No") << "\n";

        // Query documents (in a real implementation, this would query the database)
        auto sqlite_results = sqlite_connector->query_documents("SELECT * FROM documents WHERE content LIKE '%sample%'");
        std::cout << "Found " << sqlite_results.size() << " documents in SQLite database\n\n";

        sqlite_connector->disconnect();
    } else {
        std::cout << "Failed to connect to SQLite database\n\n";
    }

    // Example 2: Using Redis Connector
    std::cout << "2. Creating Redis Connector...\n";
    auto redis_connector = DataConnectorFactory::create_nosql_connector("redis", "redis://localhost:6379");

    if (redis_connector->connect()) {
        std::cout << "Connected to Redis successfully!\n";

        // Create a sample document
        Document redis_document("This is a sample document from Redis database", {{"source", "redis_db"}, {"key", "doc1"}});

        // Save document (in a real implementation, this would save to Redis)
        bool saved = redis_connector->put_document("doc1", redis_document);
        std::cout << "Document saved to Redis: " << (saved ? "Yes" : "No") << "\n";

        // Get document (in a real implementation, this would retrieve from Redis)
        Document retrieved_doc = redis_connector->get_document("doc1");
        std::cout << "Retrieved document: " << retrieved_doc.content << "\n\n";

        redis_connector->disconnect();
    } else {
        std::cout << "Failed to connect to Redis\n\n";
    }

    // Example 3: Using REST API Connector
    std::cout << "3. Creating REST API Connector...\n";
    auto rest_connector = DataConnectorFactory::create_web_api_connector("rest", "https://api.example.com", "your-api-key");

    if (rest_connector->connect()) {
        std::cout << "Connected to REST API successfully!\n";

        // Make a GET request (in a real implementation, this would make an HTTP request)
        std::map<String, String> params = {{"limit", "10"}, {"offset", "0"}};
        auto rest_documents = rest_connector->get_request("/documents", params);
        std::cout << "Retrieved " << rest_documents.size() << " documents from REST API\n\n";

        rest_connector->disconnect();
    } else {
        std::cout << "Failed to connect to REST API\n\n";
    }

    // Example 4: Using DataConnector with VectorStore
    std::cout << "4. Integrating Data Connector with VectorStore...\n";

    // Create a vector store
    auto vector_store = std::make_shared<InMemoryVectorStore>();

    // Create a SQLite connector
    auto data_connector = DataConnectorFactory::create_sql_connector("sqlite", "sqlite:///example.db");

    if (data_connector->connect()) {
        // Load documents from the data source
        auto documents = data_connector->load_documents();
        std::cout << "Loaded " << documents.size() << " documents from data source\n";

        // Add documents to vector store
        StringList ids = vector_store->add_documents(documents);
        std::cout << "Added " << ids.size() << " documents to vector store\n";

        // Perform similarity search
        auto results = vector_store->similarity_search("sample document", 2);
        std::cout << "Found " << results.size() << " similar documents\n";

        data_connector->disconnect();
    }

    std::cout << "\nData Connectors example completed!\n";
    return 0;
}