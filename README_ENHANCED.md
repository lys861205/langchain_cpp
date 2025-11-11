# LangChain C++ Enhanced Framework

## Overview

This enhanced version of the LangChain C++ framework provides comprehensive support for modern AI development, including:

1. **Dual HTTP Client Implementation** - Supports both brpc and curl for maximum compatibility and performance
2. **Major LLM API Integration** - Direct support for OpenAI, Anthropic Claude, Google AI, and Qwen
3. **Advanced Data Connectors** - SQL, NoSQL, and REST API connectors
4. **Enhanced Retrieval System** - Advanced search capabilities with filtering and multiple algorithms

## Key Features

### HTTP Client
- Unified HttpClient interface with dual implementation (brpc/curl)
- Automatic fallback when brpc is not available
- Support for custom headers, timeouts, and error handling

### LLM API Models
- **OpenAIModel** - Support for GPT-3.5, GPT-4, and other OpenAI models
- **ClaudeModel** - Support for Anthropic Claude 3 models
- **GoogleModel** - Support for Google Gemini models
- **QwenModel** - Support for Alibaba Cloud Qwen models
- **GenericModel** - Flexible model for custom APIs
- Built-in retry mechanisms with exponential backoff
- Configurable timeouts and authentication

### Data Connectors
- **SQLDatabaseConnector** with SQLite implementation
- **NoSQLDatabaseConnector** with Redis implementation
- **WebAPIConnector** with REST API implementation
- Factory pattern for easy connector creation
- Unified interface for document loading, saving, and querying

### Advanced Retrieval
- **AdvancedRetriever** with filtering and multiple similarity algorithms
- **MultiQueryRetriever** for enhanced search capabilities
- **ContextualCompressionRetriever** for optimized results
- Support for COSINE, JACCARD, EUCLIDEAN, and BM25 algorithms

## Building the Project

```bash
mkdir build && cd build
cmake ..
make
```

The build system automatically detects available dependencies:
- brpc support (if available)
- Redis support (if hiredis is available)
- JSON support for API models (if nlohmann/json is available)

## Running Examples

```bash
# Test HTTP client functionality
./http_client_test

# Test API model functionality
./comprehensive_api_test

# Test data connector functionality
./data_connectors_test

# Test advanced retrieval functionality
./advanced_retrieval_example

# Run comprehensive test of all features
./comprehensive_test

# Run the final demonstration
./final_demo
```

## API Usage Examples

### Using LLM API Models

```cpp
#include "langchain/models.h"

// OpenAI
auto openai = std::make_shared<langchain::OpenAIModel>("your-api-key");
openai->set_model_name("gpt-4");
openai->set_temperature(0.8);

// Claude
auto claude = std::make_shared<langchain::ClaudeModel>("your-api-key");
claude->set_model_name("claude-3-opus-20240229");

// Google AI
auto google = std::make_shared<langchain::GoogleModel>("your-api-key");
google->set_model_name("gemini-pro");

// Generate text
std::string response = openai->generate("Hello, world!");
```

### Using Data Connectors

```cpp
#include "langchain/data_connectors.h"

// SQLite connector
auto sqlite = langchain::DataConnectorFactory::create_sql_connector("sqlite", "sqlite:///example.db");
sqlite->connect();

langchain::Document doc("Sample content");
doc.metadata["category"] = "example";
sqlite->save_documents({doc});

// Redis connector
auto redis = langchain::DataConnectorFactory::create_nosql_connector("redis", "redis://localhost:6379");
redis->connect();
redis->put_document("key1", doc);

// REST API connector
auto rest = langchain::DataConnectorFactory::create_web_api_connector("rest", "https://api.example.com", "your-api-key");
rest->connect();
```

### Using Advanced Retrieval

```cpp
#include "langchain/advanced_retrievers.h"

// Create vector store and add documents
auto vector_store = std::make_shared<langchain::InMemoryVectorStore>();
vector_store->add_documents(documents);

// Create advanced retriever
auto retriever = std::make_shared<langchain::AdvancedRetriever>(vector_store, langchain::SimilarityAlgorithm::COSINE);

// Search with filters
std::map<langchain::String, langchain::String> filters = {{"category", "programming"}};
auto results = retriever->search("machine learning", 5, filters);
```

## Dependencies

### Required
- C++17 compiler
- CMake 3.10+
- SQLite3 development libraries

### Optional (automatically detected)
- brpc (for enhanced HTTP performance)
- hiredis (for Redis support)
- nlohmann/json (for API model support)
- curl (fallback HTTP client)

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.