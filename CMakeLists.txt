cmake_minimum_required(VERSION 3.10)
project(langchain_cpp VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
# find_package(CURL REQUIRED)  # CURL is no longer needed with brpc

# Try to find brpc
find_path(BRPC_INCLUDE_DIR
    NAMES brpc/channel.h
    PATHS /usr/include /usr/local/include /opt/local/include
)

find_library(BRPC_LIBRARY
    NAMES brpc
    PATHS /usr/lib /usr/local/lib /opt/local/lib
)

# Try to find SQLite3
find_package(PkgConfig REQUIRED)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)

# Check if brpc is available
if(BRPC_INCLUDE_DIR AND BRPC_LIBRARY)
    set(BRPC_AVAILABLE TRUE)
    # Add compile definition for brpc
    add_compile_definitions(USE_BRPC)
else()
    set(BRPC_AVAILABLE FALSE)
endif()

# Try to find nlohmann/json
find_path(NLOHMANN_JSON_INCLUDE_DIR
    NAMES nlohmann/json.hpp
    PATHS /usr/include /usr/local/include /opt/local/include
)

# Try to find hiredis for Redis support
find_path(HIREDIS_INCLUDE_DIR
    NAMES hiredis/hiredis.h
    PATHS /usr/include /usr/local/include /opt/local/include
)

find_library(HIREDIS_LIBRARY
    NAMES hiredis
    PATHS /usr/lib /usr/local/lib /opt/local/lib
)

# Check if Redis support is available
if(HIREDIS_INCLUDE_DIR AND HIREDIS_LIBRARY)
    set(REDIS_AVAILABLE TRUE)
else()
    set(REDIS_AVAILABLE FALSE)
endif()

# Option to build with or without API models
option(BUILD_API_MODELS "Build with API model support" ON)

# Include directories
include_directories(include)

# Add nlohmann/json include directory if found
if(NLOHMANN_JSON_INCLUDE_DIR)
    include_directories(${NLOHMANN_JSON_INCLUDE_DIR})
endif()

# Add hiredis include directory if found
if(HIREDIS_INCLUDE_DIR)
    include_directories(${HIREDIS_INCLUDE_DIR})
endif()

# Source files
set(LANGCHAIN_SOURCES
    src/core.cpp
    src/llms.cpp
    src/chains.cpp
    src/tools.cpp
    src/agents.cpp
    src/vectorstores.cpp
    src/memory.cpp
    src/mcp.cpp
    src/http_client.cpp
    src/data_connectors.cpp
    src/sqlite_connector.cpp
    src/redis_connector.cpp
    src/rest_api_connector.cpp
    src/advanced_retrievers.cpp
    src/simple_connectors.cpp
)

# Add models.cpp only if building with API models and dependencies are found
if(BUILD_API_MODELS AND NLOHMANN_JSON_INCLUDE_DIR)
    list(APPEND LANGCHAIN_SOURCES src/models.cpp)
    set(API_MODELS_AVAILABLE TRUE)
else()
    set(API_MODELS_AVAILABLE FALSE)
endif()

# Add library
add_library(langchain_cpp ${LANGCHAIN_SOURCES})
target_include_directories(langchain_cpp PUBLIC include)
target_link_libraries(langchain_cpp ${SQLITE3_LIBRARIES})
if(BRPC_AVAILABLE)
    target_link_libraries(langchain_cpp ${BRPC_LIBRARY})
else()
    # Fallback to CURL if brpc is not available
    find_package(CURL REQUIRED)
    target_link_libraries(langchain_cpp ${CURL_LIBRARIES})
endif()
if(REDIS_AVAILABLE)
    target_link_libraries(langchain_cpp ${HIREDIS_LIBRARY})
endif()

# Example executables
add_executable(simple_example examples/simple_example.cpp)
target_link_libraries(simple_example langchain_cpp ${SQLITE3_LIBRARIES})
if(BRPC_AVAILABLE)
    target_link_libraries(simple_example ${BRPC_LIBRARY})
else()
    target_link_libraries(simple_example ${CURL_LIBRARIES})
endif()
if(REDIS_AVAILABLE)
    target_link_libraries(simple_example ${HIREDIS_LIBRARY})
endif()

add_executable(chain_example examples/chain_example.cpp)
target_link_libraries(chain_example langchain_cpp ${SQLITE3_LIBRARIES})
if(BRPC_AVAILABLE)
    target_link_libraries(chain_example ${BRPC_LIBRARY})
else()
    target_link_libraries(chain_example ${CURL_LIBRARIES})
endif()
if(REDIS_AVAILABLE)
    target_link_libraries(chain_example ${HIREDIS_LIBRARY})
endif()

add_executable(agent_example examples/agent_example.cpp)
target_link_libraries(agent_example langchain_cpp ${SQLITE3_LIBRARIES})
if(BRPC_AVAILABLE)
    target_link_libraries(agent_example ${BRPC_LIBRARY})
