// LICENSE: ☝️🤓

// Including every single header that might ever be needed
#include <condition_variable>
#include <initializer_list>
#include <unordered_map>
#include <string_view>
#include <stop_token>
#include <filesystem>
#include <functional>
#include <stdexcept>
#include <algorithm>
#include <iostream> // Already contains many libraries
#include <optional>
#include <fstream>
#include <variant>
#include <cstring>
#include <cstdint>
#include <cstddef>
#include <cctype>
#include <cstdio>
#include <random>
#include <thread>
#include <vector>
#include <chrono>
#include <thread>
#include <array>
#include <deque>
#include <cmath>
#include <map>


#pragma once
/*
  Include this header file only once pwease. No support for
  linking and stuff.
  Prevent multiple inclusions of this header file
*/


#if __cplusplus < 202002L
  #error "Requires C++ >= 20"
#endif



namespace cslib {
  // Jack of all trades (Helper functions and classes)

  // Other
  using wstr_t = std::wstring;
  using o_str_t = std::string;
  #define SHARED inline // Alias inline for shared functions, etc.
  #define MACRO inline constexpr auto // Macros for macro definitions
  #define FIXED inline constexpr // Explicit alternative for MACRO
  #define EXIT_HERE(reason) { \
    std::wcout << __FILE__ << L':' << __LINE__ << L": " << reason << std::endl; \
    std::exit(EXIT_FAILURE); \
  }
  MACRO Black = L"\033[30m";
  MACRO Red = L"\033[31m";
  MACRO Green = L"\033[32m";
  MACRO Yellow = L"\033[33m";
  MACRO Blue = L"\033[34m";
  MACRO Magenta = L"\033[35m";
  MACRO Cyan = L"\033[36m";
  MACRO White = L"\033[37m";
  MACRO Reset = L"\033[0m";
  #ifdef _WIN32
    MACRO PATH_DELIMITER = L'\\';
  #else
    MACRO PATH_DELIMITER = L'/';
  #endif



  // Functions
  void sleep(size_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
  }



  // Find the correct way to convert T to (w-)string
  o_str_t to_str(const char *const value) {
    return o_str_t(value);
  }
  wstr_t to_wstr(const wchar_t *const value) {
    return wstr_t(value);
  }
  wstr_t to_wstr(const char *const value) {
    return wstr_t(value, value + std::strlen(value));
  }
  o_str_t to_str(char value) {
    return o_str_t(1, value);
  }
  wstr_t to_wstr(wchar_t value) {
    return wstr_t(1, value);
  }
  wstr_t to_wstr(char value) {
    return wstr_t(1, static_cast<wchar_t>(value));
  }
  o_str_t to_str(wstr_t value) {
    return o_str_t(value.begin(), value.end());
  }
  o_str_t to_str(std::string_view value) {
    return o_str_t(value.data());
  }
  wstr_t to_wstr(std::wstring_view value) {
    return wstr_t(value.data());
  }
  wstr_t to_wstr(std::string_view value) {
    return wstr_t(value.data(), value.data() + value.size());
  }
  template <std::integral T>
  o_str_t to_str(T value) {
    return o_str_t(std::to_string(value));
  }
  template <std::integral T>
  wstr_t to_wstr(T value) {
    return wstr_t(std::to_wstring(value));
  }
  template <std::floating_point T>
  o_str_t to_str(T value) {
    return o_str_t(std::to_string(value));
  }
  template <std::floating_point T>
  wstr_t to_wstr(T value) {
    return wstr_t(std::to_wstring(value));
  }



  void sh_call(o_str_t command) {
    // Blocking system call
    if (system(command.c_str()) != 0)
      EXIT_HERE(L"Failed: " + to_wstr(command));
  }


  
  void clear_console() {
    // wipe previous output and move cursor to the top
    #ifdef _WIN32
      sh_call("cls");
    #else
      sh_call("clear");
    #endif
  }


  
  template <typename Key, typename Container>
  bool contains(Container& lookIn, Key& lookFor) {
    // does `container` contain `key`?
    return std::find(lookIn.begin(), lookIn.end(), lookFor) != lookIn.end();
  }
  template <typename Containers>
  bool have_something_common(Containers& c1, Containers& c2) {
    // do `c1` and `c2` contain similar keys?
    for (auto item : c1)
      if (contains(c2, item))
        return true;
    return false;
  }



  o_str_t get_env(o_str_t var) {
    // Get environment variable by name
    char* envCStr = getenv(var.c_str());
    if (envCStr == NULL)
      EXIT_HERE(L"Environment variable '" + to_wstr(var) + L"' not found");
    return o_str_t(envCStr);
  }



  std::vector<int> range(int start, int end) {
    /*
      Simplified range function that takes two integers
      and returns a vector of integers (inclusive)
    */
    std::vector<int> result;
    if (start > end) // reverse
      for (int i = start; i >= end; --i)
        result.push_back(i);
    else if (start < end) // start to end
      for (int i = start; i <= end; ++i)
        result.push_back(i);
    else // just start
      result.push_back(start);
    return result;
  }
  std::vector<int> range(int end) {
    // Exact same as the function above, but with `start = 0`
    return range(0, end);
  }
  std::vector<size_t> range(size_t start, size_t end) {
    /*
      Same as above, but with size_t.
      Example:
        cslib::range(0, 10); // {0, 1, 2, ..., 10}
    */
    std::vector<size_t> result;
    if (start > end) // reverse
      for (size_t i = start; i >= end; --i)
        result.push_back(i);
    else if (start < end) // start to end
      for (size_t i = start; i <= end; ++i)
        result.push_back(i);
    else // just start
      result.push_back(start);
    return result;
  }
  std::vector<size_t> range(size_t end) {
    // Same as above, but with `start = 0`
    return range(size_t(0), end);
  }



  void retry(std::function<void()> target, size_t retries, size_t delay = 0) {
    /*
      Retry a function up to `retries` times with a delay
      of `delay` milliseconds between each retry.
      Example:
        cslib::retry([]() {
          // Do something that might fail
        });
    */
    if (retries == 0)
      EXIT_HERE(L"Retries must be greater than 0");
    for (size_t tried : range(retries)) {
      try {
        target(); // Try to execute the function
        return; // If successful, exit the function
      } catch (const std::exception& e) {
        if (tried == retries - 1) {
          // If this was the last retry, throw the exception
          EXIT_HERE(L"Failed after " + to_wstr(retries) + L" retries: " + to_wstr(e.what()));
        }
      }
      sleep(delay);
    }
  }



  std::vector<wstr_t> parse_cli_args(int argc, const char *const argv[]) {
    // Parse command line arguments and return them as a vector of strings.
    std::vector<wstr_t> args;
    for (int i : range(argc - 1))
      args.push_back(to_wstr(argv[i]));
    return args;
  }



  wstr_t shorten_end(wstr_t str, size_t maxLength) {
    /*
      Trim and add "..." to the end of the string
      if it exceeds `maxLength`.
    */
    if (str.length() > maxLength)
      str = str.substr(0, maxLength - 3) + L"..."; // 3 for "..."
    return str;
  }
  wstr_t shorten_begin(wstr_t str, size_t maxLength) {
    // Same as above but trimming the beginning of the string
    if (str.length() > maxLength)
      str = L"..." + str.substr(str.length() - maxLength + 3); // 3 for "..."
    return str;
  }



  wstr_t upper(wstr_t str) {
    /*
      This function takes a string and converts it to uppercase.
      Example:
        to_upper("csLib.h++");
        // is "CSLIB.H++"
    */
    return std::transform(str.begin(), str.end(), str.begin(), ::toupper), str;
  }
  wstr_t lower(wstr_t str) {
    /*
      This function takes a string and converts it to lowercase.
      Example:
        to_lower("csLib.h++");
        // is "cslib.h++"
    */
    return std::transform(str.begin(), str.end(), str.begin(), ::tolower), str;
  }



  wstr_t escape_str(wstr_t str) {
    // Escape special characters in a string.
    wstr_t result;
    for (wchar_t c : str) {
      switch (c) {
        case L'"': result += L"\\\""; break;
        case L'\\': result += L"\\\\"; break;
        case L'\n': result += L"\\n"; break;
        case L'\r': result += L"\\r"; break;
        case L'\t': result += L"\\t"; break;
        default: result += c; break;
      }
    }
    return result;
  }
  wstr_t unescape_str(wstr_t str) {
    // Unescape special characters in a string.
    wstr_t result;
    bool escape = false; // Flag to check if the next character is escaped
    for (wchar_t c : str) {
      if (escape) {
        switch (c) {
          case L'"': result += L'"'; break;
          case L'\\': result += L'\\'; break;
          case L'n': result += L'\n'; break;
          case L'r': result += L'\r'; break;
          case L't': result += L'\t'; break;
          default: result += c; break; // If not a special character, just add it
        }
        escape = false;
      } else if (c == '\\') {
        escape = true; // Next character is escaped
      } else {
        result += c; // Just add the character
      }
    }
    return result;
  }



  std::vector<o_str_t> separate(o_str_t str, wchar_t delimiter) {
    /*
      Create vector of strings from a string by separating it with a delimiter.
      Note:
        - If between two delimiters is nothing, an empty string is added to the vector
        - If the string ends with a delimiter, an empty string is added to the vector
        - If delimiter is not found, the whole string is added to the vector
    */

    std::vector<o_str_t> result;
    o_str_t temp;

    if (str.empty() or delimiter == L'\0')
      return result;

    for (wchar_t c : str) {
      if (c == delimiter) {
        result.push_back(temp);
        temp.clear();
      } else {
        temp += c;
      }
    }

    result.push_back(temp);

    return result;
  }
  std::vector<wstr_t> separate(wstr_t str, wchar_t delimiter) {
    /*
      Same as above, but for wide strings.
      Example:
        cslib::separate(L"Hello World", ' ') // {"Hello", "World"}
    */
    std::vector<wstr_t> result;
    wstr_t temp;

    if (str.empty() or delimiter == L'\0')
      return result;

    for (wchar_t c : str) {
      if (c == delimiter) {
        result.push_back(temp);
        temp.clear();
      } else {
        temp += c;
      }
    }

    result.push_back(temp);

    return result;
  }



  size_t roll_dice(size_t min, size_t max) {
    /*
      This function takes a minimum and maximum value and returns a random
      number between them (inclusive).
    */

    if (min > max) std::swap(min, max);

    // Special thanks to copilot. No idea what this does
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dis(min, max);
    return dis(gen);
  }



  wstr_t in() {
    wstr_t input;
    std::getline(std::wcin, input);
    return input;
  }



  // Classes
  class Out { public:
    /*
      Print to console with color and optionally into an existing file.
      Usage:
        cslib::Out error("Error: ", cslib::Out::Color::RED);
        error << "Something went wrong";
    */
    static void set_us_utf8_encoding() {
      // Set all io-streaming globally to UTF-8 encoding

      std::locale utf8_locale = std::locale("en_US.UTF-8");
      if (utf8_locale.name() == "C" or utf8_locale.name() == "POSIX")
        EXIT_HERE(L"UTF-8 locale not found. Please set the locale to UTF-8 in your system settings.");

      std::locale::global(utf8_locale);
      std::wcout.imbue(utf8_locale);
      std::wcin.imbue(utf8_locale);
      std::wclog.imbue(utf8_locale);
      std::wcerr.imbue(utf8_locale);
      std::cout.imbue(utf8_locale);
      std::cin.imbue(utf8_locale);
      std::clog.imbue(utf8_locale);
      std::cerr.imbue(utf8_locale);
    }
    wstr_t prefix;
    Out(wstr_t pref, wstr_t color = L"") {
      prefix = color;
      prefix += pref;
      prefix += Reset;
      prefix += L" ";
    }
    template <typename T>
    std::wostream& operator<<(const T& msg) {
      // Same as std::wcout << msg, but with color and prefix
      std::wcout << prefix << msg;
      return std::wcout;
    }
  };



  class TimeStamp { public:
    // A wrapper around std::chrono that I have control over
    std::chrono::system_clock::time_point timePoint;
    TimeStamp() {update();}
    void update() {
      timePoint = std::chrono::system_clock::now();
    }
    o_str_t asStr() const {
      /*
        Convert the time point to (lighter form of) ISO 8601
        in format YYYY-MM-DD HH:MM:SS).
      */
      std::time_t time = std::chrono::system_clock::to_time_t(timePoint);
      return (std::stringstream() << std::put_time(std::gmtime(&time), "%Y-%m-%d %H:%M:%S")).str();
    }
    wstr_t asWstr() const {
      // Same as above, but returns a wide string.
      std::time_t time = std::chrono::system_clock::to_time_t(timePoint);
      return (std::wstringstream() << std::put_time(std::gmtime(&time), L"%Y-%m-%d %H:%M:%S")).str();
    }
    static void sleep_until(TimeStamp untilPoint) {
      // Sleep until the given time point.
      if (untilPoint.timePoint <= std::chrono::system_clock::now())
        EXIT_HERE(L"Cannot sleep until a time point in the past");
      std::this_thread::sleep_until(untilPoint.timePoint);
    }
  };



  class Benchmark { public:
    /*
      Measures the time taken by a function or a block of code.
      Example:
        cslib::Benchmark benchmark;
        // Do something
        std::cout << "Time taken: " << benchmark.elapsed_ms() << " ms\n";
    */
    std::chrono::high_resolution_clock::time_point startTime;
    Benchmark() {
      startTime = std::chrono::high_resolution_clock::now();
    }
    size_t elapsed_ms() {
      return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
    }
    size_t elapsed_us() {
      return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
    }
    size_t elapsed_ns() {
      return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
    }
  };



  class VirtualPath { public:
    // Wrapper around std::filesystem::path

    std::filesystem::path isAt; 

    VirtualPath() = default;
    VirtualPath(std::wstring where) : isAt(std::filesystem::canonical(where)) {}
    VirtualPath(std::string where) : isAt(std::filesystem::canonical(where)) {}
    /*
      Constructor that takes a string and checks if it's a valid path.
      Notes:
        - If where is relative, it will be converted to an absolute path.
        - If where is empty, you will crash.
    */
    VirtualPath(wstr_t where, std::filesystem::file_type shouldBe) : VirtualPath(where) {
      // Same as above, but checks if the path is of a specific type.
      if (this->type() != shouldBe)
        EXIT_HERE(L"Path '" + where + L"' of (" + to_wstr(static_cast<int>(this->type())) + L") should be a " + to_wstr(static_cast<int>(shouldBe)));
    }

    std::filesystem::file_type type() const {
      /*
        Returns the type of the path.
        Example:
          VirtualPath path("/gitstuff/cslib/cslib.h++");
          std::filesystem::file_type type = path.type();
          // type = std::filesystem::file_type::regular
      */
      if (isAt.empty())
        EXIT_HERE(L"Uninitialized path");
      return std::filesystem::status(isAt).type();
    }
    VirtualPath parent() const {
      /*
        Returns the parent path of the path.
        Example:
          VirtualPath path("/gitstuff/cslib/cslib.h++");
          VirtualPath parent = path.parent_path();
          // parent = "/gitstuff/cslib"
      */
      if (isAt.empty())
        EXIT_HERE(L"Uninitialized path");
      if (isAt.parent_path().empty())
        EXIT_HERE(L"Path has no parent");
      return VirtualPath(isAt.parent_path().wstring());
    }
    size_t depth() const {
      /*
        Returns the depth of the path.
        Example:
          VirtualPath path("/gitstuff/cslib/cslib.h++");
          size_t depth = path.depth();
          // depth = 2 (because there are 2 directories before the file)
      */
      if (isAt.empty())
        EXIT_HERE(L"Uninitialized path");
      return separate(isAt.wstring(), PATH_DELIMITER).size() - 1; // -1 for the last element
    }
    TimeStamp last_modified() const {
      /*
        Last modified date of this instance.
        Example:
          VirtualPath path("/gitstuff/cslib/cslib.h++");
          TimeStamp lastModified = path.last_modified();
          // lastModified = 2023-10-01 12:34:56
        Note:
          Once more, special thanks to copilot. I scurried
          through the std::filesystem documentation and
          couldn't find a usable way to get the last modified
          date of a file.
      */
      std::filesystem::file_time_type ftime = std::filesystem::last_write_time(isAt);
      std::chrono::system_clock::time_point timePoint = std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
      std::time_t cftime = std::chrono::system_clock::to_time_t(timePoint);
      struct tm* timeinfo = std::localtime(&cftime);
      TimeStamp ts;
      ts.timePoint = std::chrono::system_clock::from_time_t(std::mktime(timeinfo));
      return ts;
    }
    void move_to(VirtualPath moveTo) {
      // Move this instance to a new location and apply changes.
      if (this->isAt.empty())
        EXIT_HERE(L"Uninitialized path");
      if (moveTo.isAt.empty())
        EXIT_HERE(L"Target path is empty");
      if (moveTo.type() != std::filesystem::file_type::directory)
        EXIT_HERE(L"Target path '" + moveTo.isAt.wstring() + L"' is not a directory");
      if (moveTo.isAt == this->isAt)
        EXIT_HERE(L"Cannot move to the same path: " + this->isAt.wstring());
      std::wstring willBecome = moveTo.isAt.wstring() + to_wstr(PATH_DELIMITER) + this->isAt.filename().wstring();
      if (std::filesystem::exists(willBecome))
        EXIT_HERE(L"Target path '" + willBecome + L"' already exists");
      std::filesystem::rename(this->isAt, willBecome);
      this->isAt = VirtualPath(willBecome).isAt; // Apply changes
    }
    VirtualPath copy_into(VirtualPath targetDict) const {
      /*
        Copies this instance to a new location and returns
        a new VirtualPath instance pointing to the copied file.
      */
      if (this->isAt.empty())
        EXIT_HERE(L"Uninitialized path");
      if (targetDict.isAt.empty())
        EXIT_HERE(L"Target path is empty");
      if (targetDict.type() != std::filesystem::file_type::directory)
        EXIT_HERE(L"Target path '" + targetDict.isAt.wstring() + L"' is not a directory");
      if (targetDict.isAt == this->isAt)
        EXIT_HERE(L"Cannot copy to the same path: " + this->isAt.wstring());
      std::wstring willBecome = targetDict.isAt.wstring() + to_wstr(PATH_DELIMITER) + this->isAt.filename().wstring();
      if (std::filesystem::exists(willBecome))
        EXIT_HERE(L"Element '" + willBecome + L"' already exists in target directory. Overwriting is avoided by default.");
      std::filesystem::copy(this->isAt, willBecome);
      return VirtualPath(willBecome);
    }
  };



  class File { public:
    /*
      Child class of VirtualPath that represents a file.
      Example:
        File file("/gitstuff/cslib/cslib.h++");
        std::string content = file.content();
        // content = "Hello World"
    */

    VirtualPath is; // Composition over inheritance

    File() = default;
    File(wstr_t where) : is(where, std::filesystem::file_type::regular) {}

    wstr_t content(std::ios_base::openmode openMode = std::ios::in) const {
      /*
        Read the content of the file and return it as a string.
        Note:
          - No error-handling for files larger than available memory
      */
      if (is.isAt.empty())
        EXIT_HERE(L"Uninitialized path");
      std::wifstream file(is.isAt, openMode);
      if (!file.is_open())
        EXIT_HERE(L"Failed to open file '" + is.isAt.wstring() + L"'");
      if (!file.good())
        EXIT_HERE(L"Failed to read file '" + is.isAt.wstring() + L"'");
      return wstr_t((std::istreambuf_iterator<wchar_t>(file)), std::istreambuf_iterator<wchar_t>());
    }
    wstr_t wstr() const {
      if (is.isAt.empty())
        EXIT_HERE(L"Uninitialized path");
      return is.isAt.wstring();
    }
    wstr_t extension() const {
      // Get file extension with dot 
      if (is.isAt.empty())
        EXIT_HERE(L"Uninitialized path");
      return is.isAt.extension().wstring();
    }
    size_t bytes() const {
      // Get the file size in bytes.
      if (is.isAt.empty())
        EXIT_HERE(L"Uninitialized path");
      return std::filesystem::file_size(is.isAt);
    }
  };



  class Folder { public:
    /*
      Child class of VirtualPath that represents a folder and
      everything in it.
    */
    std::vector<VirtualPath> content;
    VirtualPath is;

    Folder() = default;
    Folder(wstr_t where) : is(where, std::filesystem::file_type::directory) {update();}
    wstr_t str() const {
      if (is.isAt.empty())
        EXIT_HERE(L"Uninitialized path");
      return is.isAt.wstring();
    }
    void update() {
      if (is.isAt.empty())
        EXIT_HERE(L"Uninitialized path");
      content.clear();
      for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(is.isAt))
        content.push_back(VirtualPath(entry.path().wstring()));
      content.shrink_to_fit();
    }
  };




  // Namespaces
  namespace TinySTL {
    // The stl is good but sometimes you need something smaller

    template <typename T>
    class Optional { public: // TODO: Look into it further. Might not be necessary
      /*
        A small optional class that can hold a value or be empty.
        Example:
          Optional<int> opt;
          opt.set(5);
          if (opt.on())
            std::cout << opt.cast() << '\n';
      */
      char storage[sizeof(T) + 1]; // extra byte for the flag
      Optional() : storage{0} {}
      T cast() const {
        return *reinterpret_cast<const T *const>(storage);
      }
      bool on() const {
        return storage[sizeof(T)];
      }
      void reset() {
        if (on())
          reinterpret_cast<T*>(storage)->~T();
        storage[sizeof(T)] = 0;
      }
      void set(T value) {
        if (on())
          *reinterpret_cast<T*>(storage) = value;
        else {
          new (storage) T(value);
          storage[sizeof(T)] = 1;
        }
      }
      T& operator=(T value) {
        set(value);
        return *reinterpret_cast<T*>(storage);
      }
      T& get() {
        if (!on())
          EXIT_HERE(L"Uninitialized optional");
        return *reinterpret_cast<T*>(storage);
      }
      ~Optional() {
        if (on())
          reinterpret_cast<T*>(storage)->~T();
      }
      // Operator stuff for access
      T& operator*() {
        if (!on())
          EXIT_HERE(L"Uninitialized optional");
        return *reinterpret_cast<T*>(storage);
      }
      T* operator->() {
        if (!on())
          EXIT_HERE(L"Uninitialized optional");
        return reinterpret_cast<T*>(storage);
      }
      operator T() {
        if (!on())
          EXIT_HERE(L"Uninitialized optional");
        return *reinterpret_cast<T*>(storage);
      }
    };


    template <typename T>
    class Vector { public:
      /*
        Same as std::vector but smaller and less features.
        Note:
          The size and capacity are 4 bytes each, when padding
          is done, memory-usage isn't doubled.
      */
      T* data;
      uint32_t size;
      uint32_t capacity;
      Vector() : data(nullptr), size(0), capacity(0) {}
      ~Vector() { delete[] data; }
      T* begin() { return data; }
      T* end() { return data + size; }
      void increment_capacity() {
        T* new_data = new T[++capacity];
        size = -1;
        for (T& ownData : *this)
          new_data[++size] = ownData;
        delete[] data;
        data = new_data;
      }
      void push_back(T value) {
        if (size == capacity)
          increment_capacity();
        data[++size] = value;
      }
      T& operator[](uint32_t index) {
        if (index >= size)
          EXIT_HERE(L"Index out of bounds: " + to_wstr(index) + L" >= " + to_wstr(size));
        return data[index];
      }
      void pop_back() {
        if (size == 0)
          EXIT_HERE(L"Cannot pop from an empty vector");
        --size;
      }
    };



    template <uint8_t N>
    class String { public:
      /*
        Static fixed size string. It can hold up to `N` characters and is
        only null-terminated when cslib::String's capacity isn't maxed.
        If it is, the size-limit acts as a terminator.
        Example:
          String<2> str("Hi");
          // str = {'H', 'i'}
          String<3> str2("Hi");
          // str2 = {'H', 'i', '\0'}
          String<4> str3("Hi");
          // str3 = {'H', 'i', '\0', 'm' ('m' could have been used
          for something else earlier but ignored after null-termination)}
      */

      static_assert(N > 0, "String size must be greater than 0");

      char data[N] = {'\0'};

      String() = default;
      String(std::string str) {
        for (char c : str)
          append(c);
      }

      uint8_t length() {
        uint8_t size = 0;
        while (data[size] != '\0' and size < N)
          ++size;
        return size;
      }
      void append(char c) {
        uint8_t size = length();
        if (size >= N)
          EXIT_HERE(L"String capacity exceeded: " + to_wstr(N));
        data[size] = c;
        if (size + 1 < N)
          data[size + 1] = '\0';
      }
      void clear() {
        data = {'\0'};
      }
      char& at(uint8_t index) {
        if (index >= N)
          EXIT_HERE(L"Index out of bounds: " + to_wstr(index) + L" >= " + to_wstr(N));
        return data[index];
      }
      char* begin() {return data;}
      char* end() {return data + length();}
      std::string std_str() {
        std::string str;
        for (char c : data)
          str += c;
        return str;
      }
      bool operator==(String other) {
        return this->std_str() == other.std_str();
      }
      bool operator==(std::string other) {
        return this->std_str() == other;
      }
    };
    template <uint8_t N>
    class Wstring { public:
      /*
        Same as String but for wide characters.
        Example:
          Wstring<2> str(L"Hi");
          // str = {L'H', L'i'}
          Wstring<3> str2(L"Hi");
          // str2 = {L'H', L'i', L'\0'}
          Wstring<4> str3(L"Hi");
          // str3 = {L'H', L'i', L'\0', L'm' (L'm' could have been used
          for something else earlier but ignored after null-termination)}
      */

      static_assert(N > 0, "Wstring size must be greater than 0");

      wchar_t data[N] = {L'\0'};

      Wstring() = default;
      Wstring(std::wstring str) {
        for (wchar_t c : str)
          append(c);
      }

      uint8_t length() {
        uint8_t size = 0;
        while (data[size] != L'\0' and size < N)
          ++size;
        return size;
      }
      void append(wchar_t c) {
        uint8_t size = length();
        if (size >= N)
          EXIT_HERE(L"Wstring capacity exceeded: " + to_wstr(N));
        data[size] = c;
        if (size + 1 < N)
          data[size + 1] = L'\0';
      }
      void clear() {
        data = {L'\0'};
      }
      wchar_t& at(uint8_t index) {
        if (index >= N)
          EXIT_HERE(L"Index out of bounds: " + to_wstr(index) + L" >= " + to_wstr(N));
        return data[index];
      }
      wchar_t* begin() {return data;}
      wchar_t* end() {return data + length();}
      wstr_t std_str() {
        wstr_t str;
        for (wchar_t c : data)
          str += c;
        return str;
      }
    };


    template <typename K, typename V>
    class Map { public:
      /*
        A TinySTL::Vector that holds key-value pairs.
      */

      struct Node {
        K key;
        V value;
      };
      Vector<Node> data;
      void insert(K key, V value) {
        if (contains(key))
          EXIT_HERE(L"Key already exists: " + to_wstr(key));
        data.push_back({key, value});
      }
      V& at(K key) {
        for (Node& node : data)
          if (node.key == key)
            return node.value;
        EXIT_HERE(L"Key not found");
      }
      V& operator[](K key) {
        return at(key);
      }
      bool contains(K key) const {
        for (Node node : data)
          if (node.key == key)
            return true;
        return false;
      }
      std::pair<K, V>* begin() {
        return reinterpret_cast<std::pair<K, V>*>(data.begin());
      }
      std::pair<K, V>* end() {
        return reinterpret_cast<std::pair<K, V>*>(data.end());
      }
    };
  };
} // namespace cslib