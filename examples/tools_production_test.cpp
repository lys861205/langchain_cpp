#include <iostream>
#include <memory>
#include "../include/langchain/langchain.h"

using namespace langchain;

int main() {
    std::cout << "LangChain C++ Production Tools Test\n";
    std::cout << "===================================\n\n";

    // Test CalculatorTool
    std::cout << "--- Testing CalculatorTool ---\n";
    auto calculator = std::make_shared<CalculatorTool>();

    std::vector<std::string> calc_tests = {
        "2 + 3",
        "10 - 4",
        "5 * 6",
        "20 / 4",
        "2 * 3 + 4",
        "(2 + 3) * 4"
    };

    for (const auto& test : calc_tests) {
        std::string result = calculator->execute(test);
        std::cout << test << " = " << result << "\n";
    }
    std::cout << "\n";

    // Test StringTool
    std::cout << "--- Testing StringTool ---\n";
    auto string_tool = std::make_shared<StringTool>();

    std::string test_string = "Hello World";
    std::cout << "Original: " << test_string << "\n";

    string_tool->set_operation("uppercase");
    std::cout << "Uppercase: " << string_tool->execute(test_string) << "\n";

    string_tool->set_operation("lowercase");
    std::cout << "Lowercase: " << string_tool->execute(test_string) << "\n";

    string_tool->set_operation("reverse");
    std::cout << "Reverse: " << string_tool->execute(test_string) << "\n";

    string_tool->set_operation("length");
    std::cout << "Length: " << string_tool->execute(test_string) << "\n";
    std::cout << "\n";

    // Test TimeTool
    std::cout << "--- Testing TimeTool ---\n";
    auto time_tool = std::make_shared<TimeTool>();
    std::string time_result = time_tool->execute("");
    std::cout << time_result << "\n\n";

    // Test SearchTool
    std::cout << "--- Testing SearchTool ---\n";

    // Test DuckDuckGo
    std::cout << "Testing DuckDuckGo search engine:\n";
    auto search_tool_ddg = std::make_shared<SearchTool>(SearchTool::SearchEngine::DUCKDUCKGO);
    std::vector<std::string> search_queries = {
        "artificial intelligence",
        "python programming language"
    };

    for (const auto& query : search_queries) {
        std::cout << "Searching for: " << query << "\n";
        std::string result = search_tool_ddg->execute(query);
        std::cout << result << "\n\n";
    }

    // Test Baidu
    std::cout << "Testing Baidu search engine:\n";
    auto search_tool_baidu = std::make_shared<SearchTool>(SearchTool::SearchEngine::BAIDU);
    for (const auto& query : search_queries) {
        std::cout << "Searching for: " << query << "\n";
        std::string result = search_tool_baidu->execute(query);
        std::cout << result << "\n\n";
    }

    // Test Google
    std::cout << "Testing Google search engine:\n";
    auto search_tool_google = std::make_shared<SearchTool>(SearchTool::SearchEngine::GOOGLE);
    for (const auto& query : search_queries) {
        std::cout << "Searching for: " << query << "\n";
        std::string result = search_tool_google->execute(query);
        std::cout << result << "\n\n";
    }

    // Test Bing
    std::cout << "Testing Bing search engine:\n";
    auto search_tool_bing = std::make_shared<SearchTool>(SearchTool::SearchEngine::BING);
    for (const auto& query : search_queries) {
        std::cout << "Searching for: " << query << "\n";
        std::string result = search_tool_bing->execute(query);
        std::cout << result << "\n\n";
    }

    // Test dynamic engine switching
    std::cout << "Testing dynamic engine switching:\n";
    auto search_tool_dynamic = std::make_shared<SearchTool>(SearchTool::SearchEngine::DUCKDUCKGO);
    std::string test_query = "machine learning";

    search_tool_dynamic->set_search_engine(SearchTool::SearchEngine::BAIDU);
    std::cout << "Baidu search for: " << test_query << "\n";
    std::cout << search_tool_dynamic->execute(test_query) << "\n\n";

    search_tool_dynamic->set_search_engine(SearchTool::SearchEngine::GOOGLE);
    std::cout << "Google search for: " << test_query << "\n";
    std::cout << search_tool_dynamic->execute(test_query) << "\n\n";

    std::cout << "Production tools test completed successfully!\n";

    return 0;
}