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
template <typename T>
void log(std::optional<T> conditionResult, const auto&... conditionsExplained) {
  log(conditionResult.has_value(), conditionsExplained...);
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

template <typename Exception_T = cslib::any_error>
error_throw_kind_issue try_result(const auto& _func, strv_t _expectError, const auto&... args) {
  try {
    _func(args...);
    return DIDNT_THROW;
  }
  catch (const Exception_T& e) {
    if (e.what() == _expectError)
      return ALL_GOOD;
    std::cout << "Expected: " << _expectError << ", but got: " << e.what() << std::endl;
    return WRONG_WHAT;
  }
  catch (const std::exception& e) {
    return EXCEPT_T_WRONG;
  }
  catch (...) {
    throw std::runtime_error("Caught unknown exception"); // Can't continue
  }
}

str_t cslib_throw_header(int _line, str_t place, const auto&... _msgs) {
  std::ostringstream oss;
  oss << "cslib::any_error called in workspace " << std::filesystem::current_path() << ' ';
  oss << "on line " << _line << " in function '" << place << "' because: ";
  ((oss << _msgs), ...);
  return oss.str();
}

#define fn(...) [&] { __VA_ARGS__; } // inlined function aka lambda


Benchmark bm;



int main() {
  title("Testing cslib::to_str"); {
    log(to_str(123) == "123", "to_str(const auto&) int");
    log(to_str(3.14f) == "3.14", "to_str(const auto&) float");
    log(to_str(3.14) == "3.14", "to_str(const auto&) double");
    log(to_str(std::filesystem::path(L"/path/to/file")) == "\"/path/to/file\"", "to_str(const auto&)");
    log(to_str('A') == "A", "to_str(const auto&)");
    log(to_str(str_t("")) == "", "to_str(const auto&)");
    log(to_str(str_t("Hello World")) == "Hello World", "to_str(const auto&)");
    log(to_str(str_t("This is a pretty long string")) == "This is a pretty long string", "to_str(const auto&) long string");
    log(to_str(strv_t("Hello World")) == "Hello World", "to_str(const auto&)");
  }



  title("Testing cslib::any_error and cslib_throw_up (Won't fix skipping)"); {
    str_t shouldBeWhat = cslib_throw_header(__LINE__ + 1, "operator()",  "CSLIB_1234.56"); // operator() because fn macro (else main)
    log(try_result(fn(cslib_throw_up("CSLIB", '_', 123, 4.56f)), shouldBeWhat), "cslib_throw_up should throw an error with correct message");
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
    log(sh_call("echo").value() == "\n", "system call to echo");
    log(!sh_call("non_existing_command"), "sh_call should recognize errors.");
    log(sh_call("non_existing_command").error() != 0, "sh_call should return the error-code upon faliure");
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



  title("Testing cslib::have_common"); {
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
    log(have_common(vec1, vec2), "have_common(vector, vector) should be true");
    log(have_common(deq1, deq2), "have_common(deque, deque) should be true");
    log(have_common(set1, set2), "have_common(set, set) should be true");
    log(have_common(lst1, lst2), "have_common(list, list) should be true");
    log(have_common(arr1, arr2), "have_common(array, array) should be true");
    log(have_common(carr1, carr2), "have_common(constexpr array, constexpr array) should be true");
    log(!have_common(vec1, vec3), "have_common(vector, vector) should be false");
    log(!have_common(deq1, deq3), "have_common(deque, deque) should be false");
    log(!have_common(set1, set3), "have_common(set, set) should be false");
    log(!have_common(lst1, lst3), "have_common(list, list) should be false");
    log(!have_common(arr1, arr3), "have_common(array, array) should be false");
    log(!have_common(carr1, carr3), "have_common(constexpr array, constexpr array) should be false");
    log(!have_common(vec1, vec4), "have_common(vector, empty vector) should be false");
    log(!have_common(deq1, deq4), "have_common(deque, empty deque) should be false");
    log(!have_common(set1, set4), "have_common(set, empty set) should be false");
    log(!have_common(lst1, lst4), "have_common(list, empty list) should be false");
    log(!have_common(arr1, arr4), "have_common(array, empty array) should be false");
    static_assert(have_common(carr1, carr2), "have_common(constexpr array, constexpr array) should be true at compile time");
    static_assert(!have_common(carr1, carr3), "have_common(constexpr array, constexpr array) should be false at compile time");
    static_assert(!have_common(carr1, carr4), "have_common(constexpr array, empty constexpr array) should be false at compile time");
  }



  title("Testing cslib::get_env"); {
    maybe<str_t> validEnv = get_env("PATH");
    maybe<str_t> errorEnv = get_env("####");
    log(!!validEnv, "get_env should be able to get valid environment variables");
    log(!errorEnv, "get_env should return an error for non-existing environment variable");
    log(errorEnv.error() == "Environment variable '####' not found", "get_env should return the correct error message for non-existing environment variable");
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
    std::vector<int> r7 = range(0);
    log(r7 == std::vector<int>(), "range(0) should return []");
  }



  title("Testing cslib::retry part 1 stl"); {
    const std::function<int(int)> badFunc = [](int _attempts) {
      static int attempts = 0;
      if (++attempts < _attempts)
        throw std::runtime_error("Test error from std::function<int()>");
      return 42;
    };
    maybe<int> noResult = retry(badFunc, 1, 2);
    maybe<int> okResult = retry(badFunc, 2, 2);
    maybe<int> timedOut = retry(badFunc, 0, 2);
    log(!noResult, "retry should return an error for function that fails too many times");
    log(noResult.error() == "Function failed after maximum attempts: Test error from std::function<int()>",
        "retry should return the correct error message for function that fails too many times");
    log(okResult.value() == 42, "retry should return the correct value for function that succeeds");
    log(!timedOut, "retry should return an error for function when _maxAttempts is 0");
    log(timedOut.error() == "Function never invoked because _maxAttempts is 0", "retry should return the correct error message for function when _maxAttempts is 0");
  }



  title("Testing cslib::retry part 2 C-style pointer"); {
    int (*const badFunc)(int) = [](int _attempts) -> int {
      static int attempts = 0;
      if (++attempts < _attempts)
        throw std::runtime_error("Test error from c-style function pointer");
      return 42;
    };
    maybe<int> noResult = retry(badFunc, 1, 2);
    maybe<int> okResult = retry(badFunc, 2, 2);
    maybe<int> timedOut = retry(badFunc, 0, 2);
    log(!noResult, "retry should return an error for function that fails too many times");
    log(noResult.error() == "Function failed after maximum attempts: Test error from c-style function pointer",
        "retry should return the correct error message for function that fails too many times");
    log(okResult.value() == 42, "retry should return the correct value for function that succeeds");
    log(!timedOut, "retry should return an error for function when _maxAttempts is 0");
    log(timedOut.error() == "Function never invoked because _maxAttempts is 0", "retry should return the correct error message for function when _maxAttempts is 0");
  }



  title("Testing cslib::retry part 3 inline lambda"); {
    maybe<int> noResult = retry([](int _attempts) -> int {
      static int attempts = 0;
      if (++attempts < _attempts)
        throw std::runtime_error("Test error from inline lambda function");
      return 42;
    }, 1, 2);
    maybe<int> okResult = retry([](int _attempts) -> int {
      static int attempts = 0;
      if (++attempts < _attempts)
        throw std::runtime_error("Test error from inline lambda function");
      return 42;
    }, 2, 2);
    maybe<int> timedOut = retry([](int _attempts) -> int {
      static int attempts = 0;
      if (++attempts < _attempts)
        throw std::runtime_error("Test error from inline lambda function");
      return 42;
    }, 0, 2);
    log(!noResult, "retry should return an error for function that fails too many times");
    log(noResult.error() == "Function failed after maximum attempts: Test error from inline lambda function",
        "retry should return the correct error message for function that fails too many times");
    log(okResult.value() == 42, "retry should return the correct value for function that succeeds");
    log(!timedOut, "retry should return an error for function when _maxAttempts is 0");
    log(timedOut.error() == "Function never invoked because _maxAttempts is 0", "retry should return the correct error message for function when _maxAttempts is 0");
  }



  title("Testing cslib::parse_cli_args"); {
    const char *const goodArgs[] = {"program", "arg1", "arg2", "arg3"};
    const char *const badArgs[] = {nullptr};
    const char *const noArgs[] = {"program"};
    maybe<std::vector<strv_t>> parsedGoodArgs = parse_cli_args(4, goodArgs);
    maybe<std::vector<strv_t>> parsedBadArgs = parse_cli_args(0, badArgs);
    maybe<std::vector<strv_t>> parsedNoArgs = parse_cli_args(1, noArgs);
    maybe<std::vector<strv_t>> parsedInvalid = parse_cli_args(0, nullptr);
    log(!!parsedGoodArgs, "parse_cli_args should accept valid arrays");
    log(parsedGoodArgs.value() == std::vector<strv_t>{"program", "arg1", "arg2", "arg3"}, "parse_cli_args should return the correct values for valid arrays");
    log(!parsedBadArgs, "parse_cli_args should return a valid vector");
    log(parsedBadArgs.error() == "No command line arguments provided", "parse_cli_args should return the correct error message for empty arrays");
    log(!!parsedNoArgs, "parse_cli_args should return a valid vector");
    log(parsedNoArgs.value().size() == 1 && parsedNoArgs.value().at(0) == "program", "containing only the executable's name");
    log(!parsedInvalid, "parse_cli_args should return an error for null arguments");
    log(parsedInvalid.error() == "No command line arguments provided", "parse_cli_args should return the correct error message for null arguments");
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
    strv_t longString = "This string is not short-string-optimized";
    maybe<str_t> longStringCutBegin = shorten_begin(longString, 10);
    maybe<str_t> longStringCutEnd = shorten_end(longString, 10);
    maybe<str_t> longStringDefBegin = shorten_begin(longString, 100);
    maybe<str_t> longStringDefEnd = shorten_end(longString, 100);
    maybe<str_t> longStringInvBegin = shorten_begin(longString, 2); // is unexpected
    maybe<str_t> longStringInvEnd = shorten_end(longString, 2); // is unexpected
    log(longStringCutBegin.value() == "...timized", "shorten_begin with length 10 for (no sso)");
    log(longStringCutEnd.value() == "This st...", "shorten_end with length 10 (no sso)");
    log(longStringDefBegin.value() == longString, "shorten_begin with length greater than string length should return original string (no sso)");
    log(longStringDefEnd.value() == longString, "shorten_end with length greater than string length should return original string (no sso)");
    log(!longStringInvBegin, "shorten_begin with length shorter than the appended fill-in should be considered invalid (no sso)");
    log(longStringInvBegin.error() == "maxLength must be at least 3 ('...' length)", "shorten_begin with length shorter than the appended fill-in should return the correct error (no sso)");
    log(!longStringInvEnd, "shorten_end with length shorter than the appended fill-in should be considered invalid (no sso)");
    log(longStringInvEnd.error() == "maxLength must be at least 3 ('...' length)", "shorten_end with length shorter than the appended fill-in should return the correct error (no sso)");

    strv_t shortString = "but this is";
    maybe<str_t> shortStringCutBegin = shorten_begin(shortString, 10);
    maybe<str_t> shortStringCutEnd = shorten_end(shortString, 10);
    maybe<str_t> shortStringDefBegin = shorten_begin(shortString, 100);
    maybe<str_t> shortStringDefEnd = shorten_end(shortString, 100);
    maybe<str_t> shortStringInvBegin = shorten_begin(shortString, 2); // is unexpected
    maybe<str_t> shortStringInvEnd = shorten_end(shortString, 2); // is unexpected
    log(shortStringCutBegin.value() == "...this is", "shorten_begin with length 10 for (sso)");
    log(shortStringCutEnd.value() == "but thi...", "shorten_end with length 10 (sso)");
    log(shortStringDefBegin.value() == shortString, "shorten_begin with length greater than string length should return original string (sso)");
    log(shortStringDefEnd.value() == shortString, "shorten_end with length greater than string length should return original string (sso)");
    log(!shortStringInvBegin, "shorten_begin with length shorter than the appended fill-in should be considered invalid (sso)");
    log(shortStringInvBegin.error() == "maxLength must be at least 3 ('...' length)", "shorten_begin with length shorter than the appended fill-in should return the correct error (sso)");
    log(!shortStringInvEnd, "shorten_end with length shorter than the appended fill-in should be considered invalid (sso)");
    log(shortStringInvEnd.error() == "maxLength must be at least 3 ('...' length)", "shorten_end with length shorter than the appended fill-in should return the correct error (sso)");
    static_assert(shorten_end("This is a pretty long c-string", 10) == "This is...", "shorten_end constexpr with length 10");
    static_assert(shorten_begin("This is a pretty long c-string", 10) == "...-string", "shorten_begin constexpr with length 10");
    static_assert(shorten_end("This is a pretty long c-string", 100) == "This is a pretty long c-string", "shorten_end constexpr with length greater than string length should return original string");
    static_assert(shorten_begin("This is a pretty long c-string", 100) == "This is a pretty long c-string", "shorten_begin constexpr with length greater than string length should return original string");
  }



  title("Testing cslib::separate"); {
    std::vector<str_t> result = separate("John Money", ' '); // Also tests for char as delimiter
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
    TEST_FILE.edit_text("John\nMoney\n").value(); // Ignore no-discard warning
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
    TimeStamp ts2(12, 45, 30, 25, 12, 2023); // 12:45:30 on 25th December 2023
    log(ts2.year() == 2023, "TimeStamp should return correct year from constructor");
    log(ts2.month() == 12, "TimeStamp should return correct month from constructor");
    log(ts2.day() == 25, "TimeStamp should return correct day from constructor");
    log(ts2.hour() == 12, "TimeStamp should return correct hour from constructor");
    log(ts2.minute() == 45, "TimeStamp should return correct minute from constructor");
    log(ts2.second() == 30, "TimeStamp should return correct second from constructor");
    log(ts2.as_str() == "12:45:30 25-12-2023", "TimeStamp should format specific time correctly");

    // Error handling
    str_t errHeaderTime = cslib_throw_header(412, "TimeStamp", "Invalid time: 25:-1:61"); // Shifts by line
    log(try_result(fn(TimeStamp(25, -1, 61, 1, 1, 2023)), errHeaderTime), "TimeStamp should throw an error for invalid time");
    str_t errHeaderDate = cslib_throw_header(409, "TimeStamp", "Invalid date: 32-13--1");
    log(try_result(fn(TimeStamp(12, 30, 10, 32, 13, -1)), errHeaderDate), "TimeStamp should throw an error for invalid date");
  }



  title("Testing cslib::Out"); {
    std::ostringstream oss;
    Out(oss, "[checking cslib::Out]", Cyan) << "narrow" << '_' << 123 << 3.14f;
    str_t expected = to_str("[") + TimeStamp().as_str() + "]\033[36m[checking cslib::Out] \033[0mnarrow_1233.14";
    log(oss.str() == expected, "cslib::Out with a mix of streamable types should format correctly");
  }



  title("Testing cslib::Benchmark"); {
    Benchmark bm1;
    std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Sleep for 0.05 seconds
    double elapsed = bm1.elapsed_ms();
    log(elapsed >= 49 && elapsed <= 51, "Benchmark should measure time correctly (took ", elapsed, "ms)");
    std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Sleep for another 0.5 seconds
    elapsed = bm1.elapsed_ms();
    log(elapsed >= 98 && elapsed <= 102, "Benchmark should measure time correctly after another 0.05 seconds (took ", elapsed, "ms)");
    bm1.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Sleep for 0.05 seconds after reset
    elapsed = bm1.elapsed_ms();
    log(elapsed >= 49 && elapsed <= 51, "Benchmark should reset and measure time correctly after reset (took ", elapsed, "ms)");
  }



  title("Testing cslib::Road on its children"); {
    const TempFile FILE;

    // Read entry properties
    File road(FILE.str());
    log(TimeStamp(road.last_modified()).as_str() == TimeStamp().as_str(), "Road should have the correct last modified time");
    log(road.type() == std::filesystem::file_type::regular, "Road should create a regular file");

    // Name and position checks
    log(road.name() == road.isAt.filename(), "Road name should match filename");
    if constexpr (IS_WINDOWS)
      log(road.depth() == 6, "Road should have the correct depth for temp file"); // e.g., C:\Users\Username\AppData\Local\Temp\cslib_test_log.txt
    else
      log(road.depth() == 2, "Road should have the correct depth for temp file"); // e.g., /tmp/cslib
    maybe<Folder> root = road[0];
    maybe<Folder> parent = road[road.depth() - 1];
    log(!!root, "Road should have a root folder");
    if (root) {
      str_t stdRoot = std::filesystem::current_path().root_path().string(); // "C:\" or "/"
      log(root->str() == stdRoot, to_str("Road root folder name should be root disk '") + stdRoot + "' but is '" + root->str() + "'");
      log(root->depth() == 0, "Road root folder depth should be 0");
    }

    // renaming self
    str_t previousName = road.name(); // Backup previous name
    str_t tempName = TempFile().name(); // Deletes rvalue File after for access
    maybe<void> resultBefore = road.rename_self_to(tempName); // Warning: cslib::File now points to invalid file
    log(!!resultBefore, "Road should return a valid result when renaming");
    log(road.name() == tempName, "Road should rename itself correctly");
    maybe<void> resultAfter = road.rename_self_to(previousName); // Restore previous name
    log(!road.rename_self_to("f/sf"), "Road should reject an attempt to move instead of renaming");
    log(road.rename_self_to("f/sf").error() == "Filename can't be moved with this function (previous: '" + road.str() + "', new: 'f/sf')", "Road should throw the correct error upon move (on disk) attempt");
    log(road.name() == previousName, "Even after failed attempts to rename, Road should retain its original name");

    // Equality and inequality checks
    File roadCopy(road);
    log(roadCopy == road, "Road == operator should work for same file paths");
    log(roadCopy != Folder("../"), "Road != operator should work for different file paths");
    log(roadCopy == FILE.str(), "Road == operator should work for strings");
    log(roadCopy != (FILE.str() + "???"), "Road != operator should work for different strings");

    // Conversions to other types
    log(str_t(road) == FILE.str(), "Road should convert to string correctly");
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

    // Construction (via BizarreRoad as Road::Road() is protected)
    str_t errHeader = cslib_throw_header(631, "Road", "Path empty");
    log(try_result(fn(BizarreRoad("")), errHeader), "Protected Road constructor should throw an error for empty path");
  }



  title("Skipping cslib::BizarreRoad due to limited testing tools/skills");



  title("Testing child cslib::Folder class of cslib::Road"); {
    TempFolder tempFolder;

    // Construction
    str_t voidFolder = TempFolder().str();
    str_t errorHead = cslib_throw_header(681, "operator()", "Path '", voidFolder, "' is not a directory");
    log(try_result(fn(Folder nonExistingFolder(voidFolder)), errorHead), "Folder constructor should throw an error for non-existing folder when trying to determine if path is a directory");

    // Read properties
    log(tempFolder.list().empty(), "Folders should be empty at creation");
    TempFolder dummyFolder;
    TempFile dummySubFile;

    // Move
    log(!!dummyFolder.move_self_into(tempFolder), "Moving a folder should succeed");
    log(!!dummySubFile.move_self_into(dummyFolder), "Moving a file should work too");
    log(!!tempFolder.find(dummyFolder.name()), "Folder should find its own subfolder");
    log(!!dummyFolder.find(dummySubFile.name()), "Folder should find its own subfile");
    log(std::filesystem::exists(dummyFolder.str() + "/" + dummySubFile.name()), "The subfile should exist in the folder");
    log(tempFolder.list().size() == 1, "Folder should be able to read its contents");
    log(tempFolder.untyped_list().size() == 1, "Folder untyped_list should be accurate too");
    log(std::get<Folder>(tempFolder.list().front()) == dummyFolder, "Folder should contain the moved file");

    // Copy self
    TempFolder copyDest;
    maybe<Folder> copiedTempFolder = tempFolder.copy_self_into(copyDest);
    log(std::filesystem::exists(copyDest / tempFolder.name() / dummyFolder.name() / dummySubFile.name()), "Copying should be recursive by default");
    log(!tempFolder.copy_self_into(copyDest), "Copying shouldn't overwrite existing folders");
    log(tempFolder.copy_self_into(copyDest).error() == "Path '" + tempFolder.str() + "' already exists in folder '" + copyDest.str() + "'", "Copying should fail with the correct error message");

    // Copy content (skipped because those are std::filesystem tests)
  }



  title("Testing child class of cslib::Road, cslib::File"); {
    TempFile tempFile;

    log(tempFile.read_text().value().empty() && tempFile.read_binary().value().empty(), "Newly created temp file should be empty");
    log(!!tempFile.edit_binary(std::vector<int>({1}).data(), 1), "Binary-level edits should be possible");
    log(!!tempFile.read_binary(), "Binary-level reads should be possible");
    log(tempFile.read_binary().value() == std::vector<char>({1}), "Binary-level reads should return the correct data");
    log(!!tempFile.edit_text("1"), "Text-level edits should be possible");
    log(!!tempFile.read_text(), "Text-level reads should be possible");
    log(tempFile.read_text().value() == "1", "Text-level reads should return the correct data");
    log(tempFile.extension() == ".tmp", "Function should recognize the file extension including dot");
    log(tempFile.bytes() == 1, "Function should return the correct file size");
  }



  title("Skipping cslib::TempFolder and cslib::File as they were tested previously");



  title("Testing cslib::wget"); {
    log(wget("https://www.example.com").value().find("<title>") != str_t::npos, "wget should retrieve the webpage");
    log(!wget("impossible://.example.com"), "wget should fail for invalid URLs");
  }

  if (failedTests != 0) {
    std::cout << "\n >> " << failedTests << " tests failed." << std::endl;
    return EXIT_FAILURE;
  } else {
    std::cout << "\n >> " << "All tests passed." << std::endl;
    return EXIT_SUCCESS;
  }
}