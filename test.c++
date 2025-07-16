#include "./cslib.h++"
#include <cassert>


void log(bool conditionResult, std::string_view conditionExplained) {
  static std::ofstream logs;
  if (!logs.is_open()) {
    logs.open("cslib_test.log");
    logs << "New test at the '";
    std::time_t now = std::time(nullptr);
    logs << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S") << "'\n";
  }
  logs << conditionExplained << (conditionResult ? " [PASSED]" : " [FAILED]") << std::endl;
}



int main() {
  using namespace cslib;
  
  { log(true, "Testing cslib::to_str and cslib::to_wstr");
    log(to_str(L'A') == "A", "to_str(wchar_t)");
    log(to_str(L"Hello World") == "Hello World", "to_str(const wchar_t *const)");
    log(to_str(std::wstring(L"")) == "", "to_str(const wstr_t&)");
    log(to_str(std::wstring(L"Hello World")) == "Hello World", "to_str(const wstr_t&)");
    log(to_str(std::wstring_view(L"Hello World")) == "Hello World", "to_str(const wstrv_t&)");
    log(to_str(123) == "123", "to_str(const auto&)");
    log(to_str(3.14f) == "3.14", "to_str(const auto&)");
    log(to_str(std::filesystem::path(L"/path/to/file")) == "\"/path/to/file\"", "to_str(const auto&)");
    log(to_str('A') == "A", "to_str(const auto&)");
    log(to_str(std::string("")) == "", "to_str(const auto&)");
    log(to_str(std::string("Hello World")) == "Hello World", "to_str(const auto&)");
    log(to_str(std::string_view("Hello World")) == "Hello World", "to_str(const auto&)");

    log(to_wstr('A') == L"A", "to_wstr(char)");
    log(to_wstr("Hello World") == L"Hello World", "to_wstr(const char *const)");
    log(to_wstr(std::string("")) == L"", "to_wstr(const str_t&)");
    log(to_wstr(std::string("Hello World")) == L"Hello World", "to_wstr(const str_t&)");
    log(to_wstr(std::string_view("Hello World")) == L"Hello World", "to_wstr(const strv_t&)");
    log(to_wstr(123) == L"123", "to_wstr(const auto&)");
    log(to_wstr(3.14f) == L"3.14", "to_wstr(const auto&)");
    log(to_wstr(std::filesystem::path("/path/to/file")) == L"\"/path/to/file\"", "to_wstr(const auto&)");
    log(to_wstr(L'A') == L"A", "to_wstr(const auto&)");
    log(to_wstr(std::wstring(L"")) == L"", "to_wstr(const auto&)");
    log(to_wstr(std::wstring(L"Hello World")) == L"Hello World", "to_wstr(const auto&)");
    log(to_wstr(std::wstring_view(L"Hello World")) == L"Hello World", "to_wstr(const auto&)");
  }



  { // Testing throw macro and up_impl function
    std::string shouldBeStdError = "std::runtime_error called from line ";
    shouldBeStdError += std::to_string(__LINE__ + 5);
    shouldBeStdError += " in workspace '";
    shouldBeStdError += std::filesystem::current_path().string();
    shouldBeStdError += "' because: \033[1m\033[31mError: CSLIB_TEST 1233.14\033[0m";
    try {
      throw_up("CSLIB", '_', L"TEST", L' ', 123, 3.14f);
    } catch (const std::runtime_error& e) {
      log(e.what() == shouldBeStdError, "throw_up with up_impl");
    }
  }



  { log(true, "Testing TinySTL::String");
    using namespace TinySTL;

    // single wchar append and length
    String<10> s;
    log(s.length() == 0, "Initial length should be 0");
    s.append(L'A');
    log(s.length() == 1, "Length should be 1 after appending 'A'");
    log(s.at(0) == L'A', "First character should be 'A'");
    s.append(L'B');
    log(s.length() == 2, "Length should be 2 after appending 'B'");
    log(s.at(1) == L'B', "Second character should be 'B'");
    s.wipe_clean();
    log(s.length() == 0, "Length should be 0 after wipe_clean()");
    try {
      s.at(1); // Should throw
      log(false, "Expected out of bounds exception for at(1) after wipe_clean()");
    } catch (const std::runtime_error& e) {
      // Continue
    }

    // Append
    String<10> s2;
    wstrv_t wsv = L"Test";
    s2.append(wsv);
    log(s2.length() == 4, "Length should be 4 after appending 'Test'");
    log(s2 == wsv, "String should be equal to appended wide string view");
    try {
      s2.append(L"Longer than capacity");
      log(false, "Expected exception for appending beyond capacity");
    } catch (const std::runtime_error& e) {
      // Continue
    }

    // Constructor string view
    strv_t sv = "abc";
    String<5> s3(sv);
    log(s3.length() == 3, "Length should be 3 after constructing from string view");
    log(s3 == L"abc", "String should be equal to 'abc'");
    try {
      String<1> s4(sv); // Should throw
      log(false, "Expected exception for constructing from string view with insufficient capacity");
    } catch (const std::runtime_error& e) {
      // Continue
    }

    // Constructor wide string view
    wsv = L"xyz";
    String<5> s4(wsv);
    log(s4.length() == 3, "Length should be 3 after constructing from wide string view");
    log(s4 == L"xyz", "String should be equal to 'xyz'");

    // String equality
    String<10> s5(L"_A");
    String<10> s6(L"_A");
    String<10> s7(L"_B");
    log(s5 == s6, "Strings should be equal");
    log(s5 != s7, "Strings should not be equal");
    log(s5 == L"_A", "String should be equal to '_A'");
    log(s5 != L"_B", "String should not be equal to '_B'");

    // String stl conversion
    String<10> s8(L"abc");
    str_t stl_str = static_cast<str_t>(s8);
    wstr_t stl_wstr = static_cast<wstr_t>(s8);
    log(stl_str == "abc", "STL string should be equal to 'abc'");
    log(stl_wstr == L"abc", "STL wide string should be equal to 'abc'");

    // Copy/Move operations
    String<10> s9(L"copy");
    String<10> s10(s9);
    log(s10 == s9, "Copy constructor should create equal strings");
    String<10> s11(std::move(s9));
    log(s11 == L"copy", "Move constructor should transfer ownership");
    log(s9.length() == 0, "Source string should be empty after move");

    // Out of bounds handling
    try {
      String<10>(L"abc").at(10); // Should throw
      log(false, "Expected out of bounds exception for at(10)");
    } catch (const std::runtime_error& e) {
      // Continue
    }

    // Append beyond capacity
    try {
      String<5> s12;
      s12.append(L"123456"); // Should throw
      log(false, "Expected exception for appending beyond capacity");
    } catch (const std::runtime_error& e) {
      // Continue
    }
  }



  { log(true, "Testing TinySTL::Vector");
    using namespace TinySTL;

    // Basics
    Vector<int> vec;
    log(vec.size == 0, "Initial size should be 0");
    vec.push_back(1);
    log(vec.size == 1, "Size should be 1 after push_back(1)");
    log(vec[0] == 1, "First element should be 1");
    vec.push_back(2);
    log(vec.size == 2, "Size should be 2 after push_back(2)");
    log(vec[1] == 2, "Second element should be 2");
    vec.push_back(3);
    log(vec.size == 3, "Size should be 3 after push_back(3)");
    log(vec[2] == 3, "Third element should be 3");
    vec[1] = 42;
    log(vec[1] == 42, "Second element should be updated to 42");
    int popped = vec.pop_back();
    log(popped == 3, "Popped element should be 3");
    log(vec.size == 2, "Size should be 2 after pop_back()");
    log(vec[0] == 1, "First element should still be 1");

    // Initializer list
    Vector<int> vec2 = {1, 2, 3};
    log(vec2.size == 3, "Size should be 3 after initializer list");
    log(vec2[0] == 1 and vec2[1] == 2 and vec2[2] == 3, "Elements should match initializer list");
    log(vec2 == std::initializer_list<int>{1, 2, 3}, "Vector should match initializer list");
    log(vec2 != std::initializer_list<int>{1, 2, 4}, "Vector should not match different initializer list");
    log(vec2 == std::vector<int>{1, 2, 3}, "Vector should match std::vector");
    log(vec2 != std::vector<int>{1, 2, 4}, "Vector should not match different std::vector");

    // Copy/Move operations
    Vector<int> vec3{10,20,30};
    Vector<int> vec4(vec3);
    log(vec4.size == 3, "Copy constructor should create equal size vectors");
    log(vec4[0] == 10 and vec4[1] == 20 and vec4[2] == 30, "Copy constructor should create equal vector elements");
    Vector<int> vec5(std::move(vec3));
    log(vec5.size == 3, "Move constructor should create equal size vectors");
    log(vec5[0] == 10 and vec5[1] == 20 and vec5[2] == 30, "Move constructor should create equal vector elements");
    Vector<int> vec6;
    vec6 = vec5;
    log(vec6.size == 3, "Copy assignment should create equal size vectors");
    log(vec6[0] == 10 and vec6[1] == 20 and vec6[2] == 30, "Copy assignment should create equal vector elements");
    Vector<int> vec7;
    vec7 = std::move(vec5);
    log(vec7.size == 3, "Move assignment should create equal size vectors");
    log(vec7[0] == 10 and vec7[1] == 20 and vec7[2] == 30, "Move assignment should create equal vector elements");

    // Equality checks
    Vector<int> vec8 = {1, 2, 3};
    Vector<int> vec9 = {1, 2, 3};
    Vector<int> vec10 = {1, 2, 4};
    log(vec8 == vec9, "Vectors should be equal");
    log(!(vec8 != vec9), "Vectors should not be unequal");
    log(vec8 != vec10, "Vectors should not be equal");
    log(vec8 == std::initializer_list<int>{1, 2, 3}, "Vector should be equal to initializer list");
    log(vec8 != std::initializer_list<int>{1, 2, 4}, "Vector should not be equal to different initializer list");
    log(vec8 == std::vector<int>{1, 2, 3}, "Vector should be equal to std::vector");
    log(vec8 != std::vector<int>{1, 2, 4}, "Vector should not be equal to different std::vector");
    
    // Transform to STL vector
    std::vector<int> stl_vec = static_cast<std::vector<int>>(vec8);
    log(stl_vec.size() == 3, "STL vector size should be 3");
    log(stl_vec[0] == 1 and stl_vec[1] == 2 and stl_vec[2] == 3, "STL vector elements should match TinySTL vector");
    std::vector<int> stl_vec_move = static_cast<std::vector<int>&&>(std::move(vec8));
    log(stl_vec_move.size() == 3, "Moved STL vector size should be 3");
    log(stl_vec_move[0] == 1 and stl_vec_move[1] == 2 and stl_vec_move[2] == 3, "Moved STL vector elements should match TinySTL vector");

    // Capacity and bound checks
    Vector<int> vec11;
    for (int i = 0; i < 10; ++i) {
      vec11.push_back(i);
      log(vec11.size == i + 1, "Size should be " + std::to_string(i + 1) + " after pushing " + std::to_string(i));
      log(vec11[i] == i, "Element at index " + std::to_string(i) + " should be " + std::to_string(i));
    }
    try {
      vec11.push_back(10); // Should trigger capacity increment
      log(vec11.size == 11, "Size should be 11 after pushing 10");
      log(vec11[10] == 10, "Element at index 10 should be 10");
    } catch (const std::runtime_error& e) {
      log(false, "Unexpected exception during push_back: " + std::string(e.what()));
    }
    try {
      vec11.pop_back();
      log(vec11.size == 10, "Size should be 10 after pop_back()");
      log(vec11[9] == 9, "Element at index 9 should be 9");
    } catch (const std::runtime_error& e) {
      log(false, "Unexpected exception during pop_back: " + std::string(e.what()));
    }
    
  }
}