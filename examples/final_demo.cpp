#include "../include/langchain/core.h"
#include "../include/langchain/llms.h"
#include "../include/langchain/chains.h"
#include "../include/langchain/tools.h"
#include "../include/langchain/agents.h"
#include "../include/langchain/vectorstores.h"
#include "../include/langchain/memory.h"
#include "../include/langchain/models.h"
#include "../include/langchain/data_connectors.h"
#include "../include/langchain/advanced_retrievers.h"
#include "../include/langchain/http_client.h"
#include "../include/langchain/sqlite_connector.h"
#include <iostream>
#include <memory>
#include <vector>

using namespace langchain;

int main() {
    std::cout << "LangChain C++ Final Demo" << std::endl;
    std::cout << "========================" << std::endl;

    // 1. Demonstrate HTTP Client
    std::cout << "\n1. HTTP Client Demo" << std::endl;
    std::cout << "-------------------" << std::endl;
    auto http_client = std::make_shared<HttpClient>();
    auto get_response = http_client->get("https://httpbin.org/get");
    std::cout << "GET Status: " << get_response.status_code << std::endl;

    // 2. Demonstrate API Models
    std::cout << "\n2. API Models Demo" << std::endl;
    std::cout << "------------------" << std::endl;

    // OpenAI Model
    auto openai_model = std::make_shared<OpenAIModel>("sk-xxx");
    std::cout << "OpenAI Model created with default model: gpt-3.5-turbo" << std::endl;

    // Claude Model
    auto claude_model = std::make_shared<ClaudeModel>("sk-ant-xxx");
    std::cout << "Claude Model created with default model: claude-3-haiku-20240307" << std::endl;

    // Google Model
    auto google_model = std::make_shared<GoogleModel>("xxx");
    std::cout << "Google Model created with default model: gemini-pro" << std::endl;

    // 3. Demonstrate Data Connectors
    std::cout << "\n3. Data Connectors Demo" << std::endl;
    std::cout << "-----------------------" << std::endl;

    // SQLite Connector
    auto sqlite_connector = std::make_shared<SQLiteConnector>("demo.db");
    if (sqlite_connector->connect()) {
        std::cout << "SQLite connected successfully" << std::endl;

        // Create a sample document
        Document doc("This is a sample document for testing.");
        doc.metadata["source"] = "demo";
        doc.metadata["category"] = "test";

        // Save document
        std::vector<Document> docs = {doc};
        if (sqlite_connector->save_documents(docs)) {
            std::cout << "Document saved to SQLite" << std::endl;
        }

        // Load documents
        auto loaded_docs = sqlite_connector->load_documents();
        std::cout << "Loaded " << loaded_docs.size() << " documents from SQLite" << std::endl;

        sqlite_connector->disconnect();
    }

    // 4. Demonstrate Vector Store and Retrieval
    std::cout << "\n4. Vector Store and Retrieval Demo" << std::endl;
    std::cout << "----------------------------------" << std::endl;

    // Create documents
    std::vector<Document> documents = {
        Document("Python is a popular programming language", {{"category", "programming"}}),
        Document("C++ is a powerful programming language", {{"category", "programming"}}),
        Document("The quick brown fox jumps over the lazy dog", {{"category", "animals"}}),
        Document("Machine learning is a subset of artificial intelligence", {{"category", "AI"}})
    };

    // Create vector store (using in-memory implementation)
    auto vector_store = std::make_shared<InMemoryVectorStore>();
    vector_store->add_documents(documents);
    std::cout << "Added " << documents.size() << " documents to vector store" << std::endl;

    // Perform similarity search
    auto search_results = vector_store->similarity_search("programming language", 2);
    std::cout << "Found " << search_results.size() << " similar documents:" << std::endl;
    for (const auto& doc : search_results) {
        std::cout << "- " << doc.content << " (Category: " << doc.metadata.at("category") << ")" << std::endl;
    }

    // 5. Demonstrate Advanced Retrieval
    std::cout << "\n5. Advanced Retrieval Demo" << std::endl;
    std::cout << "-------------------------" << std::endl;

    auto advanced_retriever = std::make_shared<AdvancedRetriever>(vector_store, SimilarityAlgorithm::COSINE);
    std::map<String, String> filters = {{"category", "programming"}};
    auto filtered_results = advanced_retriever->search("language", 2, filters);
    std::cout << "Found " << filtered_results.size() << " filtered documents:" << std::endl;
    for (const auto& doc : filtered_results) {
        std::cout << "- " << doc.content << " (Category: " << doc.metadata.at("category") << ")" << std::endl;
    }

    // 6. Demonstrate Text Splitting
    std::cout << "\n6. Text Splitting Demo" << std::endl;
    std::cout << "----------------------" << std::endl;

    auto text_splitter = std::make_shared<TextSplitter>(50, 10);
    String long_text = "This is a long text that needs to be split into smaller chunks. "
                      "It contains multiple sentences and should be divided appropriately. "
                      "The text splitter will break this down into manageable pieces. "
                      "Each chunk should maintain some context and coherence. "
                      "This is the final part of our demonstration text.";

    auto chunks = text_splitter->split_text(long_text);
    std::cout << "Split text into " << chunks.size() << " chunks:" << std::endl;
    for (size_t i = 0; i < chunks.size(); ++i) {
        std::cout << "Chunk " << (i+1) << ": " << chunks[i] << std::endl;
    }

    std::cout << "\nFinal demo completed successfully!" << std::endl;

    return 0;
}