else()
    target_link_libraries(agent_example ${CURL_LIBRARIES})
endif()
if(REDIS_AVAILABLE)
    target_link_libraries(agent_example ${HIREDIS_LIBRARY})
endif()

add_executable(vectorstore_example examples/vectorstore_example.cpp)
target_link_libraries(vectorstore_example langchain_cpp ${SQLITE3_LIBRARIES})
if(BRPC_AVAILABLE)
    target_link_libraries(vectorstore_example ${BRPC_LIBRARY})
else()
    target_link_libraries(vectorstore_example ${CURL_LIBRARIES})
endif()
if(REDIS_AVAILABLE)
    target_link_libraries(vectorstore_example ${HIREDIS_LIBRARY})
endif()

add_executable(memory_example examples/memory_example.cpp)
target_link_libraries(memory_example langchain_cpp ${SQLITE3_LIBRARIES})
if(BRPC_AVAILABLE)
    target_link_libraries(memory_example ${BRPC_LIBRARY})
else()
    target_link_libraries(memory_example ${CURL_LIBRARIES})
endif()
if(REDIS_AVAILABLE)
    target_link_libraries(memory_example ${HIREDIS_LIBRARY})
endif()

add_executable(enhanced_react_agent_example examples/enhanced_react_agent_example.cpp)
target_link_libraries(enhanced_react_agent_example langchain_cpp ${SQLITE3_LIBRARIES})
if(BRPC_AVAILABLE)
    target_link_libraries(enhanced_react_agent_example ${BRPC_LIBRARY})
else()
    target_link_libraries(enhanced_react_agent_example ${CURL_LIBRARIES})
endif()
if(REDIS_AVAILABLE)
    target_link_libraries(enhanced_react_agent_example ${HIREDIS_LIBRARY})
endif()

add_executable(mcp_tool_example examples/mcp_tool_example.cpp)
target_link_libraries(mcp_tool_example langchain_cpp ${SQLITE3_LIBRARIES})
if(BRPC_AVAILABLE)
    target_link_libraries(mcp_tool_example ${BRPC_LIBRARY})
else()
    target_link_libraries(mcp_tool_example ${CURL_LIBRARIES})
endif()
if(REDIS_AVAILABLE)
    target_link_libraries(mcp_tool_example ${HIREDIS_LIBRARY})
endif()

add_executable(mcp_service_example examples/mcp_service_example.cpp)
target_link_libraries(mcp_service_example langchain_cpp ${SQLITE3_LIBRARIES})
if(BRPC_AVAILABLE)
    target_link_libraries(mcp_service_example ${BRPC_LIBRARY})
else()
    target_link_libraries(mcp_service_example ${CURL_LIBRARIES})
endif()
if(REDIS_AVAILABLE)
    target_link_libraries(mcp_service_example ${HIREDIS_LIBRARY})
endif()

add_executable(mcp_service_simulator examples/mcp_service_simulator.cpp)
if(BRPC_AVAILABLE)
    target_link_libraries(mcp_service_simulator ${BRPC_LIBRARY})
else()
    target_link_libraries(mcp_service_simulator ${CURL_LIBRARIES})
endif()

add_executable(agent_mcp_example examples/agent_mcp_example.cpp)
target_link_libraries(agent_mcp_example langchain_cpp ${SQLITE3_LIBRARIES})
if(BRPC_AVAILABLE)
    target_link_libraries(agent_mcp_example ${BRPC_LIBRARY})
else()
    target_link_libraries(agent_mcp_example ${CURL_LIBRARIES})
endif()
if(REDIS_AVAILABLE)
    target_link_libraries(agent_mcp_example ${HIREDIS_LIBRARY})
endif()

add_executable(rag_example examples/rag_example.cpp)
target_link_libraries(rag_example langchain_cpp ${SQLITE3_LIBRARIES})
if(BRPC_AVAILABLE)
    target_link_libraries(rag_example ${BRPC_LIBRARY})
else()
    target_link_libraries(rag_example ${CURL_LIBRARIES})
endif()
if(REDIS_AVAILABLE)
    target_link_libraries(rag_example ${HIREDIS_LIBRARY})
endif()

add_executable(text_splitting_example examples/text_splitting_example.cpp)
target_link_libraries(text_splitting_example langchain_cpp ${SQLITE3_LIBRARIES})
if(BRPC_AVAILABLE)
    target_link_libraries(text_splitting_example ${BRPC_LIBRARY})
else()
    target_link_libraries(text_splitting_example ${CURL_LIBRARIES})
endif()
if(REDIS_AVAILABLE)
    target_link_libraries(text_splitting_example ${HIREDIS_LIBRARY})
endif()

