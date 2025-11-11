#include <iostream>
#include <cassert>
#include <memory>
#include "../include/langchain/langchain.h"

using namespace langchain;

void test_document() {
    std::cout << "Testing Document class...\n";

    // Test default constructor
    Document doc1;
    assert(doc1.content.empty());
    assert(doc1.metadata.empty());
    assert(doc1.id.empty());

    // Test content constructor
    Document doc2("Hello world");
    assert(doc2.content == "Hello world");
    assert(doc2.metadata.empty());
    assert(doc2.id.empty());

    // Test content and metadata constructor
    StringMap metadata = {{"key", "value"}};
    Document doc3("Hello world", metadata);
    assert(doc3.content == "Hello world");
    assert(doc3.metadata.size() == 1);
    assert(doc3.metadata.at("key") == "value");
    assert(doc3.id.empty());

    // Test full constructor
    Document doc4("Hello world", metadata, "doc1");
    assert(doc4.content == "Hello world");
    assert(doc4.metadata.size() == 1);
    assert(doc4.metadata.at("key") == "value");
    assert(doc4.id == "doc1");

    std::cout << "Document tests passed!\n\n";
}

void test_simple_llm() {
    std::cout << "Testing SimpleLLM...\n";

    auto llm = std::make_shared<SimpleLLM>();

    // Test basic generation
    std::string response = llm->generate("hello");
    assert(!response.empty());
    assert(response == "Hello! How can I help you today?");

    // Test with different prompt
    response = llm->generate("What is your name?");
    assert(!response.empty());
    assert(response == "I'm a language model assistant created to help you with various tasks.");

    // Test batch generation
    StringList prompts = {"hello", "help"};
    StringList responses = llm->generate_batch(prompts);
    assert(responses.size() == 2);
    assert(responses[0] == "Hello! How can I help you today?");
    assert(responses[1] == "I'm here to help you with various tasks. You can ask me questions or request assistance with different topics.");

    std::cout << "SimpleLLM tests passed!\n\n";
}

void test_llm_chain() {
    std::cout << "Testing LLMChain...\n";

    auto llm = std::make_shared<SimpleLLM>();
    auto chain = std::make_shared<LLMChain>(llm, "Question: {input}\nAnswer:");

    std::string response = chain->invoke("What is 2+2?");
    assert(!response.empty());

    // Test with different template
    chain->set_prompt_template("User says: {input}\nAssistant:");
    response = chain->invoke("Hello");
    assert(!response.empty());

    std::cout << "LLMChain tests passed!\n\n";
}

void test_vector_store() {
    std::cout << "Testing InMemoryVectorStore...\n";

    auto vectorstore = std::make_shared<InMemoryVectorStore>();

    // Test adding documents
    std::vector<Document> documents = {
        Document("The quick brown fox", {{"category", "animals"}}),
        Document("Machine learning algorithms", {{"category", "technology"}})
    };

    StringList ids = vectorstore->add_documents(documents);
    assert(ids.size() == 2);
    assert(!ids[0].empty());
    assert(!ids[1].empty());

    // Test similarity search
    auto results = vectorstore->similarity_search("quick fox", 1);
    assert(results.size() == 1);
    assert(results[0].content == "The quick brown fox");

    // Test similarity search with scores
    auto results_with_scores = vectorstore->similarity_search_with_score("machine learning", 1);
    assert(results_with_scores.size() == 1);
    assert(results_with_scores[0].first.content == "Machine learning algorithms");
    assert(results_with_scores[0].second >= 0.0);

    // Test get by IDs
    auto retrieved_docs = vectorstore->get_by_ids({ids[0]});
    assert(retrieved_docs.size() == 1);
    assert(retrieved_docs[0].id == ids[0]);

    // Test deletion
    vectorstore->delete_documents({ids[0]});
    auto results_after_delete = vectorstore->similarity_search("quick fox", 5);
    assert(results_after_delete.size() == 1); // Only one document should remain

    std::cout << "InMemoryVectorStore tests passed!\n\n";
}

void test_tools() {
    std::cout << "Testing Tools...\n";

    // Test calculator
    CalculatorTool calculator;
    std::string result = calculator.execute("2+3");
    assert(result == "5.000000");

    result = calculator.execute("5*4");
    assert(result == "20.000000");

    // Test string tool
    StringTool string_tool("uppercase");
    result = string_tool.execute("hello");
    assert(result == "HELLO");

    string_tool.set_operation("reverse");
    result = string_tool.execute("hello");
    assert(result == "olleh");

    std::cout << "Tools tests passed!\n\n";
}

void test_memory() {
    std::cout << "Testing Memory...\n";

    // Test ShortTermMemory
    auto short_term_memory = std::make_shared<ShortTermMemory>(3);
    assert(short_term_memory->size() == 0);

    short_term_memory->add_message("Human", "Hello");
    short_term_memory->add_message("AI", "Hi there!");
    assert(short_term_memory->size() == 2);

    auto messages = short_term_memory->get_messages();
    assert(messages.size() == 2);
    assert(messages[0].first == "Human");
    assert(messages[0].second == "Hello");
    assert(messages[1].first == "AI");
    assert(messages[1].second == "Hi there!");

    // Test max size limit
    short_term_memory->add_message("Human", "How are you?");
    short_term_memory->add_message("AI", "I'm doing well, thanks!");
    assert(short_term_memory->size() == 3); // Should be at max size

    short_term_memory->add_message("Human", "What's your name?");
    assert(short_term_memory->size() == 3); // Should still be at max size, oldest message removed

    // Test clear
    short_term_memory->clear();
    assert(short_term_memory->size() == 0);

    // Test ConversationBufferMemory
    auto memory = std::make_shared<ShortTermMemory>(5);
    auto conversation_memory = std::make_shared<ConversationBufferMemory>(memory);

    conversation_memory->add_user_message("Hello");
    conversation_memory->add_ai_message("Hi there!");

    auto conv_messages = conversation_memory->get_messages();
    assert(conv_messages.size() == 2);
    assert(conv_messages[0].first == "Human");
    assert(conv_messages[0].second == "Hello");
    assert(conv_messages[1].first == "AI");
    assert(conv_messages[1].second == "Hi there!");

    // Test history string
    std::string history = conversation_memory->get_history();
    assert(!history.empty());
    assert(history.find("Human: Hello") != std::string::npos);
    assert(history.find("AI: Hi there!") != std::string::npos);

    std::cout << "Memory tests passed!\n\n";
}

void test_enhanced_react_agent() {
    std::cout << "Testing Enhanced ReAct Agent...\n";

    // Create an LLM
    auto llm = std::make_shared<SimpleLLM>();

    // Create tools
    auto calculator = std::make_shared<CalculatorTool>();
    auto string_tool = std::make_shared<StringTool>();
    std::vector<std::shared_ptr<Tool>> tools = {calculator, string_tool};

    // Create an enhanced ReAct agent
    EnhancedReActAgent agent(llm, tools);

    // Test basic functionality
    std::string response = agent.execute("What is 2 + 2?");
    assert(!response.empty());

    // Test with verbose mode
    agent.set_verbose(false); // Turn off verbose for cleaner test output
    std::string response2 = agent.execute("Convert 'hello' to uppercase");
    assert(!response2.empty());

    std::cout << "Enhanced ReAct Agent tests passed!\n\n";
}

int main() {
    std::cout << "Running LangChain C++ Tests\n";
    std::cout << "==========================\n\n";

    try {
        test_document();
        test_simple_llm();
        test_llm_chain();
        test_vector_store();
        test_tools();
        test_memory();
        test_enhanced_react_agent();

        std::cout << "All tests passed successfully!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}