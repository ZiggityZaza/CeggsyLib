#include "./cslib.h++"
using namespace cslib;


void title(const auto& title) {
  std::cout << "\n\033[1;34m" << title << "\033[0m\n";
}


template <typename... _Args>
void log(bool conditionResult, _Args&&... conditionsExplained) {
  if (conditionResult)
    std::cout << "\033[1;32m[PASSED]\033[0m";
  else
    std::cout << "\033[1;31m[FAILED]\033[0m";
  std::cout << " > ";
  ((std::cout << std::forward<_Args>(conditionsExplained)), ...);
  std::cout << std::endl;
}

bool find_error(const std::runtime_error& e, std::string_view lookFor) {
  return std::string(e.what()).find(lookFor) != std::string::npos;
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
    try {
      to_str(L'\x100'); // Should throw
      log(false, "to_str(wchar_t) should throw for non-representable character");
    }
    catch (const std::runtime_error &e) {
      log(find_error(e, "to_str(wchar_t) cannot convert wide character to narrow string"), "to_str(wchar_t) throws for non-representable character");
    }
    try {
      to_str(L"Hello \x100 World"); // Should throw
      log(false, "to_str(const wchar_t *const) should throw for non-representable character");
    }
    catch (const std::runtime_error &e) {
      log(find_error(e, "to_str(const wchar_t *const) cannot convert wide string to narrow string"), "to_str(const wchar_t *const) throws for non-representable character");
    }
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



  title("Testing cslib::wstrlen and cslib::strlen"); {
    log(wstrlen(L"Hello World") == 11, "wstrlen(const wchar_t *const)");
    log(strlen("Hello World") == 11, "strlen(const char *const)");
    static_assert(wstrlen(L"Hello World") == 11, "wstrlen(const wchar_t *const) constexpr");
    static_assert(strlen("Hello World") == 11, "strlen(const char *const) constexpr");
  }



  title("Testing cslib::any_error and throw_up"); {
    std::ostringstream shouldBeWhat; // e.what()
    shouldBeWhat << "cslib::any_error called in workspace " << std::filesystem::current_path();
    shouldBeWhat << " on line " << __LINE__ + 3 << " because: "; // throw_up is called 3 lines beneath
    shouldBeWhat << "Error: CSLIB_TEST 1234.56";
    try {
      throw_up("CSLIB", '_', L"TEST", L' ', 123, 4.56f);
    }
    catch (const cslib::any_error &e) {
      std::cout << e.what();
    }
  }



  title("Testing/Benchmarking cslib::pause"); {
    bm.reset();
    pause(500); // Pause for 0,5 second
    double elpsd = bm.elapsed_ms();
    log(elpsd > 499 && elpsd < 510, "pause for 500ms should take around 510ms (took ", elpsd, "ms)");
    pause(0);
    log(elpsd > 499 && elpsd < 510, "pause for 0ms shouldn't take longer than 0ms (took ", elpsd, "ms)");
  }







  title("Testing cslib::sh_call"); {
    sh_call("echo cslib testing"); // Shouldn't throw
    try {
      sh_call("non_existing_command");
      log(false, "sh_call should throw an error for non-existing command");
    }
    catch (const std::runtime_error &e) {
      log(find_error(e, "Failed to execute command: 'non_existing_command'"), "sh_call should throw an error for non-existing command");
    }
  }



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
      log(find_error(e, "Environment variable 'NON_EXISTING_ENV_VAR' not found"), "get_env should throw an error for non-existing environment variable");
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
    log(elpsd >= 200 && elpsd <= 310, "retry(std::function) should take around 200-310ms (took ", elpsd, "ms)");

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
    log(elpsd >= 200 && elpsd <= 310, "retry(c-style function pointer) should take around 200-310ms (took ", elpsd, "ms)");

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
    log(elpsd >= 200 && elpsd <= 310, "retry(lambda function) should take around 200-310ms (took ", elpsd, "ms)");

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
    log(elpsd >= 200 && elpsd <= 310, "retry(inline lambda function) should take around 200-310ms (took ", elpsd, "ms)");

    // Always throw
    try {
      retry([] -> int {
        throw std::runtime_error("This function always fails");
      }, 1, 1);
      log(false, "retry should throw an error for function that always fails");
    } catch (const std::runtime_error &e) {
      log(find_error(e, "Function failed after maximum attempts"), "retry should throw an error for function that always fails");
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
      log(find_error(e, "No command line arguments provided"), "parse_cli_args should throw an error for zero arguments");
    }
    const char* args2[] = {"program"};
    std::vector<strv_t> parsedArgs2 = parse_cli_args(1, args2);
    log(parsedArgs2.empty(), "parse_cli_args should return an empty vector for no arguments");
  }



  title("Testing cslib::stringify_container"); {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    std::deque<int> deq = {1, 2, 3, 4, 5};
    std::set<int> set = {1, 2, 3, 4, 5};
    std::list<int> lst = {1, 2, 3, 4, 5};
    std::array<int, 5> arr = {1, 2, 3, 4, 5};
    constexpr std::array<int, 5> carr = {1, 2, 3, 4, 5};
    std::initializer_list<int> initList = {1, 2, 3, 4, 5};
    strv_t expected = "{1, 2, 3, 4, 5}";
    log(stringify_container(vec) == expected, "stringify_container(vector)");
    log(stringify_container(deq) == expected, "stringify_container(deque)");
    log(stringify_container(set) == expected, "stringify_container(set)");
    log(stringify_container(lst) == expected, "stringify_container(list)");
    log(stringify_container(arr) == expected, "stringify_container(array)");
    log(stringify_container(carr) == expected, "stringify_container(constexpr array)");
    log(stringify_container(initList) == expected, "stringify_container(initializer_list)");
  }



  title("Testing cslib::shorten_end/begin"); { // No narrow string support
    str_t str = "This is a pretty long string";
    log(shorten_end(str, 10) == "This is...", "shorten_end with length 10");
    log(shorten_begin(str, 10) == "... string", "shorten_begin with length 10");
    log(shorten_end(str, 100) == str, "shorten_end with length greater than string length should return original string");
    log(shorten_begin(str, 100) == str, "shorten_begin with length greater than string length should return original string");
    try {
      shorten_end(str, 2);
      log(false, "shorten_end should throw an error for maxLength less than TRIM_WITH length");
    } catch (const std::runtime_error &e) {
      log(find_error(e, "maxLength must be at least 3 (TRIM_WITH length)"), "shorten_end should throw an error for maxLength less than TRIM_WITH length");
    }
    try {
      shorten_begin(str, 2);
      log(false, "shorten_begin should throw an error for maxLength less than TRIM_WITH length");
    } catch (const std::runtime_error &e) {
      log(find_error(e, "maxLength must be at least 3 (TRIM_WITH length)"), "shorten_begin should throw an error for maxLength less than TRIM_WITH length");
    }
    static_assert(shorten_end("This is a pretty long c-string", 10) == "This is...", "shorten_end constexpr with length 10");
    static_assert(shorten_begin("This is a pretty long c-string", 10) == "...-string", "shorten_begin constexpr with length 10");
    static_assert(shorten_end("This is a pretty long c-string", 100) == "This is a pretty long c-string", "shorten_end constexpr with length greater than string length should return original string");
    static_assert(shorten_begin("This is a pretty long c-string", 100) == "This is a pretty long c-string", "shorten_begin constexpr with length greater than string length should return original string");
  }



  title("Testing cslib::separate"); {
    std::vector<str_t> result = separate("John Money", " ");
    log(result.size() == 2, "separate should return 2 parts");
    log(result.at(0) == "John", "First part should be 'John'");
    log(result.at(1) == "Money", "Second part should be 'Money'");
    result = separate("John  Money", " ");
    log(result.size() == 3, "separate should return 3 parts for double spaces");
    log(result.at(0) == "John", "First part should be 'John'");
    log(result.at(1) == "", "Second part should be empty for double space");
    log(result.at(2) == "Money", "Third part should be 'Money'");
    log(separate("", ",").empty(), "separate should return empty vector for empty string");
    log(separate("John Money", "X").at(0) == "John Money", "separate with non-existing delimiter should return the whole string in a vector");
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
    // std::filesystem::path testFilePath = std::filesystem::temp_directory_path() / "cslib_test_io.txt";
    const TempFile TEST_FILE;
    TEST_FILE.edit_text("John\nMoney\n");
    std::wifstream inFile(TEST_FILE.isAt);
    log(read_data(inFile) == "John\nMoney\n", "read_wdata file reading should return file content");
    log(read_data(inFile) == "John\nMoney\n", "read_wdata should still same content after reading again");
    inFile.close();
    log(read_data(inFile) == "", "read_wdata should return empty after closing stream");
    std::wifstream inFile2(TEST_FILE.isAt);
    log(read_data(inFile2) == "John\nMoney\n", "read_wdata should return expected content after reopening file on different stream");

    // do_io by itself
    std::wostringstream woss;
    do_io(inFile2, woss);
    log(woss.str() == "John\nMoney\n", "do_io should read from input stream and write to output stream");
  }
}