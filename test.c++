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
template <typename T, typename Or = std::exception_ptr> // Contains error message
using maybe = std::expected<T, Or>;


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


// Catch throws
template <typename L>
maybe<std::invoke_result_t<L>> caught(L&& l) noexcept {
  try {
    if constexpr (std::is_void_v<std::invoke_result_t<L>>) {
      std::invoke(l);
      return {};
    } else
      return std::invoke(l);
  } catch (...) {
    return std::unexpected(std::current_exception());
  }
}


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
    maybe<std::vector<str_t>> result1 = caught([&]{ return parse_cli_args(3, argv1); });
    log(result1.has_value() && result1.value() == std::vector<str_t>{"program.exe", "--help", "-v"}, "parse_cli_args normal args includes program name and flags");

    maybe<std::vector<str_t>> result2 = caught([&]{ return parse_cli_args(0, argv1); });
    log(!result2.has_value(), "parse_cli_args with argc=0 should return unexpect");
    if (!result2.has_value())
      try { std::rethrow_exception(result2.error()); }
      catch (const std::invalid_argument& e) { log(str_t(e.what()) == "No proper command line arguments provided", "exception is of correct type and message"); }

    maybe<std::vector<str_t>> result3 = caught([&]{ return parse_cli_args(3, nullptr); });
    log(!result3.has_value(), "parse_cli_args with args=nullptr should return unexpect");
    if (!result3.has_value())
      try { std::rethrow_exception(result3.error()); }
      catch (const std::invalid_argument& e) { log(str_t(e.what()) == "No proper command line arguments provided", "exception is of correct type and message"); }

    const char* argv2[] = {"single_arg"};
    maybe<std::vector<str_t>> result4 = caught([&]{ return parse_cli_args(1, argv2); });
    log(result4.value() == std::vector<str_t>{"single_arg"}, "parse_cli_args with single argument returns vector of size 1");

    const char* argv3[] = {"prog", "", "arg2"};
    maybe<std::vector<str_t>> result5 = caught([&]{ return parse_cli_args(3, argv3); });
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
    static_assert(rtrim("cslib.h++", 6) == "csl...", "constexpr shorten_end truncates correctly");
    static_assert(ltrim("cslib.h++", 6) == "...h++", "constexpr shorten_begin truncates correctly");

    static_assert(rtrim("short", 10) == "short", "constexpr shorten_end returns original string if short enough");
    static_assert(ltrim("short", 10) == "short", "constexpr shorten_begin returns original string if short enough");

    static_assert(rtrim("exact", 5) == "exact", "constexpr shorten_end exact length");
    static_assert(ltrim("exact", 5) == "exact", "constexpr shorten_begin exact length");

    constexpr size_t trimLen = std::strlen(TRIM_WITH);
    static_assert(rtrim("abcdef", trimLen) == TRIM_WITH, "constexpr shorten_end with maxLength == TRIM_WITH returns only TRIM_WITH");
    static_assert(ltrim("abcdef", trimLen) == TRIM_WITH, "constexpr shorten_begin with maxLength == TRIM_WITH returns only TRIM_WITH");

    static_assert(rtrim("abcdef", trimLen + 1) == str_t("a") + TRIM_WITH, "constexpr shorten_end one char over TRIM_WITH");
    static_assert(ltrim("abcdef", trimLen + 1) == str_t(TRIM_WITH) + "f", "constexpr shorten_begin one char over TRIM_WITH");

    // maxLength smaller than TRIM_WITH length triggers unexpect
    maybe<str_t> res1 = caught([&]{ return ltrim("hello", 2); });
    log(!res1.has_value(), "shorten_end with maxLength < TRIM_WITH returns unexpect");
    if (!res1.has_value())
      try { std::rethrow_exception(res1.error()); }
      catch (const std::invalid_argument& e) { 
        log(std::string(e.what()).find("maxLength must be at least") != std::string::npos, "shorten_end unexpect exception has correct message"); 
      }

    maybe<str_t> res2 = caught([&]{ return ltrim("hello", 2); });
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

      maybe<std::vector<char>> res = caught([&] { return read_data(in); }); 
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

      maybe<std::vector<char>> res = caught([&] { return read_data(in); }); 
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

      maybe<std::vector<char>> res = caught([&] { return read_data(in); }); 
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

      maybe<std::vector<char>> res = caught([&] { return read_data(in); }); 
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

      maybe<std::vector<char>> res = caught([&] { return read_data(in); }); 
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

      maybe<std::vector<char>> res = caught([&] { return read_data(in); }); 
      log(res.has_value() && res.value().size() == s.size() && res.value() == s2v(s), "read_data preserves embedded null bytes and exact length");
      log(in.tellg() == pos, "read_data restores stream position (embedded nulls)");
      log(in.exceptions() == oldEx, "read_data restores exception mask (embedded nulls)");
    }


    // Re-read consistency: calling it twice from same position should give same result
    {
      std::istringstream in("repeatable content");
      const std::ios::iostate oldEx = in.exceptions();
      const std::streampos pos = in.tellg();

      maybe<std::vector<char>> a = caught([&] { return read_data(in); }); 
      maybe<std::vector<char>> b = caught([&] { return read_data(in); }); 
      log(a.has_value() && b.has_value() && a.value() == b.value() && a.value() == s2v("repeatable content"), "read_data is consistent across multiple invocations");
      log(in.tellg() == pos, "read_data restores stream position (repeatable)");
      log(in.exceptions() == oldEx, "read_data restores exception mask (repeatable)");
    }


    // Re-read consistency: calling it twice from same position should give same result
    {
      std::istringstream in("repeatable content");
      const std::ios::iostate oldEx = in.exceptions();
      const std::streampos pos = in.tellg();

      maybe<std::vector<char>> a = caught([&] { return read_data(in); }); 
      maybe<std::vector<char>> b = caught([&] { return read_data(in); }); 
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

      maybe<std::vector<char>> res = caught([&] { return read_data(in); }); 
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

      maybe<std::vector<char>> res = caught([&] { return read_data(in); }); 
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
      maybe<void> res = caught([&] { return do_io(in, out); }); 
      log(res.has_value(), "do_io succeeds on normal input/output");
      log(out.str() == "hello world", "do_io writes correct content to output stream");
    }

    // Empty input
    {
      std::istringstream in("");
      std::ostringstream out;
      maybe<void> res = caught([&] { return do_io(in, out); }); 
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
      maybe<void> res = caught([&] { return do_io(in, out); }); 
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

      maybe<void> res = caught([&] { return do_io(in, out); }); 
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

      maybe<void> res = caught([&] { return do_io(in, out); }); 

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

      maybe<void> res = caught([&] { return do_io(in, out); }); 
      log(res.has_value(), "do_io handles large input successfully");
      log(out.str().size() == big.size() && out.str() == big, "do_io writes full large input correctly");
    }

    // Embedded null bytes
    {
      const char raw[] = {'A','B','\0','C','D'};
      std::string s(raw, sizeof(raw));
      std::istringstream in(s);
      std::ostringstream out;

      maybe<void> res = caught([&] { return do_io(in, out); }); 
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

      maybe<void> res = caught([&] { return do_io(in, out); }); 
      log(res.has_value(), "do_io succeeds even if input eofbit already set");
      log(out.str().empty(), "do_io writes empty string when input eofbit is set");
      log(in.exceptions() == oldEx, "do_io restores exception mask even if eofbit was set");
      log(in.tellg() == oldPos, "do_io restores input position even if eofbit was set");
    }

    // Multiple sequential do_io calls: input stream restored each time
    {
      std::istringstream in("abc123");
      std::ostringstream out1, out2;

      maybe<void> res1 = caught([&] { return do_io(in, out1); }); 
      maybe<void> res2 = caught([&] { return do_io(in, out2); }); 

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



  title("Testing cslib::Road"); {

    // Temporary folder and file setup
    std::filesystem::path tempDir = stdfs::temp_directory_path() / ("cslib_test_" + scramble_name());
    stdfs::create_directory(tempDir);
    std::filesystem::path tempFile = tempDir / "file.txt";
    std::ofstream(tempFile) << "content";

    // Creation: create_self should succeed
    {
      maybe<Road> r = Road::create_self(tempFile);
      log(r.has_value(), "Road::create_self succeeds for existing file path");
      if (r.has_value()) {
        log(r->str() == tempFile.string(), "Road.str() returns correct path string");
        log(r->name() == tempFile.filename().string(), "Road.name() returns filename");
        log(r->depth() == separate(tempFile.string(), to_str(PATH_SEPARATOR)).size() - 1, "Road.depth() returns correct directory depth");
      }
    }

    // Equality operators
    {
      Road r1 = Road::create_self(tempFile);
      Road r2 = Road::create_self(tempFile);
      log(r1 == r2, "Road equality operator works");
      log(!(r1 != r2), "Road inequality operator works");
      log(r1 == tempFile, "Road == stdfs::path works");
      log(!(r1 != tempFile), "Road != stdfs::path works");
    }

    // Conversion operators
    {
      Road r = Road::create_self(tempFile);
      str_t s = r;
      log(s == tempFile.string(), "Road converts to str_t correctly");
      stdfs::path p = r;
      log(p == tempFile, "Road converts to stdfs::path correctly");
      stdfs::path& pr = r;
      log(pr == tempFile, "Road converts to stdfs::path& correctly");
      const stdfs::path& cpr = r;
      log(cpr == tempFile, "Road converts to const stdfs::path& correctly");
      stdfs::path* pp = r;
      log(*pp == tempFile, "Road converts to stdfs::path* correctly");
      const stdfs::path* cpp = const_cast<const Road&>(r);
      log(*cpp == tempFile, "Road converts to const stdfs::path* correctly");
    }

    // last_modified() and type()
    {
      Road r = Road::create_self(tempFile);
      str_t fileTimeAsStr = TimeStamp(r.last_modified()).as_str();
      str_t currentTimeAsStr = TimeStamp().as_str();
      // log(fileTimeAsStr == currentTimeAsStr, "Road.last_modified() returns the correct value for existing file"); // 1s offset
      stdfs::file_type t = r.type();
      log(t == stdfs::file_type::regular, "Road.type() detects regular file");
    }

    // rename_self_to()
    {
      Road r = Road::create_self(tempFile);
      std::filesystem::path newPath = tempDir / "renamed.txt";

      // Successful rename
      maybe<void> res = caught([&]{ r.rename_self_to("renamed.txt"); });
      log(res.has_value(), "Road.rename_self_to succeeds for valid new name");
      log(stdfs::exists(newPath), "File actually renamed on filesystem");
      log(r.str() == newPath.string(), "Road.isAt updated after rename");

      // Attempt to rename to existing file
      std::ofstream(newPath.string() + "_dup");
      maybe<void> resDup = caught([&]{ r.rename_self_to("renamed.txt_dup"); });
      log(!resDup.has_value(), "Road.rename_self_to fails if file exists");

      // Attempt to rename to invalid name (contains separator)
      maybe<void> resInvalid = caught([&]{ r.rename_self_to("bad/name.txt"); });
      log(!resInvalid.has_value(), "Road.rename_self_to fails if name contains path separator");

      // Attempt to rename to empty name
      maybe<void> resEmpty = caught([&]{ r.rename_self_to(""); });
      log(!resEmpty.has_value(), "Road.rename_self_to fails if name is empty");
    }

    // Operator[] depth and parent retrieval
    {
      Road r = Road::create_self(tempDir / "renamed.txt"); // Changed from before
      size_t d = r.depth();
      if constexpr (IS_WINDOWS) // e.g. Disk:\Users\User\Appdata\Temp\cslib_test_dir\file
        log(d == 7, "Road::depth returns correct depth for nested file (Windows 7)");
      else //  e.g., /tmp/cslib/file
        log(d == 3, "Road::depth returns correct depth for nested file (Linux/Mac 3)");
      str_t diskName = (r[0]).str();
      if constexpr (IS_WINDOWS)
        log(diskName.at(1) == ':' && diskName.at(2) == '\\', "First parent of file should always be the disk (Windows X:\\)");
      else
        log(diskName.at(0) == '/', "First parent of file should always be the disk (Linux/Mac /)");
      log(r[d-1] == r.isAt.parent_path(), "Last parent of file should be the parent of the file");
      // index out of bounds returns unexpected
      log(!caught([&]{ return r[d]; }), "Road::operator[] returns unexpected for out-of-bounds index");
    }

    // Invalid construction
    {
      maybe<Road> rEmpty = caught([&]{ return Road::create_self(""); });
      log(!rEmpty.has_value(), "Road::create_self fails on empty path");
      maybe<Road> rNonExist = caught([&]{ return Road::create_self(tempDir / "nonexistent.txt"); });
      log(!rNonExist.has_value(), "Road::create_self fails on non-existent path");
    }

    // Cleanup
    stdfs::remove_all(tempDir);
  }



  title("Testing cslib::File"); {

    // Setup temporary folder and file
    std::filesystem::path tempDir = stdfs::temp_directory_path() / ("cslib_file_test_" + scramble_name());
    stdfs::create_directory(tempDir);
    std::filesystem::path tempFile = tempDir / "test.txt";
    std::ofstream(tempFile) << "initial content";

    // Default constructor
    {
      File f;
      log(f.str().empty(), "Default-constructed File has empty path");
    }

    // Constructor with existing file
    {
      maybe<File> f = caught([&]{ return File(tempFile); });
      log(f.has_value(), "File constructor succeeds for existing file");
      if (f.has_value()) {
        log(f->str() == tempFile.string(), "File.str() returns correct path");
        log(f->name() == tempFile.filename().string(), "File.name() returns filename");
      }
    }

    // Constructor with createIfNotExists = true on non-existing file
    {
      std::filesystem::path newFile = tempDir / "newfile.txt";
      maybe<File> f = caught([&]{ return File(newFile, true); });
      log(f.has_value(), "File constructor creates non-existent file when flag is true");
      log(stdfs::exists(newFile) && stdfs::is_regular_file(newFile), "Created file actually exists and is regular file");
    }

    // Constructor throws on non-regular file (directory)
    {
      maybe<File> f = caught([&]{ return File(tempDir); });
      log(!f.has_value(), "File constructor throws when path is a directory");
      if (!f.has_value()) {
        try { std::rethrow_exception(f.error()); }
        catch (const std::invalid_argument& e) {
          log(std::string(e.what()).find("not a regular file") != std::string::npos, "Exception message contains 'not a regular file'");
        }
      }
    }

    // read_text() reads content correctly
    {
      File f(tempFile);
      str_t content = f.read_text();
      log(content == "initial content", "read_text returns correct file content");
    }

    // edit_text() overwrites file content correctly
    {
      File f(tempFile);
      f.edit_text("new content");
      str_t content = f.read_text();
      log(content == "new content", "edit_text overwrites content correctly");
    }

    // extension() returns correct file extension
    {
      File f(tempFile);
      log(f.extension() == ".txt", "extension() returns correct file extension");
      std::filesystem::path noExt = tempDir / "file";
      std::ofstream(noExt) << "x";
      File f2(noExt);
      log(f2.extension().empty(), "extension() returns empty string for file with no extension");
    }

    // bytes() returns correct file size
    {
      File f(tempFile);
      log(f.bytes() == 11, "bytes() returns correct file size after edit_text");
      f.edit_text("longer content here");
      log(f.bytes() == std::string("longer content here").size(), "bytes() updates correctly after edit_text");
    }

    // move_self_into moves file correctly
    {
      Folder folder(tempDir / "subfolder", true);
      File f(tempFile);
      f.move_self_into(folder);
      log(f.isAt.parent_path() == folder.isAt, "move_self_into updates parent path correctly");
      log(stdfs::exists(f.isAt), "File exists in new location after move_self_into");
      log(!stdfs::exists(tempFile), "Old location no longer has the file after move_self_into");
      f.move_self_into(tempDir); // Move back
    }

    // copy_self_into copies file correctly
    {
      Folder folder(tempDir / "copyfolder", true);
      File f(tempFile);
      f.edit_text("copy content");
      File copy = f.copy_self_into(folder);
      log(stdfs::exists(copy.isAt), "copy_self_into creates a file in the new location");
      log(copy.read_text() == "copy content", "copy_self_into preserves file content");
      log(f.read_text() == "copy content", "Original file remains unchanged after copy_self_into");
    }

    // copy_self_into with copy_options::overwrite_existing
    {
      Folder folder(tempDir / "overwritefolder", true);
      File f(tempFile);
      f.edit_text("original");
      File copy1 = f.copy_self_into(folder);
      f.edit_text("modified");
      File copy2 = f.copy_self_into(folder, stdfs::copy_options::overwrite_existing);
      log(copy2.read_text() == "modified", "copy_self_into with overwrite_existing replaces existing file");
    }

    // Cleanup
    stdfs::remove_all(tempDir);
  }



  title("Testing cslib::Folder"); {

    // Setup temporary folder
    std::filesystem::path tempDir = stdfs::temp_directory_path() / ("cslib_folder_test_" + scramble_name());
    stdfs::create_directory(tempDir);

    // Default constructor
    {
      Folder f;
      log(f.str().empty(), "Default-constructed Folder has empty path");
    }

    // Constructor with existing directory
    {
      Folder f(tempDir);
      log(f.str() == tempDir.string(), "Folder constructor succeeds for existing directory");
    }

    // Constructor with createIfNotExists = true on non-existing directory
    {
      std::filesystem::path newDir = tempDir / "newfolder";
      Folder f(newDir, true);
      log(stdfs::exists(newDir) && stdfs::is_directory(newDir), "Folder constructor creates non-existent directory when flag is true");
    }

    // Constructor throws on existing file instead of directory
    {
      std::filesystem::path filePath = tempDir / "afile.txt";
      std::ofstream(filePath) << "x";
      maybe<Folder> f = caught([&]{ return Folder(filePath); });
      log(!f.has_value(), "Folder constructor throws when path exists but is a file");
    }

    // operator/ returns correct path
    {
      Folder f(tempDir);
      std::filesystem::path sub = f / "subfile.txt";
      log(sub == tempDir / "subfile.txt", "operator/ correctly joins paths");
    }

    // list() lists directory contents
    {
      Folder f(tempDir);
      std::ofstream(tempDir / "file1.txt") << "1";
      std::filesystem::create_directory(tempDir / "sub");
      std::vector<Road> contents = f.list();
      bool foundFile = false, foundDir = false;
      for (auto& r : contents) {
        if (r.name() == "file1.txt") foundFile = true;
        if (r.name() == "sub") foundDir = true;
      }
      log(foundFile && foundDir, "list() returns all files and subdirectories");
    }

    // find() returns Road if exists
    {
      Folder f(tempDir);
      Road r = f.find("file1.txt");
      log(r.name() == "file1.txt", "find() returns correct Road for existing file");
    }

    // find() throws on non-existent file
    {
      Folder f(tempDir);
      maybe<Road> r = caught([&]{ return f.find("nofile.txt"); });
      log(!r.has_value(), "find() throws on non-existent file");
    }

    // find() throws on empty string
    {
      Folder f(tempDir);
      maybe<Road> r = caught([&]{ return f.find(""); });
      log(!r.has_value(), "find() throws on empty name");
    }

    // has() returns optional
    {
      Folder f(tempDir);
      auto r1 = f.has("file1.txt");
      log(r1.has_value() && r1->name() == "file1.txt", "has() returns optional with value for existing file");
      auto r2 = f.has("nofile.txt");
      log(!r2.has_value(), "has() returns nullopt for non-existent file");
    }

    // move_self_into moves folder correctly
    {
      Folder parent(tempDir / "parent", true);
      Folder f(tempDir / "subfolder", true);
      f.move_self_into(parent);
      log(f.isAt.parent_path() == parent.isAt, "move_self_into updates parent path correctly");
      log(stdfs::exists(f.isAt), "Folder exists in new location after move_self_into");
      log(!stdfs::exists(tempDir / "subfolder"), "Old location no longer has folder after move_self_into");
    }

    // move_self_into throws if destination exists
    {
      Folder parent(tempDir / "parent2", true);
      Folder f(tempDir / "subfolder2", true);
      std::filesystem::create_directory(parent / "subfolder2");
      maybe<void> moved = caught([&]{ f.move_self_into(parent); });
      log(!moved.has_value(), "move_self_into throws if destination folder already exists");
    }

    // copy_self_into copies folder correctly
    {
      Folder parent(tempDir / "copyparent", true);
      Folder f(tempDir / "copyfolder", true);
      std::ofstream(f / "file.txt") << "data";
      Folder copy = f.copy_self_into(parent);
      log(stdfs::exists(copy.isAt), "copy_self_into creates folder in new location");
      log(stdfs::exists(copy / "file.txt"), "copy_self_into preserves files inside folder");
    }

    // copy_self_into throws if destination exists
    {
      Folder parent(tempDir / "copyparent2", true);
      Folder f(tempDir / "copyfolder2", true);
      std::filesystem::create_directory(parent / f.name());
      maybe<Folder> copy = caught([&]{ return f.copy_self_into(parent); });
      log(!copy.has_value(), "copy_self_into throws if destination folder already exists");
    }

    // copy_content_into copies contents correctly
    {
      Folder f(tempDir / "source", true);
      std::ofstream(f / "file.txt") << "content";
      Folder target(tempDir / "target", true);
      f.copy_content_into(target, stdfs::copy_options::recursive);
      log(stdfs::exists(target / "file.txt"), "copy_content_into copies files into target folder");
    }

    // Cleanup
    stdfs::remove_all(tempDir);
  }



  title("Testing cslib::TempFile"); {

    // Create a temporary file
    {
      TempFile tf;
      log(stdfs::exists(std::filesystem::temp_directory_path() / tf.name()), "TempFile is created in the system temp directory");
      log(tf.extension() == ".tmp", "TempFile has .tmp extension");
    }

    // TempFile is deleted on destruction
    std::filesystem::path tempPath;
    {
      TempFile tf;
      tempPath = tf.isAt;
      log(stdfs::exists(tempPath), "TempFile exists before destruction");
    }
    log(!stdfs::exists(tempPath), "TempFile is removed after destruction");
  }



  title("Testing cslib::TempFolder"); {

    // Create a temporary folder
    {
      TempFolder tf;
      log(stdfs::exists(std::filesystem::temp_directory_path() / tf.name()), "TempFolder is created in the system temp directory");
    }

    // TempFolder is deleted on destruction
    std::filesystem::path tempPath;
    {
      TempFolder tf;
      tempPath = tf.isAt;
      log(stdfs::exists(tempPath), "TempFolder exists before destruction");

      // Add a file inside to test recursive deletion
      File innerFile(tempPath / "inner.txt", true);
      innerFile.edit_text("Hello");
      log(stdfs::exists(innerFile.isAt), "Inner file exists inside TempFolder");
    }
    log(!stdfs::exists(tempPath), "TempFolder and contents are removed after destruction");
  }



  title("Testing get() function"); {

    // Test downloading a small known URL
    {
      std::expected<str_t, int> result = get("https://www.example.com");
      log(result.has_value(), "get() returns a value for a valid URL");
      if (result.has_value()) {
        log(result.value().find("Example Domain") != str_t::npos, "Downloaded content contains expected text");
      }
    }

    // Test invalid URL returns an error
    {
      std::expected<str_t, int> result = get("http://invalid.invalid");
      log(!result.has_value(), "get() returns an error for an invalid URL");
      if (!result.has_value()) {
        log(result.error() != 0, "Error code is non-zero for failed download");
      }
    }
  }



  title("Extensive tests for to_number() template function"); {

    static_assert(to_number<int>(42L) == 42, "long to int conversion within range");
    static_assert(to_number<unsigned>(0) == 0u, "int to unsigned conversion within range");
    static_assert(to_number<float>(3) == 3.0f, "int to float conversion within range");
    static_assert(to_number<long long>(123456789) == 123456789LL, "int to long long conversion");

    // Out-of-range positive number triggers exception
    {
      bool exceptionThrown = false;
      try {
        long long large = 5000;
        char c = to_number<char>(large); // char max is typically 127
      } catch (const std::invalid_argument& e) {
        exceptionThrown = true;
        log(std::string(e.what()).find("5000") != std::string::npos, "Exception message includes actual number");
      }
      log(exceptionThrown, "Out-of-range positive number triggers exception");
    }

    // Out-of-range negative number triggers exception
    {
      bool exceptionThrown = false;
      try {
        int neg = -200;
        unsigned char uc = to_number<unsigned char>(neg);
      } catch (const std::invalid_argument& e) {
        exceptionThrown = true;
        log(std::string(e.what()).find("-200") != std::string::npos, "Exception message includes actual number");
      }
      log(exceptionThrown, "Out-of-range negative number triggers exception");
    }
  }



  title("Extensive tests for do_in_parallel()"); {

    // 1. Basic function returning int
    {
      std::future<int> fut = do_in_parallel([](int x, int y){ return x + y; }, 2, 3);
      int result = fut.get();
      log(result == 5, "Basic lambda returning int works");
    }

    // 2. Function returning void
    {
      bool flag = false;
      std::future<void> fut = do_in_parallel([&flag]{ flag = true; });
      fut.get();
      log(flag, "Lambda returning void executed correctly");
    }

    // 3. Function throwing exception
    {
      bool caught = false;
      try {
        std::future<void> fut = do_in_parallel([]{ throw std::runtime_error("fail"); });
        fut.get();
      } catch (const std::runtime_error& e) {
        caught = std::string(e.what()) == "fail";
      }
      log(caught, "Exception thrown in async function propagates correctly");
    }

    // 4. Function with reference parameters
    {
      int val = 10;
      std::future<void> fut = do_in_parallel([](int& x){ x += 5; }, std::ref(val));
      fut.get();
      log(val == 15, "Lambda with reference parameter modifies original value");
    }

    // 5. Discarded future (should still run to completion)
    {
      int counter = 0;
      do_in_parallel([&counter]{ counter = 42; });
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      log(counter == 42, "Discarded future runs to completion");
    }

    // 6. Multiple parallel tasks
    {
      int sum = 0;
      std::future<void> fut1 = do_in_parallel([&]{ pause(10); sum += 1; });
      std::future<void> fut2 = do_in_parallel([&]{ sum += 2; });
      fut1.get();
      fut2.get();
      log(sum == 3, "Multiple parallel tasks run correctly");
    }

    // 7. Returning complex type
    {
      std::future<std::vector<int>> fut = do_in_parallel([]{ return std::vector<int>{1,2,3}; });
      std::vector<int> v = fut.get();
      log(v == std::vector<int>{1, 2, 3}, "Lambda returning vector works correctly");
    }

    // 8. Function taking and returning large object
    {
      struct Big { int arr[1000]; };
      std::future<Big> fut = do_in_parallel([](Big b){ b.arr[0] = 42; return b; }, Big{});
      Big res = fut.get();
      log(res.arr[0] == 42, "Function taking/returning large object works");
    }

    // 9. Timing test: ensure function runs asynchronously
    {
      bm.reset();
      std::future<int> fut = do_in_parallel([]{ pause(50); return 123; });
      log(bm.elapsed_ms() < 1.0, "do_in_parallel returns immediately without blocking");
      bm.reset();
      int val = fut.get();
      double d = bm.elapsed_ms();
      log(val == 123 && between(bm.elapsed_ms(), 50.0, 60.1), "Future.get() waits for completion");
    }

    // 10. Ensure std::launch::async is used (cannot test directly, but we can test non-blocking)
    {
        bool executed = false;
        auto fut = do_in_parallel([&executed]{ pause(10); executed = true; });
        pause(5);
        log(!executed, "Async function not yet finished (std::launch::async likely honored)");
        fut.get();
        log(executed, "Async function completed after get()");
    }

    // 11. Test with member function
    {
        struct S { int mul(int x){ return x*2; } };
        S s;
        std::future<int> fut = do_in_parallel(&S::mul, &s, 7);
        int val = fut.get();
        log(val == 14, "Member function works with do_in_parallel");
    }

    // 12. Test with functor object
    {
        struct F { int operator()(int x){ return x+1; } };
        F f;
        std::future<int> fut = do_in_parallel(f, 10);
        int val = fut.get();
        log(val == 11, "Functor object works with do_in_parallel");
    }

    // 13. Test exception safety: discarded future with exception
    {
      bool caught = false;
      try {
        (void)do_in_parallel([]{ throw std::runtime_error("discarded"); });
        pause(10);
      } catch(...) {
        caught = true; // should not throw until future destructs, typically not observable
      }
      log(true, "Discarded future exception does not crash program");
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