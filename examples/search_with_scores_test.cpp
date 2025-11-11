#include "../include/langchain/core.h"
#include "../include/langchain/vectorstores.h"
#include "../include/langchain/advanced_retrievers.h"
#include <iostream>
#include <memory>
#include <vector>

using namespace langchain;

int main() {
    std::cout << "AdvancedRetriever search_with_scores Test" << std::endl;
    std::cout << "=========================================" << std::endl;

    // Create documents
    std::vector<Document> documents = {
        Document("Python is a popular programming language", {{"category", "programming"}, {"language", "python"}}),
        Document("C++ is a powerful programming language", {{"category", "programming"}, {"language", "cpp"}}),
        Document("The quick brown fox jumps over the lazy dog", {{"category", "animals"}, {"language", "english"}}),
        Document("Machine learning is a subset of artificial intelligence", {{"category", "AI"}, {"language", "english"}}),
        Document("Deep learning uses neural networks with multiple layers", {{"category", "AI"}, {"language", "english"}}),
        Document("Redis is an in-memory data structure store", {{"category", "database"}, {"language", "english"}})
    };

    // Create vector store and add documents
    auto vector_store = std::make_shared<InMemoryVectorStore>();
    vector_store->add_documents(documents);
    std::cout << "Added " << documents.size() << " documents to vector store" << std::endl;

    // Create advanced retriever
    auto retriever = std::make_shared<AdvancedRetriever>(vector_store, SimilarityAlgorithm::COSINE);
    std::cout << "Created AdvancedRetriever with COSINE similarity" << std::endl;

    // Test 1: Basic search with scores
    std::cout << "\nTest 1: Basic search with scores for 'programming language'" << std::endl;
    std::cout << "--------------------------------------------------------" << std::endl;
    auto results_with_scores = retriever->search_with_scores("programming language", 3);
    std::cout << "Found " << results_with_scores.size() << " documents with scores:" << std::endl;
    for (const auto& pair : results_with_scores) {
        std::cout << "- Document: " << pair.first.content << std::endl;
        std::cout << "  Score: " << pair.second << std::endl;
        std::cout << "  Metadata: ";
        for (const auto& meta : pair.first.metadata) {
            std::cout << meta.first << "=" << meta.second << " ";
        }
        std::cout << std::endl;
    }

    // Test 2: Search with filters
    std::cout << "\nTest 2: Search with filters (category=AI)" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::map<String, String> filters = {{"category", "AI"}};
    auto filtered_results = retriever->search_with_scores("learning", 3, filters);
    std::cout << "Found " << filtered_results.size() << " AI documents with scores:" << std::endl;
    for (const auto& pair : filtered_results) {
        std::cout << "- Document: " << pair.first.content << std::endl;
        std::cout << "  Score: " << pair.second << std::endl;
        std::cout << "  Metadata: ";
        for (const auto& meta : pair.first.metadata) {
            std::cout << meta.first << "=" << meta.second << " ";
        }
        std::cout << std::endl;
    }

    // Test 3: Search with threshold
    std::cout << "\nTest 3: Search with threshold (0.1)" << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    auto threshold_results = retriever->search_with_scores("database", 5, {}, 0.1);
    std::cout << "Found " << threshold_results.size() << " documents with scores above 0.1:" << std::endl;
    for (const auto& pair : threshold_results) {
        std::cout << "- Document: " << pair.first.content << std::endl;
        std::cout << "  Score: " << pair.second << std::endl;
    }

    // Test 4: Different similarity algorithms
    std::cout << "\nTest 4: Using JACCARD similarity algorithm" << std::endl;
    std::cout << "------------------------------------------" << std::endl;
    retriever->set_similarity_algorithm(SimilarityAlgorithm::JACCARD);
    auto jaccard_results = retriever->search_with_scores("artificial intelligence", 2);
    std::cout << "Found " << jaccard_results.size() << " documents with JACCARD similarity:" << std::endl;
    for (const auto& pair : jaccard_results) {
        std::cout << "- Document: " << pair.first.content << std::endl;
        std::cout << "  Score: " << pair.second << std::endl;
    }

    std::cout << "\nAll tests completed successfully!" << std::endl;

    return 0;
}