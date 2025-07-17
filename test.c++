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
    // This is a simple test to check if the function compiles and runs without errors
    // It will not check the actual command execution, as it depends on the system environment
    sh_call("echo hi > testingcslibsh_call.txt");
    std::ifstream file("hi.txt");
    std::string content;
    if (file.is_open())
      for (std::string line; std::getline(file, line);)
        content += line + '\n'; // Add newline to match the echo command output
    log(content == "hi\n", "sh_call with echo command");
    std::filesystem::remove("testingcslibsh_call.txt"); // Clean up
    log(!std::filesystem::exists("testingcslibsh_call.txt"), "sh_call should create and remove the file (stl-side)");
    try {
      sh_call("non_existing_command");
      log(false, "sh_call should throw an error for non-existing command");
    }
    catch (const std::runtime_error &e) {
      log(std::string(e.what()).find("Failed to execute command: 'non_existing_command'") != std::string::npos, "sh_call should throw an error for non-existing command");
    }
  }
}