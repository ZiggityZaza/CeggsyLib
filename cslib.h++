// LICENSE: ☝️🤓
#pragma once // Prevent multiple inclusions of this header file

// Including every single header that might ever be needed
#include <condition_variable>
#include <initializer_list>
#include <unordered_map>
#include <stop_token>
#include <filesystem>
#include <functional>
#include <algorithm>
#include <iostream> // Already contains many libraries
#include <optional>
#include <fstream>
#include <variant>
#include <random>
#include <thread>
#include <vector>
#include <chrono>
#include <thread>
#include <array>
#include <deque>
#include <cmath>
#include <map>


#if __cplusplus < 202002L
  #error "Requires C++ >= 20"
#endif





namespace cslib {
  // Jack of all trades (Helper functions and classes)

  // Other
  #define SHARED inline // Alias inline for shared functions, etc.
  #define MACRO inline constexpr auto // Macros for macro definitions
  #define FIXED inline constexpr // Explicit alternative for MACRO
  #define THROW_HERE(reason) throw std::runtime_error(std::string(__FILE__) + ":" + std::to_string(__LINE__) + std::string(": \033[1m") + reason + "\033[0m");



  SHARED void sleep(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
  }



  SHARED void sh_call(std::string command) { // TODO: Fix blocking exit
    // Blocking system call
    if (system(command.c_str()) != 0)
      THROW_HERE("Failed: " + command);
  }


  
  SHARED void clear_console() {
    // wipe previous output and move cursor to the top
    #ifdef _WIN32
      sh_call("cls");
    #else
      sh_call("clear");
    #endif
  }



  
  template <typename Key, typename Container>
  SHARED bool contains(Container lookIn, Key lookFor) {
    // does `container` contain `key`?
    return std::find(lookIn.begin(), lookIn.end(), lookFor) != lookIn.end();
  }
  template <typename Containers>
  SHARED bool have_something_common(Containers c1, Containers c2) {
    // do `c1` and `c2` contain similar keys?
    for (auto item : c1)
      if (contains(c2, item))
        return true;
    return false;
  }



  SHARED std::string get_env(std::string var) {
    // Get environment variable by name
    char* envCStr = getenv(var.c_str());
    if (envCStr == NULL)
      THROW_HERE("Environment variable '" + var + "' not found");
    return std::string(envCStr);
  }



