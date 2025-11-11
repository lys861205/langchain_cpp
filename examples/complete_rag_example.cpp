#include <iostream>
#include <memory>
#include <fstream>
#include "../include/langchain/langchain.h"

using namespace langchain;

int main() {
    std::cout << "LangChain C++ Complete RAG Workflow Example\n";
    std::cout << "==========================================\n\n";

    // Step 1: Create sample documents
    std::cout << "Step 1: Creating sample documents...\n";

    // Create a document about climate change
    std::ofstream climate_file("climate_change.txt");
    climate_file << "Climate Change and Global Warming\n\n"
                 << "Climate change refers to long-term shifts in global or regional climate patterns. "
                 << "Since the mid-20th century, scientists have observed unprecedented changes in Earth's climate, "
                 << "primarily attributed to increased levels of greenhouse gases produced by human activities.\n\n"
                 << "Global warming is the gradual increase in Earth's average surface temperature, "
                 << "caused by the buildup of greenhouse gases in the atmosphere. "
                 << "The primary greenhouse gases include carbon dioxide, methane, and nitrous oxide.\n\n"
                 << "Effects of climate change include rising sea levels, more frequent extreme weather events, "
                 << "changes in precipitation patterns, and impacts on biodiversity. "
                 << "These changes pose significant challenges to ecosystems and human societies worldwide.\n\n"
                 << "Mitigation strategies focus on reducing greenhouse gas emissions through renewable energy, "
                 << "energy efficiency, and sustainable transportation. "
                 << "Adaptation strategies aim to reduce vulnerability to climate impacts through infrastructure improvements, "
                 << "agricultural changes, and ecosystem management.";
    climate_file.close();

    // Create a document about renewable energy
    std::ofstream energy_file("renewable_energy.txt");
    energy_file << "Renewable Energy Sources\n\n"
                << "Renewable energy comes from natural sources that are constantly replenished. "
                << "These sources include solar, wind, hydroelectric, geothermal, and biomass energy.\n\n"
                << "Solar energy harnesses the power of sunlight using photovoltaic cells or solar thermal collectors. "
                << "It is one of the fastest-growing renewable energy sources worldwide.\n\n"
                << "Wind energy converts the kinetic energy of wind into electricity using wind turbines. "
                << "Wind farms can be located onshore or offshore and are particularly effective in areas with consistent wind patterns.\n\n"
                << "Hydroelectric power generates electricity by using the energy of flowing or falling water. "
                << "It is one of the most established renewable energy technologies and provides a significant portion of global electricity.\n\n"
                << "Geothermal energy taps into the Earth's internal heat to generate electricity or provide direct heating. "
                << "It is most viable in regions with significant geothermal activity.\n\n"
                << "Biomass energy is produced from organic materials such as wood, agricultural crops, "
                << "or waste from plants or animals. It can be used for heating, electricity generation, or as biofuels.";
    energy_file.close();

    std::cout << "Sample documents created successfully!\n\n";

    // Step 2: Load documents
    std::cout << "Step 2: Loading documents...\n";
    auto documents = DocumentLoader::load_documents_from_directory(".");

    // Filter out non-relevant files
    std::vector<Document> relevant_docs;
    for (const auto& doc : documents) {
        if (doc.metadata.at("source") == "./climate_change.txt" ||
            doc.metadata.at("source") == "./renewable_energy.txt") {
            relevant_docs.push_back(doc);
        }
    }

    std::cout << "Loaded " << relevant_docs.size() << " relevant documents:\n";
    for (const auto& doc : relevant_docs) {
        std::cout << "- " << doc.metadata.at("source") << std::endl;
    }
    std::cout << std::endl;

    // Step 3: Split documents into chunks
    std::cout << "Step 3: Splitting documents into chunks...\n";
    auto text_splitter = std::make_shared<TextSplitter>(800, 100); // Smaller chunks for better granularity
    auto chunked_documents = text_splitter->split_documents(relevant_docs);

    std::cout << "Documents split into " << chunked_documents.size() << " chunks\n\n";

    // Step 4: Create vector store and add documents
    std::cout << "Step 4: Creating vector store and adding documents...\n";
    auto vector_store = std::make_shared<InMemoryVectorStore>();
    auto added_ids = vector_store->add_documents(chunked_documents);

    std::cout << "Added " << added_ids.size() << " document chunks to vector store\n\n";

    // Step 5: Create LLM and RAG chain
    std::cout << "Step 5: Creating LLM and RAG chain...\n";
    auto llm = std::make_shared<SimpleLLM>();
    auto rag_chain = std::make_shared<RAGChain>(vector_store, llm);

    std::cout << "RAG chain created successfully!\n\n";

    // Step 6: Test queries
    std::cout << "Step 6: Testing RAG queries...\n\n";

    // Test query 1
    std::string question1 = "What is climate change and what causes it?";
    std::cout << "Question 1: " << question1 << std::endl;
    std::string answer1 = rag_chain->query(question1);
    std::cout << "Answer 1: " << answer1 << std::endl << std::endl;

    // Test query 2
    std::string question2 = "What are the different types of renewable energy?";
    std::cout << "Question 2: " << question2 << std::endl;
    std::string answer2 = rag_chain->query(question2);
    std::cout << "Answer 2: " << answer2 << std::endl << std::endl;

    // Test query 3
    std::string question3 = "How does solar energy work?";
    std::cout << "Question 3: " << question3 << std::endl;
    std::string answer3 = rag_chain->query(question3);
    std::cout << "Answer 3: " << answer3 << std::endl << std::endl;

    // Step 7: Demonstrate similarity search
    std::cout << "Step 7: Demonstrating similarity search...\n";
    std::string search_query = "global warming effects";
    auto similar_docs = vector_store->similarity_search_with_score(search_query, 3);

    std::cout << "Top 3 similar documents for query '" << search_query << "':\n";
    for (size_t i = 0; i < similar_docs.size(); ++i) {
        const auto& doc_pair = similar_docs[i];
        std::cout << i + 1 << ". Document from: " << doc_pair.first.metadata.at("source") << std::endl;
        std::cout << "   Similarity score: " << doc_pair.second << std::endl;
        std::cout << "   Content preview: " << doc_pair.first.content.substr(0, 150) << "..." << std::endl << std::endl;
    }

    // Step 8: Clean up
    std::cout << "Step 8: Cleaning up test files...\n";
    std::remove("climate_change.txt");
    std::remove("renewable_energy.txt");

    std::cout << "Complete RAG workflow example finished successfully!\n";

    return 0;
}