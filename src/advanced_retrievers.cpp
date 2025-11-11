#include "../include/langchain/advanced_retrievers.h"
#include <algorithm>
#include <cmath>
#include <set>
#include <sstream>

namespace langchain {

// AdvancedRetriever implementation
AdvancedRetriever::AdvancedRetriever(std::shared_ptr<VectorStore> vector_store, SimilarityAlgorithm algorithm)
    : vector_store_(vector_store), algorithm_(algorithm) {}

void AdvancedRetriever::set_custom_similarity_function(std::function<double(const String&, const String&)> fn) {
    custom_similarity_fn_ = fn;
}

std::vector<Document> AdvancedRetriever::search(const String& query, int k,
                                               const std::map<String, String>& filters,
                                               double threshold) {
    auto results_with_scores = search_with_scores(query, k, filters, threshold);
    std::vector<Document> results;
    for (const auto& pair : results_with_scores) {
        results.push_back(pair.first);
    }
    return results;
}

std::vector<std::pair<Document, double>> AdvancedRetriever::search_with_scores(const String& query, int k,
                                                                              const std::map<String, String>& filters,
                                                                              double threshold) {
    // Retrieve documents from vector store using similarity search
    // We retrieve more documents than needed to account for filtering
    auto all_documents = vector_store_->similarity_search(query, k * 10);

    // Calculate similarity scores for all documents
    std::vector<std::pair<Document, double>> similarities;

    // Filter documents based on metadata
    auto filtered_documents = filter_documents(all_documents, filters);

    // Calculate similarity scores for filtered documents
    for (const auto& doc : filtered_documents) {
        double score = 0.0;

        // Use custom similarity function if provided
        if (custom_similarity_fn_) {
            score = custom_similarity_fn_(query, doc.content);
        } else {
            score = calculate_similarity(query, doc.content);
        }

        // Only include documents above threshold
        if (score >= threshold) {
            similarities.push_back({doc, score});
        }
    }

    // Sort by similarity score (descending)
    std::sort(similarities.begin(), similarities.end(),
              [](const auto& a, const auto& b) {
                  return a.second > b.second;
              });

    // Return top k results
    if (static_cast<size_t>(k) < similarities.size()) {
        similarities.resize(k);
    }

    return similarities;
}

std::vector<Document> AdvancedRetriever::hybrid_search(const String& query, int k,
                                                      const std::map<String, String>& filters,
                                                      double keyword_weight,
                                                      double semantic_weight) {
    // This is a simplified hybrid search implementation
    // In a real implementation, this would combine keyword search and semantic search

    // For now, we'll just use the semantic search with adjusted weights
    return search(query, k, filters);
}

void AdvancedRetriever::set_similarity_algorithm(SimilarityAlgorithm algorithm) {
    algorithm_ = algorithm;
}

std::vector<Document> AdvancedRetriever::filter_documents(const std::vector<Document>& documents,
                                                         const std::map<String, String>& filters) {
    if (filters.empty()) {
        return documents;
    }

    std::vector<Document> filtered;
    for (const auto& doc : documents) {
        bool match = true;
        for (const auto& filter : filters) {
            auto it = doc.metadata.find(filter.first);
            if (it == doc.metadata.end() || it->second != filter.second) {
                match = false;
                break;
            }
        }
        if (match) {
            filtered.push_back(doc);
        }
    }
    return filtered;
}

double AdvancedRetriever::calculate_similarity(const String& str1, const String& str2) {
    switch (algorithm_) {
        case SimilarityAlgorithm::COSINE:
            return cosine_similarity(str1, str2);
        case SimilarityAlgorithm::JACCARD:
            return jaccard_similarity(str1, str2);
        case SimilarityAlgorithm::EUCLIDEAN:
            return euclidean_similarity(str1, str2);
        case SimilarityAlgorithm::BM25:
            return bm25_similarity(str1, str2);
        default:
            return cosine_similarity(str1, str2);
    }
}

double AdvancedRetriever::cosine_similarity(const String& str1, const String& str2) {
    auto words1 = split_to_words_with_frequency(str1);
    auto words2 = split_to_words_with_frequency(str2);

    // Get all unique words
    std::set<String> all_words;
    for (const auto& pair : words1) {
        all_words.insert(pair.first);
    }
    for (const auto& pair : words2) {
        all_words.insert(pair.first);
    }

    // Calculate dot product and magnitudes
    double dot_product = 0.0;
    double magnitude1 = 0.0;
    double magnitude2 = 0.0;

    for (const auto& word : all_words) {
        int freq1 = words1[word];
        int freq2 = words2[word];

        dot_product += freq1 * freq2;
        magnitude1 += freq1 * freq1;
        magnitude2 += freq2 * freq2;
    }

    if (magnitude1 == 0.0 || magnitude2 == 0.0) {
        return 0.0;
    }

    return dot_product / (std::sqrt(magnitude1) * std::sqrt(magnitude2));
}

double AdvancedRetriever::jaccard_similarity(const String& str1, const String& str2) {
    auto words1 = split_to_words_with_frequency(str1);
    auto words2 = split_to_words_with_frequency(str2);

    std::set<String> set1, set2;
    for (const auto& pair : words1) {
        set1.insert(pair.first);
    }
    for (const auto& pair : words2) {
        set2.insert(pair.first);
    }

    // Calculate intersection and union
    std::set<String> intersection;
    std::set<String> Union;  // Union is a reserved keyword, so we use Union

    std::set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(),
                         std::inserter(intersection, intersection.begin()));
    std::set_union(set1.begin(), set1.end(), set2.begin(), set2.end(),
                  std::inserter(Union, Union.begin()));

