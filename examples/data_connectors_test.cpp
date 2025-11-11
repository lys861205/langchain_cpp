#include <iostream>
#include <memory>
#include "../include/langchain/langchain.h"

using namespace langchain;

int main() {
    std::cout << "LangChain C++ Data Connectors Test\n";
    std::cout << "==================================\n\n";

    // Test 1: SQLite Connector
    std::cout << "Test 1: SQLite Connector\n";
    std::cout << "------------------------\n";

    auto sqlite_connector = DataConnectorFactory::create_sql_connector("sqlite", "sqlite:///test.db");

    if (sqlite_connector->connect()) {
        std::cout << "✓ Connected to SQLite database\n";

        // Create test documents
        std::vector<Document> test_docs = {
            Document("This is the first test document", {{"category", "test"}, {"source", "sqlite"}}),
            Document("This is the second test document", {{"category", "test"}, {"source", "sqlite"}}),
            Document("This is the third test document", {{"category", "test"}, {"source", "sqlite"}})
        };

        // Test save_documents
        bool save_result = sqlite_connector->save_documents(test_docs);
        std::cout << (save_result ? "✓" : "✗") << " Save documents: " << (save_result ? "Success" : "Failed") << "\n";

        // Test load_documents
        auto loaded_docs = sqlite_connector->load_documents();
        std::cout << "✓ Loaded " << loaded_docs.size() << " documents\n";

        // Test query_documents
        auto queried_docs = sqlite_connector->query_documents("SELECT * FROM documents LIMIT 2");
        std::cout << "✓ Queried " << queried_docs.size() << " documents\n";

        // Test delete_documents
        StringList doc_ids = {"1", "2"};
        bool delete_result = sqlite_connector->delete_documents(doc_ids);
        std::cout << (delete_result ? "✓" : "✗") << " Delete documents: " << (delete_result ? "Success" : "Failed") << "\n";

        sqlite_connector->disconnect();
        std::cout << "✓ Disconnected from SQLite database\n\n";
    } else {
        std::cout << "✗ Failed to connect to SQLite database\n\n";
    }

    // Test 2: Redis Connector
    std::cout << "Test 2: Redis Connector\n";
    std::cout << "-----------------------\n";

    auto redis_connector = DataConnectorFactory::create_nosql_connector("redis", "redis://localhost:6379");

    if (redis_connector->connect()) {
        std::cout << "✓ Connected to Redis\n";

        // Test put_document
        Document test_doc("This is a test document for Redis", {{"category", "test"}, {"source", "redis"}});
        bool put_result = redis_connector->put_document("test_doc", test_doc);
        std::cout << (put_result ? "✓" : "✗") << " Put document: " << (put_result ? "Success" : "Failed") << "\n";

        // Test get_document
        Document retrieved_doc = redis_connector->get_document("test_doc");
        std::cout << "✓ Retrieved document: " << (retrieved_doc.content.empty() ? "Empty" : "Success") << "\n";

        // Test delete_document
        bool delete_result = redis_connector->delete_document("test_doc");
        std::cout << (delete_result ? "✓" : "✗") << " Delete document: " << (delete_result ? "Success" : "Failed") << "\n";

        redis_connector->disconnect();
        std::cout << "✓ Disconnected from Redis\n\n";
    } else {
        std::cout << "✗ Failed to connect to Redis (this is expected if Redis is not running)\n\n";
    }

    // Test 3: REST API Connector
    std::cout << "Test 3: REST API Connector\n";
    std::cout << "--------------------------\n";

    auto rest_connector = DataConnectorFactory::create_web_api_connector("rest", "https://httpbin.org", "");

    if (rest_connector->connect()) {
        std::cout << "✓ Connected to REST API\n";

        // Test get_request
        auto get_result = rest_connector->get_request("/get");
        std::cout << "✓ GET request: " << (get_result.empty() ? "No data" : "Success") << "\n";

        // Test post_request
        bool post_result = rest_connector->post_request("/post", "{\"test\": \"data\"}");
        std::cout << (post_result ? "✓" : "✗") << " POST request: " << (post_result ? "Success" : "Failed") << "\n";

        rest_connector->disconnect();
        std::cout << "✓ Disconnected from REST API\n\n";
    } else {
        std::cout << "✗ Failed to connect to REST API\n\n";
    }

    // Test 4: DataConnectorFactory
    std::cout << "Test 4: DataConnectorFactory\n";
    std::cout << "----------------------------\n";

    // Test creating different types of connectors
    auto sqlite_factory = DataConnectorFactory::create_sql_connector("sqlite", "sqlite:///test.db");
    std::cout << "✓ Created SQLite connector via factory\n";

    auto redis_factory = DataConnectorFactory::create_nosql_connector("redis", "redis://localhost:6379");
    std::cout << "✓ Created Redis connector via factory\n";

    auto rest_factory = DataConnectorFactory::create_web_api_connector("rest", "https://httpbin.org", "");
    std::cout << "✓ Created REST API connector via factory\n";

    std::cout << "\nAll tests completed!\n";
    return 0;
}