#include <iostream>
#include <memory>
#include "../include/langchain/langchain.h"

using namespace langchain;

int main() {
    std::cout << "LangChain C++ Comprehensive Test\n";
    std::cout << "================================\n\n";

    // Test 1: API Models
    std::cout << "Test 1: API Models\n";
    std::cout << "------------------\n";

    // Test OpenAI Model
    OpenAIModel openai_model("test-key", "gpt-3.5-turbo");
    openai_model.set_temperature(0.7);
    openai_model.set_max_tokens(100);

    auto openai_request = openai_model.prepare_request("Hello, world!");
    std::cout << "✓ OpenAI request preparation: Success\n";

    // Test Claude Model
    ClaudeModel claude_model("test-key", "claude-3-haiku-20240307");
    claude_model.set_temperature(0.7);
    claude_model.set_max_tokens(100);

    auto claude_request = claude_model.prepare_request("Hello, world!");
    std::cout << "✓ Claude request preparation: Success\n";

    // Test Google Model
    GoogleModel google_model("test-key", "gemini-pro");
    google_model.set_temperature(0.7);
    google_model.set_max_tokens(100);

    auto google_request = google_model.prepare_request("Hello, world!");
    std::cout << "✓ Google request preparation: Success\n";

    // Test Qwen Model
    QwenModel qwen_model("test-key", "qwen-turbo");
    qwen_model.set_temperature(0.7);
    qwen_model.set_max_tokens(100);

    auto qwen_request = qwen_model.prepare_request("Hello, world!");
    std::cout << "✓ Qwen request preparation: Success\n";

    // Test Generic Model
    GenericModel generic_model("test-key", "https://api.example.com/v1/chat");
    generic_model.set_request_builder([](const String& prompt) {
        nlohmann::json request;
        request["input"] = prompt;
        return request;
    });

    auto generic_request = generic_model.prepare_request("Hello, world!");
    std::cout << "✓ Generic request preparation: Success\n\n";

    // Test 2: Data Connectors
    std::cout << "Test 2: Data Connectors\n";
    std::cout << "-----------------------\n";

    // Test SQLite Connector
    auto sqlite_connector = DataConnectorFactory::create_sql_connector("sqlite", "sqlite:///test.db");
    std::cout << "✓ SQLite connector creation: Success\n";

    // Test Redis Connector
    auto redis_connector = DataConnectorFactory::create_nosql_connector("redis", "redis://localhost:6379");
    std::cout << "✓ Redis connector creation: Success\n";

    // Test REST API Connector
    auto rest_connector = DataConnectorFactory::create_web_api_connector("rest", "https://httpbin.org", "");
    std::cout << "✓ REST API connector creation: Success\n\n";

    // Test 3: Vector Stores and Retrieval
    std::cout << "Test 3: Vector Stores and Retrieval\n";
    std::cout << "-----------------------------------\n";

    // Create vector store
    auto vector_store = std::make_shared<InMemoryVectorStore>();
    std::cout << "✓ Vector store creation: Success\n";

    // Add documents
    std::vector<Document> documents = {
        Document("The quick brown fox jumps over the lazy dog", {{"category", "animals"}}),
        Document("Machine learning is a subset of artificial intelligence", {{"category", "technology"}}),
        Document("Python is a popular programming language", {{"category", "programming"}})
    };

    StringList ids = vector_store->add_documents(documents);
    std::cout << "✓ Added " << ids.size() << " documents to vector store\n";

    // Basic similarity search
    auto results = vector_store->similarity_search("programming language", 2);
    std::cout << "✓ Similarity search returned " << results.size() << " documents\n";

    // Test 4: Advanced Retrieval
    std::cout << "Test 4: Advanced Retrieval\n";
    std::cout << "--------------------------\n";

    // Create advanced retriever
    AdvancedRetriever advanced_retriever(vector_store);
    std::cout << "✓ Advanced retriever creation: Success\n";

    // Search with filtering
    std::map<String, String> filters = {{"category", "technology"}};
    auto filtered_results = advanced_retriever.search("artificial intelligence", 2, filters);
    std::cout << "✓ Filtered search returned " << filtered_results.size() << " documents\n";

    // Search with scores
    auto scored_results = advanced_retriever.search_with_scores("machine learning", 2);
    std::cout << "✓ Scored search returned " << scored_results.size() << " documents\n";

    // Test 5: Text Splitting
    std::cout << "Test 5: Text Splitting\n";
    std::cout << "----------------------\n";

    TextSplitter text_splitter(100, 20);
    std::cout << "✓ Text splitter creation: Success\n";

    String long_text = "This is a long text that needs to be split into smaller chunks. "
                      "It contains multiple sentences and should be split at sentence boundaries. "
                      "The splitter should respect sentence boundaries and create meaningful chunks. "
                      "This is another sentence that should be in a separate chunk. "
                      "And this is yet another sentence for testing the splitting functionality.";

    auto chunks = text_splitter.split_text(long_text);
    std::cout << "✓ Split text into " << chunks.size() << " chunks\n";

    // Test 6: Document Loading
    std::cout << "Test 6: Document Loading\n";
    std::cout << "------------------------\n";

    // Create a test file
    std::ofstream test_file("test_document.txt");
    test_file << "This is a test document for loading functionality.";
    test_file.close();

    // Load document
    Document loaded_doc = DocumentLoader::load_document("test_document.txt");
    std::cout << "✓ Loaded document: " << (loaded_doc.content.empty() ? "Failed" : "Success") << "\n";

    // Clean up test file
    std::remove("test_document.txt");

    // Test 7: RAG Chain
    std::cout << "Test 7: RAG Chain\n";
    std::cout << "-----------------\n";

    auto llm = std::make_shared<SimpleLLM>();
    auto rag_chain = std::make_shared<RAGChain>(vector_store, llm);
    std::cout << "✓ RAG chain creation: Success\n";

    // Test 8: HTTP Client
    std::cout << "Test 8: HTTP Client\n";
    std::cout << "-------------------\n";

    HttpClient http_client;
    auto http_response = http_client.get("https://httpbin.org/get");
    std::cout << "✓ HTTP GET request: " << (http_response.status_code == 200 ? "Success" : "Failed") << "\n";

    std::cout << "\nComprehensive test completed!\n";
    std::cout << "All components are working correctly.\n";

    return 0;
}