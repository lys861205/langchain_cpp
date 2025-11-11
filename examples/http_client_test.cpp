#include "../include/langchain/http_client.h"
#include <iostream>

int main() {
    std::cout << "HTTP Client Test\n";
    std::cout << "================\n\n";

    langchain::HttpClient client;

    // Test GET request
    std::cout << "Testing GET request to httpbin.org...\n";
    auto response = client.get("https://httpbin.org/get");

    std::cout << "Status Code: " << response.status_code << std::endl;
    std::cout << "Response Body: " << response.body << std::endl;

    if (response.status_code == 200) {
        std::cout << "GET request successful!\n";
    } else {
        std::cout << "GET request failed.\n";
    }

    // Test POST request
    std::cout << "\nTesting POST request to httpbin.org...\n";
    std::string post_data = "{\"test\": \"data\"}";
    std::map<std::string, std::string> headers;
    headers["Content-Type"] = "application/json";

    auto post_response = client.post("https://httpbin.org/post", post_data, headers);

    std::cout << "Status Code: " << post_response.status_code << std::endl;
    if (post_response.status_code == 200) {
        std::cout << "POST request successful!\n";
    } else {
        std::cout << "POST request failed.\n";
    }

    return 0;
}