  SHARED std::vector<int> range(int start, int end) {
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
  SHARED std::vector<int> range(int end) {
    // Exact same as the function above, but with `start = 0`
    return range(0, end);
  }
  template <int S>
  SHARED consteval std::array<int, S> range() {
    // Compile-time range function
    std::array<int, S> result{};
    for (int i = 0; i < S; ++i)
      result[i] = i;
    return result;
  }
  template <int S, int E>
  SHARED consteval std::array<int, (E - S) + 1> range() {
    // Compile-time range function (without reverse)
    static_assert(E >= S, "End must be greater or equal to start");
    std::array<int, (E - S) + 1> result{};
    for (int i = S; i <= E; ++i)
      result[i - S] = i;
    return result;
  }



  SHARED float progress_bar(size_t currentStep, size_t stepsAmount, size_t visualLimit) {
    /*
      Simulate and calculate a progress bar based
      on the amount of steps and the current step.
      Returns the percentage of the progress bar.
      Example:
        int STEPS = 4;
        for (int i : range(STEPS)) {
          size_t prog = cslib::progress_bar(STEPS, i, 100);
          // i = 0, prog = 0
          // i = 1, prog = 25
          // i = 2, prog = 50
          // i = 3, prog = 75
          // i = 4, prog = 100
        }
    */

    if (stepsAmount == 0)
      THROW_HERE("Steps amount cannot be 0");
    if (visualLimit < stepsAmount)
      THROW_HERE("Visual limit cannot be less than the amount of steps");
    if (currentStep > stepsAmount)
      THROW_HERE("Current step cannot be greater than the amount of steps");

    if (currentStep == stepsAmount)
      return 100; // 100% when done
    if (currentStep == 0)
      return 0; // 0% when not started

    return (currentStep * visualLimit) / stepsAmount;
  }



  SHARED void retry(std::function<void()> target, size_t retries, size_t delay = 0) {
    /*
      Retry a function up to `retries` times with a delay
      of `delay` milliseconds between each retry.
      Example:
        cslib::retry([]() {
          // Do something that might fail
        });
    */
    if (retries == 0)
      THROW_HERE("Retries must be greater than 0");
    for (int tried : range(retries)) {
      try {
        target(); // Try to execute the function
        return; // If successful, exit the function
      } catch (const std::exception& e) {
        if (tried == retries - 1) {
          // If this was the last retry, throw the exception
          THROW_HERE("Failed after " + std::to_string(retries) + " retries: " + e.what());
        }
      }
    }
  }



  SHARED std::vector<std::string> parse_cli_args(int argc, const char *const argv[]) {
    /*
      Parse command line arguments and return them as a vector of strings.
      Example:
        int main(int argc, char* argv[]) {
          std::vector<std::string> args = cslib::parse_cli_args(argc, argv);
          // args = {"program_name", "arg1", "arg2", ...}
        }
    */
    std::vector<std::string> args;
    for (int i : range(argc)) {
      // Convert each argument to a string and add it to the vector
      args.push_back(std::string(argv[i]));
    }
    return args;
  }



  template <typename T>
  SHARED std::string to_str(T value) {
    // Find the correct way to convert T to string

    // Try std::string(T) in string-like types
    if constexpr (std::is_same_v<T, std::string>)
      return value;
    else if constexpr (std::is_same_v<T, char*>)
      return std::string(value);
    else if constexpr (std::is_same_v<T, const char*>)
      return std::string(value);
    else if constexpr (std::is_same_v<T, char>)
      return std::string(1, value);
    else if constexpr (std::is_same_v<T, std::string_view>)
      return std::string(value);

    // Try std::to_string(T) in numbers
    else if constexpr (std::is_integral_v<T> or std::is_floating_point_v<T>)
      return std::to_string(value);

    // Give up
    THROW_HERE("Failed str conversion");
    return "";
  }



  SHARED std::string shorten_end(std::string str, size_t maxLength) {
    /*
      Trim and add "..." to the end of the string
      if it exceeds `maxLength`.
    */
    if (str.length() > maxLength)
      str = str.substr(0, maxLength - 3) + "..."; // 3 for "..."
    return str;
  }
  std::string shorten_begin(std::string str, size_t maxLength) {
    // Same as above but trimming the beginning of the string
    if (str.length() > maxLength)
      str = "..." + str.substr(str.length() - maxLength + 3); // 3 for "..."
    return str;
  }



  SHARED std::string upper(std::string str) {
    /*
      This function takes a string and converts it to uppercase.
      Example:
        to_upper("csLib.h++");
        // is "CSLIB.H++"
    */
    for (char& c : str)
      c = std::toupper(c);
    return str;
  }
  SHARED std::string lower(std::string str) {
    /*
      Same as above but to lowercase
      Example:
        to_lower("CSLib.H++");
        // is "cslib.h++"
    */
    for (char& c : str)
      c = std::tolower(c);
    return str;
  }



  SHARED std::string escape_str(std::string str) {
    // Escape special characters in a string.
    std::string result;
    for (char c : str) {
      switch (c) {
        case '"': result += "\\\""; break;
        case '\\': result += "\\\\"; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        default: result += c; break;
      }
    }
    return result;
  }
  SHARED std::string unescape_str(std::string str) {
    // Unescape special characters in a string.
    std::string result;
    bool escape = false; // Flag to check if the next character is escaped
    for (char c : str) {
      if (escape) {
        switch (c) {
          case '"': result += '"'; break;
          case '\\': result += '\\'; break;
          case 'n': result += '\n'; break;
          case 'r': result += '\r'; break;
          case 't': result += '\t'; break;
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



  SHARED std::string format(std::string str, std::map<std::string, std::string> replacements) {
    /*
      String and a map of replacements keys to replaces keys with
      the corresponding values.
      Example:
        str = "Hello {name}, welcome to {place}";
        replacements = {
          {"name", "John Money"}
        };
        str = format(str, replacements);
        // str = "Hello John Money, welcome to {place}"
    */

    // Iterate over all possible keys to replace
    for (auto [key, value] : replacements) {

      std::string placeholder = "{" + key + "}";
      size_t pos = str.find(placeholder);

      while (pos != std::string::npos) {
        str.replace(pos, placeholder.length(), value);
        pos = str.find(placeholder, pos + value.length());
      }
    }

    return str;
  }



  SHARED std::vector<std::string> separate(std::string str, char delimiter) {
    /*
      Create vector of strings from a string by separating it with a delimiter.
      Note:
        - If between two delimiters is nothing, an empty string is added to the vector
        - If the string ends with a delimiter, an empty string is added to the vector
        - If delimiter is not found, the whole string is added to the vector
    */

    std::vector<std::string> result;
    std::string temp;

    if (str.empty() or delimiter == '\0')
      return result;

    for (char c : str) {
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



  SHARED size_t roll_dice(size_t min, size_t max) {
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



  SHARED std::map<std::string, std::map<std::string, std::string>> quick_parse_ini(std::string iniFile) {
    /*
      Opens and parses an INI file. Returns a map of sections
      and their key-value pairs.
    */

    if (!std::filesystem::exists(iniFile))
      THROW_HERE("INI file '" + iniFile + "' does not exist");

    std::ifstream ini(iniFile);
    if (!ini.is_open())
      THROW_HERE("Failed to open INI file '" + iniFile + "'");

    std::string iniContent((std::istreambuf_iterator<char>(ini)), std::istreambuf_iterator<char>());

    std::map<std::string, std::map<std::string, std::string>> result;
    std::string currentSection;

    for (std::string line : separate(iniContent, '\n')) {

      if (line.empty() or line.at(0) == ';') continue; // Skip empty lines and comments

      // Parse sections
      if (line.at(0) == '[') {
        currentSection = line.substr(1, line.find(']') - 1); // Skip []
        if (result.find(currentSection) == result.end()) // Avoid reloading
          result.insert({currentSection, {}});
      }

      // Assign key-value pairs
      else {
        size_t pos = line.find('=');
        if (pos == std::string::npos)
          THROW_HERE("Invalid line in INI file: '" + line + "'");
        std::string key = line.substr(0, pos);
        if (key.empty())
          THROW_HERE("Empty key in INI file: '" + line + "'");
        std::string value = line.substr(pos + 1);
        if (value.empty())
          THROW_HERE("Empty value for key '" + key + "' in INI file: '" + line + "'");
        result.at(currentSection)[key] = value;
      }
    }

    return result;
  }



  // Classes
  class Out { public:
    /*
      Print to console with color and optionally into an existing file.
      Usage:
        cslib::Out error("Error: ", cslib::Out::Color::RED);
        error << "Something went wrong";
    */

    static FIXED char Black[] = "\033[30m";
    static FIXED char Red[] = "\033[31m";
    static FIXED char Green[] = "\033[32m";
    static FIXED char Yellow[] = "\033[33m";
    static FIXED char Blue[] = "\033[34m";
    static FIXED char Magenta[] = "\033[35m";
    static FIXED char Cyan[] = "\033[36m";
    static FIXED char White[] = "\033[37m";
    static FIXED char Reset[] = "\033[0m";

    std::string prefix;

    Out(std::string pref, std::string color) {
      prefix = color;
      prefix += pref;
    }
    Out(Out&& becomes) : prefix(std::move(becomes.prefix)) {}
    Out& operator=(Out&& becomes) {
      if (this == &becomes)
        prefix = std::move(becomes.prefix);
      return *this;
    }
    Out(const Out&) = delete;
    Out& operator=(const Out&) = delete;

    template <typename T>
    std::ostream& operator<<(const T& msg) {
      // Same as std::cout << msg, but with color and prefix
      std::cout << prefix << Reset << ' ' << msg;
      return std::cout;
    }
    static std::string in() {
      std::string input;
      std::getline(std::cin, input);
      return input;
    }
  };



  class TimeStamp { public:
    // A wrapper around std::chrono that I have control over

    std::chrono::system_clock::time_point timePoint;

    TimeStamp() {update();}
    TimeStamp(std::chrono::system_clock::time_point tp) : timePoint(tp) {}
    TimeStamp(TimeStamp&& becomes) : timePoint(std::move(becomes.timePoint)) {}
    TimeStamp& operator=(TimeStamp&& becomes) {
      if (this != &becomes)
        timePoint = std::move(becomes.timePoint);
      return *this;
    }
    TimeStamp(const TimeStamp&) = delete;
    TimeStamp& operator=(const TimeStamp&) = delete;

    void update() {
      timePoint = std::chrono::system_clock::now();
    }
    std::string asStr() const {
      /*
        Convert the time point to (lighter form of) ISO 8601
        in format YYYY-MM-DD HH:MM:SS).
      */
      std::time_t time = std::chrono::system_clock::to_time_t(timePoint);
      return (std::stringstream() << std::put_time(std::gmtime(&time), "%Y-%m-%d %H:%M:%S")).str();
    }

    static void sleep_until(TimeStamp untilPoint) {
      // Sleep until the given time point.
      if (untilPoint.timePoint <= std::chrono::system_clock::now())
        THROW_HERE("Cannot sleep until a time point in the past");
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
    Benchmark(Benchmark&& becomes) : startTime(std::move(becomes.startTime)) {}
    Benchmark& operator=(Benchmark&& becomes) {
      if (this != &becomes)
        startTime = std::move(becomes.startTime);
      return *this;
    }
    Benchmark(const Benchmark&) = delete; // Disable copy constructor
    Benchmark& operator=(const Benchmark&) = delete; // Disable copy assignment operator

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

    #ifdef _WIN32
      static MACRO PATH_SEPARATOR = '\\';
    #else
      static MACRO PATH_SEPARATOR = '/';
    #endif

    std::filesystem::path isAt; 

    VirtualPath() = default;
    VirtualPath(std::string where) : isAt(std::filesystem::canonical(where)) {
      /*
        Constructor that takes a string and checks if it's a valid path.
        Notes:
          - If where is relative, it will be converted to an absolute path.
          - If where is empty, you will crash.
      */

      #ifdef _WIN32
        if (where.find('/') != std::string::npos)
          THROW_HERE("Path '" + where + "' cannot contain '/' on Windows. Use '\\' instead.");
      #else
        if (where.find('\\') != std::string::npos)
          THROW_HERE("Path '" + where + "' cannot contain '\\' on Linux. Use '/' instead.");
        #endif
    }
    VirtualPath(std::string where, std::filesystem::file_type shouldBe) : VirtualPath(where) {
      // Same as above, but checks if the path is of a specific type.
      if (this->type() != shouldBe) {
        THROW_HERE("Path '" + where + "' is not of type " + std::to_string(static_cast<int>(shouldBe)) + ". It is " + std::to_string(static_cast<int>(this->type())));
      }
    }
    VirtualPath(VirtualPath&& becomes) : isAt(std::move(becomes.isAt)) {}
    VirtualPath& operator=(VirtualPath&& becomes) {
      if (this != &becomes) {
        isAt = std::move(becomes.isAt);
      }
      return *this;
    }
    VirtualPath(const VirtualPath&) = delete;
    VirtualPath& operator=(const VirtualPath&) = delete;

    std::filesystem::file_type type() const {
      /*
        Returns the type of the path.
        Example:
          VirtualPath path("/gitstuff/cslib/cslib.h++");
          std::filesystem::file_type type = path.type();
          // type = std::filesystem::file_type::regular
      */
      if (isAt.empty())
        THROW_HERE("Uninitialized path");
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
        THROW_HERE("Uninitialized path");
      if (isAt.parent_path().empty()) THROW_HERE("Path has no parent");
      return VirtualPath(isAt.parent_path().string());
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
        THROW_HERE("Uninitialized path");
      return separate(isAt.string(), PATH_SEPARATOR).size() - 1; // -1 for the last element
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
          couldn't find a usable way to get the last modified date
          of a file. Copilot suggested this and it works.
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
        THROW_HERE("Uninitialized path");
      if (moveTo.isAt.empty())
        THROW_HERE("Target path is empty");
      if (moveTo.type() != std::filesystem::file_type::directory)
        THROW_HERE("Target path '" + moveTo.isAt.string() + "' is not a directory");
      if (moveTo.isAt == this->isAt)
        THROW_HERE("Cannot move to the same path: " + this->isAt.string());
      std::string willBecome = moveTo.isAt.string() + to_str(PATH_SEPARATOR) + this->isAt.filename().string();
      if (std::filesystem::exists(willBecome))
        THROW_HERE("Target path '" + willBecome + "' already exists");
      std::filesystem::rename(this->isAt, willBecome);
      this->isAt = VirtualPath(willBecome).isAt; // Apply changes
    }
    VirtualPath copy_into(VirtualPath targetDict) const {
      /*
        Copies this instance to a new location and returns
        a new VirtualPath instance pointing to the copied file.
      */
      if (this->isAt.empty())
        THROW_HERE("Uninitialized path");
      if (targetDict.isAt.empty())
        THROW_HERE("Target path is empty");
      if (targetDict.type() != std::filesystem::file_type::directory)
        THROW_HERE("Target path '" + targetDict.isAt.string() + "' is not a directory");
      if (targetDict.isAt == this->isAt)
        THROW_HERE("Cannot copy to the same path: " + this->isAt.string());
      std::string willBecome = targetDict.isAt.string() + to_str(PATH_SEPARATOR) + this->isAt.filename().string();
      if (std::filesystem::exists(willBecome))
        THROW_HERE("Element '" + willBecome + "' already exists in target directory. Overwriting is avoided by default.");
      std::filesystem::copy(this->isAt, willBecome);
      return VirtualPath(willBecome);
    }
    bool operator==(VirtualPath other) const {
      return this->isAt == other.isAt;
    }
    bool operator!=(VirtualPath other) const {
      return !(this->isAt == other.isAt);
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

    VirtualPath is; // Composition over inheritance to evade direct inheritance

    File() = default;
    File(std::string where) : is(where, std::filesystem::file_type::regular) {}
    File(File&& becomes) : is(std::move(becomes.is)) {}
    File& operator=(File&& becomes) {
      if (this != &becomes) {
        is = std::move(becomes.is);
      }
      return *this;
    }
    File(const File&) = delete;
    File& operator=(const File&) = delete; // Disable copy assignment operator

    std::string content() const {
      /*
        Read the content of the file and return it as a string.
        Note:
          - No error-handling for files larger than available memory
      */
      if (is.isAt.empty())
        THROW_HERE("Uninitialized path");
      std::ifstream file(is.isAt, std::ios::binary);
      if (!file.is_open())
        THROW_HERE("Failed to open file '" + is.isAt.string() + "'");
      if (!file.good())
        THROW_HERE("Failed to read file '" + is.isAt.string() + "'");
      return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }
    std::string extension() const {
      /*
        Get the file extension.
        Example:
          File file("/gitstuff/cslib/cslib.h++");
          std::string ext = file.extension();
          // ext = ".h++"
      */
      if (is.isAt.empty())
        THROW_HERE("Uninitialized path");
      return is.isAt.extension().string();
    }
    size_t bytes() const {
      /*
        Get the file size in bytes.
        Example:
          File file("/gitstuff/cslib/cslib.h++");
          size_t size = file.bytes();
          // size = ~400.000 bytes (50000 characters with each 1 byte)
      */
      if (is.isAt.empty())
        THROW_HERE("Uninitialized path");
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
    Folder(std::string where) : is(where, std::filesystem::file_type::directory) {
      /*
        Constructor that takes a string and checks if it's a valid folder path.
        Notes:
          - If path is relative, it will be converted to an absolute path.
          - If path is empty, you will crash.
          - Make sure to use the correct path separator for platform.
      */
      update();
    }
    Folder(Folder&& becomes) : content(std::move(becomes.content)), is(std::move(becomes.is)) {}
    Folder& operator=(Folder&& becomes) {
      if (this != &becomes) {
        content = std::move(becomes.content);
        is = std::move(becomes.is);
      }
      return *this;
    }
    Folder(const Folder&) = delete; // Disable copy constructor
    Folder& operator=(const Folder&) = delete; // Disable copy assignment operator


    void update() {
      /*
        Update the content of the folder.
        Example:
          Folder folder("/gitstuff");
          folder.update();
          // folder.content() = {File("rl"), File("a36m"), Folder("cslib")}
      */
      if (is.isAt.empty())
        THROW_HERE("Uninitialized path");
      content.clear();
      for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(is.isAt))
        content.push_back(VirtualPath(entry.path().string()));
      content.shrink_to_fit();
    }

    // Search functions
    bool isIn(VirtualPath path) const {
      /*
        Check if `path` exists in this folder.
        Example:
          Folder folder("/gitstuff/cslib");
          VirtualPath path("/gitstuff/cslib/cslib.h++");
          bool inFolder = folder.isIn(path);
          // inFolder = true
      */
      if (is.isAt.empty() or path.isAt.empty())
        THROW_HERE("Uninitialized path");
      return is == path.parent();
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
          THROW_HERE("Uninitialized optional");
        return *reinterpret_cast<T*>(storage);
      }
      ~Optional() {
        if (on())
          reinterpret_cast<T*>(storage)->~T();
      }
      // Operator stuff for access
      T& operator*() {
        if (!on())
          THROW_HERE("Uninitialized optional");
        return *reinterpret_cast<T*>(storage);
      }
      T* operator->() {
        if (!on())
          THROW_HERE("Uninitialized optional");
        return reinterpret_cast<T*>(storage);
      }
      operator T() {
        if (!on())
          THROW_HERE("Uninitialized optional");
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
          THROW_HERE("Index out of bounds: " + std::to_string(index) + " >= " + std::to_string(size));
        return data[index];
      }
      void pop_back() {
        if (size == 0)
          THROW_HERE("Cannot pop from an empty vector");
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
          THROW_HERE("String capacity exceeded: " + std::to_string(N));
        data[size] = c;
        if (size + 1 < N)
          data[size + 1] = '\0';
      }
      void clear() {
        data = {'\0'};
      }
      char& at(uint8_t index) {
        if (index >= N)
          THROW_HERE("Index out of bounds: " + std::to_string(index) + " >= " + std::to_string(N));
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
          THROW_HERE("Key already exists: " + to_str(key));
        data.push_back({key, value});
      }
      V& at(K key) {
        for (Node& node : data)
          if (node.key == key)
            return node.value;
        THROW_HERE("Key not found");
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