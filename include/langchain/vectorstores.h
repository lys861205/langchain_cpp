#ifndef LANGCHAIN_VECTORSTORES_H
#define LANGCHAIN_VECTORSTORES_H

#include "core.h"
#include <cmath>
#include <algorithm>
#include <random>
#include <chrono>
#include <fstream>
#include <sstream>

namespace langchain {

// Document loader for loading documents from local files
class DocumentLoader {
public:
    // Load a single document from a file
    static Document load_document(const String& file_path);

    // Load multiple documents from a directory
    static std::vector<Document> load_documents_from_directory(const String& directory_path);

    // Load documents from various file types
    static Document load_text_file(const String& file_path);
    static Document load_markdown_file(const String& file_path);

private:
    // Extract file extension
    static String get_file_extension(const String& file_path);
};

// Text splitter for chunking large documents
class TextSplitter {
private:
    size_t chunk_size_;
    size_t chunk_overlap_;

public:
    TextSplitter(size_t chunk_size = 1000, size_t chunk_overlap = 200);

    // Split text into chunks
    std::vector<String> split_text(const String& text) const;

    // Split document into multiple documents
    std::vector<Document> split_document(const Document& document) const;

    // Split multiple documents
    std::vector<Document> split_documents(const std::vector<Document>& documents) const;

private:
    // Find sentence boundaries in text
    std::vector<size_t> find_sentence_boundaries(const String& text) const;

    // Find the best sentence boundary near a target position
    size_t find_best_sentence_boundary(const std::vector<size_t>& boundaries, size_t start, size_t target_end) const;
};

// Simple in-memory vector store implementation
class InMemoryVectorStore : public VectorStore {
private:
    std::vector<Document> documents_;
    std::vector<String> ids_;
    std::mt19937 rng_;

public:
    InMemoryVectorStore();

    // Add documents to the vector store
    StringList add_documents(const std::vector<Document>& documents) override;

    // Search for similar documents based on content similarity
    std::vector<Document> similarity_search(const String& query, int k = 4) override;

    // Search for similar documents with similarity scores
    std::vector<std::pair<Document, double>> similarity_search_with_score(
        const String& query, int k = 4) override;

    // Delete documents by IDs
    void delete_documents(const StringList& ids) override;

    // Get documents by IDs
    std::vector<Document> get_by_ids(const StringList& ids) override;

    // Get all documents
    std::vector<Document> get_all_documents() const;

private:
    // Generate a random ID
    String generate_id();

    // Calculate similarity between two strings (simple implementation)
    double calculate_similarity(const String& str1, const String& str2);

    // Split string into words
    StringList split_to_words(const String& str);
};

// RAG (Retrieval-Augmented Generation) chain
class RAGChain {
private:
    std::shared_ptr<VectorStore> vector_store_;
    std::shared_ptr<LLM> llm_;
    std::shared_ptr<TextSplitter> text_splitter_;

public:
    RAGChain(std::shared_ptr<VectorStore> vector_store, std::shared_ptr<LLM> llm);

    // Add documents to the RAG chain
    void add_documents(const std::vector<Document>& documents);

    // Query the RAG chain
    String query(const String& question);

    // Set text splitter
    void set_text_splitter(std::shared_ptr<TextSplitter> text_splitter);
};

} // namespace langchain

#endif // LANGCHAIN_VECTORSTORES_H