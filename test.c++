#include "./cslib.h++"
using namespace cslib;


int failedTests = 0;

void title(const auto& title) {
  std::cout << "\n\033[1;34m" << title << "\033[0m\n";
}


enum error_throw_kind_issue {
  WRONG_WHAT, // .what doesn't match expected
  EXCEPT_T_WRONG, // Exception type doesn't match expected
  DIDNT_THROW, // Function didn't throw
  ALL_GOOD // Throwing behavior is as expected
};

void log(bool conditionResult, const auto&... conditionsExplained) {
  if (conditionResult)
    std::cout << "\033[1;32m[PASSED]\033[0m";
  else {
    std::cout << "\033[1;31m[FAILED]\033[0m";
    failedTests++;
  }
  std::cout << " > ";
  ((std::cout << std::forward<decltype(conditionsExplained)>(conditionsExplained)), ...);
  std::cout << std::endl;
  if (!conditionResult)
    cslib::pause(1000);
}

void log(error_throw_kind_issue kind, const auto&... conditionsExplained) {
  // Same as above but listing error type
  if (kind == ALL_GOOD)
    std::cout << "\033[1;32m[PASSED]\033[0m";
  else {
    std::cout << "\033[1;31m[FAILED]\033[0m";
    failedTests++;
  }
  std::cout << " > ";
  ((std::cout << std::forward<decltype(conditionsExplained)>(conditionsExplained)), ...);
  switch (kind) {
    case ALL_GOOD: std::cout << " (all good)"; break;
    case WRONG_WHAT: std::cout << " (wrong .what)"; break;
    case EXCEPT_T_WRONG: std::cout << " (wrong exception type)"; break;
    case DIDNT_THROW: std::cout << " (didn't throw at all)"; break;
    default: std::cout << " (unknown error type)"; break;
  }
  std::cout << std::endl;
  if (kind != ALL_GOOD)
    cslib::pause(1000);
}

bool find_error(const std::exception& e, std::string_view lookFor) {
  return std::string(e.what()).find(lookFor) != std::string::npos;
}

bool did_throw(const auto& _func, std::string_view _expectError) {
  try {
    _func();
    return false;
  }
  catch (const std::exception& e) {
    return find_error(e, _expectError);
  }
}

template <typename Exception_T = cslib::any_error>
error_throw_kind_issue try_result(const auto& _func, std::string_view _expectError) {
  try {
    _func();
    return DIDNT_THROW;
  }
  catch (const Exception_T& e) {
    if (find_error(e, _expectError))
      return ALL_GOOD;
    return WRONG_WHAT;
  }
  catch (const std::exception& e) {
    return EXCEPT_T_WRONG;
  }
  catch (...) {
    throw std::runtime_error("Caught unknown exception"); // Can't continue
  }
}

