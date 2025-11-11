#include <iostream>
#include <memory>
#include <fstream>
#include "../include/langchain/langchain.h"

using namespace langchain;

int main() {
    std::cout << "LangChain C++ RAG (Retrieval-Augmented Generation) Example\n";
    std::cout << "==========================================================\n\n";

    // Create test files
    std::cout << "Creating test documents...\n";

    // Create a test document about artificial intelligence
    std::ofstream ai_file("ai_document.txt");
    ai_file << "Artificial Intelligence (AI) is a branch of computer science that aims to create software or machines that exhibit human-like intelligence. "
            << "This can include learning from experience, understanding natural language, solving problems, and recognizing patterns. "
            << "Machine learning is a subset of AI that focuses on algorithms that can learn and improve from data without being explicitly programmed. "
            << "Deep learning is a further subset of machine learning that uses neural networks with multiple layers to model complex patterns in data. "
            << "Natural Language Processing (NLP) is another important area of AI that deals with the interaction between computers and humans using natural language. "
            << "AI has many applications including image recognition, speech recognition, autonomous vehicles, and recommendation systems.";
    ai_file.close();

    // Create a test document about machine learning
    std::ofstream ml_file("ml_document.txt");
    ml_file << "Machine Learning is a field of artificial intelligence that uses statistical techniques to give computer systems the ability to 'learn' from data. "
            << "There are three main types of machine learning: supervised learning, unsupervised learning, and reinforcement learning. "
            << "Supervised learning uses labeled training data to teach algorithms to predict outcomes or classify data. "
            << "Unsupervised learning finds hidden patterns in unlabeled data. "
            << "Reinforcement learning uses a system of rewards and penalties to teach an agent to make decisions. "
            << "Common algorithms include linear regression, decision trees, random forests, support vector machines, and neural networks. "
            << "Feature engineering is an important step in machine learning that involves selecting and transforming raw data into features that can be used by algorithms.";
    ml_file.close();

    // Create a test document about data science
    std::ofstream ds_file("ds_document.txt");
    ds_file << "Data Science is an interdisciplinary field that uses scientific methods, processes, algorithms and systems to extract knowledge and insights from structured and unstructured data. "
            << "It combines expertise from statistics, mathematics, computer science, and domain knowledge. "
            << "The data science process typically involves data collection, data cleaning, exploratory data analysis, feature engineering, model building, and model evaluation. "
            << "Popular tools used in data science include Python, R, SQL, pandas, NumPy, scikit-learn, and TensorFlow. "
            << "Data visualization is an important aspect of data science that helps communicate findings to stakeholders. "
            << "Big data technologies like Hadoop and Spark are often used when dealing with large datasets. "
            << "Data scientists work in various industries including finance, healthcare, e-commerce, and technology.";
    ds_file.close();

    std::cout << "Test documents created successfully!\n\n";

    // Load documents
    std::cout << "Loading documents...\n";
    auto documents = DocumentLoader::load_documents_from_directory(".");

    std::cout << "Loaded " << documents.size() << " documents:\n";
    for (const auto& doc : documents) {
        std::cout << "- " << doc.metadata.at("source") << " (type: " << doc.metadata.at("type") << ")\n";
    }
    std::cout << std::endl;

    // Create vector store
    std::cout << "Creating vector store...\n";
    auto vector_store = std::make_shared<InMemoryVectorStore>();

    // Create LLM
    std::cout << "Creating LLM...\n";
    auto llm = std::make_shared<SimpleLLM>();

    // Create RAG chain
    std::cout << "Creating RAG chain...\n";
    auto rag_chain = std::make_shared<RAGChain>(vector_store, llm);

    // Add documents to RAG chain
    std::cout << "Adding documents to RAG chain...\n";
    rag_chain->add_documents(documents);

    // Test RAG functionality
    std::cout << "Testing RAG functionality:\n\n";

    // Test query 1
    std::string question1 = "What is artificial intelligence?";
    std::cout << "Question 1: " << question1 << std::endl;
    std::string answer1 = rag_chain->query(question1);
    std::cout << "Answer 1: " << answer1 << std::endl << std::endl;

    // Test query 2
    std::string question2 = "What are the types of machine learning?";
    std::cout << "Question 2: " << question2 << std::endl;
    std::string answer2 = rag_chain->query(question2);
    std::cout << "Answer 2: " << answer2 << std::endl << std::endl;

    // Test query 3
    std::string question3 = "What tools are used in data science?";
    std::cout << "Question 3: " << question3 << std::endl;
    std::string answer3 = rag_chain->query(question3);
    std::cout << "Answer 3: " << answer3 << std::endl << std::endl;

    // Test similarity search directly
    std::cout << "Testing similarity search directly:\n";
    auto similar_docs = vector_store->similarity_search("machine learning algorithms", 2);
    std::cout << "Found " << similar_docs.size() << " similar documents:\n";
    for (const auto& doc : similar_docs) {
        std::cout << "- Document from: " << doc.metadata.at("source") << std::endl;
        std::cout << "  Content: " << doc.content.substr(0, 100) << "..." << std::endl << std::endl;
    }

    // Clean up test files
    std::cout << "Cleaning up test files...\n";
    std::remove("ai_document.txt");
    std::remove("ml_document.txt");
    std::remove("ds_document.txt");

    std::cout << "RAG example completed successfully!\n";

    return 0;
}