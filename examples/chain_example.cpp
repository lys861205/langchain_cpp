#include <iostream>
#include <memory>
#include "../include/langchain/langchain.h"

using namespace langchain;

int main() {
    std::cout << "LangChain C++ Chain Example\n";
    std::cout << "==========================\n\n";

    // Create an LLM
    auto llm = std::make_shared<SimpleLLM>();

    // Create a simple LLM chain
    auto llm_chain = std::make_shared<LLMChain>(llm, "Question: {input}\nAnswer:");

    // Test the chain
    std::string question = "What is the capital of France?";
    std::string response = llm_chain->invoke(question);

    std::cout << "Question: " << question << std::endl;
    std::cout << "Response: " << response << std::endl << std::endl;

    // Create a transform chain
    auto transform_chain = std::make_shared<TransformChain>(
        [](const std::string& input) -> std::string {
            std::string result = input;
            for (auto& c : result) {
                c = std::toupper(c);
            }
            return "Uppercase: " + result;
        },
        "uppercase_transform"
    );

    // Create a sequential chain
    std::vector<std::shared_ptr<Chain>> chains = {llm_chain, transform_chain};
    auto sequential_chain = std::make_shared<SequentialChain>(chains);

    // Test the sequential chain
    std::string seq_question = "What is the largest planet in our solar system?";
    std::string seq_response = sequential_chain->invoke(seq_question);

    std::cout << "Sequential Chain Test:" << std::endl;
    std::cout << "Question: " << seq_question << std::endl;
    std::cout << "Response: " << seq_response << std::endl << std::endl;

    // Test prompt template
    auto prompt_template = PromptTemplate::from_template("Translate to French: {input}");
    std::string formatted_prompt = prompt_template.format({{"input", "Hello, world!"}});
    std::cout << "Prompt Template Example:" << std::endl;
    std::cout << "Formatted Prompt: " << formatted_prompt << std::endl;

    return 0;
}