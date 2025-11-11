#ifndef LANGCHAIN_TOOLS_H
#define LANGCHAIN_TOOLS_H

#include "core.h"
#include "mcp.h"
#include "http_client.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <chrono>

namespace langchain {

// Calculator tool
class CalculatorTool : public Tool {
public:
    CalculatorTool();
    String execute(const String& input) override;

private:
    double evaluate_expression(const String& expr);
};

// String manipulation tool
class StringTool : public Tool {
private:
    String operation_;

public:
    StringTool(const String& operation = "uppercase");
    String execute(const String& input) override;
    void set_operation(const String& operation);
};

// Search tool with multiple search engine support
class SearchTool : public Tool {
public:
    enum class SearchEngine {
        DUCKDUCKGO,
        BAIDU,
        GOOGLE,
        BING
    };

private:
    SearchEngine engine_;

    String search_duckduckgo(const String& query);
    String search_baidu(const String& query);
    String search_google(const String& query);
    String search_bing(const String& query);

public:
    SearchTool(SearchEngine engine = SearchEngine::DUCKDUCKGO);
    String execute(const String& input) override;
    void set_search_engine(SearchEngine engine);
};

// Current time tool (mock implementation)
class TimeTool : public Tool {
public:
    TimeTool();
    String execute(const String& input) override;
};

} // namespace langchain

#endif // LANGCHAIN_TOOLS_H