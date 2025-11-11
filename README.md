# LangChain C++ Implementation

This is a C++ implementation of the core concepts from the LangChain framework. It provides a simplified version of the main components including LLMs, Chains, Tools, Agents, and Vector Stores, with support for calling real API-based models like OpenAI and Qwen.

## Features

- **LLMs**: Base LLM interface with implementations for SimpleLLM, EchoLLM, ReverseLLM, OpenAI, Qwen, and Generic API models
- **Chains**: LLMChain, SequentialChain, and TransformChain implementations
- **Tools**: CalculatorTool, StringTool, SearchTool, and TimeTool
- **Agents**: SimpleAgent, ReActAgent, and EnhancedReActAgent implementations
- **Vector Stores**: InMemoryVectorStore implementation
- **Memory**: Short-term and long-term memory implementations with conversation buffer
- **Documents**: Document class for representing text with metadata
- **API Models**: Support for OpenAI, Qwen (千问), and other API-based models

## Installation

See [INSTALL.md](INSTALL.md) for detailed installation instructions.

### Quick Installation (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install libcurl4-openssl-dev nlohmann-json3-dev cmake build-essential
```

### Quick Installation (macOS)
```bash
brew install curl nlohmann-json cmake
```

## Building

This project uses CMake for building. To build the project:

```bash
cd langchain_cpp
mkdir build
cd build
cmake ..
make
```

## Running Examples

After building, you can run the examples:

```bash
./simple_example
./chain_example
./agent_example
./vectorstore_example
./memory_example
./enhanced_react_agent_example
./api_model_example
```

## Running Tests

To run the tests:

```bash
./langchain_test
```

## Project Structure

```
langchain_cpp/
├── include/
│   └── langchain/
│       ├── core.h          # Core classes (Document, LLM, Chain, Tool, Agent, VectorStore)
│       ├── llms.h          # LLM implementations
│       ├── chains.h        # Chain implementations
│       ├── tools.h         # Tool implementations
│       ├── agents.h        # Agent implementations
│       ├── vectorstores.h  # Vector store implementations
│       ├── memory.h        # Memory implementations (ShortTermMemory, LongTermMemory)
│       ├── models.h        # API model implementations (OpenAI, Qwen, etc.)
│       └── langchain.h     # Main header file
├── src/                    # Implementation files
├── examples/               # Example programs
├── tests/                  # Test files
├── CMakeLists.txt          # CMake build configuration
├── README.md               # This file
└── INSTALL.md              # Installation guide
```

## Usage

To use the LangChain C++ framework in your project, include the main header file:

```cpp
#include "langchain/langchain.h"

using namespace langchain;

// Create an LLM
auto llm = std::make_shared<SimpleLLM>();

// Create a chain
auto chain = std::make_shared<LLMChain>(llm);

// Use the chain
std::string response = chain->invoke("Hello, world!");
```

### Using Memory

The memory module provides short-term and long-term memory capabilities for agents:

```cpp
#include "langchain/langchain.h"

using namespace langchain;

// Create short-term memory
auto short_term_memory = std::make_shared<ShortTermMemory>(5); // Keep last 5 messages
auto conversation_memory = std::make_shared<ConversationBufferMemory>(short_term_memory);

// Create an agent with memory
auto llm = std::make_shared<SimpleLLM>();
std::vector<std::shared_ptr<Tool>> tools = {}; // Add your tools here
auto agent = std::make_shared<SimpleAgent>(llm, tools, conversation_memory);

// The agent will now remember previous conversations
std::string response1 = agent->execute("What is the capital of France?");
std::string response2 = agent->execute("What did I just ask you?"); // Agent remembers the previous question
```

### Using Enhanced ReAct Agent

The EnhancedReActAgent provides a more sophisticated implementation of the ReAct pattern with better support for multiple tools:

```cpp
#include "langchain/langchain.h"

using namespace langchain;

// Create tools
auto calculator = std::make_shared<CalculatorTool>();
auto string_tool = std::make_shared<StringTool>();
auto search_tool = std::make_shared<SearchTool>();
std::vector<std::shared_ptr<Tool>> tools = {calculator, string_tool, search_tool};

// Create an enhanced ReAct agent
auto llm = std::make_shared<SimpleLLM>();
EnhancedReActAgent agent(llm, tools);

// Enable verbose mode to see intermediate steps
agent.set_verbose(true);

// The agent will use the ReAct pattern to solve complex queries
std::string response = agent.execute("Calculate 12 * 15 and convert to uppercase");
```

### Using Long-Term Memory

For long-term memory, you can combine memory with vector stores:

```cpp
#include "langchain/langchain.h"

using namespace langchain;

// Create a vector store for long-term memory
auto vector_store = std::make_shared<InMemoryVectorStore>();

// Create long-term memory with vector store
auto long_term_memory = std::make_shared<LongTermMemory>(vector_store, 3); // Keep 3 recent messages in buffer
auto conversation_memory = std::make_shared<ConversationBufferMemory>(long_term_memory);

// Create an agent with long-term memory
auto llm = std::make_shared<SimpleLLM>();
std::vector<std::shared_ptr<Tool>> tools = {}; // Add your tools here
auto agent = std::make_shared<SimpleAgent>(llm, tools, conversation_memory);
```

### Using API Models

```cpp
#include "langchain/langchain.h"

