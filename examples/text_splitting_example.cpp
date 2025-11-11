#include <iostream>
#include <memory>
#include <fstream>
#include "../include/langchain/langchain.h"

using namespace langchain;

int main() {
    std::cout << "LangChain C++ Text Splitting Example\n";
    std::cout << "====================================\n\n";

    // Create a large document for testing text splitting
    std::cout << "Creating a large test document...\n";

    std::ofstream large_file("large_document.txt");
    for (int i = 0; i < 100; ++i) {
        large_file << "This is paragraph " << i << " of the large document. "
                   << "It contains some sample text to demonstrate text splitting functionality. "
                   << "The document is designed to be large enough to require splitting into multiple chunks. "
                   << "Each paragraph is similar but contains a unique identifier to distinguish it. "
                   << "This helps us test the text splitting functionality of the LangChain C++ framework.\n\n";
    }
    large_file.close();

    std::cout << "Large document created successfully!\n\n";

    // Load the document
    std::cout << "Loading document...\n";
    auto document = DocumentLoader::load_document("large_document.txt");

    std::cout << "Original document size: " << document.content.length() << " characters\n\n";

    // Test text splitting with different chunk sizes
    std::cout << "Testing text splitting with different configurations:\n\n";

    // Test 1: Default text splitter
    std::cout << "Test 1: Default text splitter (chunk_size=1000, chunk_overlap=200)\n";
    auto default_splitter = std::make_shared<TextSplitter>();
    auto default_chunks = default_splitter->split_document(document);
    std::cout << "Number of chunks: " << default_chunks.size() << std::endl;
    if (!default_chunks.empty()) {
        std::cout << "First chunk size: " << default_chunks[0].content.length() << " characters" << std::endl;
        std::cout << "First chunk preview: " << default_chunks[0].content.substr(0, 100) << "..." << std::endl;
    }
    std::cout << std::endl;

    // Test 2: Custom text splitter with smaller chunks
    std::cout << "Test 2: Custom text splitter (chunk_size=500, chunk_overlap=100)\n";
    auto custom_splitter = std::make_shared<TextSplitter>(500, 100);
    auto custom_chunks = custom_splitter->split_document(document);
    std::cout << "Number of chunks: " << custom_chunks.size() << std::endl;
    if (!custom_chunks.empty()) {
        std::cout << "First chunk size: " << custom_chunks[0].content.length() << " characters" << std::endl;
        std::cout << "First chunk preview: " << custom_chunks[0].content.substr(0, 100) << "..." << std::endl;
    }
    std::cout << std::endl;

    // Test 3: Custom text splitter with larger chunks
    std::cout << "Test 3: Custom text splitter (chunk_size=2000, chunk_overlap=300)\n";
    auto large_splitter = std::make_shared<TextSplitter>(2000, 300);
    auto large_chunks = large_splitter->split_document(document);
    std::cout << "Number of chunks: " << large_chunks.size() << std::endl;
    if (!large_chunks.empty()) {
        std::cout << "First chunk size: " << large_chunks[0].content.length() << " characters" << std::endl;
        std::cout << "First chunk preview: " << large_chunks[0].content.substr(0, 100) << "..." << std::endl;
    }
    std::cout << std::endl;

    // Test metadata preservation
    std::cout << "Testing metadata preservation:\n";
    if (!default_chunks.empty()) {
        std::cout << "First chunk metadata:" << std::endl;
        std::cout << "  ID: " << default_chunks[0].id << std::endl;
        std::cout << "  Source: " << default_chunks[0].metadata.at("source") << std::endl;
        std::cout << "  Chunk index: " << default_chunks[0].metadata.at("chunk_index") << std::endl;
        std::cout << "  Total chunks: " << default_chunks[0].metadata.at("total_chunks") << std::endl;
    }
    std::cout << std::endl;

    // Test splitting multiple documents
    std::cout << "Testing splitting multiple documents:\n";

    // Create another document
    std::ofstream another_file("another_document.txt");
    another_file << "This is another document for testing multiple document splitting. "
                 << "It's a shorter document but still useful for testing the functionality. "
                 << "We can see how the text splitter handles documents of different sizes.";
    another_file.close();

    auto doc1 = DocumentLoader::load_document("large_document.txt");
    auto doc2 = DocumentLoader::load_document("another_document.txt");

    std::vector<Document> documents = {doc1, doc2};
    auto multi_chunks = default_splitter->split_documents(documents);

    std::cout << "Total documents before splitting: " << documents.size() << std::endl;
    std::cout << "Total chunks after splitting: " << multi_chunks.size() << std::endl;

    // Clean up test files
    std::cout << "\nCleaning up test files...\n";
    std::remove("large_document.txt");
    std::remove("another_document.txt");

    std::cout << "Text splitting example completed successfully!\n";

    return 0;
}