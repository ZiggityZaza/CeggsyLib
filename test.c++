#include "./cslib.h++"
#include <cassert>


int main() {
  using namespace cslib;

  
  { // Testing to_str/to_wstr functions
    assert(to_str(L'A') == "A"); // to_str(wchar_t)
    assert(to_str(L"Hello World") == "Hello World"); // to_str(const wchar_t *const)
    assert(to_str(std::wstring(L"")) == ""); // to_str(const wstr_t&)
    assert(to_str(std::wstring(L"Hello World")) == "Hello World"); // to_str(const wstr_t&)
    assert(to_str(std::wstring_view(L"Hello World")) == "Hello World"); // to_str(const wstrv_t&)
    assert(to_str(123) == "123"); // to_str(const auto&)
    assert(to_str(3.14f) == "3.14"); // to_str(const auto&)
    assert(to_str(std::filesystem::path(L"/path/to/file")) == "\"/path/to/file\""); // to_str(const auto&)
    assert(to_str('A') == "A"); // to_str(const auto&)
    assert(to_str(std::string("")) == ""); // to_str(const auto&)
    assert(to_str(std::string("Hello World")) == "Hello World"); // to_str(const auto&)
    assert(to_str(std::string_view("Hello World")) == "Hello World"); // to_str(const auto&)

    assert(to_wstr('A') == L"A"); // to_wstr(char)
    assert(to_wstr("Hello World") == L"Hello World"); // to_wstr(const char *const)
    assert(to_wstr(std::string("")) == L""); // to_wstr(const str_t&)
    assert(to_wstr(std::string("Hello World")) == L"Hello World"); // to_wstr(const str_t&)
    assert(to_wstr(std::string_view("Hello World")) == L"Hello World"); // to_wstr(const strv_t&)
    assert(to_wstr(123) == L"123"); // to_wstr(const auto&)
    assert(to_wstr(3.14f) == L"3.14"); // to_wstr(const auto&)
    assert(to_wstr(std::filesystem::path("/path/to/file")) == L"\"/path/to/file\""); // to_wstr(const auto&)
    assert(to_wstr(L'A') == L"A"); // to_wstr(const auto&)
    assert(to_wstr(std::wstring(L"")) == L""); // to_wstr(const auto&)
    assert(to_wstr(std::wstring(L"Hello World")) == L"Hello World"); // to_wstr(const auto&)
    assert(to_wstr(std::wstring_view(L"Hello World")) == L"Hello World"); // to_wstr(const auto&)
  }



  { // Testing throw macro and up_impl function
    std::string shouldBeStdError = "std::runtime_error called from line ";
    shouldBeStdError += std::to_string(__LINE__ + 5); // Replace ??? with the line number where this assert is placed
    shouldBeStdError += " in workspace '";
    shouldBeStdError += std::filesystem::current_path().string();
    shouldBeStdError += "' because: \033[1m\033[31mError: CSLIB_TEST 1233.14\033[0m";
    try {
      throw_up("CSLIB", '_', L"TEST", L' ', 123, 3.14f);
    } catch (const std::runtime_error& e) {
      assert(e.what() == shouldBeStdError);
    }
  }



  { // Testing TinySTL::String
    using namespace TinySTL;

    // single wchar append and length
    String<10> s;
    assert(s.length() == 0);
    s.append(L'A');
    assert(s.length() == 1);
    assert(s.at(0) == L'A');
    s.append(L'B');
    assert(s.length() == 2);
    assert(s.at(1) == L'B');
    s.wipe_clean();
    assert(s.length() == 0);
    try {
      s.at(0); // Should throw
      assert(false); // If we reach here, the test failed
    } catch (const std::runtime_error& e) {
      assert(std::string(e.what()).find("Index out of bounds: 0 >= 10") != std::string::npos);
    }

    // Append
    String<10> s2;
    wstrv_t wsv = L"Test";
    s2.append(wsv);
    assert(s2.length() == 4);
    assert(s2 == wsv);
    try {
      s2.append(L"Longer than capacity");
      assert(false); // If we reach here, the test failed
    } catch (const std::runtime_error& e) {
      assert(std::string(e.what()).find("Appending string exceeds capacity (current: 4, appending: 20)") != std::string::npos);
    }

    // Constructor string view
    strv_t sv = "abc";
    String<5> s3(sv);
    assert(s3.length() == 3);
    assert(s3 == L"abc");
    try {
      String<1> s4(sv); // Should throw
      assert(false); // If we reach here, the test failed
    } catch (const std::runtime_error& e) {
      assert(std::string(e.what()).find("Constructing wide string view with size 1 exceeds capacity 3") != std::string::npos);
    }

    // Constructor wide string view
    wstrv_t wsv = L"xyz";
    String<5> s4(wsv);
    assert(s4.length() == 3);
    assert(s4 == L"xyz");

    // String equality
    String<10> s5(L"_A");
    String<10> s6(L"_A");
    String<10> s7(L"_B");
    assert(s5 == s6);
    assert(s5 != s7);
    assert(s5 == L"_A");
    assert(s5 != L"_B");

    // String stl conversion
    String<10> s8(L"abc");
    str_t stl_str = static_cast<str_t>(s8);
    wstr_t stl_wstr = static_cast<wstr_t>(s8);
    assert(stl_str == "abc");
    assert(stl_wstr == L"abc");

    // Copy/Move operations
    String<10> s9(L"copy");
    String<10> s10(s9);
    assert(s10 == s9);
    String<10> s11(std::move(s9));
    assert(s11 == L"copy");
    assert(s9.length() == 0); // s9 should be empty after move
  }
}