using namespace langchain;

// OpenAI model
auto openai = std::make_shared<OpenAIModel>("your-openai-api-key", "gpt-3.5-turbo");
openai->set_temperature(0.7);
std::string response = openai->generate("What is artificial intelligence?");

// Qwen model
auto qwen = std::make_shared<QwenModel>("your-qwen-api-key", "qwen-turbo");
qwen->set_temperature(0.7);
std::string response2 = qwen->generate("What is machine learning?");

// Generic API model
auto generic = std::make_shared<GenericModel>("api-key", "https://your-api-endpoint.com/v1/completions");
// Configure with custom request builder and response parser
```

## Components

### LLMs

The LLM base class provides an interface for language models:

```cpp
class LLM {
public:
    virtual String generate(const String& prompt) = 0;
    virtual std::future<String> generate_async(const String& prompt);
    virtual StringList generate_batch(const StringList& prompts);
    virtual std::future<StringList> generate_batch_async(const StringList& prompts);
    virtual void stream(const String& prompt, std::function<void(const String&)> callback);
};
```

### API Models

API models extend the LLM interface to work with cloud-based language models:

```cpp
class APIModel : public LLM {
protected:
    String api_key_;
    String base_url_;
    String model_name_;

public:
    virtual nlohmann::json prepare_request(const String& prompt) = 0;
    virtual String parse_response(const nlohmann::json& response) = 0;
};

class OpenAIModel : public APIModel {
    // Specific implementation for OpenAI API
};

class QwenModel : public APIModel {
    // Specific implementation for Qwen API
};
```

### Chains

Chains combine components in a specific order:

```cpp
class Chain {
public:
    virtual String invoke(const String& input) = 0;
    virtual std::future<String> invoke_async(const String& input);
    virtual StringList invoke_batch(const StringList& inputs);
};
```

### Tools

Tools are functions that agents can use:

```cpp
class Tool {
public:
    String name;
    String description;
    virtual String execute(const String& input) = 0;
};
```

### Vector Stores

Vector stores manage document embeddings:

```cpp
class VectorStore {
public:
    virtual StringList add_documents(const std::vector<Document>& documents) = 0;
    virtual std::vector<Document> similarity_search(const String& query, int k = 4) = 0;
    virtual std::vector<std::pair<Document, double>> similarity_search_with_score(
        const String& query, int k = 4) = 0;
    virtual void delete_documents(const StringList& ids) = 0;
    virtual std::vector<Document> get_by_ids(const StringList& ids) = 0;
};
```

### Memory

The memory module provides short-term and long-term memory capabilities:

```cpp
// Base Memory interface
class Memory {
public:
    virtual ~Memory() = default;
    virtual void add_message(const String& role, const String& content) = 0;
    virtual std::vector<std::pair<String, String>> get_messages() const = 0;
    virtual void clear() = 0;
    virtual size_t size() const = 0;
};

// Short-term memory implementation
class ShortTermMemory : public Memory {
    // Keeps recent messages in a buffer with configurable size
};

// Long-term memory implementation
class LongTermMemory : public Memory {
    // Combines recent memory buffer with vector store for long-term storage
};

// Conversation buffer memory
class ConversationBufferMemory {
    // Provides convenient interface for managing conversation history
};
```

### Agents

Agents are autonomous entities that can use tools to accomplish tasks:

```cpp
// Simple agent that can use tools
class SimpleAgent {
    // Basic agent implementation
};

// ReAct agent (Reasoning and Acting)
class ReActAgent {
    // Implements the ReAct pattern with thought-action-observation cycles
};

// Enhanced ReAct agent with better ReAct pattern support
class EnhancedReActAgent {
    // More sophisticated ReAct implementation with better tool handling
};
```

## API Model Support

### OpenAI

Supports all OpenAI models including GPT-3.5, GPT-4, etc.

```cpp
auto openai = std::make_shared<OpenAIModel>("your-api-key", "gpt-3.5-turbo");
openai->set_temperature(0.7);
openai->set_max_tokens(1000);
```

### Qwen (千问)

Support for Alibaba's Qwen models.

```cpp
auto qwen = std::make_shared<QwenModel>("your-api-key", "qwen-turbo");
qwen->set_temperature(0.7);
qwen->set_max_tokens(1000);
```

### Generic Models

Support for any API that follows a similar pattern.

```cpp
auto generic = std::make_shared<GenericModel>("api-key", "https://api.example.com/v1/chat");
generic->set_request_builder([](const String& prompt) {
    // Custom request building logic
});
generic->set_response_parser([](const nlohmann::json& response) {
    // Custom response parsing logic
});
```

## Limitations

This is a simplified implementation for educational purposes and has several limitations compared to the full Python LangChain:

1. Simplified vector similarity calculations
2. Limited tool implementations
3. No real database backends for vector stores
4. API models require internet connectivity

## Extending

To extend this framework:

1. Implement new LLM classes by inheriting from the LLM base class
2. Create new chain types by inheriting from the Chain base class
3. Add new tools by inheriting from the Tool base class
4. Implement new vector stores by inheriting from the VectorStore base class
5. Add support for new APIs by extending the APIModel base class

## License

This project is licensed under the MIT License.