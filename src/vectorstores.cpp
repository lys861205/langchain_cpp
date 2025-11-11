#include "../include/langchain/vectorstores.h"
#include <cmath>
#include <algorithm>
#include <random>
#include <chrono>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>

namespace langchain {

// DocumentLoader implementation
String DocumentLoader::get_file_extension(const String& file_path) {
    size_t pos = file_path.find_last_of(".");
    if (pos != String::npos) {
        return file_path.substr(pos);
    }
    return "";
}

Document DocumentLoader::load_document(const String& file_path) {
    String extension = get_file_extension(file_path);

    if (extension == ".txt") {
        return load_text_file(file_path);
    } else if (extension == ".md") {
        return load_markdown_file(file_path);
    } else {
        // Default to text file loading for unknown extensions
        return load_text_file(file_path);
    }
}

Document DocumentLoader::load_text_file(const String& file_path) {
    Document doc;
    doc.id = file_path;
    doc.metadata["source"] = file_path;
    doc.metadata["type"] = "text";

    std::ifstream file(file_path);
    if (!file.is_open()) {
        doc.content = "Error: Could not open file " + file_path;
        return doc;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    doc.content = buffer.str();

    file.close();
    return doc;
}

Document DocumentLoader::load_markdown_file(const String& file_path) {
    Document doc;
    doc.id = file_path;
    doc.metadata["source"] = file_path;
    doc.metadata["type"] = "markdown";

    std::ifstream file(file_path);
    if (!file.is_open()) {
        doc.content = "Error: Could not open file " + file_path;
        return doc;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    doc.content = buffer.str();

    file.close();
    return doc;
}

std::vector<Document> DocumentLoader::load_documents_from_directory(const String& directory_path) {
    std::vector<Document> documents;

    try {
        for (const auto& entry : std::filesystem::directory_iterator(directory_path)) {
            if (entry.is_regular_file()) {
                String file_path = entry.path().string();
                String extension = get_file_extension(file_path);

                if (extension == ".txt" || extension == ".md") {
                    Document doc = load_document(file_path);
                    documents.push_back(doc);
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& ex) {
        std::cerr << "Error reading directory: " << ex.what() << std::endl;
    }

    return documents;
}

// TextSplitter implementation
TextSplitter::TextSplitter(size_t chunk_size, size_t chunk_overlap)
    : chunk_size_(chunk_size), chunk_overlap_(chunk_overlap) {}

std::vector<String> TextSplitter::split_text(const String& text) const {
    std::vector<String> chunks;

    if (text.empty()) {
        return chunks;
    }

    // Find sentence boundaries
    std::vector<size_t> sentence_boundaries = find_sentence_boundaries(text);

    size_t start = 0;
    size_t text_length = text.length();

    while (start < text_length) {
        // Find the best end position that respects sentence boundaries
        size_t end = std::min(start + chunk_size_, text_length);

        // Try to adjust end position to sentence boundary
        size_t adjusted_end = find_best_sentence_boundary(sentence_boundaries, start, end);
        if (adjusted_end != String::npos && adjusted_end > start) {
            end = adjusted_end;
        }

        String chunk = text.substr(start, end - start);
        chunks.push_back(chunk);

        if (end == text_length) {
            break;
        }

        // Find overlap start position
        size_t overlap_start = end - std::min(chunk_overlap_, chunk_size_);

        // Try to adjust overlap start to sentence boundary
        size_t adjusted_overlap_start = find_best_sentence_boundary(sentence_boundaries, overlap_start, end);
        if (adjusted_overlap_start != String::npos && adjusted_overlap_start > start) {
            start = adjusted_overlap_start;
        } else {
            start = overlap_start;
        }
    }

    return chunks;
}

std::vector<size_t> TextSplitter::find_sentence_boundaries(const String& text) const {
    std::vector<size_t> boundaries;

    for (size_t i = 0; i < text.length(); ++i) {
        unsigned char c = static_cast<unsigned char>(text[i]);

        // Check for sentence ending punctuation
        if (c == '.' || c == '!' || c == '?' ||  // English sentence endings
            c == ';' || c == 0xe3 || c == 0xef) {  // Potential UTF-8 start bytes
            // For UTF-8 multi-byte characters, we need to check the full sequence
            if (i + 2 < text.length()) {
                // Check for Chinese period '。' (UTF-8: 0xe3 0x80 0x82)
                if (static_cast<unsigned char>(text[i]) == 0xe3 &&
                    static_cast<unsigned char>(text[i+1]) == 0x80 &&
                    static_cast<unsigned char>(text[i+2]) == 0x82) {
                    boundaries.push_back(i + 3);  // Position after the punctuation
                    i += 2;  // Skip the next two bytes
                    continue;
                }
                // Check for Chinese exclamation mark '！' (UTF-8: 0xef 0xbc 0x81)
                else if (static_cast<unsigned char>(text[i]) == 0xef &&
                         static_cast<unsigned char>(text[i+1]) == 0xbc &&
                         static_cast<unsigned char>(text[i+2]) == 0x81) {
                    boundaries.push_back(i + 3);  // Position after the punctuation
                    i += 2;  // Skip the next two bytes
                    continue;
                }
                // Check for Chinese question mark '？' (UTF-8: 0xef 0xbc 0x9f)
                else if (static_cast<unsigned char>(text[i]) == 0xef &&
                         static_cast<unsigned char>(text[i+1]) == 0xbc &&
                         static_cast<unsigned char>(text[i+2]) == 0x9f) {
                    boundaries.push_back(i + 3);  // Position after the punctuation
                    i += 2;  // Skip the next two bytes
                    continue;
                }
                // Check for Chinese semicolon '；' (UTF-8: 0xef 0xbc 0x9b)
                else if (static_cast<unsigned char>(text[i]) == 0xef &&
                         static_cast<unsigned char>(text[i+1]) == 0xbc &&
                         static_cast<unsigned char>(text[i+2]) == 0x9b) {
                    boundaries.push_back(i + 3);  // Position after the punctuation
                    i += 2;  // Skip the next two bytes
                    continue;
                }
            }

            // Check for single-byte punctuation
            if (c == '.' || c == '!' || c == '?' || c == ';') {
                boundaries.push_back(i + 1);  // Position after the punctuation
            }
        }
    }

    // Always add the end of text as a boundary
    boundaries.push_back(text.length());

    return boundaries;
}

size_t TextSplitter::find_best_sentence_boundary(const std::vector<size_t>& boundaries, size_t start, size_t target_end) const {
    size_t best_boundary = String::npos;
    size_t min_distance = String::npos;

    for (size_t boundary : boundaries) {
        // Only consider boundaries after start and before or at target_end
        if (boundary > start && boundary <= target_end) {
            size_t distance = target_end - boundary;
            if (distance < min_distance) {
                min_distance = distance;
                best_boundary = boundary;
            }
        }
    }

    return best_boundary;
}

std::vector<Document> TextSplitter::split_document(const Document& document) const {
    std::vector<Document> documents;
    std::vector<String> chunks = split_text(document.content);

    for (size_t i = 0; i < chunks.size(); ++i) {
        Document doc;
        doc.content = chunks[i];
        doc.id = document.id + "_chunk_" + std::to_string(i);

        // Copy metadata from original document
        doc.metadata = document.metadata;
        doc.metadata["chunk_index"] = std::to_string(i);
        doc.metadata["total_chunks"] = std::to_string(chunks.size());

        documents.push_back(doc);
    }

    return documents;
}

std::vector<Document> TextSplitter::split_documents(const std::vector<Document>& documents) const {
    std::vector<Document> split_documents;

    for (const auto& doc : documents) {
        std::vector<Document> chunks = split_document(doc);
        split_documents.insert(split_documents.end(), chunks.begin(), chunks.end());
    }

    return split_documents;
}

// InMemoryVectorStore implementation
InMemoryVectorStore::InMemoryVectorStore()
    : rng_(std::chrono::steady_clock::now().time_since_epoch().count()) {}

StringList InMemoryVectorStore::add_documents(const std::vector<Document>& documents) {
    StringList new_ids;
    for (const auto& doc : documents) {
        String id = doc.id.empty() ? generate_id() : doc.id;
        Document doc_with_id = doc;
        doc_with_id.id = id;

        documents_.push_back(doc_with_id);
        ids_.push_back(id);
        new_ids.push_back(id);
    }
    return new_ids;
}

std::vector<Document> InMemoryVectorStore::similarity_search(const String& query, int k) {
    auto results_with_scores = similarity_search_with_score(query, k);
    std::vector<Document> results;
    for (const auto& pair : results_with_scores) {
        results.push_back(pair.first);
    }
    return results;
}

std::vector<std::pair<Document, double>> InMemoryVectorStore::similarity_search_with_score(
    const String& query, int k) {

    std::vector<std::pair<Document, double>> similarities;

    // Calculate similarity scores for all documents
    for (const auto& doc : documents_) {
        double score = calculate_similarity(query, doc.content);
        similarities.push_back({doc, score});
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

void InMemoryVectorStore::delete_documents(const StringList& ids) {
    for (const auto& id : ids) {
        auto it = std::find(ids_.begin(), ids_.end(), id);
        if (it != ids_.end()) {
            size_t index = std::distance(ids_.begin(), it);
            ids_.erase(it);
            documents_.erase(documents_.begin() + index);
        }
    }
}

std::vector<Document> InMemoryVectorStore::get_by_ids(const StringList& ids) {
    std::vector<Document> result;
    for (const auto& id : ids) {
        auto it = std::find(ids_.begin(), ids_.end(), id);
        if (it != ids_.end()) {
            size_t index = std::distance(ids_.begin(), it);
            result.push_back(documents_[index]);
        }
    }
    return result;
}

std::vector<Document> InMemoryVectorStore::get_all_documents() const {
    return documents_;
}

// Private methods
String InMemoryVectorStore::generate_id() {
    static const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    String result;
    result.reserve(16);

    for (int i = 0; i < 16; ++i) {
        result += charset[rng_() % (sizeof(charset) - 1)];
    }

    return result;
}

double InMemoryVectorStore::calculate_similarity(const String& str1, const String& str2) {
    // Simple approach: count common words
    auto words1 = split_to_words(str1);
    auto words2 = split_to_words(str2);

    int common_words = 0;
    for (const auto& word1 : words1) {
        for (const auto& word2 : words2) {
            if (word1 == word2) {
                common_words++;
                break;
            }
        }
    }

    // Normalize by the maximum length
    int max_words = std::max(words1.size(), words2.size());
    if (max_words == 0) return 0.0;

    return static_cast<double>(common_words) / max_words;
}

StringList InMemoryVectorStore::split_to_words(const String& str) {
    StringList words;
    String word;

    for (char c : str) {
        if (std::isspace(c)) {
            if (!word.empty()) {
                // Convert to lowercase for comparison
                std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                words.push_back(word);
                word.clear();
            }
        } else {
            word += std::tolower(c);
        }
    }

    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

// RAGChain implementation
RAGChain::RAGChain(std::shared_ptr<VectorStore> vector_store, std::shared_ptr<LLM> llm)
    : vector_store_(vector_store), llm_(llm) {
    text_splitter_ = std::make_shared<TextSplitter>();
}

void RAGChain::add_documents(const std::vector<Document>& documents) {
    // Split documents if needed
    std::vector<Document> split_docs = text_splitter_->split_documents(documents);

    // Add documents to vector store
    vector_store_->add_documents(split_docs);
}

String RAGChain::query(const String& question) {
    // Search for relevant documents
    auto relevant_docs = vector_store_->similarity_search(question, 4);

    // Create context from relevant documents
    String context;
    for (const auto& doc : relevant_docs) {
        context += doc.content + "\n\n";
    }

    // Create prompt with context and question
    String prompt = "Use the following context to answer the question at the end. "
                    "If you don't know the answer, just say that you don't know, "
                    "don't try to make up an answer.\n\n"
                    "Context:\n" + context +
                    "Question: " + question + "\n"
                    "Answer:";

    // Generate response using LLM
    return llm_->generate(prompt);
}

void RAGChain::set_text_splitter(std::shared_ptr<TextSplitter> text_splitter) {
    text_splitter_ = text_splitter;
}

} // namespace langchain