#define fn(...) [&] { __VA_ARGS__; } // inlined function aka lambda


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
    log(try_result<std::runtime_error>(fn(to_str(L'\x100')), "Can't convert wide character to narrow ones"), "to_str(wchar_t) should throw for non-representable character");
    log(try_result<std::runtime_error>(fn(to_str(L"Hello \x100 World")), "Can't convert wide string to narrow string"), "to_str(const wchar_t *const) should throw for non-representable character");
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
    shouldBeWhat << " on line " << __LINE__ + 2 << " because: "; // throw_up is called 2 lines beneath
    shouldBeWhat << "CSLIB_TEST 1234.56";
    log(try_result(fn(throw_up("CSLIB", '_', L"TEST", L' ', 123, 4.56f)), shouldBeWhat.str()), "throw_up should throw an error");
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
    log(sh_call("echo") == "\n", "system call to echo");
    log(try_result(fn(sh_call("non_existing_command")), "Command failed or not found: 'non_existing_command'"), "sh_call should throw an error for non-existing command");
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
    std::function<int()> stdFunc = [] {
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
    int (*cPtrFunc)() = [] {
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
    auto lambdaFunc = [] {
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
    result = retry([] {
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
      retry([] {
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
    log(try_result<std::runtime_error>(fn(parse_cli_args(0, nullptr)), "No command line arguments provided"), "parse_cli_args should throw an error for zero arguments");
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
    log(try_result(fn(shorten_end(str, 2)), "maxLength must be at least 3 (TRIM_WITH length)"), "shorten_end should throw an error for maxLength less than TRIM_WITH length");
    log(try_result(fn(shorten_begin(str, 2)), "maxLength must be at least 3 (TRIM_WITH length)"), "shorten_begin should throw an error for maxLength less than TRIM_WITH length");
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
    for ([[maybe_unused]] auto _ : range(100'000)) {
      int result = roll_dice(min, max);
      if (!(result >= min && result <= max))
        correctRange = false;
    }
    log(correctRange, "roll_dice should return a value between ", min, " and ", max);
    log(roll_dice(1, 1) == 1, "roll_dice with same min and max should return that value");
  }



  title("Testing cslib::read_data and cslib::do_io"); {
    const TempFile TEST_FILE;
    TEST_FILE.edit_text("John\nMoney\n");
    std::ifstream inFile(TEST_FILE);
    log(read_data(inFile) == "John\nMoney\n", "read_wdata file reading should return file content");
    log(read_data(inFile) == "John\nMoney\n", "read_wdata should still same content after reading again");
    inFile.close();
    log(read_data(inFile) == "", "read_wdata should return empty after closing stream");
    std::ifstream inFile2(TEST_FILE);
    log(read_data(inFile2) == "John\nMoney\n", "read_wdata should return expected content after reopening file on different stream");

    // do_io by itself
    std::ostringstream woss;
    do_io(inFile2, woss);
    log(woss.str() == "John\nMoney\n", "do_io should read from input stream and write to output stream");
  }



  title("Testing cslib::TimeStamp"); {
    TimeStamp ts1;
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Must be away from 1 second to avoid test issues below
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(TimeStamp().timePoint - ts1.timePoint).count();
    log(elapsed >= 499 and elapsed <= 510, "TimeStamp should be within 510ms of current time");

    // Correctness of formatting
    std::time_t now = std::chrono::system_clock::to_time_t(ts1.timePoint);
    std::tm *tm = std::localtime(&now);
    log(((std::ostringstream() << std::put_time(tm, "%H:%M:%S %d-%m-%Y")).str() == TimeStamp().as_str()), "TimeStamp should format time correctly");

    // Making sure TimeStamp returns correct values
    TimeStamp rightNow;
    log(rightNow.year() == unsigned(tm->tm_year + 1900), "TimeStamp should return correct year");
    log(rightNow.month() == unsigned(tm->tm_mon + 1), "TimeStamp should return correct month");
    log(rightNow.day() == unsigned(tm->tm_mday), "TimeStamp should return correct day");
    log(rightNow.hour() == unsigned(tm->tm_hour), "TimeStamp should return correct hour");
    log(rightNow.minute() == unsigned(tm->tm_min), "TimeStamp should return correct minute");
    log(rightNow.second() == unsigned(tm->tm_sec), "TimeStamp should return correct second");

    // Testing constructor
    TimeStamp ts2(30, 45, 12, 25, 12, 2023); // 12:45:30 on 25th December 2023
    log(ts2.year() == 2023, "TimeStamp should return correct year from constructor");
    log(ts2.month() == 12, "TimeStamp should return correct month from constructor");
    log(ts2.day() == 25, "TimeStamp should return correct day from constructor");
    log(ts2.hour() == 12, "TimeStamp should return correct hour from constructor");
    log(ts2.minute() == 45, "TimeStamp should return correct minute from constructor");
    log(ts2.second() == 30, "TimeStamp should return correct second from constructor");
    log(ts2.as_str() == "12:45:30 25-12-2023", "TimeStamp should format specific time correctly");

    // Error handling
    log(try_result(fn(TimeStamp(61, 0, 0, 1, 1, 2023)), "Invalid time"), "TimeStamp should throw an error for invalid time (seconds > 59)");
  }



  title("Testing cslib::Out"); {
    std::ostringstream oss;
    Out(oss, "[checking cslib::Out]", Cyan) << "narrow" << '_' << 123 << 3.14f;
    str_t expected = to_str("[") + TimeStamp().as_str() + "]\033[36m[checking cslib::Out] \033[0mnarrow_1233.14";
    log(oss.str() == expected, "cslib::Out with a mix of streamable types should format correctly");
  }



  title("Testing cslib::Benchmark"); {
    Benchmark bm1;
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Sleep for 0.5 seconds
    double elapsed = bm1.elapsed_ms();
    log(elapsed >= 499 && elapsed <= 510, "Benchmark should measure time correctly (took ", elapsed, "ms)");
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Sleep for another 0.5 seconds
    elapsed = bm1.elapsed_ms();
    log(elapsed >= 998 && elapsed <= 1020, "Benchmark should measure time correctly after another 0.5 seconds (took ", elapsed, "ms)");
    bm1.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Sleep for 0.5 seconds after reset
    elapsed = bm1.elapsed_ms();
    log(elapsed >= 499 && elapsed <= 510, "Benchmark should reset and measure time correctly after reset (took ", elapsed, "ms)");
  }



  title("Testing cslib::Road on its own"); {
    const TempFile FILE;

    // Testing ability to read file properties
    Road road(FILE.wstr());
    log(TimeStamp(road.last_modified()).as_str() == TimeStamp().as_str(), "Road should have the correct last modified time");
    log(road.type() == std::filesystem::file_type::regular, "Road should create a regular file");

    // Depth and position checks
    Road rofFileParent(FILE.parent());
    log(rofFileParent.type() == std::filesystem::file_type::directory, "Road should create a directory for the parent path");
    log(rofFileParent == std::filesystem::temp_directory_path(), "Road parent path should be the temp directory path");
    #ifdef _WIN32
      const size_t EXPECTED_DEPTH = 6; // e.g., C:\Users\Username\AppData\Local\Temp\cslib_test_log.txt
    #else
      const size_t EXPECTED_DEPTH = 2; // e.g., /tmp/cslib
    #endif
    log(road.depth() == EXPECTED_DEPTH, "Road should have the correct depth for temp file");

    // renaming self
    str_t previousName = road.name(); // Backup previous name
    str_t tempName = TempFile().name(); // Deletes rvalue File after for access
    road.rename_self_to(tempName); // Warning: FILE now points to invalid file
    log(road.name() == tempName, "Road should rename itself correctly");
    road.rename_self_to(previousName); // Restore previous name
    log(try_result(fn(road.rename_self_to("/someFolder")), "contains path separators"), "Road should throw an error when trying to change location");
    {
      TempFile occupyFileName;
      log(try_result(fn(road.rename_self_to(occupyFileName.name())), " already exists"), "Road should throw an error when trying to rename self to something existing");
    }

    // Equality and inequality checks
    Road roadCopy(road);
    log(roadCopy == road, "Road == operator should work for same file paths");
    log(roadCopy != Road("../"), "Road != operator should work for different file paths");
    log(roadCopy == FILE.str(), "Road == operator should work for strings");
    log(roadCopy != (FILE.str() + "???"), "Road != operator should work for different strings");

    // Conversions to other types
    log(std::string(road) == FILE.str(), "Road should convert to string correctly");
    std::filesystem::path& logFileAsFsRef = road;
    log(logFileAsFsRef == road, "Road should convert to filesystem path reference correctly");
    const std::filesystem::path& logFileAsFsConstRef = road;
    log(logFileAsFsConstRef == road, "Road should convert to filesystem path const reference correctly");
    std::filesystem::path logFileAsFsCopy = road;
    log(logFileAsFsCopy == road, "Road should convert to filesystem path copy correctly");
    std::filesystem::path* logFileAsFsPtr = road;
    log(logFileAsFsPtr == (void*)&road, "Road should return its filesystem path pointer correctly");
    const std::filesystem::path* logFileAsFsConstPtr = road;
    log(logFileAsFsConstPtr == (void*)&road, "Road should return its filesystem path const pointer correctly");

    // Constructors
    log(try_result<std::filesystem::filesystem_error>(fn(Road invalidPath("non_existing_path/cslib_test_log.txt")), "No such file or directory"), "Road should throw an error for non-existing path");
    log(try_result(fn(Road emptyPath("../", std::filesystem::file_type::regular)), "initialized with unexpected file type"), "Road should throw an error for unexpected file type at construction");
  }



  title("Testing child cslib::Folder class of cslib::Path"); {
    TempFolder tempFolder;

    // Creation
    log(try_result(fn(Folder nonExistingFolder("_its_unlikely_that_there_is_a_folder_with_this_name_")), "is not a directory"), "Folder constructor should throw an error for non-existing folder"); // Implicit for stl ::is_directory
    log(try_result(fn(Folder emptyFolder("")), "Path empty"), "Folder constructor should throw an error for empty path");
    log(try_result(fn(Folder invalidTypeFolder(TempFile().str())), "is not a directory"), "Folder constructor should throw an error for file path");

    // Valid folder creation
    log(std::filesystem::exists(tempFolder.wstr()), "Folder should be created at the specified path");
    log(tempFolder.type() == std::filesystem::file_type::directory, "Folder should be of type directory");

    // Try folder listing and checking
    {
      // Put stuff into place
      TempFolder subFolder;
      subFolder.move_self_into(tempFolder);
      TempFolder subsubFolder;
      subsubFolder.move_self_into(subFolder);
      TempFile subsubFile1, subFile2, subFile3;
      subsubFile1.move_self_into(subsubFolder);
      subFile2.move_self_into(subFolder);
      subFile3.move_self_into(subFolder);
      log(subFolder.list().size() == 3, "Folder should have 3 items in the list");
      str_t content = stringify_container(subFolder.list());
      log(content.find(subsubFolder.str()) != str_t::npos, "Folder list should contain subfolder");
      log(subFolder.has(subsubFolder), "Folder .has() method should recognize subfolder");
      log(content.find(subFile2.str()) != str_t::npos, "Folder list should contain subFile2");
      log(subFolder.has(subFile2), "Folder .has() method should recognize subFile2");
      log(content.find(subFile3.str()) != str_t::npos, "Folder list should contain subFile3");
      log(subFolder.has(subFile3), "Folder .has() method should recognize subFile3");
    }
    log(tempFolder.list().size() == 0, "Folder should be empty after temporary items are out of scope");

    // Testing copy on disk methods
    {
      TempFolder dummyFolder;
      TempFolder subFolder;
      TempFile subFile, subsubFile;
      subFolder.move_self_into(dummyFolder);
      subFile.move_self_into(dummyFolder);
      subsubFile.move_self_into(subFolder);
      TempFolder targetFolder;
      Folder dummyFolderCopy = dummyFolder.copy_self_into(targetFolder);
      Folder dummySubCopyFolder;
      for (const Road &item : dummyFolderCopy.list())
        if (item.type() == std::filesystem::file_type::directory) {
          dummySubCopyFolder = Folder(item.wstr());
          break;
        }
      File dummySubFile(dummySubCopyFolder.list().front());
      log(std::filesystem::exists(targetFolder.wstr()), "Target folder should exist after copy");
      log(targetFolder.has(dummyFolderCopy), "Target folder should have dummyFolder after copy");
      log(dummyFolderCopy.has(dummySubCopyFolder), "Dummy folder copy should have subfolder copy");
      log(dummySubCopyFolder.has(dummySubFile), "Dummy subfolder copy should have subfile copy");
    }
  }



  title("Testing child class of cslib::Path, cslib::File"); {
    TempFile tempFile;

    // Creation
    log(try_result(fn(File nonExistingFile("_its_unlikely_that_there_is_a_file_with_this_name_")), "is not a regular file"), "File constructor should throw an error for non-existing file"); // Implicit for stl ::is_regular_file
    log(try_result(fn(File emptyFile("")), "Path empty"), "File constructor should throw an error for empty path");
    log(try_result(fn(File invalidTypeFile(TempFolder().str())), "is not a regular file"), "File constructor should throw an error for directory path");

    // Valid file creation
    log(std::filesystem::exists(tempFile.wstr()), "File should be created at the specified path");
    log(tempFile.type() == std::filesystem::file_type::regular, "File should be of type regular");

    // File I/O
    log(tempFile.read_text() == "", "File should be empty after creation");
    tempFile.edit_text("Hello, world!");
    log(tempFile.read_text() == "Hello, world!", "File should be written correctly");
    std::string binaryData;
    for (char i : range(lowest_value_of<char>(), highest_value_of<char>())) // Append all kinds of possible data
      binaryData += i;
    tempFile.edit_binary(binaryData.data(), binaryData.size());
    log(tempFile.read_binary() == std::vector<char>(binaryData.begin(), binaryData.end()), "File should read binary data correctly");

    // Other
    log(tempFile.extension() == ".tmp", "Function should recognize the file extension");
    log(tempFile.bytes() == binaryData.size(), "Function should return the correct file size");

    // Move and copy on disk
    TempFolder srcFolder;
    TempFolder dstFolder;
    tempFile.move_self_into(srcFolder);
    log(tempFile.copy_self_into(dstFolder).read_binary() == tempFile.read_binary(), "File and its copy should have the same content");
  }



  title("Testing cslib:Folder::typed_list (was implemented after declartion of cslib::File)"); {
    TempFolder tempFolder;
    TempFolder dummyFolder;
    TempFile dummyFile, dummySubFile;
    log(dummyFolder.typed_list().size() == 0, "Folder should be empty");
    dummyFolder.move_self_into(tempFolder);
    dummyFile.move_self_into(tempFolder);
    dummySubFile.move_self_into(dummyFolder);
    const std::vector<std::variant<File, Folder, Road>> typedList = tempFolder.typed_list();
    int items = 0;
    for (std::variant<File, Folder, Road> item : typedList)
      if (std::holds_alternative<File>(item))
        items++;
    log(items == 1, "TempFolder should contain one file");
    items = 0;
    for (std::variant<File, Folder, Road> item : typedList)
      if (std::holds_alternative<Folder>(item))
        items++;
    log(items == 1, "TempFolder should contain one subfolder");
    items = 0;
    for (std::variant<File, Folder, Road> item : typedList)
      if (std::holds_alternative<Road>(item))
        items++;
    log(items == 0, "TempFolder shouldn't contain any other file types");
    items = 0;
    for (std::variant<File, Folder, Road> item : typedList)
      if (std::holds_alternative<Folder>(item))
        for (std::variant<File, Folder, Road> subItem : std::get<Folder>(item).typed_list())
          if (std::holds_alternative<File>(subItem))
            items++;
    log(items == 1, "Subfolder of TempFolder should contain one file");
  }



  title("Testing cslib::scramble_filename"); {
    log(cslib::scramble_filename().length() == SCRAMBLE_LEN, "Scrambled filename should have the correct length");
    log(cslib::scramble_filename().find('?') == std::string::npos, "Scrambled filename should not contain the invalid character");
  }



  title("Skipping cslib::TempFolder and cslib::File as they were tested previously");



  title("Testing cslib::wget"); {
    log(cslib::wget("https://www.example.com").find("<title>") != std::string::npos, "wget should retrieve the webpage");
  }



  title("Testing cslib::highest_value_of and cslib::lowest_value_of"); {
    log(cslib::highest_value_of<char>() == std::numeric_limits<char>::max(), "highest_value_of<char> should return the maximum value of char");
    log(cslib::lowest_value_of<char>() == std::numeric_limits<char>::min(), "lowest_value_of<char> should return the minimum value of char");
  }


  if (failedTests != 0) {
    std::cout << "\n >> " << failedTests << " tests failed." << std::endl;
    return EXIT_FAILURE;
  } else {
    std::cout << "\n >> " << "All tests passed." << std::endl;
    return EXIT_SUCCESS;
  }
}