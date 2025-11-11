#ifndef LANGCHAIN_CHAINS_H
#define LANGCHAIN_CHAINS_H

#include "core.h"
#include "llms.h"
#include <sstream>
#include <iomanip>

namespace langchain {

// Simple LLM chain that combines a prompt template with an LLM
class LLMChain : public Chain {
private:
    std::shared_ptr<LLM> llm_;
    String prompt_template_;

public:
    LLMChain(std::shared_ptr<LLM> llm, const String& prompt_template = "{input}");
    String invoke(const String& input) override;
    void set_prompt_template(const String& template_str);
};

// Sequential chain that runs multiple chains in sequence
class SequentialChain : public Chain {
private:
    std::vector<std::shared_ptr<Chain>> chains_;

public:
    SequentialChain(const std::vector<std::shared_ptr<Chain>>& chains);
    String invoke(const String& input) override;
    void add_chain(std::shared_ptr<Chain> chain);
};

// Transform chain that applies a function to the input
class TransformChain : public Chain {
private:
    std::function<String(const String&)> transform_func_;
    String name_;

public:
    TransformChain(std::function<String(const String&)> func, const String& name = "TransformChain");
    String invoke(const String& input) override;
};

// Simple prompt template class
class PromptTemplate {
private:
    String template_str_;
    StringList input_variables_;

public:
    PromptTemplate(const String& template_str, const StringList& input_variables);
    String format(const StringMap& kwargs) const;
    static PromptTemplate from_template(const String& template_str);
};

} // namespace langchain

#endif // LANGCHAIN_CHAINS_H