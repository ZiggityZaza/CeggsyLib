#include <iostream>
#include <string>

int main() {
    std::string text = "Hello, world! 🌍🚀";
    std::cout << text << "🧌" << std::endl;
    throw std::runtime_error("test");
}