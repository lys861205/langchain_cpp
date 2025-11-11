#include <iostream>
#include <memory>
#include "../include/langchain/langchain.h"

using namespace langchain;

int main() {
    std::cout << "LangChain C++ Advanced Retrieval Example\n";
    std::cout << "========================================\n\n";

    // Create a vector store
    auto vector_store = std::make_shared<InMemoryVectorStore>();

    // Create some sample documents
    std::vector<Document> documents = {
        Document("The quick brown fox jumps over the lazy dog", {{"category", "animals"}, {"source", "example"}}),
        Document("Machine learning is a subset of artificial intelligence", {{"category", "technology"}, {"source", "example"}}),
        Document("Python is a popular programming language", {{"category", "programming"}, {"source", "example"}}),
        Document("The weather is sunny today", {{"category", "weather"}, {"source", "example"}}),
        Document("I love reading books about science", {{"category", "interests"}, {"source", "example"}}),
        Document("The cat sat on the mat", {{"category", "animals"}, {"source", "example"}}),
        Document("Deep learning is a subset of machine learning", {{"category", "technology"}, {"source", "example"}}),
        Document("C++ is a powerful programming language", {{"category", "programming"}, {"source", "example"}})
    };

    // Add documents to the vector store
    std::cout << "Adding documents to vector store...\n";
    StringList ids = vector_store->add_documents(documents);
    std::cout << "Added " << ids.size() << " documents\n\n";

    // Example 1: Basic similarity search
    std::cout << "Example 1: Basic similarity search\n";
    std::cout << "----------------------------------\n";
    auto basic_results = vector_store->similarity_search("programming language", 3);
    std::cout << "Found " << basic_results.size() << " documents:\n";
    for (const auto& doc : basic_results) {
        std::cout << "- " << doc.content << " (Category: " << doc.metadata.at("category") << ")\n";
    }
    std::cout << "\n";

    // Example 2: Advanced retriever with filtering
    std::cout << "Example 2: Advanced retriever with filtering\n";
    std::cout << "-------------------------------------------\n";
    AdvancedRetriever advanced_retriever(vector_store);

    // Set similarity algorithm to Jaccard
    advanced_retriever.set_similarity_algorithm(SimilarityAlgorithm::JACCARD);

    // Search with category filter
    std::map<String, String> filters = {{"category", "programming"}};
    auto filtered_results = advanced_retriever.search("language", 3, filters);
    std::cout << "Found " << filtered_results.size() << " programming documents:\n";
    for (const auto& doc : filtered_results) {
        std::cout << "- " << doc.content << " (Category: " << doc.metadata.at("category") << ")\n";
    }
    std::cout << "\n";

    // Example 3: Search with scores
    std::cout << "Example 3: Search with similarity scores\n";
    std::cout << "----------------------------------------\n";
    auto scored_results = advanced_retriever.search_with_scores("artificial intelligence", 3);
    std::cout << "Found " << scored_results.size() << " documents with scores:\n";
    for (const auto& pair : scored_results) {
        std::cout << "- " << pair.first.content << " (Score: " << pair.second << ")\n";
    }
    std::cout << "\n";

    // Example 4: Hybrid search
    std::cout << "Example 4: Hybrid search\n";
    std::cout << "------------------------\n";
    auto hybrid_results = advanced_retriever.hybrid_search("machine learning", 3);
    std::cout << "Found " << hybrid_results.size() << " documents:\n";
    for (const auto& doc : hybrid_results) {
        std::cout << "- " << doc.content << " (Category: " << doc.metadata.at("category") << ")\n";
    }
    std::cout << "\n";

    // Example 5: Custom similarity function
    std::cout << "Example 5: Custom similarity function\n";
    std::cout << "-------------------------------------\n";
    advanced_retriever.set_custom_similarity_function([](const String& str1, const String& str2) {
        // Simple exact match count
        int matches = 0;
        if (str1.find(str2) != String::npos || str2.find(str1) != String::npos) {
            matches = 1;
        }
        return static_cast<double>(matches);
    });

    auto custom_results = advanced_retriever.search("python", 3);
    std::cout << "Found " << custom_results.size() << " documents with custom similarity:\n";
    for (const auto& doc : custom_results) {
        std::cout << "- " << doc.content << " (Category: " << doc.metadata.at("category") << ")\n";
    }
    std::cout << "\n";

    // Example 6: Contextual compression (if LLM is available)
    std::cout << "Example 6: Contextual compression\n";
    std::cout << "---------------------------------\n";
    auto llm = std::make_shared<SimpleLLM>();
    ContextualCompressionRetriever compression_retriever(vector_store, llm);

    auto compressed_results = compression_retriever.retrieve("What is Python?", 3);
    std::cout << "Found " << compressed_results.size() << " compressed documents:\n";
    for (const auto& doc : compressed_results) {
        std::cout << "- " << doc.content << "\n";
        if (doc.metadata.find("compressed") != doc.metadata.end()) {
            std::cout << "  (Compressed: " << doc.metadata.at("compressed") << ")\n";
        }
    }
    std::cout << "\n";

    std::cout << "Advanced retrieval example completed!\n";
    return 0;
}