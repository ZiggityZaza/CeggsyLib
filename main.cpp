#include <iostream>
#include "a.hpp"
#include "b.hpp"
int main() {
    std::cout << "Result from a: " << wrap() << std::endl;
    std::cout << "Result from b: " << wrap_again() << std::endl;
    std::cout << "Test count: " << Test::count << std::endl;
}