    if (Union.empty()) {
        return 0.0;
    }

    return static_cast<double>(intersection.size()) / Union.size();
}

double AdvancedRetriever::euclidean_similarity(const String& str1, const String& str2) {
    auto words1 = split_to_words_with_frequency(str1);
    auto words2 = split_to_words_with_frequency(str2);

    // Get all unique words
    std::set<String> all_words;
    for (const auto& pair : words1) {
        all_words.insert(pair.first);
    }
    for (const auto& pair : words2) {
        all_words.insert(pair.first);
    }

    // Calculate Euclidean distance
    double distance_squared = 0.0;
    for (const auto& word : all_words) {
        int freq1 = words1[word];
        int freq2 = words2[word];
        int diff = freq1 - freq2;
        distance_squared += diff * diff;
    }

    double distance = std::sqrt(distance_squared);

    // Convert distance to similarity (higher distance = lower similarity)
    // Add 1 to avoid division by zero
    return 1.0 / (1.0 + distance);
}

double AdvancedRetriever::bm25_similarity(const String& query, const String& document) {
    // Simplified BM25 implementation
    // In a real implementation, this would be more complex and consider document frequency

    auto query_words = split_to_words_with_frequency(query);
    auto doc_words = split_to_words_with_frequency(document);

    double score = 0.0;
    double k1 = 1.5;  // BM25 parameter
    double b = 0.75;  // BM25 parameter

    // Average document length (simplified)
    double avg_doc_length = 100.0;  // This would be calculated from all documents
    double doc_length = doc_words.size();

    for (const auto& query_pair : query_words) {
        const String& word = query_pair.first;
        int query_freq = query_pair.second;

        auto doc_it = doc_words.find(word);
        if (doc_it != doc_words.end()) {
            int doc_freq = doc_it->second;

            // Simplified BM25 formula
            double tf = static_cast<double>(doc_freq);
            double idf = std::log(1.0 + (1.0 / (1.0 + tf)));  // Simplified IDF
            double numerator = tf * (k1 + 1);
            double denominator = tf + k1 * (1 - b + b * (doc_length / avg_doc_length));
            double tf_idf = idf * (numerator / denominator);

            score += tf_idf * query_freq;
        }
    }

    return score;
}

std::map<String, int> AdvancedRetriever::split_to_words_with_frequency(const String& str) {
    std::map<String, int> word_freq;
    String word;

    for (char c : str) {
        if (std::isspace(c)) {
            if (!word.empty()) {
                // Convert to lowercase for comparison
                std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                word_freq[word]++;
                word.clear();
            }
        } else {
            word += std::tolower(c);
        }
    }

    if (!word.empty()) {
        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
        word_freq[word]++;
    }

    return word_freq;
}

