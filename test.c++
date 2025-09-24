#include "./cslib.h++"
#include <iostream>
#include <deque>
#include <set>
#include <list>
#include <functional>
using namespace cslib;


// Configs
MACRO HOLD_IF_ERROR = true;
int failedTests = 0;


// Title
void title(strv_t title) {
  std::cout << "\n" << Blue << title << "\033[0m\n";
}


// Error categories for throw tests
enum ThrowCheck {
  WRONG_WHAT, // .what doesn't match expected
  WRONG_TYPE, // Exception type doesn't match expected
  DIDNT_THROW, // Function didn't throw
  ALL_GOOD // Throwing behavior is as expected
};


// Core logger
template <typename... Msgs>
void log_impl(bool conditionResult, strv_t suffix, Msgs&... msgs) {
  if (conditionResult)
    std::cout << Green << "[PASSED]" << Reset;
  else {
    std::cout << Red << "[FAILED]" << Reset;
    ++failedTests;
  }

  std::cout << " > ";
  (std::cout << ... << msgs);
  std::cout << suffix << std::endl;

  if (!conditionResult) {
    if constexpr (HOLD_IF_ERROR)
      std::cin.get();
    else
      cslib::pause(1000);
  }
}


// Overload for simple boolean check
template <typename... Msgs>
void log(bool condition, Msgs&&... msgs) {
  log_impl(condition, "", std::forward<Msgs>(msgs)...);
}


// Overload for exception check
template <typename... Msgs>
void log(ThrowCheck result, Msgs&&... msgs) {
    strv_t suffix;
    switch (result) {
      case ALL_GOOD:    suffix = "(all good)"; break;
      case WRONG_WHAT:  suffix = "(wrong .what)"; break;
      case WRONG_TYPE:  suffix = "(wrong exception type)"; break;
      case DIDNT_THROW: suffix = "(didn't throw)"; break;
    }
    log_impl(result == ALL_GOOD, suffix, std::forward<Msgs>(msgs)...);
}



// Utility macro: inline lambda
#define fn(...) [&]{ __VA_ARGS__; }


Benchmark bm;



