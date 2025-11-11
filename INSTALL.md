# Installation Guide

This guide explains how to install the dependencies needed to build and use the LangChain C++ framework with API model support.

## Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.10 or higher
- Git

## Dependencies

### 1. CURL Development Libraries

The API model implementations require libcurl for HTTP requests.

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install libcurl4-openssl-dev
```

**macOS (with Homebrew):**
```bash
brew install curl
```

**Windows (with vcpkg):**
```bash
vcpkg install curl
```

### 2. JSON Library (nlohmann/json)

The API models use nlohmann/json for JSON parsing.

**Ubuntu/Debian:**
```bash
sudo apt-get install nlohmann-json3-dev
```

**macOS (with Homebrew):**
```bash
brew install nlohmann-json
```

**Windows (with vcpkg):**
```bash
vcpkg install nlohmann-json
```

**Manual Installation:**
You can also download the single header file directly:
```bash
curl -O https://raw.githubusercontent.com/nlohmann/json/v3.11.2/single_include/nlohmann/json.hpp
```
Then place it in `/usr/local/include/nlohmann/` or update the include path in CMakeLists.txt.

## Building the Project

1. Clone the repository:
```bash
git clone <repository-url>
cd langchain_cpp
```

2. Create a build directory:
```bash
mkdir build
cd build
```

3. Configure with CMake:
```bash
cmake ..
```

4. Build the project:
```bash
make
```

## Setting API Keys

To use the API models, you need to set environment variables:

**OpenAI:**
```bash
export OPENAI_API_KEY="your-openai-api-key"
```

**Qwen (千问):**
```bash
export QWEN_API_KEY="your-qwen-api-key"
```

## Testing API Models

After setting your API keys, you can test the API models:

```bash
# Test OpenAI model
export OPENAI_API_KEY="your-openai-api-key"
./api_model_example

# Test Qwen model
export QWEN_API_KEY="your-qwen-api-key"
./api_model_example
```

## Troubleshooting

### CURL Not Found
If CMake cannot find CURL, try specifying the path:
```bash
cmake -DCURL_INCLUDE_DIR=/usr/local/include -DCURL_LIBRARY=/usr/local/lib ..
```

### JSON Library Not Found
If the JSON library is not found, you may need to specify its location:
```bash
cmake -DCMAKE_INCLUDE_PATH=/usr/local/include ..
```

### Linking Errors
If you encounter linking errors, make sure all dependencies are properly installed and the linker can find them.

## Using in Your Own Projects

To use the LangChain C++ framework in your own projects:

1. Build and install the library:
```bash
make
sudo make install  # Optional: installs to /usr/local
```

2. In your CMakeLists.txt:
```cmake
find_package(CURL REQUIRED)
find_package(PkgConfig REQUIRED)

add_executable(your_app your_source.cpp)
target_link_libraries(your_app langchain_cpp ${CURL_LIBRARIES})
```

3. In your source code:
```cpp
#include <langchain/langchain.h>

using namespace langchain;

// Create and use models
auto openai_model = std::make_shared<OpenAIModel>(api_key);
```