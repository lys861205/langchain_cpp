#ifndef LANGCHAIN_ADVANCED_RETRIEVERS_H
#define LANGCHAIN_ADVANCED_RETRIEVERS_H

#include "core.h"
#include "vectorstores.h"
#include <memory>
#include <functional>
#include <set>

namespace langchain {

// Enum for similarity algorithms
enum class SimilarityAlgorithm {
    COSINE,
    JACCARD,
    EUCLIDEAN,
    BM25
};

// Advanced retriever with filtering and multiple similarity algorithms
class AdvancedRetriever {
private:
    std::shared_ptr<VectorStore> vector_store_;
    SimilarityAlgorithm algorithm_;
    std::function<double(const String&, const String&)> custom_similarity_fn_;

public:
    AdvancedRetriever(std::shared_ptr<VectorStore> vector_store, SimilarityAlgorithm algorithm = SimilarityAlgorithm::COSINE);

    // Set custom similarity function
    void set_custom_similarity_function(std::function<double(const String&, const String&)> fn);

    // Search with filters
    std::vector<Document> search(const String& query, int k = 4,
                                const std::map<String, String>& filters = {},
                                double threshold = 0.0);

    // Search with scores
    std::vector<std::pair<Document, double>> search_with_scores(const String& query, int k = 4,
                                                               const std::map<String, String>& filters = {},
                                                               double threshold = 0.0);

    // Hybrid search combining multiple approaches
    std::vector<Document> hybrid_search(const String& query, int k = 4,
                                       const std::map<String, String>& filters = {},
                                       double keyword_weight = 0.5,
                                       double semantic_weight = 0.5);

    // Set similarity algorithm
    void set_similarity_algorithm(SimilarityAlgorithm algorithm);

private:
    // Filter documents based on metadata
    std::vector<Document> filter_documents(const std::vector<Document>& documents,
                                          const std::map<String, String>& filters);

    // Calculate similarity based on selected algorithm
    double calculate_similarity(const String& str1, const String& str2);

    // Cosine similarity implementation
    double cosine_similarity(const String& str1, const String& str2);

    // Jaccard similarity implementation
    double jaccard_similarity(const String& str1, const String& str2);

    // Euclidean distance implementation (converted to similarity)
    double euclidean_similarity(const String& str1, const String& str2);

    // BM25 implementation (simplified)
    double bm25_similarity(const String& query, const String& document);

    // Split string into words (with frequency)
    std::map<String, int> split_to_words_with_frequency(const String& str);

    // Get all unique words from a collection of strings
    std::set<String> get_unique_words(const std::vector<String>& strings);
};

// Multi-query retriever that generates multiple queries and combines results
class MultiQueryRetriever {
private:
    std::shared_ptr<VectorStore> vector_store_;
    std::shared_ptr<LLM> llm_;
    int num_queries_;

public:
    MultiQueryRetriever(std::shared_ptr<VectorStore> vector_store,
                       std::shared_ptr<LLM> llm,
                       int num_queries = 3);

    // Generate multiple queries and retrieve documents
    std::vector<Document> retrieve(const String& query, int k = 4);

    // Generate multiple queries
    std::vector<String> generate_queries(const String& query);
};

// Contextual compression retriever
class ContextualCompressionRetriever {
private:
    std::shared_ptr<VectorStore> vector_store_;
    std::shared_ptr<LLM> llm_;

public:
    ContextualCompressionRetriever(std::shared_ptr<VectorStore> vector_store,
                                  std::shared_ptr<LLM> llm);

    // Retrieve and compress documents based on context
    std::vector<Document> retrieve(const String& query, int k = 4);

    // Compress a document based on relevance to query
    Document compress_document(const Document& document, const String& query);
};

} // namespace langchain

#endif // LANGCHAIN_ADVANCED_RETRIEVERS_H