int main() {
  title("Testing cslib::to_str"); {
    log(to_str(42) == "42", "int 42");
    log(to_str(3.14) == "3.14", "double 3.14");
    log(to_str('x') == "x", "char x");
    log(to_str("hello") == "hello", "const char* hello");
    log(to_str(std::string("world")) == "world", "std::string world");
    log(to_str(1, " + ", 2, " = ", 3) == "1 + 2 = 3", "mix int + string");
    log(to_str() == "", "empty call should return empty string");
    struct {
      int x, y;
    } point = {3, 4};
    std::ostringstream tmp;
    tmp << point.x << "," << point.y;
    log(to_str(point.x, ",", point.y) == tmp.str(), "Point struct output");
    log(to_str("Value: ", 99, ", Pi: ", 3.14, ", Char: ", 'Z') == "Value: 99, Pi: 3.14, Char: Z", "mixed types");
    log(to_str("Nested: ", to_str(1,2,3)) == "Nested: 123", "nested to_str");
    log(to_str("Line1", "\n", "Line2") == "Line1\nLine2", "newlines preserved");
    log(to_str(std::move(std::string("rvalue"))) == "rvalue", "rvalue string forwarded correctly");
  }



  title("Testing/Benchmarking cslib::pause"); {
    bm.reset();
    pause(0);
    log(bm.elapsed_ms() <= 0.1, "pause(0) should return immediately");
    bm.reset();
    pause(50);
    log(bm.elapsed_ms() >= 50, "pause(50) should wait at least 50ms");
    bm.reset();
    pause(200);
    log(bm.elapsed_ms() >= 200, "pause(200) should wait at least 200ms");
    bm.reset();
    pause(30);
    pause(40);
    log(bm.elapsed_ms() >= 70, "pause(30) + pause(40) should wait at least 70ms");
  }



  title("Testing cslib::unexpect"); {
    auto ue1 = unexpect<std::runtime_error>("simple error");
    try {
      std::rethrow_exception(ue1.error());
    } catch (const std::runtime_error& e) {
      log(std::string(e.what()) == "simple error", "runtime_error with simple message");
    } catch (...) {
      log(false, "Caught wrong exception type for simple message");
    }

    auto ue2 = unexpect<std::logic_error>("Value ", 42, " is invalid");
    try {
      std::rethrow_exception(ue2.error());
    } catch (const std::logic_error& e) {
      log(std::string(e.what()) == "Value 42 is invalid", "logic_error with variadic message");
    } catch (...) {
      log(false, "Caught wrong exception type for variadic message");
    }

    auto ue3 = unexpect<std::runtime_error>();
    try {
      std::rethrow_exception(ue3.error());
    } catch (const std::runtime_error& e) {
      log(std::string(e.what()) == "", "runtime_error with empty message");
    } catch (...) {
      log(false, "Caught wrong exception type for empty message");
    }

    auto ue4 = unexpect<std::runtime_error>("Error code ", 404, ", reason: ", "not found");
    try {
      std::rethrow_exception(ue4.error());
    } catch (const std::runtime_error& e) {
      log(std::string(e.what()) == "Error code 404, reason: not found", "runtime_error with mixed types");
    } catch (...) {
      log(false, "Caught wrong exception type for mixed types");
    }

    struct MyEx : public std::runtime_error { using std::runtime_error::runtime_error; };
    auto ue5 = unexpect<MyEx>("custom exception message");
    try {
      std::rethrow_exception(ue5.error());
    } catch (const MyEx& e) {
      log(std::string(e.what()) == "custom exception message", "user-defined exception works");
    } catch (...) {
      log(false, "Caught wrong type for user-defined exception");
    }
  }



  title("Testing cslib::sh_call"); {
    log(sh_call("cd ./").has_value(), "system call to run");
    log(!sh_call(scramble_name(64)/*random name*/), "sh_call should recognize errors.");
    log(sh_call(scramble_name(64)/*random name*/).error() != 0, "sh_call should return the error-code upon faliure");
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
    std::list<int> myLst1 = {1, 2, 3};
    std::list<int> myLst2 = {3, 4, 5};
    std::list<int> myLst3 = {6, 7, 8};
    std::list<int> myLst4 = {};
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
    log(have_common(myLst1, myLst2), "have_common(list, list) should be true");
    log(have_common(arr1, arr2), "have_common(array, array) should be true");
    log(have_common(carr1, carr2), "have_common(constexpr array, constexpr array) should be true");
    log(!have_common(vec1, vec3), "have_common(vector, vector) should be false");
    log(!have_common(deq1, deq3), "have_common(deque, deque) should be false");
    log(!have_common(set1, set3), "have_common(set, set) should be false");
    log(!have_common(myLst1, myLst3), "have_common(list, list) should be false");
    log(!have_common(arr1, arr3), "have_common(array, array) should be false");
    log(!have_common(carr1, carr3), "have_common(constexpr array, constexpr array) should be false");
    log(!have_common(vec1, vec4), "have_common(vector, empty vector) should be false");
    log(!have_common(deq1, deq4), "have_common(deque, empty deque) should be false");
    log(!have_common(set1, set4), "have_common(set, empty set) should be false");
    log(!have_common(myLst1, myLst4), "have_common(list, empty list) should be false");
    log(!have_common(arr1, arr4), "have_common(array, empty array) should be false");
    static_assert(have_common(carr1, carr2), "have_common(constexpr array, constexpr array) should be true at compile time");
    static_assert(!have_common(carr1, carr3), "have_common(constexpr array, constexpr array) should be false at compile time");
    static_assert(!have_common(carr1, carr4), "have_common(constexpr array, empty constexpr array) should be false at compile time");
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



  title("Testing cslib::retry"); {

    auto r1 = retry([] { return 42; }, 3);
    log(r1.has_value() and *r1 == 42, "function succeeds immediately returns value");

    int counter = 0;
    maybe<int> r2 = retry([&] {
      if (counter++ < 1) throw std::runtime_error("fail once");
      return 99;
    }, 3);
    log(r2.has_value() and *r2 == 99, "function throws once then succeeds");

    maybe<int> r3 = retry([] { throw std::logic_error("always fails"); return 0; }, 2);
    log(!r3.has_value(), "function always throws returns unexpected");
    if (!r3.has_value())
      try { std::rethrow_exception(r3.error()); }
      catch (const std::logic_error& e) { log(std::string(e.what()) == "always fails", "exception preserved"); }

    int x = 0;
    maybe<void> r4 = retry([&] { x = 123; }, 2);
    log(r4.has_value() and x == 123, "void-returning function sets value correctly");

    int y = 0;
    maybe<int> r5 = retry([&] { y = 1; return 1; }, 0);
    log(!r5.has_value() and r5.error() == NULL, "maxAttempts = 0 returns an unexpected null exception ptr");

    auto add = [](int a, int b) { return a + b; };
    maybe<int> r6 = retry(add, 3, 2, 3);
    log(r6.has_value() and *r6 == 5, "function with arguments returns correct sum");

    int z = 0;
    maybe<int> r7 = retry([&](int inc) {
      if (z == 0) { z++; throw std::runtime_error("fail once"); }
      z += inc;
      return z;
    }, 3, 10);
    log(r7.has_value() and *r7 == 11, "throw once with args then succeed returns correct value");
  }


  title("Testing cslib::parse_cli_args"); {

    const char* argv1[] = {"program.exe", "--help", "-v"};
    maybe<std::vector<str_t>> result1 = parse_cli_args(3, argv1);
    log(result1.has_value() && result1.value() == std::vector<str_t>{"program.exe", "--help", "-v"}, "parse_cli_args normal args includes program name and flags");

    maybe<std::vector<str_t>> result2 = parse_cli_args(0, argv1);
    log(!result2.has_value(), "parse_cli_args with argc=0 should return unexpect");
    if (!result2.has_value())
      try { std::rethrow_exception(result2.error()); }
      catch (const std::invalid_argument& e) { log(str_t(e.what()) == "No proper command line arguments provided", "exception is of correct type and message"); }

    maybe<std::vector<str_t>> result3 = parse_cli_args(3, nullptr);
    log(!result3.has_value(), "parse_cli_args with args=nullptr should return unexpect");
    if (!result3.has_value())
      try { std::rethrow_exception(result3.error()); }
      catch (const std::invalid_argument& e) { log(str_t(e.what()) == "No proper command line arguments provided", "exception is of correct type and message"); }

    const char* argv2[] = {"single_arg"};
    maybe<std::vector<str_t>> result4 = parse_cli_args(1, argv2);
    log(result4.value() == std::vector<str_t>{"single_arg"}, "parse_cli_args with single argument returns vector of size 1");

    const char* argv3[] = {"prog", "", "arg2"};
    maybe<std::vector<str_t>> result5 = parse_cli_args(3, argv3);
    log(result5.value() == std::vector<str_t>{"prog", "", "arg2"}, "parse_cli_args handles empty string arguments correctly");
  }



  title("Testing cslib::stringify_container"); {
    log(stringify_container(std::vector<int>{}) == "{}", "empty vector<int> returns {}");
    log(stringify_container(std::deque<int>{}) == "{}", "empty deque<int> returns {}");
    log(stringify_container(std::list<int>{}) == "{}", "empty list<int> returns {}");
    log(stringify_container(std::set<int>{}) == "{}", "empty set<int> returns {}");
    log(stringify_container(std::array<int, 0>{}) == "{}", "empty array<int,0> returns {}");
    log(stringify_container(std::initializer_list<int>{}) == "{}", "empty initializer_list<int> returns {}");

    log(stringify_container(std::vector<int>{42}) == "{42}", "single-element vector<int>");
    log(stringify_container(std::deque<char>{'x'}) == "{x}", "single-element deque<char>");
    log(stringify_container(std::list<std::string>{"hi"}) == "{hi}", "single-element list<string>");
    log(stringify_container(std::set<int>{7}) == "{7}", "single-element set<int>");
    log(stringify_container(std::array<int, 1>{99}) == "{99}", "single-element array<int,1>");
    log(stringify_container(std::initializer_list<int>{1}) == "{1}", "single-element initializer_list<int>");

    log(stringify_container(std::vector<int>{1,2,3}) == "{1, 2, 3}", "multi-element vector<int>");
    log(stringify_container(std::deque<char>{'a','b','c'}) == "{a, b, c}", "multi-element deque<char>");
    log(stringify_container(std::list<str_t>{"x","y"}) == "{x, y}", "multi-element list<cslib::str_t/*aka std::string*/>");
    log(stringify_container(std::set<int>{3,1,2}) == "{1, 2, 3}", "multi-element set<int> (ordered)");
    log(stringify_container(std::array<double,3>{1.1,2.2,3.3}) == "{1.1, 2.2, 3.3}", "multi-element array<double>");
    log(stringify_container(std::initializer_list<int>{1, 2, 3, 4, 5}) == "{1, 2, 3, 4, 5}", "multi-element initializer_list<int>");
  }



  title("Testing cslib::shorten_end and shorten_begin"); {
    static_assert(*rtrim("cslib.h++", 6) == "csl...", "constexpr shorten_end truncates correctly");
    static_assert(*ltrim("cslib.h++", 6) == "...h++", "constexpr shorten_begin truncates correctly");

    static_assert(*rtrim("short", 10) == "short", "constexpr shorten_end returns original string if short enough");
    static_assert(*ltrim("short", 10) == "short", "constexpr shorten_begin returns original string if short enough");

    static_assert(*rtrim("exact", 5) == "exact", "constexpr shorten_end exact length");
    static_assert(*ltrim("exact", 5) == "exact", "constexpr shorten_begin exact length");

    constexpr size_t trimLen = std::strlen(TRIM_WITH);
    static_assert(*rtrim("abcdef", trimLen) == TRIM_WITH, "constexpr shorten_end with maxLength == TRIM_WITH returns only TRIM_WITH");
    static_assert(*ltrim("abcdef", trimLen) == TRIM_WITH, "constexpr shorten_begin with maxLength == TRIM_WITH returns only TRIM_WITH");

    static_assert(*rtrim("abcdef", trimLen + 1) == str_t("a") + TRIM_WITH, "constexpr shorten_end one char over TRIM_WITH");
    static_assert(*ltrim("abcdef", trimLen + 1) == str_t(TRIM_WITH) + "f", "constexpr shorten_begin one char over TRIM_WITH");

    // maxLength smaller than TRIM_WITH length triggers unexpect
    maybe<str_t> res1 = ltrim("hello", 2);
    log(!res1.has_value(), "shorten_end with maxLength < TRIM_WITH returns unexpect");
    if (!res1.has_value())
      try { std::rethrow_exception(res1.error()); }
      catch (const std::invalid_argument& e) { 
        log(std::string(e.what()).find("maxLength must be at least") != std::string::npos, "shorten_end unexpect exception has correct message"); 
      }

    maybe<str_t> res2 = ltrim("hello", 2);
    log(!res2.has_value(), "shorten_begin with maxLength < TRIM_WITH returns unexpect");
    if (!res2.has_value())
      try { std::rethrow_exception(res2.error()); }
      catch (const std::invalid_argument& e) { 
        log(std::string(e.what()).find("maxLength must be at least") != std::string::npos,  "shorten_begin unexpect exception has correct message"); 
      }
  }



  title("Testing cslib::separate"); {
    log(separate("a,b,c", ",") == std::vector<str_t>{"a","b","c"}, "split by \",\"");
    log(separate("hello world test", " ") == std::vector<str_t>{"hello","world","test"}, "split by space");
    log(separate("one--two--three", "--") == std::vector<str_t>{"one","two","three"}, "split by multi-char delimiter");
    log(separate("abc", "x") == std::vector<str_t>{"abc"}, "delimiter not found returns full string");
    log(separate("", ",").empty(), "empty string input returns empty vector");
    log(separate("abc", "") == std::vector<str_t>{"a", "b", "c"}, "empty delimiter returns empty vector");

    log(separate(",a,,b,", ",") == std::vector<str_t>{"", "a", "" ,"b",""}, "trailing/floating delimiters add empty strings");
  }



  title("Testing cslib::roll_dice"); {
    bool good = true;
    for ([[maybe_unused]] int _ : range(1'000))
      if (!between(roll_dice(1, 6), 1, 6))
        good = false;
    log(good, "roll_dice(1,6) returns value in [1,6]");
    
    good = true;
    for ([[maybe_unused]] int _ : range(1'000))
      if (!between(roll_dice(6, 1), 1, 6))
        good = false;
    log(good, "roll_dice(6,1) handles swapped bounds correctly");

    good = true;
    for ([[maybe_unused]] int _ : range(1'000))
      if (!between(roll_dice(-3, 3), -3, 3))
        good = false;
    log(good, "roll_dice(-3,3) returns value in [-3,3]");

    good = true;
    for ([[maybe_unused]] int _ : range(1'000))
      if (!between(roll_dice(0, 1), 0, 1))
        good = false;
    log(good, "roll_dice(0,1) returns either 0 or 1");

    log(roll_dice(5, 5) == 5, "roll_dice(5,5) always returns 5");
  }



  auto s2v = [](strv_t s) {
    return std::vector<char>{s.begin(), s.end()};
  };
  title("Testing cslib::read_data"); {
  
    // Read full stream and restore state
    {
      std::istringstream in("hello world");
      const std::ios::iostate oldEx = in.exceptions();
      const std::ios::iostate oldState = in.rdstate();
      const std::streampos pos = in.tellg();

      maybe<std::vector<char>> res = read_data(in);
      log(res.has_value() && res.value() == s2v("hello world"), "read_data reads full content");
      log(in.tellg() == pos, "read_data restores stream position (start)");
      log(in.exceptions() == oldEx, "read_data restores exception mask (start)");
      log(in.rdstate() == oldState, "read_data preserves rdstate (start)");
    }


    // Empty input
    {
      std::istringstream in("");
      const std::ios::iostate oldEx = in.exceptions();
      const std::streampos pos = in.tellg();

      maybe<std::vector<char>> res = read_data(in);
      log(res.has_value() && res.value().empty(), "read_data on empty stream returns empty string");
      log(in.tellg() == pos, "read_data restores stream position (empty)");
      log(in.exceptions() == oldEx, "read_data restores exception mask (empty)");
    }


    // Start in the middle of the stream
    {
      std::istringstream in("abcdef");
      in.seekg(2); // position -> points at 'c'
      const std::ios::iostate oldEx = in.exceptions();
      const std::streampos pos = in.tellg();

      maybe<std::vector<char>> res = read_data(in);
      log(res.has_value() && res.value() == s2v("cdef"), "read_data reads from current position to end");
      log(in.tellg() == pos, "read_data restores stream position (middle)");
      log(in.exceptions() == oldEx, "read_data restores exception mask (middle)");
    }


    // Start at EOF
    {
      std::istringstream in("xyz");
      in.seekg(3); // position at end
      const std::ios::iostate oldEx = in.exceptions();
      const std::streampos pos = in.tellg();

      maybe<std::vector<char>> res = read_data(in);
      log(res.has_value() && res.value().empty(), "read_data at EOF returns empty string");
      log(in.tellg() == pos, "read_data restores stream position (EOF)");
      log(in.exceptions() == oldEx, "read_data restores exception mask (EOF)");
    }


    // Large input
    {
      str_t big(9'999'999, 'A');
      std::istringstream in(big);
      const std::ios::iostate oldEx = in.exceptions();
      const std::streampos pos = in.tellg();

      maybe<std::vector<char>> res = read_data(in);
      log(res.has_value() && res.value().size() == big.size() && res.value() == s2v(big), "read_data handles large input correctly");
      log(in.tellg() == pos, "read_data restores stream position (large)");
      log(in.exceptions() == oldEx, "read_data restores exception mask (large)");
    }


    // Embedded null bytes
    {
      const char raw[] = {'a','b', '\0', 'c', 'd'};
      str_t s(raw, sizeof(raw));
      std::istringstream in(s);
      const std::ios::iostate oldEx = in.exceptions();
      const std::streampos pos = in.tellg();

      maybe<std::vector<char>> res = read_data(in);
      log(res.has_value() && res.value().size() == s.size() && res.value() == s2v(s), "read_data preserves embedded null bytes and exact length");
      log(in.tellg() == pos, "read_data restores stream position (embedded nulls)");
      log(in.exceptions() == oldEx, "read_data restores exception mask (embedded nulls)");
    }


    // Re-read consistency: calling it twice from same position should give same result
    {
      std::istringstream in("repeatable content");
      const std::ios::iostate oldEx = in.exceptions();
      const std::streampos pos = in.tellg();

      maybe<std::vector<char>> a = read_data(in);
      maybe<std::vector<char>> b = read_data(in);
      log(a.has_value() && b.has_value() && a.value() == b.value() && a.value() == s2v("repeatable content"), "read_data is consistent across multiple invocations");
      log(in.tellg() == pos, "read_data restores stream position (repeatable)");
      log(in.exceptions() == oldEx, "read_data restores exception mask (repeatable)");
    }


    // Re-read consistency: calling it twice from same position should give same result
    {
      std::istringstream in("repeatable content");
      const std::ios::iostate oldEx = in.exceptions();
      const std::streampos pos = in.tellg();

      maybe<std::vector<char>> a = read_data(in);
      maybe<std::vector<char>> b = read_data(in);
      log(a.has_value() && b.has_value() && a.value() == b.value() && a.value() == s2v("repeatable content"), "read_data is consistent across multiple invocations");
      log(in.tellg() == pos, "read_data restores stream position (repeatable)");
      log(in.exceptions() == oldEx, "read_data restores exception mask (repeatable)");
    }


    // Non-seekable stream consistency: calling it twice
    {
      struct NonSeekableBuf : public std::stringbuf {
        NonSeekableBuf(const str_t& s = "") { this->str(s); }
        std::streampos seekoff(std::streamoff, std::ios_base::seekdir, std::ios_base::openmode) override {
          return std::streampos(std::streamoff(-1));
        }
        std::streampos seekpos(std::streampos, std::ios_base::openmode) override {
          return std::streampos(std::streamoff(-1));
        }
      } buf("nonseekable");

      std::istream in(&buf);
      const std::ios::iostate oldEx = in.exceptions();
      const std::streampos pos = in.tellg(); // expected to be -1 / fail

      maybe<std::vector<char>> res = read_data(in);
      log(res.has_value(), "read_data also accepts non-seekable streams");
      log(in.exceptions() == oldEx, "read_data restores exception mask after non-seekable failure");
      // position may remain fail (-1), but make sure it's unchanged (compare to captured pos)
      log(in.tellg() == pos, "read_data does not silently change tellg() for non-seekable stream");
    }


    // Ensure rdstate preservation on a stream that already has eofbit set
    {
      std::istringstream in("abc");
      // consume all to set eofbit
      std::string tmp;
      std::getline(in, tmp, '\0'); // read to end
      const std::streampos pos = in.tellg();
      const std::ios::iostate oldState = in.rdstate();
      const std::ios::iostate oldEx = in.exceptions();

      maybe<std::vector<char>> res = read_data(in);
      // Result should be empty (already at EOF) and state/exceptions should be restored
      if (!res)
        std::rethrow_exception(res.error());
      log(res.has_value() && res.value().empty(), "read_data at pre-set EOF returns empty string");
      log(in.rdstate() == oldState, "read_data preserves rdstate when EOF was already set");
      log(in.exceptions() == oldEx, "read_data restores exception mask when EOF was already set");
      log(in.tellg() == pos, "read_data restores stream position when EOF was already set");
    }
  }



  title("Testing cslib::do_io"); {

    // Basic success: simple string transfer
    {
      std::istringstream in("hello world");
      std::ostringstream out;
      maybe<void> res = do_io(in, out);
      log(res.has_value(), "do_io succeeds on normal input/output");
      log(out.str() == "hello world", "do_io writes correct content to output stream");
    }

    // Empty input
    {
      std::istringstream in("");
      std::ostringstream out;
      maybe<void> res = do_io(in, out);
      log(res.has_value(), "do_io succeeds with empty input");
      log(out.str().empty(), "do_io writes empty string to output stream");
    }

    // Non-seekable input stream
    {
      struct NonSeekableBuf : public std::stringbuf {
        NonSeekableBuf(const std::string& s = "") { this->str(s); }
        std::streampos seekoff(std::streamoff, std::ios_base::seekdir, std::ios_base::openmode) override { return std::streampos(-1); }
        std::streampos seekpos(std::streampos, std::ios_base::openmode) override { return std::streampos(-1); }
      } buf("Unseekable");
      std::istream in(&buf);
      std::ostringstream out;
      maybe<void> res = do_io(in, out);
      log(res.has_value() && out.str() == "Unseekable", "do_io returns unexpected when input stream cannot be read");
    }

    // Output stream throws on write
    {
      struct ThrowOnWriteBuf : public std::stringbuf {
        std::streamsize xsputn(const char_type* s, std::streamsize n) override {
          (void)s;
          (void)n;
          throw std::runtime_error("write error");
        }
      } buf;

      std::istringstream in("data");
      std::ostream out(&buf);

      maybe<void> res = do_io(in, out);
      log(!res.has_value(), "do_io returns unexpected when output stream throws");
      if (!res.has_value()) {
        try {
          std::rethrow_exception(res.error());
        } catch (const std::runtime_error& e) {
          log(std::string(e.what()) == "write error", "do_io preserves output write exception message");
        } catch (...) {
          log(false, "do_io preserves wrong exception type from output write");
        }
      }
    }

    // RAII guard restores exception masks and positions
    {
      std::istringstream in("restore test");
      std::ostringstream out;

      // set initial exception masks and positions
      const std::ios::iostate inOldEx = in.exceptions();
      const std::ios::iostate outOldEx = out.exceptions();
      const std::streampos inPos = in.tellg();

      maybe<void> res = do_io(in, out);

      log(res.has_value(), "do_io succeeds normally with RAII guards");
      log(in.exceptions() == inOldEx, "do_io restores input exception mask after successful write");
      log(out.exceptions() == outOldEx, "do_io restores output exception mask after successful write");
      log(in.tellg() == inPos, "do_io restores input stream position after successful write");
    }

    // Large data transfer
    {
      std::string big(9'999'999, 'X');
      std::istringstream in(big);
      std::ostringstream out;

      maybe<void> res = do_io(in, out);
      log(res.has_value(), "do_io handles large input successfully");
      log(out.str().size() == big.size() && out.str() == big, "do_io writes full large input correctly");
    }

    // Embedded null bytes
    {
      const char raw[] = {'A','B','\0','C','D'};
      std::string s(raw, sizeof(raw));
      std::istringstream in(s);
      std::ostringstream out;

      maybe<void> res = do_io(in, out);
      log(res.has_value(), "do_io handles embedded null bytes without failure");
      log(out.str().size() == s.size() && out.str() == s, "do_io preserves embedded null bytes in output");
    }

    // Input with eofbit already set
    {
      std::istringstream in("xyz");
      std::string tmp;
      std::getline(in, tmp, '\0'); // consume all -> eofbit set
      const std::ios::iostate oldEx = in.exceptions();
      const std::streampos oldPos = in.tellg();
      std::ostringstream out;

      maybe<void> res = do_io(in, out);
      log(res.has_value(), "do_io succeeds even if input eofbit already set");
      log(out.str().empty(), "do_io writes empty string when input eofbit is set");
      log(in.exceptions() == oldEx, "do_io restores exception mask even if eofbit was set");
      log(in.tellg() == oldPos, "do_io restores input position even if eofbit was set");
    }

    // Multiple sequential do_io calls: input stream restored each time
    {
      std::istringstream in("abc123");
      std::ostringstream out1, out2;

      maybe<void> res1 = do_io(in, out1);
      maybe<void> res2 = do_io(in, out2);

      log(res1.has_value() && res2.has_value(), "do_io succeeds on sequential calls");
      log(out1.str() == "abc123" && out2.str() == "abc123", "do_io produces identical output for sequential calls");
    }
  }



  title("Testing cslib::TimeStamp"); {
    // Default constructor: should produce a time near now
    {
      auto before = std::chrono::system_clock::now();
      TimeStamp ts;
      auto after = std::chrono::system_clock::now();

      log(ts.timePoint >= before && ts.timePoint <= after, "TimeStamp() default constructor produces timePoint near now");
    }
    // Constructor from time_point
    {
      auto now = std::chrono::system_clock::now();
      TimeStamp ts(now);
      log(ts.timePoint == now, "TimeStamp(time_point) stores correct timePoint");
    }
    // Constructor from valid date/time
    {
      TimeStamp ts(14, 30, 15, 10, 8, 2025); // 14:30:15 10-08-2025
      log(ts.year() == 2025, "TimeStamp(year) returns correct year");
      log(ts.month() == 8, "TimeStamp(month) returns correct month");
      log(ts.day() == 10, "TimeStamp(day) returns correct day");
      log(ts.hour() == 14, "TimeStamp(hour) returns correct hour");
      log(ts.minute() == 30, "TimeStamp(minute) returns correct minute");
      log(ts.second() == 15, "TimeStamp(second) returns correct second");

      std::string str = ts.as_str();
      log(str.find("14:30:15") != std::string::npos && str.find("10-08-2025") != std::string::npos,
          "TimeStamp::as_str() contains correct formatted time and date");
    }
    // Leap year February 29th
    {
      try {
        TimeStamp ts(0, 0, 0, 29, 2, 2024);
        log(ts.year() == 2024 && ts.month() == 2 && ts.day() == 29, "TimeStamp allows valid leap year date");
      } catch (...) {
        log(false, "TimeStamp threw exception for valid leap year date");
      }

      try {
        TimeStamp ts(0, 0, 0, 29, 2, 2023);
        log(false, "TimeStamp should throw for invalid non-leap year February 29th");
      } catch (const std::logic_error& e) {
        log(std::string(e.what()).find("Invalid date") != std::string::npos, "Exception message indicates invalid date");
      } catch (...) {
        log(false, "TimeStamp threw wrong type for invalid February 29th");
      }
    }
    // Invalid months/days
    {
      try { TimeStamp ts(0,0,0,0,1,2025); log(false, "Day 0 should throw"); } catch (...) { log(true, "Day 0 throws"); }
      try { TimeStamp ts(0,0,0,32,1,2025); log(false, "Day 32 should throw"); } catch (...) { log(true, "Day 32 throws"); }
      try { TimeStamp ts(0,0,0,1,0,2025); log(false, "Month 0 should throw"); } catch (...) { log(true, "Month 0 throws"); }
      try { TimeStamp ts(0,0,0,1,13,2025); log(false, "Month 13 should throw"); } catch (...) { log(true, "Month 13 throws"); }
    }
    // Invalid times
    {
      try { TimeStamp ts(-1,0,0,1,1,2025); log(false, "Hour -1 should throw"); } catch (...) { log(true, "Hour -1 throws"); }
      try { TimeStamp ts(24,0,0,1,1,2025); log(false, "Hour 24 should throw"); } catch (...) { log(true, "Hour 24 throws"); }
      try { TimeStamp ts(0,-1,0,1,1,2025); log(false, "Minute -1 should throw"); } catch (...) { log(true, "Minute -1 throws"); }
      try { TimeStamp ts(0,60,0,1,1,2025); log(false, "Minute 60 should throw"); } catch (...) { log(true, "Minute 60 throws"); }
      try { TimeStamp ts(0,0,-1,1,1,2025); log(false, "Second -1 should throw"); } catch (...) { log(true, "Second -1 throws"); }
      try { TimeStamp ts(0,0,60,1,1,2025); log(false, "Second 60 should throw"); } catch (...) { log(true, "Second 60 throws"); }
    }
    // Consistency between constructors
    {
      TimeStamp ts1(14, 45, 30, 15, 7, 2025);
      TimeStamp ts2(std::chrono::system_clock::time_point(std::chrono::sys_days(
        std::chrono::year(2025)/7/15
      ) + std::chrono::hours(14) + std::chrono::minutes(45) + std::chrono::seconds(30)));
      log(ts1.timePoint == ts2.timePoint, "TimeStamp(hour,min,sec,day,month,year) matches equivalent time_point constructor");
    }
    // as_str() format
    {
      TimeStamp ts(1,2,3,4,5,2025);
      std::string str = ts.as_str();
      log(str.size() > 0, "TimeStamp::as_str() returns non-empty string");
      log(str.find("01:02:03") != std::string::npos, "as_str() formats hours, minutes, seconds with leading zeros");
      log(str.find("04-05-2025") != std::string::npos, "as_str() formats day-month-year correctly");
    }
    // Edge: midnight at start of epoch
    {
      TimeStamp ts(0,0,0,1,1,1970);
      log(ts.hour() == 0 && ts.minute() == 0 && ts.second() == 0, "TimeStamp midnight epoch correct");
      log(ts.day() == 1 && ts.month() == 1 && ts.year() == 1970, "TimeStamp date epoch correct");
    }
    // Edge: end of day
    {
      TimeStamp ts(23,59,59,31,12,2025);
      log(ts.hour() == 23 && ts.minute() == 59 && ts.second() == 59, "TimeStamp end of day correct");
      log(ts.day() == 31 && ts.month() == 12 && ts.year() == 2025, "TimeStamp end of year correct");
    }
  }



  title("Testing cslib::where_is_path_really"); {

    // Existing file
    {
      std::ofstream tmp("tmp_file.txt");
      tmp << "hello";
      tmp.close();

      maybe<stdfs::path> res = where_is_path_really("tmp_file.txt");
      log(res.has_value(), "where_is_path_really returns value for existing file");
      if (res.has_value())
        log(stdfs::exists(*res), "returned path actually exists on disk");
      std::remove("tmp_file.txt");
    }

    // Non-existent file
    {
      maybe<stdfs::path> res = where_is_path_really("definitely_nonexistent_file.txt");
      log(!res.has_value(), "where_is_path_really returns unexpected for non-existent file");
      if (!res.has_value()) {
        try { std::rethrow_exception(res.error()); }
        catch (...) { log(true, "exception preserved for non-existent file"); }
      }
    }

    // Existing directory
    {
      std::filesystem::create_directory("tmp_dir");
      maybe<stdfs::path> res = where_is_path_really("tmp_dir");
      log(res.has_value(), "where_is_path_really returns value for existing directory");
      if (res.has_value())
        log(stdfs::exists(*res) && stdfs::is_directory(*res), "returned path exists and is a directory");
      std::filesystem::remove("tmp_dir");
    }

    // Relative vs canonical
    {
      std::ofstream tmp("tmp_file2.txt");
      tmp.close();
      maybe<stdfs::path> res = where_is_path_really("tmp_file2.txt");
      log(res.has_value(), "where_is_path_really returns value for file");
      if (res.has_value())
        log(res->is_absolute(), "returned path is absolute (canonical)");
      std::remove("tmp_file2.txt");
    }
  }




  if (failedTests != 0) {
    std::cout << "\n >> " << failedTests << " tests failed." << std::endl;
    return EXIT_FAILURE;
  } else {
    std::cout << "\n >> " << "All tests passed." << std::endl;
    return EXIT_SUCCESS;
  }
}