add_executable(complete_rag_example examples/complete_rag_example.cpp)
target_link_libraries(complete_rag_example langchain_cpp ${SQLITE3_LIBRARIES})
if(BRPC_AVAILABLE)
    target_link_libraries(complete_rag_example ${BRPC_LIBRARY})
else()
    target_link_libraries(complete_rag_example ${CURL_LIBRARIES})
endif()
if(REDIS_AVAILABLE)
    target_link_libraries(complete_rag_example ${HIREDIS_LIBRARY})
endif()

add_executable(chinese_english_text_splitting examples/chinese_english_text_splitting.cpp)
target_link_libraries(chinese_english_text_splitting langchain_cpp ${SQLITE3_LIBRARIES})
if(BRPC_AVAILABLE)
    target_link_libraries(chinese_english_text_splitting ${BRPC_LIBRARY})
else()
    target_link_libraries(chinese_english_text_splitting ${CURL_LIBRARIES})
endif()
if(REDIS_AVAILABLE)
    target_link_libraries(chinese_english_text_splitting ${HIREDIS_LIBRARY})
endif()

# Add API model example only if API models are available
if(API_MODELS_AVAILABLE)
    add_executable(api_model_example examples/api_model_example.cpp)
    target_link_libraries(api_model_example langchain_cpp ${SQLITE3_LIBRARIES})
    if(BRPC_AVAILABLE)
        target_link_libraries(api_model_example ${BRPC_LIBRARY})
    else()
        target_link_libraries(api_model_example ${CURL_LIBRARIES})
    endif()
    if(REDIS_AVAILABLE)
        target_link_libraries(api_model_example ${HIREDIS_LIBRARY})
    endif()

    add_executable(comprehensive_api_test examples/comprehensive_api_test.cpp)
    target_link_libraries(comprehensive_api_test langchain_cpp ${SQLITE3_LIBRARIES})
    if(BRPC_AVAILABLE)
        target_link_libraries(comprehensive_api_test ${BRPC_LIBRARY})
    else()
        target_link_libraries(comprehensive_api_test ${CURL_LIBRARIES})
    endif()
    if(REDIS_AVAILABLE)
        target_link_libraries(comprehensive_api_test ${HIREDIS_LIBRARY})
    endif()

    add_executable(qwen_model_test examples/qwen_model_test.cpp)
    target_link_libraries(qwen_model_test langchain_cpp ${SQLITE3_LIBRARIES})
    if(BRPC_AVAILABLE)
        target_link_libraries(qwen_model_test ${BRPC_LIBRARY})
    else()
        target_link_libraries(qwen_model_test ${CURL_LIBRARIES})
    endif()
    if(REDIS_AVAILABLE)
        target_link_libraries(qwen_model_test ${HIREDIS_LIBRARY})
    endif()

    add_executable(comprehensive_qwen_test examples/comprehensive_qwen_test.cpp)
    target_link_libraries(comprehensive_qwen_test langchain_cpp ${SQLITE3_LIBRARIES})
    if(BRPC_AVAILABLE)
        target_link_libraries(comprehensive_qwen_test ${BRPC_LIBRARY})
    else()
        target_link_libraries(comprehensive_qwen_test ${CURL_LIBRARIES})
    endif()
    if(REDIS_AVAILABLE)
        target_link_libraries(comprehensive_qwen_test ${HIREDIS_LIBRARY})
    endif()
endif()

# Enable testing
enable_testing()

# Add a simple test
add_executable(langchain_test tests/test_langchain.cpp)
target_link_libraries(langchain_test langchain_cpp ${SQLITE3_LIBRARIES})
if(BRPC_AVAILABLE)
    target_link_libraries(langchain_test ${BRPC_LIBRARY})
else()
    target_link_libraries(langchain_test ${CURL_LIBRARIES})
endif()
if(REDIS_AVAILABLE)
    target_link_libraries(langchain_test ${HIREDIS_LIBRARY})
endif()
add_test(NAME langchain_test COMMAND langchain_test)

# Add tools production test
add_executable(tools_production_test examples/tools_production_test.cpp)
target_link_libraries(tools_production_test langchain_cpp ${SQLITE3_LIBRARIES})
if(BRPC_AVAILABLE)
    target_link_libraries(tools_production_test ${BRPC_LIBRARY})
else()
    target_link_libraries(tools_production_test ${CURL_LIBRARIES})
endif()
if(REDIS_AVAILABLE)
    target_link_libraries(tools_production_test ${HIREDIS_LIBRARY})
endif()

# Add HTTP client test
add_executable(http_client_test examples/http_client_test.cpp)
target_link_libraries(http_client_test langchain_cpp ${SQLITE3_LIBRARIES})
if(BRPC_AVAILABLE)
    target_link_libraries(http_client_test ${BRPC_LIBRARY})