std::set<String> AdvancedRetriever::get_unique_words(const std::vector<String>& strings) {
    std::set<String> unique_words;
    for (const auto& str : strings) {
        auto words = split_to_words_with_frequency(str);
        for (const auto& pair : words) {
            unique_words.insert(pair.first);
        }
    }
    return unique_words;
}

// MultiQueryRetriever implementation
MultiQueryRetriever::MultiQueryRetriever(std::shared_ptr<VectorStore> vector_store,
                                       std::shared_ptr<LLM> llm,
                                       int num_queries)
    : vector_store_(vector_store), llm_(llm), num_queries_(num_queries) {}

std::vector<Document> MultiQueryRetriever::retrieve(const String& query, int k) {
    auto queries = generate_queries(query);

    // Add original query to the list
    queries.insert(queries.begin(), query);

    std::map<String, int> doc_count;
    std::map<String, Document> doc_map;

    // Retrieve documents for each query
    for (const auto& q : queries) {
        auto docs = vector_store_->similarity_search(q, k);
        for (const auto& doc : docs) {
            doc_count[doc.id]++;
            doc_map[doc.id] = doc;
        }
    }

    // Sort documents by count (number of queries that retrieved them)
    std::vector<std::pair<String, int>> sorted_docs(doc_count.begin(), doc_count.end());
    std::sort(sorted_docs.begin(), sorted_docs.end(),
              [](const auto& a, const auto& b) {
                  return a.second > b.second;
              });

    // Return top k documents
    std::vector<Document> results;
    int count = 0;
    for (const auto& pair : sorted_docs) {
        if (count >= k) break;
        results.push_back(doc_map[pair.first]);
        count++;
    }

    return results;
}

std::vector<String> MultiQueryRetriever::generate_queries(const String& query) {
    if (!llm_) {
        // Return empty list if no LLM is available
        return {};
    }

    String prompt = "Generate " + std::to_string(num_queries_) +
                   " different ways to ask the following question:\n" + query +
                   "\n\nProvide each question on a separate line without any numbering or bullet points.";

    String response = llm_->generate(prompt);

    // Split response into lines
    std::vector<String> queries;
    std::istringstream iss(response);
    String line;
    while (std::getline(iss, line)) {
        if (!line.empty()) {
            queries.push_back(line);
        }
    }

    return queries;
}

// ContextualCompressionRetriever implementation
ContextualCompressionRetriever::ContextualCompressionRetriever(std::shared_ptr<VectorStore> vector_store,
                                                              std::shared_ptr<LLM> llm)
    : vector_store_(vector_store), llm_(llm) {}

std::vector<Document> ContextualCompressionRetriever::retrieve(const String& query, int k) {
    // First, retrieve relevant documents
    auto docs = vector_store_->similarity_search(query, k * 2);  // Retrieve more documents to compress

    std::vector<Document> compressed_docs;
    for (const auto& doc : docs) {
        auto compressed = compress_document(doc, query);
        if (!compressed.content.empty()) {
            compressed_docs.push_back(compressed);
            if (static_cast<int>(compressed_docs.size()) >= k) {
                break;
            }
        }
    }

    return compressed_docs;
}

Document ContextualCompressionRetriever::compress_document(const Document& document, const String& query) {
    if (!llm_) {
        // Return original document if no LLM is available
        return document;
    }

    String prompt = "Given the following document and query, extract only the information "
                   "that is relevant to answering the query. If no relevant information is found, "
                   "return 'NO_RELEVANT_INFO'.\n\n"
                   "Document:\n" + document.content + "\n\n"
                   "Query:\n" + query + "\n\n"
                   "Relevant information:";

    String response = llm_->generate(prompt);

    // If no relevant information is found, return empty document
    if (response.find("NO_RELEVANT_INFO") != String::npos) {
        return Document();
    }

    // Create compressed document
    Document compressed_doc;
    compressed_doc.id = document.id;
    compressed_doc.content = response;
    compressed_doc.metadata = document.metadata;
    compressed_doc.metadata["compressed"] = "true";

    return compressed_doc;
}

} // namespace langchain