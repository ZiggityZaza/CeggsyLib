#include "./cslib.h++"
#include <assert.h>



int main() {
    using namespace cslib;

    // Testing to_str/to_wstr functions
    assert(to_str(L"Hello World") == "Hello World");
    assert(to_str(std::wstring(L"Hello World")) == "Hello World");
    assert(to_str(std::wstring_view(L"Hello World")) == "Hello World");
    assert(to_str(int(123)) == "123");
    assert(to_str(float(3.14f)) == "3.14");
    assert(to_str(bool(true)) == "1");
    assert(to_str(std::filesystem::path("/path/to/file")) == "\"path/to/file\"");
    
    
}