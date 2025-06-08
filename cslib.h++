// LICENSE: Don't pretend like you made this but other than that, do whatever you want.


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
  #define MACRO inline constexpr auto // Macros for macro definitions
  #define THROW_HERE(reason) throw std::runtime_error(std::string(__FILE__) + ":" + std::to_string(__LINE__) + std::string(": \033[1m") + reason + "\033[0m");
  #define MAKE_SURE(condition) if (!(condition)) THROW_HERE("Assertion failed: " + std::string(#condition));



  void sleep(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
  }



  void sh_call(std::string command) { // TODO: Fix blocking exit
    // Blocking system call
    if (system(command.c_str()) != 0)
      THROW_HERE("Failed: " + command);
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
  bool contains(Container lookIn, Key lookFor) {
    // does `container` contain `key`?
    return std::find(lookIn.begin(), lookIn.end(), lookFor) != lookIn.end();
  }
  template <typename Containers>
  bool have_something_common(Containers c1, Containers c2) {
    // do `c1` and `c2` contain similar keys?
    for (auto item : c1)
      if (contains(c2, item))
        return true;
    return false;
  }



  std::string get_env(std::string var) {
    // Get environment variable by name
    char* envCStr = getenv(var.c_str());
    MAKE_SURE(envCStr != NULL);
    return std::string(envCStr);
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
  template <int S>
  consteval std::array<int, S> range() {
    // Compile-time range function
    std::array<int, S> result{};
    for (int i = 0; i < S; ++i)
      result[i] = i;
    return result;
  }
  template <int S, int E>
  consteval std::array<int, (E - S) + 1> range() {
    // Compile-time range function (without reverse)
    static_assert(E >= S, "End must be greater or equal to start");
    std::array<int, (E - S) + 1> result{};
    for (int i = S; i <= E; ++i)
      result[i - S] = i;
    return result;
  }



  float progress_bar(size_t currentStep, size_t stepsAmount, size_t visualLimit) {
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

    MAKE_SURE(stepsAmount > 0);
    MAKE_SURE(visualLimit >= stepsAmount);
    MAKE_SURE(currentStep <= stepsAmount);

    if (currentStep == stepsAmount)
      return 100; // 100% when done
    if (currentStep == 0)
      return 0; // 0% when not started

    return (currentStep * visualLimit) / stepsAmount;
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
    MAKE_SURE(retries > 0);
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



  std::vector<std::string> parse_cli_args(int argc, const char *const argv[]) {
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
  std::string to_str(T value) {
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



  std::string shorten_end(std::string str, size_t maxLength) {
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



  std::string upper(std::string str) {
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
  std::string lower(std::string str) {
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



  std::string escape_str(std::string str) {
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
  std::string unescape_str(std::string str) {
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



  std::string format(std::string str, std::map<std::string, std::string> replacements) {
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



  std::vector<std::string> separate(std::string str, char delimiter) {
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



  std::map<std::string, std::map<std::string, std::string>> quick_parse_ini(std::string iniFile) {
    /*
      Opens and parses an INI file. Returns a map of sections
      and their key-value pairs.
    */

    MAKE_SURE(std::filesystem::exists(iniFile));

    std::ifstream ini(iniFile);
    MAKE_SURE(ini.is_open());
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
        MAKE_SURE(pos != std::string::npos);
        std::string key = line.substr(0, pos);
        MAKE_SURE(!key.empty());
        std::string value = line.substr(pos + 1);
        MAKE_SURE(!value.empty());
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

    static MACRO OUT_WRITE = false;
    static MACRO OUT_WRITE_TO_FILE = "cslib.log";
    static std::ofstream& log() {
      /*
        This function returns a reference to the output file stream.
        It creates the file if it doesn't exist and opens it in append mode.
        Note: File is never opened, this function isn't called.
      */
      static std::ofstream writeInto;
      if (!std::filesystem::exists(OUT_WRITE_TO_FILE))
        std::cerr << "Out file '" << OUT_WRITE_TO_FILE << "' doesn't exist. It shall be created.";
      if (!writeInto.is_open()) {
        writeInto.open(OUT_WRITE_TO_FILE, std::ios::app);
        MAKE_SURE(writeInto.is_open());
      }
      return writeInto;
    }


    static MACRO _BLACK = "\033[30m";
    static MACRO _RED = "\033[31m";
    static MACRO _GREEN = "\033[32m";
    static MACRO _YELLOW = "\033[33m";
    static MACRO _BLUE = "\033[34m";
    static MACRO _MAGENTA = "\033[35m";
    static MACRO _CYAN = "\033[36m";
    static MACRO _WHITE = "\033[37m";
    static MACRO _RESET = "\033[0m";
    /*
      Some C developers like to 'enjoy' the legacy design
      of C and claim commonly used names for their own
      headers. The '_' prefix is used to avoid name
      collisions with C code.
    */


    class Proxy { public:
      /*
        A proxy class that allows chaining of << operators
        without prefix.
      */
      Proxy() = default;
      template <typename T>
      Proxy& operator<<(T msg) {
        std::cout << msg << std::flush;
        if (OUT_WRITE)
          log() << msg << std::flush;
        return *this;
      }
    };


    std::string prefix;
    std::string colorCode;
    Proxy proxy; // Allow chaining without prefix

    Out(std::string pref, std::string color) : prefix(pref), colorCode(color), proxy() {}

    template <typename T>
    Proxy& operator<<(T& msg) {
      /*
        Acts the same as std::cout << msg, but with color and prefix
        for readability of output and logging.
        Note:
          Flushed because bad experience with threading etc.
      */
      std::cout << colorCode << prefix << _RESET << msg << std::flush;
      if (OUT_WRITE)
        log() << prefix << msg << std::flush;
      return proxy;
    }

    static std::string in() {
      std::string input;
      std::getline(std::cin, input);
      if (OUT_WRITE)
        log() << input << std::flush;
      return input;
    }
  };



  class TimeStamp { public:
    // A wrapper around std::chrono that I have control over

    std::chrono::system_clock::time_point timePoint;

    TimeStamp() {update();}

    void update() {
      /*
        Update the time point to the current time
        Example:
          TimeStamp now;
          now.update();
          // now.timePoint is updated to the current time
      */
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
      MAKE_SURE(untilPoint.timePoint > std::chrono::system_clock::now());
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
  


  class Helper { public:
    /*
      Runs asynchronously and destructs itself when done.
      Example:
        cslib::Helper helper;
        helper.add_task([]() {
          std::cout << "Hello World\n" << std::flush; // Don't forget the goddamn flush
        });
        helper.wait();
        // Prints "Hello World"
      Note: You WILL crash if helper is destructed while
            tasks are running. Run this on global scope.
    */

    std::jthread worker; // Uses RAII to clean up
    std::deque<std::function<void()>> tasks;
    std::mutex lockTaskDeque; // RAII to protect tasks deque
    std::condition_variable tasksAvailable;
    std::condition_variable_any tasksDoneCv;

    Helper() : worker([this](std::stop_token st) {

      std::function<void()> task;

      // Continue until either told to stop or RAII
      while (!st.stop_requested()) {

        // Wait for tasks to become available
        {
          std::unique_lock<std::mutex> lock(lockTaskDeque);
          tasksAvailable.wait(lock, [&] {
            return !tasks.empty() or st.stop_requested();
          });
          if (st.stop_requested() and tasks.empty()) return;
          task = std::move(tasks.front());
          tasks.pop_front();
        }

        task();

        // Notify that task is done (to exit)
        {
          std::unique_lock<std::mutex> lock(lockTaskDeque);
          if (tasks.empty())
            tasksDoneCv.notify_all();
        }
      }
    }) {}

    void wait() {
      /*
        Pauses calling thread until the helper thread
        is done.
      */
      std::unique_lock<std::mutex> lock(lockTaskDeque);
      tasksDoneCv.wait(lock, [&] { return tasks.empty(); });
    }

    ~Helper() {
      worker.request_stop();
      tasksAvailable.notify_all();
    }

    // Add task to the queue
    void add_task(std::function<void()>& task) {
      {
        std::lock_guard<std::mutex> lock(lockTaskDeque);
        tasks.push_back(task);
      }
      tasksAvailable.notify_one();
    }
    void add_task(std::function<void()>&& task) {
      {
        std::lock_guard<std::mutex> lock(lockTaskDeque);
        tasks.push_back(std::move(task));
      }
      tasksAvailable.notify_one();
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
        MAKE_SURE(where.find('/') == std::string::npos);
      #else
        MAKE_SURE(where.find('\\') == std::string::npos);
      #endif
    }
    VirtualPath(std::string where, std::filesystem::file_type shouldBe) : VirtualPath(where) {
      // Same as above, but checks if the path is of a specific type.
      MAKE_SURE(this->type() == shouldBe);
    }

    std::filesystem::file_type type() const {
      /*
        Returns the type of the path.
        Example:
          VirtualPath path("/gitstuff/cslib/cslib.h++");
          std::filesystem::file_type type = path.type();
          // type = std::filesystem::file_type::regular
      */
      MAKE_SURE(!isAt.empty());
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
      MAKE_SURE(!isAt.empty());
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
      MAKE_SURE(!isAt.empty());
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
      MAKE_SURE(!this->isAt.empty() and !moveTo.isAt.empty());
      MAKE_SURE(moveTo.type() == std::filesystem::file_type::directory);
      MAKE_SURE(moveTo.isAt != this->isAt);
      std::string willBecome = moveTo.isAt.string() + to_str(PATH_SEPARATOR) + this->isAt.filename().string();
      MAKE_SURE(!std::filesystem::exists(willBecome));
      std::filesystem::rename(this->isAt, willBecome);
      this->isAt = VirtualPath(willBecome).isAt; // Apply changes
    }
    VirtualPath copy_into(VirtualPath targetDict) const {
      /*
        Copies this instance to a new location and returns
        a new VirtualPath instance pointing to the copied file.
      */
      MAKE_SURE(!this->isAt.empty() and !targetDict.isAt.empty());
      MAKE_SURE(targetDict.type() == std::filesystem::file_type::directory);
      MAKE_SURE(targetDict.isAt != this->isAt);
      std::string willBecome = targetDict.isAt.string() + to_str(PATH_SEPARATOR) + this->isAt.filename().string();
      MAKE_SURE(!std::filesystem::exists(willBecome));
      std::filesystem::copy(this->isAt, willBecome);
      return VirtualPath(willBecome);
    }
    bool operator==(VirtualPath other) const {
      return this->isAt == other.isAt;
    }
    bool operator!=(VirtualPath other) const {
      return !(*this == other);
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

    std::string content() const {
      /*
        Read the content of the file and return it as a string.
        Note:
          - No error-handling for files larger than available memory
      */
      MAKE_SURE(!is.isAt.empty());
      std::ifstream file(is.isAt, std::ios::binary);
      MAKE_SURE(file.is_open());
      MAKE_SURE(file.good());
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
      MAKE_SURE(!is.isAt.empty());
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
      MAKE_SURE(!is.isAt.empty());
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

    void update() {
      /*
        Update the content of the folder.
        Example:
          Folder folder("/gitstuff");
          folder.update();
          // folder.content() = {File("rl"), File("a36m"), Folder("cslib")}
      */
      MAKE_SURE(!is.isAt.empty());
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
      MAKE_SURE(!is.isAt.empty() and !path.isAt.empty());
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
        MAKE_SURE(on());
        return *reinterpret_cast<T*>(storage);
      }
      ~Optional() {
        if (on())
          reinterpret_cast<T*>(storage)->~T();
      }

      // Operator stuff for access
      T& operator*() {
        MAKE_SURE(on());
        return *reinterpret_cast<T*>(storage);
      }
      T* operator->() {
        MAKE_SURE(on());
        return reinterpret_cast<T*>(storage);
      }
      operator T() {
        MAKE_SURE(on());
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
        MAKE_SURE(index < size);
        return data[index];
      }
      void pop_back() {
        MAKE_SURE(size > 0);
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

      uint8_t length() {
        uint8_t size = 0;
        while (data[size] != '\0' and size < N)
          ++size;
        return size;
      }
      void append(char c) {
        uint8_t size = length();
        MAKE_SURE(size < N);
        data[size] = c;
        if (size + 1 < N)
          data[size + 1] = '\0';
      }
      void clear() {
        data = {'\0'};
      }
      char& at(uint8_t index) {
        MAKE_SURE(index < N);
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
      String() = default;
      String(std::string str) {
        for (char c : str)
          append(c);
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
        MAKE_SURE(!contains(key));
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