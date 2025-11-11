#include "../include/langchain/tools.h"
#include "../include/langchain/http_client.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <chrono>

namespace langchain {


// CalculatorTool implementation
CalculatorTool::CalculatorTool()
    : Tool("calculator", "A simple calculator that can perform basic arithmetic operations") {}

String CalculatorTool::execute(const String& input) {
    try {
        // Simple calculator implementation
        // This is a very basic implementation for demonstration purposes
        // In a real implementation, you would want a proper expression parser

        // Remove spaces
        String expr = input;
        expr.erase(std::remove_if(expr.begin(), expr.end(), ::isspace), expr.end());

        // Simple evaluation for basic operations
        double result = evaluate_expression(expr);
        return std::to_string(result);
    } catch (const std::exception& e) {
        return "Error: Invalid expression";
    }
}

double CalculatorTool::evaluate_expression(const String& expr) {
    // More robust implementation that handles complex expressions
    // This is still a simplified version but handles more cases

    // Remove spaces
    String clean_expr = expr;
    clean_expr.erase(std::remove_if(clean_expr.begin(), clean_expr.end(), ::isspace), clean_expr.end());

    // Handle parentheses first (recursive)
    size_t open_paren = clean_expr.find('(');
    if (open_paren != String::npos) {
        size_t close_paren = clean_expr.rfind(')');
        if (close_paren != String::npos && close_paren > open_paren) {
            String inner_expr = clean_expr.substr(open_paren + 1, close_paren - open_paren - 1);
            String result_str = std::to_string(evaluate_expression(inner_expr));

            String new_expr = clean_expr.substr(0, open_paren) + result_str +
                             clean_expr.substr(close_paren + 1);
            return evaluate_expression(new_expr);
        }
    }

    // Handle multiplication and division first (left to right)
    for (size_t i = 0; i < clean_expr.length(); ++i) {
        char op = clean_expr[i];
        if (op == '*' || op == '/') {
            // Find left operand
            size_t left_start = i;
            while (left_start > 0 && (std::isdigit(clean_expr[left_start-1]) ||
                   clean_expr[left_start-1] == '.' || clean_expr[left_start-1] == '-')) {
                left_start--;
            }
            String left_str = clean_expr.substr(left_start, i - left_start);

            // Find right operand
            size_t right_end = i + 1;
            if (right_end < clean_expr.length() && clean_expr[right_end] == '-') {
                right_end++;
            }
            while (right_end < clean_expr.length() &&
                   (std::isdigit(clean_expr[right_end]) || clean_expr[right_end] == '.')) {
                right_end++;
            }
            String right_str = clean_expr.substr(i + 1, right_end - i - 1);

            double left = std::stod(left_str);
            double right = std::stod(right_str);

            double result;
            if (op == '*') {
                result = left * right;
            } else {
                if (right == 0) throw std::runtime_error("Division by zero");
                result = left / right;
            }

            String result_str = std::to_string(result);
            String new_expr = clean_expr.substr(0, left_start) + result_str +
                             clean_expr.substr(right_end);
            return evaluate_expression(new_expr);
        }
    }

    // Handle addition and subtraction (left to right)
    for (size_t i = 0; i < clean_expr.length(); ++i) {
        char op = clean_expr[i];
        if (op == '+' || op == '-') {
            // For subtraction at the beginning, treat it as a negative number
            if (op == '-' && i == 0) {
                // Check if the entire expression is just a negative number
                bool is_negative_number = true;
                for (size_t j = 1; j < clean_expr.length(); ++j) {
                    if (clean_expr[j] == '+' || clean_expr[j] == '-' || clean_expr[j] == '*' || clean_expr[j] == '/') {
                        is_negative_number = false;
                        break;
                    }
                }
                if (is_negative_number) {
                    // This is just a negative number
                    return std::stod(clean_expr);
                }
                // If this is a subtraction at the beginning, continue to find the next operator
                continue;
            }

            // Find left operand
            size_t left_start = i;
            while (left_start > 0 && (std::isdigit(clean_expr[left_start-1]) ||
                   clean_expr[left_start-1] == '.')) {
                left_start--;
            }
            // Handle negative sign for left operand
            if (left_start > 0 && clean_expr[left_start-1] == '-') {
                left_start--;
            }
            String left_str = clean_expr.substr(left_start, i - left_start);

            // Find right operand
            size_t right_end = i + 1;
            // Handle negative sign for right operand
            if (right_end < clean_expr.length() && clean_expr[right_end] == '-') {
                right_end++;
            }
            while (right_end < clean_expr.length() &&
                   (std::isdigit(clean_expr[right_end]) || clean_expr[right_end] == '.')) {
                right_end++;
            }
            String right_str = clean_expr.substr(i + 1, right_end - i - 1);

            double left = std::stod(left_str);
            double right = std::stod(right_str);

            double result;
            if (op == '+') {
                result = left + right;
            } else {
                result = left - right;
            }

            String result_str = std::to_string(result);
            String new_expr = clean_expr.substr(0, left_start) + result_str +
                             clean_expr.substr(right_end);
            return evaluate_expression(new_expr);
        }
    }

    // If no operator found, try to convert to number
    return std::stod(clean_expr);
}

// StringTool implementation
StringTool::StringTool(const String& operation)
    : Tool("string_tool", "A tool for string manipulation operations"), operation_(operation) {}

String StringTool::execute(const String& input) {
    if (operation_ == "uppercase") {
        String result = input;
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        return result;
    } else if (operation_ == "lowercase") {
        String result = input;
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    } else if (operation_ == "reverse") {
        String result = input;
        std::reverse(result.begin(), result.end());
        return result;
    } else if (operation_ == "length") {
        return std::to_string(input.length());
    } else {
        return "Unknown operation: " + operation_;
    }
}

void StringTool::set_operation(const String& operation) {
    operation_ = operation;
}

// SearchTool implementation
SearchTool::SearchTool(SearchEngine engine)
    : Tool("search", "A tool that searches for information using various search engines"), engine_(engine) {}

String SearchTool::execute(const String& input) {
    switch (engine_) {
        case SearchEngine::DUCKDUCKGO:
            return search_duckduckgo(input);
        case SearchEngine::BAIDU:
            return search_baidu(input);
        case SearchEngine::GOOGLE:
            return search_google(input);
        case SearchEngine::BING:
            return search_bing(input);
        default:
            return "Error: Unknown search engine";
    }
}

void SearchTool::set_search_engine(SearchEngine engine) {
    engine_ = engine;
}

String SearchTool::search_duckduckgo(const String& query) {
    // 使用HttpClient替换curl
    HttpClient http_client;

    // URL编码查询
    // 简化处理，实际应该使用proper URL编码
    std::string encoded_query = query;
    // 简单替换空格为+
    size_t pos = 0;
    while ((pos = encoded_query.find(" ", pos)) != std::string::npos) {
        encoded_query.replace(pos, 1, "+");
        pos += 1;
    }

    std::string url = "https://api.duckduckgo.com/?q=" + encoded_query + "&format=json&no_html=1&skip_disambig=1";

    // 准备请求头
    std::map<std::string, std::string> headers;
    headers["User-Agent"] = "LangChain-CPP/1.0";

    // 发送GET请求
    auto response = http_client.get(url, headers, 10);

    if (response.status_code != 200) {
        return "Error: Failed to perform search - HTTP " + std::to_string(response.status_code);
    }

    // 简单解析DuckDuckGo响应
    if (response.body.find("\"AbstractText\"") != std::string::npos) {
        size_t start = response.body.find("\"AbstractText\":\"");
        if (start != std::string::npos) {
            start += 16; // Length of "\"AbstractText\":\""
            size_t end = response.body.find("\"", start);
            if (end != std::string::npos) {
                std::string abstract = response.body.substr(start, end - start);
                // 简单处理转义字符
                size_t pos = 0;
                while ((pos = abstract.find("\\\"", pos)) != std::string::npos) {
                    abstract.replace(pos, 2, "\"");
                    pos += 1;
                }
                if (!abstract.empty()) {
                    return "Search results for '" + query + "': " + abstract;
                }
            }
        }
    }

    return "Search results for '" + query + "': No detailed information found. "
           "Try rephrasing your query or check if the topic is too specific.";
}

String SearchTool::search_baidu(const String& query) {
    HttpClient http_client;

    // URL编码查询
    std::string encoded_query = query;
    size_t pos = 0;
    while ((pos = encoded_query.find(" ", pos)) != std::string::npos) {
        encoded_query.replace(pos, 1, "+");
        pos += 1;
    }

    std::string url = "https://www.baidu.com/s?wd=" + encoded_query + "&ie=utf-8";

    // 准备请求头
    std::map<std::string, std::string> headers;
    headers["User-Agent"] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36";

    // 发送GET请求
    auto response = http_client.get(url, headers, 10);

    if (response.status_code != 200) {
        return "Error: Failed to perform search - HTTP " + std::to_string(response.status_code);
    }

    // 简单解析Baidu响应
    if (response.body.find("<h3") != std::string::npos) {
        size_t start = response.body.find("<h3");
        if (start != std::string::npos) {
            // 尝试提取第一个搜索结果标题
            size_t title_start = response.body.find(">", start);
            if (title_start != std::string::npos) {
                title_start++;
                size_t title_end = response.body.find("<", title_start);
                if (title_end != std::string::npos) {
                    std::string title = response.body.substr(title_start, title_end - title_start);
                    // 移除HTML实体并清理
                    size_t pos = 0;
                    while ((pos = title.find("&nbsp;", pos)) != std::string::npos) {
                        title.replace(pos, 6, " ");
                    }
                    pos = 0;
                    while ((pos = title.find("&amp;", pos)) != std::string::npos) {
                        title.replace(pos, 5, "&");
                    }
                    if (!title.empty()) {
                        return "Baidu search results for '" + query + "': " + title + "...";
                    }
                }
            }
        }
    }

    return "Baidu search results for '" + query + "': Search completed but no detailed results extracted.";
}

String SearchTool::search_google(const String& query) {
    HttpClient http_client;

    // URL编码查询
    std::string encoded_query = query;
    size_t pos = 0;
    while ((pos = encoded_query.find(" ", pos)) != std::string::npos) {
        encoded_query.replace(pos, 1, "+");
        pos += 1;
    }

    std::string url = "https://www.google.com/search?q=" + encoded_query + "&ie=utf-8&as_qdr=all&as_sitesearch=&safe=off";

    // 准备请求头
    std::map<std::string, std::string> headers;
    headers["User-Agent"] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36";

    // 发送GET请求
    auto response = http_client.get(url, headers, 10);

    if (response.status_code != 200) {
        return "Error: Failed to perform search - HTTP " + std::to_string(response.status_code);
    }

    // 简单解析Google响应
    if (response.body.find("<h3") != std::string::npos) {
        size_t start = response.body.find("<h3");
        if (start != std::string::npos) {
            // 尝试提取第一个搜索结果标题
            size_t title_start = response.body.find(">", start);
            if (title_start != std::string::npos) {
                title_start++;
                size_t title_end = response.body.find("<", title_start);
                if (title_end != std::string::npos) {
                    std::string title = response.body.substr(title_start, title_end - title_start);
                    if (!title.empty()) {
                        return "Google search results for '" + query + "': " + title + "...";
                    }
                }
            }
        }
    }

    return "Google search results for '" + query + "': Search completed but no detailed results extracted.";
}

String SearchTool::search_bing(const String& query) {
    HttpClient http_client;

    // URL编码查询
    std::string encoded_query = query;
    size_t pos = 0;
    while ((pos = encoded_query.find(" ", pos)) != std::string::npos) {
        encoded_query.replace(pos, 1, "+");
        pos += 1;
    }

    std::string url = "https://www.bing.com/search?q=" + encoded_query;

    // 准备请求头
    std::map<std::string, std::string> headers;
    headers["User-Agent"] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36";

    // 发送GET请求
    auto response = http_client.get(url, headers, 10);

    if (response.status_code != 200) {
        return "Error: Failed to perform search - HTTP " + std::to_string(response.status_code);
    }

    // 简单解析Bing响应
    if (response.body.find("<h2") != std::string::npos) {
        size_t start = response.body.find("<h2");
        if (start != std::string::npos) {
            // 尝试提取第一个搜索结果标题
            size_t title_start = response.body.find(">", start);
            if (title_start != std::string::npos) {
                title_start++;
                size_t title_end = response.body.find("<", title_start);
                if (title_end != std::string::npos) {
                    std::string title = response.body.substr(title_start, title_end - title_start);
                    if (!title.empty()) {
                        return "Bing search results for '" + query + "': " + title + "...";
                    }
                }
            }
        }
    }

    return "Bing search results for '" + query + "': Search completed but no detailed results extracted.";
}

// TimeTool implementation
TimeTool::TimeTool()
    : Tool("time", "A tool that returns the current time") {}

String TimeTool::execute(const String& input) {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");

    return "Current time: " + ss.str();
}

} // namespace langchain