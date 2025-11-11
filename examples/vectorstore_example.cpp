#include <iostream>
#include <memory>
#include <vector>
#include "../include/langchain/langchain.h"

using namespace langchain;

int main() {
    std::cout << "LangChain C++ VectorStore Example\n";
    std::cout << "================================\n\n";

    // Create a vector store
    auto vectorstore = std::make_shared<InMemoryVectorStore>();

    // Create documents
    std::vector<Document> documents = {
        Document("The quick brown fox jumps over the lazy dog", {{"category", "animals"}}),
        Document("Machine learning is a subset of artificial intelligence", {{"category", "technology"}}),
        Document("Python is a popular programming language", {{"category", "programming"}}),
        Document("The weather is sunny today", {{"category", "weather"}}),
        Document("I love reading books about science", {{"category", "interests"}})
    };

    // Add documents to the vector store
    std::cout << "Adding documents to vector store...\n";
    StringList ids = vectorstore->add_documents(documents);

    for (size_t i = 0; i < ids.size(); ++i) {
        std::cout << "Added document with ID: " << ids[i] << std::endl;
    }
    std::cout << std::endl;

    // Perform similarity search
    std::cout << "Performing similarity search for 'artificial intelligence':\n";
    auto results = vectorstore->similarity_search("artificial intelligence", 3);

    for (const auto& doc : results) {
        std::cout << "Content: " << doc.content << std::endl;
        std::cout << "ID: " << doc.id << std::endl;
        std::cout << "Metadata: ";
        for (const auto& pair : doc.metadata) {
            std::cout << pair.first << "=" << pair.second << " ";
        }
        std::cout << std::endl << std::endl;
    }

    // Perform similarity search with scores
    std::cout << "Performing similarity search with scores for 'programming':\n";
    auto results_with_scores = vectorstore->similarity_search_with_score("programming", 2);

    for (const auto& pair : results_with_scores) {
        std::cout << "Content: " << pair.first.content << std::endl;
        std::cout << "Score: " << pair.second << std::endl;
        std::cout << "ID: " << pair.first.id << std::endl << std::endl;
    }

    // Test get by IDs
    std::cout << "Getting documents by IDs:\n";
    StringList query_ids = {ids[0], ids[2]};
    auto retrieved_docs = vectorstore->get_by_ids(query_ids);

    for (const auto& doc : retrieved_docs) {
        std::cout << "Retrieved: " << doc.content << std::endl;
    }
    std::cout << std::endl;

    // Test deletion
    std::cout << "Deleting document with ID: " << ids[0] << std::endl;
    vectorstore->delete_documents({ids[0]});

    std::cout << "Searching again after deletion:\n";
    auto results_after_delete = vectorstore->similarity_search("quick brown fox", 5);
    std::cout << "Found " << results_after_delete.size() << " documents\n";

    return 0;
}