else()
    target_link_libraries(http_client_test ${CURL_LIBRARIES})
endif()
if(REDIS_AVAILABLE)
    target_link_libraries(http_client_test ${HIREDIS_LIBRARY})
endif()

# Add Data Connectors example
add_executable(data_connectors_example examples/data_connectors_example.cpp)
target_link_libraries(data_connectors_example langchain_cpp ${SQLITE3_LIBRARIES})
if(BRPC_AVAILABLE)
    target_link_libraries(data_connectors_example ${BRPC_LIBRARY})
else()
    target_link_libraries(data_connectors_example ${CURL_LIBRARIES})
endif()
if(REDIS_AVAILABLE)
    target_link_libraries(data_connectors_example ${HIREDIS_LIBRARY})
endif()

# Add Data Connectors test
add_executable(data_connectors_test examples/data_connectors_test.cpp)
target_link_libraries(data_connectors_test langchain_cpp ${SQLITE3_LIBRARIES})
if(BRPC_AVAILABLE)
    target_link_libraries(data_connectors_test ${BRPC_LIBRARY})
else()
    target_link_libraries(data_connectors_test ${CURL_LIBRARIES})
endif()
if(REDIS_AVAILABLE)
    target_link_libraries(data_connectors_test ${HIREDIS_LIBRARY})
endif()

# Add Advanced Retrieval example
add_executable(advanced_retrieval_example examples/advanced_retrieval_example.cpp)
target_link_libraries(advanced_retrieval_example langchain_cpp ${SQLITE3_LIBRARIES})
if(BRPC_AVAILABLE)
    target_link_libraries(advanced_retrieval_example ${BRPC_LIBRARY})
else()
    target_link_libraries(advanced_retrieval_example ${CURL_LIBRARIES})
endif()
if(REDIS_AVAILABLE)
    target_link_libraries(advanced_retrieval_example ${HIREDIS_LIBRARY})
endif()

# Add Comprehensive test
add_executable(comprehensive_test examples/comprehensive_test.cpp)
target_link_libraries(comprehensive_test langchain_cpp ${SQLITE3_LIBRARIES})
if(BRPC_AVAILABLE)
    target_link_libraries(comprehensive_test ${BRPC_LIBRARY})
else()
    target_link_libraries(comprehensive_test ${CURL_LIBRARIES})
endif()
if(REDIS_AVAILABLE)
    target_link_libraries(comprehensive_test ${HIREDIS_LIBRARY})
endif()

# Add Final Demo
add_executable(final_demo examples/final_demo.cpp)
target_link_libraries(final_demo langchain_cpp ${SQLITE3_LIBRARIES})
if(BRPC_AVAILABLE)
    target_link_libraries(final_demo ${BRPC_LIBRARY})
else()
    target_link_libraries(final_demo ${CURL_LIBRARIES})
endif()
if(REDIS_AVAILABLE)
    target_link_libraries(final_demo ${HIREDIS_LIBRARY})
endif()

# Add Redis memory example
if(REDIS_AVAILABLE)
    add_executable(redis_memory_example examples/redis_memory_example.cpp)
    target_link_libraries(redis_memory_example langchain_cpp ${SQLITE3_LIBRARIES})
    if(BRPC_AVAILABLE)
        target_link_libraries(redis_memory_example ${BRPC_LIBRARY} ${HIREDIS_LIBRARY})
    else()
        target_link_libraries(redis_memory_example ${CURL_LIBRARIES} ${HIREDIS_LIBRARY})
    endif()
endif()

# Print configuration status
message(STATUS "brpc available: ${BRPC_AVAILABLE}")
message(STATUS "nlohmann/json found: ${NLOHMANN_JSON_INCLUDE_DIR}")
message(STATUS "Redis support available: ${REDIS_AVAILABLE}")
message(STATUS "API models available: ${API_MODELS_AVAILABLE}")
if(NOT BRPC_AVAILABLE)
    message(STATUS "To enable brpc support, install brpc:")
    message(STATUS "  Ubuntu/Debian: sudo apt-get install libbrpc-dev")
    message(STATUS "  macOS: brew install brpc")
    message(STATUS "  Or download from: https://github.com/apache/brpc")
endif()
if(NOT NLOHMANN_JSON_INCLUDE_DIR)
    message(STATUS "To enable API model support, install nlohmann/json:")
    message(STATUS "  Ubuntu/Debian: sudo apt-get install nlohmann-json3-dev")
    message(STATUS "  macOS: brew install nlohmann-json")
    message(STATUS "  Or download from: https://github.com/nlohmann/json")
endif()
if(NOT REDIS_AVAILABLE)
    message(STATUS "To enable Redis support, install hiredis:")
    message(STATUS "  Ubuntu/Debian: sudo apt-get install libhiredis-dev")
    message(STATUS "  macOS: brew install hiredis")
endif()