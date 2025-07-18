#include "./cslib.h++"
using namespace cslib;
#include <cassert>

void log(bool conditionResult, std::string_view conditionExplained) {
  if (!isWcharIOEnabled)
    enable_wchar_io();
  if (conditionResult)
    std::wcout << L"\033[1;32m[PASSED]\033[0m";
  else
    std::wcout << L"\033[1;31m[FAILED]\033[0m";
  std::wcout << " > " << conditionExplained.data() << L'\n';
}
int main() {

  { log(true, "Testing cslib::to_str");
    log(to_str(L'A') == "A", "to_str(wchar_t)");
    log(to_str(L"Hello World") == "Hello World", "to_str(const wchar_t *const)");
    log(to_str(std::wstring(L"")) == "", "to_str(const wstr_t&) empty");
    log(to_str(std::wstring(L"Hello World")) == "Hello World", "to_str(const wstr_t&) short string");
    log(to_str(std::wstring(L"This is a pretty long string")) == "This is a pretty long string", "to_str(const wstr_t&) long string");
    log(to_str(std::wstring_view(L"Hello World")) == "Hello World", "to_str(const wstrv_t&)");
    log(to_str(123) == "123", "to_str(const auto&) int");
    log(to_str(3.14f) == "3.14", "to_str(const auto&) float");
    log(to_str(3.14) == "3.14", "to_str(const auto&) double");
    log(to_str(std::filesystem::path(L"/path/to/file")) == "\"/path/to/file\"", "to_str(const auto&)");
    log(to_str('A') == "A", "to_str(const auto&)");
    log(to_str(std::string("")) == "", "to_str(const auto&)");
    log(to_str(std::string("Hello World")) == "Hello World", "to_str(const auto&)");
    log(to_str(std::string("This is a pretty long string")) == "This is a pretty long string", "to_str(const auto&) long string");
    log(to_str(std::string_view("Hello World")) == "Hello World", "to_str(const auto&)");
    static_assert(to_str(L'A') == "A", "to_str(wchar_t) constexpr");
    static_assert(to_str(L"Hello World") == "Hello World", "to_str(const wchar_t *const) constexpr");
    static_assert(to_str(std::wstring(L"")) == "", "to_str(const wstr_t&) empty constexpr");
    static_assert(to_str(std::wstring(L"Hello World")) == "Hello World", "to_str(const wstr_t&) short string constexpr");
    static_assert(to_str(std::wstring(L"This is a pretty long string")) == "This is a pretty long string", "to_str(const wstr_t&) long string constexpr");
    static_assert(to_str(std::wstring_view(L"Hello World")) == "Hello World", "to_str(const wstrv_t&) constexpr");
    // Cant constexpr to_str(const auto&) because it uses std::ostringstream
  }



  { log(true, "Testing cslib::to_wstr");
    log(to_wstr('A') == L"A", "to_wstr(char)");
    log(to_wstr("Hello World") == L"Hello World", "to_wstr(const char *const)");
    log(to_wstr(std::string("")) == L"", "to_wstr(const str_t&) empty");
    log(to_wstr(std::string("Hello World")) == L"Hello World", "to_wstr(const str_t&) short string");
    log(to_wstr(std::string("This is a pretty long string")) == L"This is a pretty long string", "to_wstr(const str_t&) long string");
    log(to_wstr(std::string_view("Hello World")) == L"Hello World", "to_wstr(const strv_t&)");
    log(to_wstr(123) == L"123", "to_wstr(const auto&) int");
    log(to_wstr(3.14f) == L"3.14", "to_wstr(const auto&) float");
    log(to_wstr(3.14) == L"3.14", "to_wstr(const auto&) double");
    log(to_wstr(std::filesystem::path(L"/path/to/file")) == L"\"/path/to/file\"", "to_wstr(const auto&)");
    static_assert(to_wstr('A') == L"A", "to_wstr(char) constexpr");
    static_assert(to_wstr("Hello World") == L"Hello World", "to_wstr(const char *const) constexpr");
    static_assert(to_wstr(std::string("")) == L"", "to_wstr(const str_t&) empty constexpr");
    static_assert(to_wstr(std::string("Hello World")) == L"Hello World", "to_wstr(const str_t&) short string constexpr");
    static_assert(to_wstr(std::string("This is a pretty long string")) == L"This is a pretty long string", "to_wstr(const str_t&) long string constexpr");
    static_assert(to_wstr(std::string_view("Hello World")) == L"Hello World", "to_wstr(const strv_t&) constexpr");
    // Again, cant constexpr to_wstr(const auto&) because it uses std::wostringstream
  }



  { log(true, "Testing cslib::up_impl and throw_up");
    std::string shouldBeStdError = "std::runtime_error called from line ";
    shouldBeStdError += std::to_string(__LINE__ + 5);
    shouldBeStdError += " in workspace '";
    shouldBeStdError += std::filesystem::current_path().string();
    shouldBeStdError += "' because: \033[1m\033[31mError: CSLIB_TEST 1233.14\033[0m";
    try {
      throw_up("CSLIB", '_', L"TEST", L' ', 123, 3.14f);
    }
    catch (const std::runtime_error &e) {
      log(e.what() == shouldBeStdError, "throw_up with up_impl");
    }
  }



  { log(true, "Testing/Benchmarking cslib::pause");
    auto start = std::chrono::high_resolution_clock::now();
    pause(1000); // Pause for 1 second
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    log(duration > 999 && duration < 1001, "pause for 1000ms should take around 1000ms"); // Margin of 1ms
  }



  { log(true, "Testing (constexpr-) cslib::wstrlen(wstrv_t) with implicit conversions");
    log(wstrlen(L"Hello World") == 11, "wstrlen(const wchar_t *const)");
    log(wstrlen(L"") == 0, "wstrlen(const wchar_t *const) empty");
    log(wstrlen(L"This is a pretty long string") == 28, "wstrlen(const wchar_t *const) long string");
    log(wstrlen(std::wstring(L"Hello World")) == 11, "wstrlen(const wstr_t&)");
    log(wstrlen(std::wstring(L"")) == 0, "wstrlen(const wstr_t&) empty");
    log(wstrlen(std::wstring(L"This is a pretty long string")) == 28, "wstrlen(const wstr_t&) long string");
    log(wstrlen(std::wstring_view(L"Hello World")) == 11, "wstrlen(const wstrv_t&)");
    log(wstrlen(std::wstring_view(L"")) == 0, "wstrlen(const wstrv_t&) empty");
    log(wstrlen(L"Hello World") == 11, "wstrlen(wstrv_t) with implicit conversions");
    static_assert(wstrlen(L"Hello World") == 11, "wstrlen(const wchar_t *const) constexpr");
    static_assert(wstrlen(L"") == 0, "wstrlen(const wchar_t *const) empty constexpr");
    static_assert(wstrlen(L"This is a pretty long string") == 28, "wstrlen(const wchar_t *const) long string constexpr");
    static_assert(wstrlen(std::wstring(L"Hello World")) == 11, "wstrlen(const wstr_t&) constexpr");
    static_assert(wstrlen(std::wstring(L"")) == 0, "wstrlen(const wstr_t&) empty constexpr");
    static_assert(wstrlen(std::wstring(L"This is a pretty long string")) == 28, "wstrlen(const wstr_t&) long string constexpr");
    static_assert(wstrlen(std::wstring_view(L"Hello World")) == 11, "wstrlen(const wstrv_t&) constexpr");
    static_assert(wstrlen(std::wstring_view(L"")) == 0, "wstrlen(const wstrv_t&) empty constexpr");
    static_assert(wstrlen(std::wstring_view(L"This is a pretty long string")) == 28, "wstrlen(const wstrv_t&) long string constexpr");
  }



  { log(true, "Testing cslib::sh_call");
    sh_call("echo cslib testing"); // Shouldn't throw
    try {
      sh_call("non_existing_command");
      log(false, "sh_call should throw an error for non-existing command");
    }
    catch (const std::runtime_error &e) {
      log(std::string(e.what()).find("Failed to execute command: 'non_existing_command'") != std::string::npos, "sh_call should throw an error for non-existing command");
    }
  }



  { log(true, "Testing cslib::contains");
    std::vector<int> vec = {1, 2, 3, 4, 5};
    std::deque<int> deq = {1, 2, 3, 4, 5};
    std::set<int> set = {1, 2, 3, 4, 5};
    std::list<int> lst = {1, 2, 3, 4, 5};
    std::array<int, 5> arr = {1, 2, 3, 4, 5};
    constexpr std::array<int, 5> carr = {1, 2, 3, 4, 5};
    log(contains(vec, 3), "contains(vector, 3)");
    log(contains(deq, 3), "contains(deque, 3)");
    log(contains(set, 3), "contains(set, 3)");
    log(contains(lst, 3), "contains(list, 3)");
    log(contains(arr, 3), "contains(array, 3)");
    log(contains(carr, 3), "contains(constexpr array, 3)");
    log(!contains(vec, 6), "contains(vector, 6) should be false");
    log(!contains(deq, 6), "contains(deque, 6) should be false");
    log(!contains(set, 6), "contains(set, 6) should be false");
    log(!contains(lst, 6), "contains(list, 6) should be false");
    log(!contains(arr, 6), "contains(array, 6) should be false");
    log(!contains(carr, 6), "contains(constexpr array, 6) should be false");
    static_assert(contains(carr, 3), "contains(constexpr array, 3) should be true at compile time");
    static_assert(!contains(carr, 6), "contains(constexpr array, 6) should be false at compile time");
  }



  { log(true, "Testing cslib::have_something_common");
    std::vector<int> vec1 = {1, 2, 3};
    std::vector<int> vec2 = {3, 4, 5};
    std::vector<int> vec3 = {6, 7, 8};
    std::vector<int> vec4 = {};
    std::deque<int> deq1 = {1, 2, 3};
    std::deque<int> deq2 = {3, 4, 5};
    std::deque<int> deq3 = {6, 7, 8};
    std::deque<int> deq4 = {};
    std::set<int> set1 = {1, 2, 3};
    std::set<int> set2 = {3, 4, 5};
    std::set<int> set3 = {6, 7, 8};
    std::set<int> set4 = {};
    std::list<int> lst1 = {1, 2, 3};
    std::list<int> lst2 = {3, 4, 5};
    std::list<int> lst3 = {6, 7, 8};
    std::list<int> lst4 = {};
    std::array<int, 3> arr1 = {1, 2, 3};
    std::array<int, 3> arr2 = {3, 4, 5};
    std::array<int, 3> arr3 = {6, 7, 8};
    std::array<int, 3> arr4 = {};
    constexpr std::array<int, 3> carr1 = {1, 2, 3};
    constexpr std::array<int, 3> carr2 = {3, 4, 5};
    constexpr std::array<int, 3> carr3 = {6, 7, 8};
    constexpr std::array<int, 3> carr4 = {};
    log(have_something_common(vec1, vec2), "have_something_common(vector, vector) should be true");
    log(have_something_common(deq1, deq2), "have_something_common(deque, deque) should be true");
    log(have_something_common(set1, set2), "have_something_common(set, set) should be true");
    log(have_something_common(lst1, lst2), "have_something_common(list, list) should be true");
    log(have_something_common(arr1, arr2), "have_something_common(array, array) should be true");
    log(have_something_common(carr1, carr2), "have_something_common(constexpr array, constexpr array) should be true");
    log(!have_something_common(vec1, vec3), "have_something_common(vector, vector) should be false");
    log(!have_something_common(deq1, deq3), "have_something_common(deque, deque) should be false");
    log(!have_something_common(set1, set3), "have_something_common(set, set) should be false");
    log(!have_something_common(lst1, lst3), "have_something_common(list, list) should be false");
    log(!have_something_common(arr1, arr3), "have_something_common(array, array) should be false");
    log(!have_something_common(carr1, carr3), "have_something_common(constexpr array, constexpr array) should be false");
    log(!have_something_common(vec1, vec4), "have_something_common(vector, empty vector) should be false");
    log(!have_something_common(deq1, deq4), "have_something_common(deque, empty deque) should be false");
    log(!have_something_common(set1, set4), "have_something_common(set, empty set) should be false");
    log(!have_something_common(lst1, lst4), "have_something_common(list, empty list) should be false");
    log(!have_something_common(arr1, arr4), "have_something_common(array, empty array) should be false");
    static_assert(have_something_common(carr1, carr2), "have_something_common(constexpr array, constexpr array) should be true at compile time");
    static_assert(!have_something_common(carr1, carr3), "have_something_common(constexpr array, constexpr array) should be false at compile time");
    static_assert(!have_something_common(carr1, carr4), "have_something_common(constexpr array, empty constexpr array) should be false at compile time");
  }



  { log(true, "Testing cslib::get_env");
    try {
      std::string envValue = get_env("PATH");
      log(!envValue.empty(), "get_env should return a non-empty value for existing environment variable");
    }
    catch (const std::runtime_error &e) {
      log(false, "get_env should not throw an error for existing environment variable");
    }
    try {
      std::string envValue = get_env("NON_EXISTING_ENV_VAR");
      log(false, "get_env did not throw an error for non-existing environment variable");
    }
    catch (const std::runtime_error &e) {
      log(std::string(e.what()).find("Environment variable 'NON_EXISTING_ENV_VAR' not found") != std::string::npos, "get_env should throw an error for non-existing environment variable");
    }
  }



  { log(true, "Testing cslib::range");
    std::vector<int> r1 = range(5);
    log(r1 == std::vector<int>({0, 1, 2, 3, 4}), "range(5) should return [0, 1, 2, 3, 4]");
    std::vector<int> r2 = range(2, 5);
    log(r2 == std::vector<int>({2, 3, 4, 5}), "range(2, 5) should return [2, 3, 4, 5]");
    std::vector<int> r3 = range(5, 2);
    log(r3 == std::vector<int>({5, 4, 3, 2}), "range(5, 2) should return [5, 4, 3, 2]");
    std::vector<int> r4 = range(-3);
    log(r4 == std::vector<int>({0, -1, -2}), "range(-3) should return [0, -1, -2]");
    std::vector<int> r5 = range(-5, -2);
    log(r5 == std::vector<int>({-5, -4, -3}), "range(-5, -2) should return [-5, -4, -3]");
    std::vector<int> r6 = range(-2, -5);
    log(r6 == std::vector<int>({-2, -3, -4}), "range(-2, -5) should return [-2, -3, -4]");
  }



  { log(true, "Testing cslib::retry");
    auto func = []() {
      static int count = 0;
      if (count < 2) {
        ++count;
        throw std::runtime_error("c-function test error");
      }
      return 42; // Success after 2 retries
    };
    std::function<int()> stdFunc = []() {
      static int count = 0;
      if (count < 2) {
        ++count;
        throw std::runtime_error("std::function Test error");
      }
      return 42; // Success after 2 retries
    };
    retry(func, 3, 100);
    retry(stdFunc, 3, 100);
  }
}