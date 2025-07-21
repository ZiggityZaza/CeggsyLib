#include "./cslib.h++"
using namespace cslib;
#include <cassert>


void title(std::string_view title) {
  if (!isWcharIOEnabled)
    enable_wchar_io();
  std::wcout << L"\n\033[1;34m" << title.data() << L"\033[0m\n";
}


template <typename... _Args>
void log(bool conditionResult, _Args&&... conditionsExplained) {
  if (!isWcharIOEnabled)
    enable_wchar_io();
  if (conditionResult)
    std::wcout << L"\033[1;32m[PASSED]\033[0m";
  else
    std::wcout << L"\033[1;31m[FAILED]\033[0m";
  std::wcout << " > ";
  ((std::wcout << std::forward<_Args>(conditionsExplained)), ...);
  std::wcout << L'\n';
}


Benchmark bm;



int main() {

  title("Testing cslib::to_str"); {
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



  title("Testing cslib::to_wstr"); {
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



  title("Testing cslib::up_impl and throw_up"); {
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



  title("Testing/Benchmarking cslib::pause"); {
    bm.reset();
    pause(500); // Pause for 0,5 second
    size_t elpsd = bm.elapsed_ms();
    log(elpsd > 499 && elpsd < 501, "pause for 1000ms should take around 500ms (took ", elpsd, "ms)");
    pause(0);
    log(elpsd > 499 && elpsd < 501, "pause for 0ms shouldn't take longer than 0ms (took ", elpsd, "ms)");
  }



  title("Testing (constexpr-) cslib::wstrlen(wstrv_t) with implicit conversions"); {
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



  title("Testing cslib::upper and cslib::lower"); {
    std::wstring_view mixedStrv = L"csLib.h++";
    std::wstring mixedStr = L"csLib.h++";
    const wchar_t *const mixedCStr = L"csLib.h++";
    log(upper_str(mixedStrv) == L"CSLIB.H++", "upper(wstrv_t) implicit conversion");
    log(upper_str(mixedStr) == L"CSLIB.H++", "upper(wstr_t) implicit conversion");
    log(upper_str(mixedCStr) == L"CSLIB.H++", "upper(const wchar_t *const) implicit conversion");
    log(lower_str(mixedStrv) == L"cslib.h++", "lower(wstrv_t) implicit conversion");
    log(lower_str(mixedStr) == L"cslib.h++", "lower(wstr_t) implicit conversion");
    log(lower_str(mixedCStr) == L"cslib.h++", "lower(const wchar_t *const) implicit conversion");
    static_assert(upper_str(L"csLib.h++") == L"CSLIB.H++", "upper_str constexpr with implicit conversion");
    static_assert(lower_str(L"csLib.h++") == L"cslib.h++", "lower_str constexpr with implicit conversion");
    static_assert(upper_str(std::wstring_view(L"csLib.h++")) == L"CSLIB.H++", "upper_str constexpr with wstrv_t");
    static_assert(lower_str(std::wstring_view(L"csLib.h++")) == L"cslib.h++", "lower_str constexpr with wstrv_t");
    static_assert(upper_str(std::wstring(L"csLib.h++")) == L"CSLIB.H++", "upper_str constexpr with wstr_t");
    static_assert(lower_str(std::wstring(L"csLib.h++")) == L"cslib.h++", "lower_str constexpr with wstr_t");
  }



  title("Testing cslib::sh_call"); {
    sh_call("echo cslib testing"); // Shouldn't throw
    try {
      sh_call("non_existing_command");
      log(false, "sh_call should throw an error for non-existing command");
    }
    catch (const std::runtime_error &e) {
      log(std::string(e.what()).find("Failed to execute command: 'non_existing_command'") != std::string::npos, "sh_call should throw an error for non-existing command");
    }
  }



  title("Skipped clear_console cuz difficult to test in a non-interactive environment");



  title("Testing cslib::contains"); {
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



  title("Testing cslib::have_something_common"); {
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



  title("Testing cslib::get_env"); {
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



  title("Testing cslib::range"); {
    std::vector<int> r1 = range(5);
    log(r1 == std::vector<int>({0, 1, 2, 3, 4}), "range(5) should return [0, 1, 2, 3, 4]");
    std::vector<int> r2 = range(2, 5);
    log(r2 == std::vector<int>({2, 3, 4}), "range(2, 5) should return [2, 3, 4]");
    std::vector<int> r3 = range(5, 2);
    log(r3 == std::vector<int>({5, 4, 3}), "range(5, 2) should return [5, 4, 3]");
    std::vector<int> r4 = range(-3);
    log(r4 == std::vector<int>({0, -1, -2}), "range(-3) should return [0, -1, -2]");
    std::vector<int> r5 = range(-5, -2);
    log(r5 == std::vector<int>({-5, -4, -3}), "range(-5, -2) should return [-5, -4, -3]");
    std::vector<int> r6 = range(-2, -5);
    log(r6 == std::vector<int>({-2, -3, -4}), "range(-2, -5) should return [-2, -3, -4]");
  }



  title("Testing cslib::retry"); {
    // stl
    bm.reset();
    std::function<int()> stdFunc = [] -> int {
      static int attempts = 0;
      if (++attempts < 3)
        throw std::runtime_error("Test error from std::function");
      return 42;
    };
    int result = retry(stdFunc, 3, 100);
    size_t elpsd = bm.elapsed_ms();
    log(result == 42, "retry(std::function) should return the correct result");
    log(elpsd >= 200 && elpsd <= 300, "retry(std::function) should take around 200-300ms (took ", elpsd, "ms)");

    // C-style pointer
    bm.reset();
    int (*cPtrFunc)() = [] -> int {
      static int attempts = 0;
      if (++attempts < 3)
        throw std::runtime_error("Test error from c-style function pointer");
      return 42;
    };
    result = retry(cPtrFunc, 3, 100);
    elpsd = bm.elapsed_ms();
    log(result == 42, "retry(c-style function pointer) should return the correct result");
    log(elpsd >= 200 && elpsd <= 300, "retry(c-style function pointer) should take around 200-300ms (took ", elpsd, "ms)");

    // Lambda
    bm.reset();
    auto lambdaFunc = [] -> int {
      static int attempts = 0;
      if (++attempts < 3)
        throw std::runtime_error("Test error from lambda function");
      return 42;
    };
    result = retry(lambdaFunc, 3, 100);
    elpsd = bm.elapsed_ms();
    log(result == 42, "retry(lambda function) should return the correct result");
    log(elpsd >= 200 && elpsd <= 300, "retry(lambda function) should take around 200-300ms (took ", elpsd, "ms)");
  
    // Inline lambda
    bm.reset();
    result = retry([] -> int {
      static int attempts = 0;
      if (++attempts < 3)
        throw std::runtime_error("Test error from inline lambda function");
      return 42; // Success on the third attempt
    }, 3, 100);
    elpsd = bm.elapsed_ms();
    log(result == 42, "retry(inline lambda function) should return the correct result");
    log(elpsd >= 200 && elpsd <= 300, "retry(inline lambda function) should take around 200-300ms (took ", elpsd, "ms)");

    // Always throw
    try {
      retry([] -> int {
        throw std::runtime_error("This function always fails");
      }, 1, 1);
      log(false, "retry should throw an error for function that always fails");
    } catch (const std::runtime_error &e) {
      log(std::string(e.what()).find("Function failed after maximum attempts") != std::string::npos, "retry should throw an error for function that always fails");
    }
  }



  title("Testing cslib::parse_cli_args"); {
    const char *args1[] = {"program", "arg1", "arg2", "arg3"};
    std::vector<strv_t> parsedArgs1 = parse_cli_args(4, args1);
    log(parsedArgs1.size() == 3, "parse_cli_args should return 3 arguments");
    log(parsedArgs1.at(0) == "arg1", "First argument should be 'arg1'");
    log(parsedArgs1.at(1) == "arg2", "Second argument should be 'arg2'");
    log(parsedArgs1.at(2) == "arg3", "Third argument should be 'arg3'");
    try {
      parse_cli_args(0, nullptr);
      log(false, "parse_cli_args should throw an error for zero arguments");
    } catch (const std::runtime_error &e) {
      log(std::string(e.what()).find("No command line arguments provided") != std::string::npos, "parse_cli_args should throw an error for zero arguments");
    }
    const char* args2[] = {"program"};
    std::vector<strv_t> parsedArgs2 = parse_cli_args(1, args2);
    log(parsedArgs2.empty(), "parse_cli_args should return an empty vector for no arguments");
  }



  title("Testing cslib::shorten_end/begin"); { // No narrow string support
    std::wstring str = L"This is a pretty long string";
    log(shorten_end(str, 10) == L"This is...", "shorten_end with length 10");
    log(shorten_begin(str, 10) == L"... string", "shorten_begin with length 10");
    log(shorten_end(str, 100) == str, "shorten_end with length greater than string length should return original string");
    log(shorten_begin(str, 100) == str, "shorten_begin with length greater than string length should return original string");
    try {
      shorten_end(str, 2);
      log(false, "shorten_end should throw an error for maxLength less than TRIM_WITH length");
    } catch (const std::runtime_error &e) {
      log(std::string(e.what()).find("maxLength must be at least 3 (TRIM_WITH length)") != std::string::npos, "shorten_end should throw an error for maxLength less than TRIM_WITH length");
    }
    try {
      shorten_begin(str, 2);
      log(false, "shorten_begin should throw an error for maxLength less than TRIM_WITH length");
    } catch (const std::runtime_error &e) {
      log(std::string(e.what()).find("maxLength must be at least 3 (TRIM_WITH length)") != std::string::npos, "shorten_begin should throw an error for maxLength less than TRIM_WITH length");
    }
    static_assert(shorten_end(L"This is a pretty long string", 10) == L"This is...", "shorten_end constexpr with length 10");
    static_assert(shorten_begin(L"This is a pretty long string", 10) == L"... string", "shorten_begin constexpr with length 10");
    static_assert(shorten_end(L"This is a pretty long string", 100) == L"This is a pretty long string", "shorten_end constexpr with length greater than string length should return original string");
    static_assert(shorten_begin(L"This is a pretty long string", 100) == L"This is a pretty long string", "shorten_begin constexpr with length greater than string length should return original string");
  }



  title("Testing cslib::separate"); {
    std::vector<std::wstring> result = separate(L"John Money", ' ');
    log(result.size() == 2, "separate should return 2 parts");
    log(result.at(0) == L"John", "First part should be 'John'");
    log(result.at(1) == L"Money", "Second part should be 'Money'");
    result = separate(L"John  Money", ' ');
    log(result.size() == 3, "separate should return 3 parts for double spaces");
    log(result.at(0) == L"John", "First part should be 'John'");
    log(result.at(1) == L"", "Second part should be empty for double space");
    log(result.at(2) == L"Money", "Third part should be 'Money'");
    log(separate(L"", ',').empty(), "separate should return empty vector for empty string");
    log(separate(L"John Money", L'X').at(0) == L"John Money", "separate with non-existing delimiter should return the whole string in a vector");
  }



  title("Testing cslib::roll_dice"); {
    int min = -3, max = 3;
    bool correctRange = true;
    for ([[maybe_unused]] int64_t _ : range(100'000)) {
      int result = roll_dice(min, max);
      if (!(result >= min && result <= max))
        correctRange = false;
    }
    log(correctRange, "roll_dice should return a value between ", min, " and ", max);
    log(roll_dice(1, 1) == 1, "roll_dice with same min and max should return that value");
  }



  title("Testing cslib::read_wdata and cslib::do_io"); {
    // read_wdata read-only
    std::filesystem::path testFilePath = std::filesystem::temp_directory_path() / "cslib_test_io.txt";
    std::filesystem::remove(testFilePath);
    std::wofstream(testFilePath) << L"John\nMoney\n";
    std::wifstream inFile(testFilePath);
    log(read_wdata(inFile) == L"John\nMoney\n", "read_wdata file reading should return file content");
    log(read_wdata(inFile) == L"John\nMoney\n", "read_wdata should still same content after reading again");;
    inFile.close();
    log(read_wdata(inFile) == L"", "read_wdata should return empty after closing stream");
    std::wifstream inFile2(testFilePath);
    log(read_wdata(inFile2) == L"John\nMoney\n", "read_wdata should return expected content after reopening file on different stream");
  
    // do_io writes to a new file
    std::filesystem::path testFilePath2 = std::filesystem::temp_directory_path() / "cslib_test_io2.txt";
    std::wofstream outFile(testFilePath2);
    outFile << read_wdata(inFile2);
    outFile.close();
    std::wifstream inFile3(testFilePath2);
    log(read_wdata(inFile3) == L"John\nMoney\n", "read_wdata should return expected content after writing to a new file");
    inFile3.close();

    // do_io by itself
    std::wostringstream woss;
    do_io(inFile2, woss);
    log(woss.str() == L"John\nMoney\n", "do_io should read from input stream and write to output stream");
    inFile2.close();
  }



  title("Skipping wchar support functions/variables tests because they need cli interaction");



  title("Testing cslib::Out"); {
    std::wostringstream woss;
    Out(woss, L"[checking cslib::Out]", Cyan) << "narrow" << '_' << 123 << L'_' << 3.14f << L" wide";
    log(woss.str() == L"\033[36m[checking cslib::Out] \033[0mnarrow_123_3.14 wide", "cslib::Out with narrow and wide types");
    woss.str(L""); // Clear the stream
  }



  title("Testing cslib::TimeStamp"); {
    // Accuracy
    TimeStamp ts1;
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Must be away from 1 second to avoid test issues below
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(TimeStamp().timePoint - ts1.timePoint).count();
    log(elapsed >= 499 and elapsed <= 501, "TimeStamp should be within 500ms of current time");

    // Correctness of formatting
    std::time_t now = std::chrono::system_clock::to_time_t(ts1.timePoint);
    std::tm *tm = std::localtime(&now);
    log(((std::wostringstream() << std::put_time(tm, L"%H:%M:%S %d-%m-%Y")).str() == TimeStamp().as_wstr()), "TimeStamp should format time correctly");

    // Making sure TimeStamp returns correct values
    TimeStamp rightNow;
    log(rightNow.year() == uint(tm->tm_year + 1900), "TimeStamp should return correct year");
    log(rightNow.month() == uint(tm->tm_mon + 1), "TimeStamp should return correct month");
    log(rightNow.day() == uint(tm->tm_mday), "TimeStamp should return correct day");
    log(rightNow.hour() == uint(tm->tm_hour), "TimeStamp should return correct hour");
    log(rightNow.minute() == uint(tm->tm_min), "TimeStamp should return correct minute");
    log(rightNow.second() == uint(tm->tm_sec), "TimeStamp should return correct second");

    // Testing constructor
    TimeStamp ts2(30, 45, 12, 25, 12, 2023); // 12:45:30 on 25th December 2023
    log(ts2.year() == 2023, "TimeStamp should return correct year from constructor");
    log(ts2.month() == 12, "TimeStamp should return correct month from constructor");
    log(ts2.day() == 25, "TimeStamp should return correct day from constructor");
    log(ts2.hour() == 12, "TimeStamp should return correct hour from constructor");
    log(ts2.minute() == 45, "TimeStamp should return correct minute from constructor");
    log(ts2.second() == 30, "TimeStamp should return correct second from constructor");
    log(ts2.as_wstr() == L"12:45:30 25-12-2023", "TimeStamp should format specific time correctly");

    // Error handling
    try {
      TimeStamp invalidTs(61, 0, 0, 1, 1, 2023); // Invalid second
      log(false, "TimeStamp should throw an error for invalid time");
    } catch (const std::runtime_error &e) {
      log(std::string(e.what()).find("Invalid time") != std::string::npos, "TimeStamp should throw an error for invalid time");
    }
  }



  title("Testing cslib::Benchmark"); {
    Benchmark bm1;
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Sleep for 0.5 seconds
    size_t elapsed = bm1.elapsed_ms();
    log(elapsed >= 499 && elapsed <= 501, "Benchmark should measure time correctly (took ", elapsed, "ms)");
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Sleep for another 0.5 seconds
    elapsed = bm1.elapsed_ms();
    log(elapsed >= 998 && elapsed <= 1002, "Benchmark should measure time correctly after another 0.5 seconds (took ", elapsed, "ms)");
    bm1.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Sleep for 0.5 seconds after reset
    elapsed = bm1.elapsed_ms();
    log(elapsed >= 499 && elapsed <= 501, "Benchmark should reset and measure time correctly after reset (took ", elapsed, "ms)");
  }



  title("Testing cslib::RouteToFile"); {
    // Setting up a temporary log file
    const std::filesystem::path STD_PATH = std::filesystem::temp_directory_path() / "cslib_test_log.txt";
    std::filesystem::remove(STD_PATH);
    std::wofstream(STD_PATH) << L"Initial log content";

    // Testing ability to read file properties
    RouteToFile rtfFile(STD_PATH);
    log(TimeStamp(rtfFile.last_modified()).as_wstr() == TimeStamp().as_wstr(), "RouteToFile should have the correct last modified time");
    // Formatting to wstring for implicit reduce comparison accuracy 
    log(rtfFile.type() == std::filesystem::file_type::regular, "RouteToFile should create a regular file");

    // Depth and position checks
    RouteToFile rofFileParent(STD_PATH.parent_path());
    log(rofFileParent.type() == std::filesystem::file_type::directory, "RouteToFile should create a directory for the parent path");
    log(rofFileParent.isAt == std::filesystem::temp_directory_path(), "RouteToFile parent path should be the temp directory path");
    #ifdef _WIN32
      const size_t EXPECTED_DEPTH = 6; // e.g., C:\Users\Username\AppData\Local\Temp\cslib_test_log.txt
    #else
      const size_t EXPECTED_DEPTH = 2; // e.g., /tmp/cslib
    #endif
    log(rtfFile.depth() == EXPECTED_DEPTH, "RouteToFile should have the correct depth for temp file");

    // Equality and inequality checks
    RouteToFile rtoFileCopy(STD_PATH);
    log(rtoFileCopy == rtfFile, "RouteToFile == operator should work for same file paths");
    log(rtoFileCopy != RouteToFile("../"), "RouteToFile != operator should work for different file paths");
    log(rtoFileCopy == STD_PATH.wstring().data(), "RouteToFile == operator should work for strings");
    log(rtoFileCopy != (STD_PATH.wstring() + L"???"), "RouteToFile != operator should work for different strings");

    // Conversions to other types
    log(std::wstring(rtfFile) == STD_PATH.wstring(), "RouteToFile should convert to wstring correctly");
    std::filesystem::path& logFileAsFsRef = rtfFile;
    log(logFileAsFsRef == rtfFile.isAt, "RouteToFile should convert to filesystem path reference correctly");
    const std::filesystem::path& logFileAsFsConstRef = rtfFile;
    log(logFileAsFsConstRef == rtfFile.isAt, "RouteToFile should convert to filesystem path const reference correctly");
    std::filesystem::path logFileAsFsCopy = rtfFile;
    log(logFileAsFsCopy == rtfFile.isAt, "RouteToFile should convert to filesystem path copy correctly");
    std::filesystem::path* logFileAsFsPtr = rtfFile;
    log(logFileAsFsPtr == &rtfFile.isAt, "RouteToFile should return its filesystem path pointer correctly");
    const std::filesystem::path* logFileAsFsConstPtr = rtfFile;
    log(logFileAsFsConstPtr == &rtfFile.isAt, "RouteToFile should return its filesystem path const pointer correctly");
    
    // Constructors
    try {
      RouteToFile invalidPath(L"non_existing_path/cslib_test_log.txt");
      log(false, "RouteToFile should throw an error for non-existing path");
    } catch (const std::filesystem::filesystem_error &e) {
      log(std::string(e.what()).find("No such file or directory") != std::string::npos, "RouteToFile should throw an error for non-existing path");
    }
    try {
      RouteToFile emptyPath(L"../", std::filesystem::file_type::regular);
      log(false, "RouteToFile should recognize between file types at construction");
    } catch (const std::runtime_error &e) {
      log(std::string(e.what()).find("Path '" /*...*/) != std::string::npos, "RouteToFile should throw an error for unexpected file type at construction");
    }
  }



  title("Testing cslib::File"); {
    // File creation and properties
    const std::filesystem::path STD_PATH = std::filesystem::temp_directory_path() / "cslib_test_file.txt";
    std::filesystem::remove(STD_PATH);
    std::wofstream(STD_PATH) << L"Test content for cslib::File";
    File file(STD_PATH);
    try {
      File folderFile(STD_PATH.parent_path());
      log(false, "File should throw an error for directory path");
    } catch (const std::runtime_error &e) {
      log(std::string(e.what()).find("Path '" /*...*/) != std::string::npos, "File should throw an error for directory path");
    }

    // File reading
    log(file.content() == L"Test content for cslib::File", "File should read content correctly"); // Default std::ios::in for reading
    log(file.content(std::ios::binary) == L"Test content for cslib::File", "File should read content correctly in binary mode");

    // File writing
    file.edit(L"New content for cslib::File", std::ios::out);
    log(file.content() == L"New content for cslib::File", "File should read updated content correctly");

    // File properties
    log(file.extension() == L".txt", "File should return correct extension");
    log(file.bytes() >= std::filesystem::file_size(STD_PATH), "File should return correct size");
  }



  title("Testing cslib::Folder"); {
    // Folder creation and properties
    const std::filesystem::path STD_PATH = std::filesystem::temp_directory_path() / "cslib_test_folder";
    std::filesystem::remove_all(STD_PATH);
    std::filesystem::create_directory(STD_PATH);
    Folder folder(STD_PATH);
    log(folder.is == STD_PATH, "Folder should be at the correct path");

    // content correctness
    std::wofstream(STD_PATH / "test_file.txt") << L"Test content for cslib::Folder";
    std::wofstream(STD_PATH / "another_file.txt") << L"Another test content for cslib::Folder";
    std::vector<RouteToFile> expectedFiles = {STD_PATH / "test_file.txt", STD_PATH / "another_file.txt"};
    log(folder.content == expectedFiles, "Folder should return correct content (won't fix for now)");
    std::wofstream(STD_PATH / "additional_file.txt") << L"Additional test content for cslib::Folder";
    expectedFiles.push_back(STD_PATH / "additional_file.txt");
    folder.update();
    log(folder.content == expectedFiles, "Folder should update its content correctly after adding a new file");

    // Constructor
    try {
      Folder invalidFolder(L"non_existing_path");
      log(false, "Folder should throw an error for non-existing path");
    } catch (const std::filesystem::filesystem_error &e) {
      log(std::string(e.what()).find("No such file or directory") != std::string::npos, "Folder should throw an error for non-existing path");
    }
    try {
      Folder fileFolder(STD_PATH / "test_file.txt");
      log(false, "Folder should throw an error for file path");
    } catch (const std::runtime_error &e) {
      log(std::string(e.what()).find("Path '" /*...*/) != std::string::npos, "Folder should throw an error for file path");
    }

    // .has functions
    log(folder.has(RouteToFile(STD_PATH / "test_file.txt")), "Folder should have test_file.txt");
    log(folder.has(File(STD_PATH / "test_file.txt")), "Folder should have test_file.txt as File");
    std::filesystem::create_directories(STD_PATH / "subfolder");
    folder.update(); // Update folder content after creating subfolder
    log(folder.has(Folder(STD_PATH / "subfolder")), "Folder should have subfolder");
  }



  title("Testing cslib::create_file"); {
    // Create a file and check its content
    const std::filesystem::path STD_PATH = std::filesystem::temp_directory_path() / "cslib_test_create_file.txt";
    std::filesystem::remove(STD_PATH);
    File createdFile = create_file(std::filesystem::temp_directory_path() / L"cslib_test_create_file.txt");
    log(createdFile.is == STD_PATH, "create_file should create the file at the correct path");
    createdFile.edit(L"Content for cslib::create_file test", std::ios::out);
    log(createdFile.content() == L"Content for cslib::create_file test", "create_file should create a file with the correct content");
  }



  title("Testing cslib::create_folder"); {
    // Create a folder and check its content
    const std::filesystem::path STD_PATH = std::filesystem::temp_directory_path() / "cslib_test_create_folder";
    std::filesystem::remove_all(STD_PATH);
    Folder createdFolder = create_folder(std::filesystem::temp_directory_path() / L"cslib_test_create_folder");
    log(createdFolder.is == STD_PATH, "create_folder should create the folder at the correct path");
    std::wofstream(STD_PATH / "test_file.txt") << L"Content for cslib::create_folder test";
    createdFolder.update();
    log(createdFolder.content.size() == 1, "create_folder should have one file in its content");
    log(createdFolder.content.at(0).isAt.filename() == L"test_file.txt", "create_folder should have the correct file in its content");
  }



  title("Testing cslib::move_entry_to"); {
    // Get the current folder and check its content
    const std::filesystem::path STD_MOVED_INTO = std::filesystem::temp_directory_path() / "cslib_test_moved_into";
    std::filesystem::remove_all(STD_MOVED_INTO);
    std::filesystem::create_directory(STD_MOVED_INTO);
    const std::filesystem::path STD_TARGET = std::filesystem::temp_directory_path() / "cslib_move_target.txt";
    std::filesystem::remove(STD_TARGET);
    std::wofstream(STD_TARGET) << L"Content for cslib::move_entry_to test";
    RouteToFile rtfTarget(STD_TARGET, std::filesystem::file_type::regular);
    move_entry_to(Folder(STD_MOVED_INTO), rtfTarget);
    log(!std::filesystem::exists(STD_TARGET), "cslib::move_entry_to should remove the original file after moving");
    log(std::filesystem::exists(STD_MOVED_INTO / "cslib_move_target.txt"), "cslib::move_entry_to should move the file to the new folder");
  }



  title("Testing cslib::copy_entry_to"); {
    // Get the current folder and check its content
    const std::filesystem::path STD_COPIED_INTO = std::filesystem::temp_directory_path() / "cslib_test_copied_into";
    std::filesystem::remove_all(STD_COPIED_INTO);
    std::filesystem::create_directory(STD_COPIED_INTO);
    const std::filesystem::path STD_SOURCE = std::filesystem::temp_directory_path() / "cslib_copy_source.txt";
    std::filesystem::remove(STD_SOURCE);
    std::wofstream(STD_SOURCE) << L"Content for cslib::copy_entry_to test";
    RouteToFile rtfSource(STD_SOURCE, std::filesystem::file_type::regular);
    copy_entry_to(Folder(STD_COPIED_INTO), rtfSource);
    log(std::filesystem::exists(STD_SOURCE), "cslib::copy_entry_to should keep the original file after copying");
    log(std::filesystem::exists(STD_COPIED_INTO / "cslib_copy_source.txt"), "cslib::copy_entry_to should copy the file to the new folder");
  }



  title("Testing cslib::remove_entry"); {
    // Get the current folder and check its content
    const std::filesystem::path STD_REMOVE_FROM = std::filesystem::temp_directory_path() / "cslib_test_remove_from";
    std::filesystem::remove_all(STD_REMOVE_FROM);
    std::filesystem::create_directory(STD_REMOVE_FROM);
    const std::filesystem::path STD_TARGET = STD_REMOVE_FROM / "cslib_remove_target.txt";
    std::wofstream(STD_TARGET) << L"Content for cslib::remove_entry test";
    log(std::filesystem::exists(STD_TARGET), "cslib::remove_entry should create the target file");
    remove_entry(STD_TARGET);
    log(!std::filesystem::exists(STD_TARGET), "cslib::remove_entry should remove the target file");
  }



  title("Testing cslib::TempFile"); {
    // Create a temporary file and check its properties
    std::filesystem::path stdTempFile;
    {
      TempFile tempFile;
      stdTempFile = tempFile.file.is.isAt;
    }
    log(!std::filesystem::exists(stdTempFile), "TempFile should remove itself after going out of scope");
    std::filesystem::path stdTempFilePersistent;
    {
      std::filesystem::remove(stdTempFilePersistent);
      TempFile tempFilePersistent;
      stdTempFilePersistent = tempFilePersistent.file.is.isAt;
      tempFilePersistent.keep = true;
    }
    log(std::filesystem::exists(stdTempFilePersistent), "TempFile with keep=true should not remove itself after going out of scope");
  }



  title("Testing cslib::TempFolder"); {
    // Create a temporary folder and check its properties
    std::filesystem::path stdTempFolder;
    {
      TempFolder tempFolder;
      stdTempFolder = tempFolder.folder.is.isAt;
    }
    log(!std::filesystem::exists(stdTempFolder), "TempFolder should remove itself after going out of scope");
    std::filesystem::path stdTempFolderPersistent;
    {
      std::filesystem::remove_all(stdTempFolderPersistent);
      TempFolder tempFolderPersistent;
      stdTempFolderPersistent = tempFolderPersistent.folder.is.isAt;
      tempFolderPersistent.keep = true;
    }
    log(std::filesystem::exists(stdTempFolderPersistent), "TempFolder with keep=true should not remove itself after going out of scope");
  }
}