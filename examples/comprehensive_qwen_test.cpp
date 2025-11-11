#include <iostream>
#include <memory>
#include <cstdlib>
#include "../include/langchain/langchain.h"

using namespace langchain;

int main() {
    std::cout << "LangChain C++ Framework Comprehensive Test with Qwen API\n";
    std::cout << "======================================================\n\n";

    // Get Qwen API key from environment variable
    const char* qwen_api_key = std::getenv("QWEN_API_KEY");
    if (!qwen_api_key) {
        std::cout << "Error: QWEN_API_KEY environment variable not set.\n";
        std::cout << "Please set your Qwen API key to run this comprehensive test.\n";
        return 1;
    }

    std::cout << "Using real Qwen API key for comprehensive testing...\n\n";

    // 1. Test QwenModel directly
    std::cout << "1. Testing QwenModel directly:\n";
    auto qwen_model = std::make_shared<QwenModel>(String(qwen_api_key), "qwen-turbo");
    qwen_model->set_temperature(0.7);
    qwen_model->set_max_tokens(300);

    String prompt = "请用中文简要介绍人工智能的发展历程。";
    String response = qwen_model->generate(prompt);
    std::cout << "Prompt: " << prompt << std::endl;
    std::cout << "Response: " << response << std::endl << std::endl;

    // 2. Test LLMChain with QwenModel
    std::cout << "2. Testing LLMChain with QwenModel:\n";
    auto chain = std::make_shared<LLMChain>(qwen_model, "问题: {input}\n回答:");

    String chain_prompt = "什么是机器学习？";
    String chain_response = chain->invoke(chain_prompt);
    std::cout << "Chain Prompt: " << chain_prompt << std::endl;
    std::cout << "Chain Response: " << chain_response << std::endl << std::endl;

    // 3. Test Tools with real functionality
    std::cout << "3. Testing real tools:\n";
    CalculatorTool calculator;
    String calc_result = calculator.execute("15 * 8");
    std::cout << "Calculator result for '15 * 8': " << calc_result << std::endl;

    StringTool string_tool;
    string_tool.set_operation("uppercase");
    String str_result = string_tool.execute("hello world");
    std::cout << "String tool uppercase result for 'hello world': " << str_result << std::endl;

    TimeTool time_tool;
    String time_result = time_tool.execute("");
    std::cout << "Time tool result: " << time_result << std::endl << std::endl;

    // 4. Test EnhancedReActAgent with QwenModel
    std::cout << "4. Testing EnhancedReActAgent with QwenModel:\n";
    auto calculator_tool = std::make_shared<CalculatorTool>();
    auto string_tool_ptr = std::make_shared<StringTool>();
    auto time_tool_ptr = std::make_shared<TimeTool>();
    std::vector<std::shared_ptr<Tool>> tools = {calculator_tool, string_tool_ptr, time_tool_ptr};

    EnhancedReActAgent agent(qwen_model, tools);
    agent.set_verbose(true); // Enable verbose mode to see the ReAct process

    String agent_query = "请计算12乘以15，然后告诉我当前时间。";
    std::cout << "Agent Query: " << agent_query << std::endl;
    String agent_response = agent.execute(agent_query);
    std::cout << "Agent Final Answer: " << agent_response << std::endl << std::endl;

    // 5. Test RAG functionality with QwenModel
    std::cout << "5. Testing RAG functionality with QwenModel:\n";

    // Create sample documents
    Document doc1("人工智能是计算机科学的一个分支，它企图了解智能的实质，并生产出一种新的能以人类智能相似的方式做出反应的智能机器。",
                  {{"source", "ai_basics.txt"}, {"category", "introduction"}}, "doc1");

    Document doc2("机器学习是人工智能的一个重要分支，它使计算机能够从数据中学习并做出预测或决策，而无需明确编程。",
                  {{"source", "ml_basics.txt"}, {"category", "ml"}}, "doc2");

    // Create vector store and add documents
    auto vector_store = std::make_shared<InMemoryVectorStore>();
    std::vector<Document> docs = {doc1, doc2};
    vector_store->add_documents(docs);

    // Create RAG chain
    auto rag_chain = std::make_shared<RAGChain>(vector_store, qwen_model);

    String rag_query = "什么是机器学习？";
    String rag_response = rag_chain->query(rag_query);
    std::cout << "RAG Query: " << rag_query << std::endl;
    std::cout << "RAG Response: " << rag_response << std::endl << std::endl;

    // 6. Test Memory functionality
    std::cout << "6. Testing Memory functionality:\n";
    auto short_term_memory = std::make_shared<ShortTermMemory>(5);
    auto conversation_memory = std::make_shared<ConversationBufferMemory>(short_term_memory);

    conversation_memory->add_user_message("你好，我叫张三。");
    conversation_memory->add_ai_message("你好张三！很高兴认识你。");
    conversation_memory->add_user_message("我是一名软件工程师。");

    String memory_history = conversation_memory->get_history();
    std::cout << "Conversation history:\n" << memory_history << std::endl << std::endl;

    std::cout << "Comprehensive test with real Qwen API completed successfully!\n";

    return 0;
}