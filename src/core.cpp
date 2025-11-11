#include "../include/langchain/core.h"
#include <algorithm>

namespace langchain {

// Document class implementation
Document::Document() : content(""), metadata(StringMap()), id("") {}

Document::Document(const String& content) : content(content), metadata(StringMap()), id("") {}

Document::Document(const String& content, const StringMap& metadata)
    : content(content), metadata(metadata), id("") {}

Document::Document(const String& content, const StringMap& metadata, const String& id)
    : content(content), metadata(metadata), id(id) {}

// Tool class implementation
Tool::Tool(const String& name, const String& description)
    : name(name), description(description) {}

// LLM class implementation
std::future<String> LLM::generate_async(const String& prompt) {
    return std::async(std::launch::async, [this, prompt]() {
        return generate(prompt);
    });
}

StringList LLM::generate_batch(const StringList& prompts) {
    StringList results;
    for (const auto& prompt : prompts) {
        results.push_back(generate(prompt));
    }
    return results;
}

std::future<StringList> LLM::generate_batch_async(const StringList& prompts) {
    return std::async(std::launch::async, [this, prompts]() {
        return generate_batch(prompts);
    });
}

void LLM::stream(const String& prompt, std::function<void(const String&)> callback) {
    String result = generate(prompt);
    callback(result);
}

// Chain class implementation
std::future<String> Chain::invoke_async(const String& input) {
    return std::async(std::launch::async, [this, input]() {
        return invoke(input);
    });
}

StringList Chain::invoke_batch(const StringList& inputs) {
    StringList results;
    for (const auto& input : inputs) {
        results.push_back(invoke(input));
    }
    return results;
}

} // namespace langchain