#ifndef LANGCHAIN_CORE_H
#define LANGCHAIN_CORE_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <future>

namespace langchain {

// Forward declarations
class Document;
class LLM;
class Chain;
class Tool;
class Agent;
class VectorStore;

// Basic types
using String = std::string;
using StringList = std::vector<String>;
using StringMap = std::map<String, String>;

// Document class
class Document {
public:
    String content;
    StringMap metadata;
    String id;

    Document();
    Document(const String& content);
    Document(const String& content, const StringMap& metadata);
    Document(const String& content, const StringMap& metadata, const String& id);
};

// Base LLM interface
class LLM {
public:
    virtual ~LLM() = default;

    // Generate response for a single prompt
    virtual String generate(const String& prompt) = 0;

    // Generate response asynchronously
    virtual std::future<String> generate_async(const String& prompt);

    // Generate responses for multiple prompts
    virtual StringList generate_batch(const StringList& prompts);

    // Generate responses for multiple prompts asynchronously
    virtual std::future<StringList> generate_batch_async(const StringList& prompts);

    // Stream response (simplified implementation)
    virtual void stream(const String& prompt, std::function<void(const String&)> callback);
};

// Base Chain interface
class Chain {
public:
    virtual ~Chain() = default;

    // Invoke the chain with input
    virtual String invoke(const String& input) = 0;

    // Invoke the chain asynchronously
    virtual std::future<String> invoke_async(const String& input);

    // Batch invoke
    virtual StringList invoke_batch(const StringList& inputs);
};

// Base Tool interface
class Tool {
public:
    String name;
    String description;

    Tool(const String& name, const String& description);
    virtual ~Tool() = default;

    // Execute the tool with input
    virtual String execute(const String& input) = 0;
};

// Base VectorStore interface
class VectorStore {
public:
    virtual ~VectorStore() = default;

    // Add documents to the vector store
    virtual StringList add_documents(const std::vector<Document>& documents) = 0;

    // Search for similar documents
    virtual std::vector<Document> similarity_search(const String& query, int k = 4) = 0;

    // Search for similar documents with scores
    virtual std::vector<std::pair<Document, double>> similarity_search_with_score(
        const String& query, int k = 4) = 0;

    // Delete documents by IDs
    virtual void delete_documents(const StringList& ids) = 0;

    // Get documents by IDs
    virtual std::vector<Document> get_by_ids(const StringList& ids) = 0;
};

} // namespace langchain

#endif // LANGCHAIN_CORE_H