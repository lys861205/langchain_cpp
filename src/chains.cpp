#include "../include/langchain/chains.h"
#include <sstream>
#include <iomanip>

namespace langchain {

// LLMChain implementation
LLMChain::LLMChain(std::shared_ptr<LLM> llm, const String& prompt_template)
    : llm_(llm), prompt_template_(prompt_template) {}

String LLMChain::invoke(const String& input) {
    // Format the prompt by replacing {input} with the actual input
    String prompt = prompt_template_;
    size_t pos = prompt.find("{input}");
    if (pos != String::npos) {
        prompt.replace(pos, 7, input);
    }

    return llm_->generate(prompt);
}

void LLMChain::set_prompt_template(const String& template_str) {
    prompt_template_ = template_str;
}

// SequentialChain implementation
SequentialChain::SequentialChain(const std::vector<std::shared_ptr<Chain>>& chains)
    : chains_(chains) {}

String SequentialChain::invoke(const String& input) {
    String current_input = input;
    for (const auto& chain : chains_) {
        current_input = chain->invoke(current_input);
    }
    return current_input;
}

void SequentialChain::add_chain(std::shared_ptr<Chain> chain) {
    chains_.push_back(chain);
}

// TransformChain implementation
TransformChain::TransformChain(std::function<String(const String&)> func, const String& name)
    : transform_func_(func), name_(name) {}

String TransformChain::invoke(const String& input) {
    return transform_func_(input);
}

// PromptTemplate implementation
PromptTemplate::PromptTemplate(const String& template_str, const StringList& input_variables)
    : template_str_(template_str), input_variables_(input_variables) {}

String PromptTemplate::format(const StringMap& kwargs) const {
    String result = template_str_;
    for (const auto& pair : kwargs) {
        String placeholder = "{" + pair.first + "}";
        size_t pos = result.find(placeholder);
        while (pos != String::npos) {
            result.replace(pos, placeholder.length(), pair.second);
            pos = result.find(placeholder, pos + pair.second.length());
        }
    }
    return result;
}

PromptTemplate PromptTemplate::from_template(const String& template_str) {
    // Simple implementation that assumes {input} as the only variable
    return PromptTemplate(template_str, {"input"});
}

} // namespace langchain