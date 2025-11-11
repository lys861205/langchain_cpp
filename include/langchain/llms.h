#ifndef LANGCHAIN_LLM_H
#define LANGCHAIN_LLM_H

#include "core.h"

namespace langchain {

// Simple mock LLM that generates responses based on keywords
class SimpleLLM : public LLM {
private:
    StringMap responses_;

public:
    SimpleLLM();
    String generate(const String& prompt) override;
};

// Echo LLM that simply returns the input prompt
class EchoLLM : public LLM {
public:
    String generate(const String& prompt) override;
};

// Reverse LLM that returns the reversed input prompt
class ReverseLLM : public LLM {
public:
    String generate(const String& prompt) override;
};

} // namespace langchain

#endif // LANGCHAIN_LLM_H