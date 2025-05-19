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



#if __has_include("httplib.h")
  #include "httplib.h"
#endif

#if __cplusplus < 202002L
  #error "Requires C++ >= 20"
#endif



// Note: Doxygen comments aren't supposed to be professional/technical. Only meant to make stuff readable.
namespace cslib {
  // Jack of all trades (Helper functions and classes)

  // Other
  #define MACRO inline constexpr auto // Macros for macro definitions
  #define THROW_HERE(reason) throw std::runtime_error(std::string(__FILE__) + ":" + std::to_string(__LINE__) + std::string(": \033[1m") + reason + "\033[0m");
  #define MAKE_SURE(condition) if (!(condition)) THROW_HERE("Assertion failed: " + std::string(#condition));



  void sleep(int ms) {
    /*
      This function takes a number and sleeps for that many milliseconds.
      Example:
        sleep(1000);
        // Sleeps for 1 second
    */
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
  }



  void sh_call(std::string command) { // TODO: Fix blocking exit
    /*
      Executes a shell command
      Example:
        sh_call("echo Hello World");
        // Prints "Hello World" to the console
    */
    if (system(command.c_str()) != 0)
      THROW_HERE("Failed: " + command);
  }



  void clear_console(uint8_t lines = 0) {
    /*
      Print so many new lines that the text before is gone
      Example:
        clear_console();
        // prints 100 new lines
    */
    for (uint8_t i = 0; i < lines; ++i)
      std::cout << '\n';
    std::cout << std::flush;
  }



  std::string get_env(std::string var) {
    /*
      Returns the value of the environment variable with that name.
      Example:
        // get_env("PATH") is "/usr/local/bin" depending on the system
    */
    std::string env = getenv(var.c_str());
    MAKE_SURE(!env.empty());
    return env;
  }



  std::vector<int> range(int start, int end) {
    /*
      Start and end value and returns a vector of integers from
      start to end (inclusive).
      Example:
        // range(1, 10) is {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}
    */
    std::vector<int> result;
    if (start > end)
      for (int i = start; i >= end; --i)
        result.push_back(i);
    else if (start < end)
      for (int i = start; i <= end; ++i)
        result.push_back(i);
    else
      result.push_back(start);
    return result;
  }
  std::vector<int> range(int end) {
    /*
      Exact same as the function above, but with start = 0
    */
    return range(0, end);
  }



  template <typename T>
  std::string str(T value) {
    /*
      Tries to find the correct function to convert T to
      string.
      Example:
        str = str(42);
        // str = "42"
        str = str("Hello World");
        // str = "Hello World"
    */

    // Try for std::string(T)
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

    // Try for std::to_string(T)
    if constexpr (std::is_arithmetic_v<T>)
      return std::to_string(value);
    else if constexpr (std::is_same_v<T, bool>)
      return value ? "true" : "false";
    else if constexpr (std::is_same_v<T, std::nullptr_t>)
      return "nullptr";

    // Give up
    THROW_HERE("Failed str conversion");
    return "";
  }



  std::string shorten_end(std::string str, size_t maxLength) {
    /*
      Cut and add "..." to the end of the string if it's too long
      Example:
        str = "Hello World";
        str = shorten_end(str, 5);
        // str = "He..."
        str = shorten_end(str, 300);
        // str = "Hello World"
        str = shorten_end(str, -1);
        // because of size_t underflow, maxLength is 2^64
    */
    if (str.length() > maxLength)
      str = str.substr(0, maxLength - 3) + "..."; // 3 for "..."
    return str;
  }
  std::string shorten_begin(std::string str, size_t maxLength) {
    /*
      Same as above but cutting the beginning of the string
    */
    if (str.length() > maxLength)
      str = "..." + str.substr(str.length() - maxLength + 3); // 3 for "..."
    return str;
  }



  std::string to_upper(std::string str) {
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
  std::string to_lower(std::string str) {
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
      Example:
        // roll_dice(1, 6) is 1, 2, 3, 4, 5 or 6
    */

    if (min > max) std::swap(min, max);

    // Special thanks to copilot. No idea what this does
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dis(min, max);
    return dis(gen);
  }



  std::map<std::string, std::map<std::string, std::string>> parse_ini(std::string iniFile) {
    /*
      Parses an INI file. Returns a map of sections and their key-value pairs.
      Note:
        Function tolerates a lot of errors.
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
    static std::ofstream& write_into() {
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
      libraries. The '_' prefix is used to avoid name
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
          write_into() << msg << std::flush;
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
      std::cout << colorCode << prefix << RESET << msg << std::flush;
      if (OUT_WRITE)
        write_into() << prefix << msg << std::flush;
      return proxy;
    }

    static std::string in() {
      std::string input;
      std::getline(std::cin, input);
      if (OUT_WRITE)
        write_into() << input << std::flush;
      return input;
    }
  };
  inline Out cslibOut("[Custom Library++]: ", Out::_BLACK); // Out channel for this lib



  class TimeStamp { public:
    /*
      A class that represents a timestamp. It can be used to measure
      the time taken by a function or a block of code.
    */

    static void sleep_until_tomorrow(int hour, int minute, int second = 0) {
      /*
        This function takes a time and sleeps until that time tomorrow.
        Example:
          cslib::TimeStamp::sleep_until_tomorrow(3, 33);
          // Sleeps until 3:33:00 tomorrow
        Note:
          Again, special thanks to copilot.
      */
      std::time_t now = std::time(nullptr);
      std::tm* local_time = std::localtime(&now);
      cslibOut << "Right now: " << std::put_time(local_time, "%Y-%m-%d %H:%M:%S") << '\n';
      local_time->tm_hour = hour;
      local_time->tm_min = minute;
      local_time->tm_sec = second;
      std::time_t target_time = std::mktime(local_time);
      local_time->tm_mday += 1;
      target_time = std::mktime(local_time);
      std::tm* target_tm = std::localtime(&target_time);
      cslibOut << "Ending at: " << std::put_time(target_tm, "%Y-%m-%d %H:%M:%S") << '\n';
      cslibOut << "Will take: " << std::difftime(target_time, now) << " seconds\n";
      std::this_thread::sleep_until(std::chrono::system_clock::from_time_t(target_time));
    }

    static std::string add_zero(uint8_t value) {
      return (value < 10 ? "0" : "") + std::to_string(value);
    }

    // I wouldn't change them directly
    uint8_t day;
    uint8_t month;
    uint8_t year; // Year since 1900 (had 21st century wouldn't fit into uint8_t)
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    void update() {
      /*
        Apply current time data into all attributes of
        instances.
      */
      std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
      std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
      struct tm* timeinfo = std::localtime(&currentTime);
      day = timeinfo->tm_mday;
      month = timeinfo->tm_mon + 1;
      year = timeinfo->tm_year; // Year since 1900
      hour = timeinfo->tm_hour;
      minute = timeinfo->tm_min;
      second = timeinfo->tm_sec;
    }

    std::string to_str() const {
      /*
        Convert to string (YYYY-MM-DD HH:MM:SS)
      */
      return add_zero(day) + '-' +
      add_zero(month) + '-' +
      std::to_string(year + 1900) + ' ' +
      add_zero(hour) + ':' +
      add_zero(minute) + ':' +
      add_zero(second);
    }

    int64_t diff_in_s(TimeStamp& other) const {
      /*
        Calculate the difference in seconds
        Note:
          This is nowhere near accurate, but
          should be enough for most cases.
      */
      int64_t diff = 0;
      diff += (year - other.year) * 365 * 24 * 60 * 60;
      diff += (month - other.month) * 30 * 24 * 60 * 60;
      diff += (day - other.day) * 24 * 60 * 60;
      diff += (hour - other.hour) * 60 * 60;
      diff += (minute - other.minute) * 60;
      diff += (second - other.second);
      return diff;
    }

    TimeStamp() {update();}
    TimeStamp(uint8_t day, uint8_t month, uint8_t year,
              uint8_t hour, uint8_t minute, uint8_t second) :
              day(day), month(month), year(year), hour(hour),
              minute(minute), second(second) {}
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
    /*
      std::filesystem is very chunky and not very documented, so
      this class is a smaller wrapper around it. Memory efficiency
      is clearly favored over speed.
      Important Note:
        - std::string is 1 byte but linux/windows/macOS allow
          utf-8 characters in paths. This class doesn't support
          utf-8 characters and assumes all paths are ASCII.
    */

    static MACRO PATH_SEPARATOR = '/';

    enum Type : char {
      File = 'F',
      Directory = 'D',
    };

    std::string locationAsStr;

    VirtualPath(std::string where) : locationAsStr(where) {
      /*
        Constructor that takes a string and checks if it's a valid path.
        Notes:
          - If path is relative, it will be converted to an absolute path.
          - If path is empty, you will crash.
          - To unify path separators across instances, all backslashes
            are converted to forward slashes.
          - Main folder ('/') is not tolerated as a path.
      */

      if (where.back() == '\\' or where.back() == '/')
        where.pop_back();

      MAKE_SURE(!where.empty());
      if (where.find('\\') == std::string::npos and where.find('/') == std::string::npos) // If path is directly in the working dir
        where = std::filesystem::current_path().string() + "\\" + where;
      for (char& c : where)
        if (c == '\\') c = PATH_SEPARATOR;

      MAKE_SURE(std::filesystem::exists(where));
      locationAsStr = std::filesystem::absolute(where).string();
    }
    VirtualPath(std::string where, Type shouldBe) : VirtualPath(where) {
      /*
        Same as above, but checks if the path is of a specific type.
      */
      MAKE_SURE(type() == shouldBe);
    }

    std::string operator[](uint8_t index) const {
      /*
        Grab a specific parent path from the route
        to the current element.
        Example:
          VirtualPath path("/gitstuff/cslib/cslib.h++");
          std::string parent = path[1];
          // parent = "cslib"
      */
      #ifdef _WIN32
        return separate(locationAsStr, PATH_SEPARATOR).at(index);
      #endif
      #ifdef __linux__
        return separate(locationAsStr.substr(1), PATH_SEPARATOR).at(index); // Remove leading /
      #endif
    }
    Type type() const {
      /*
        Enum file-type of this instance.
        Example:
          VirtualPath path("/gitstuff/cslib/cslib.h++");
          VirtualPath::Type type = path.type();
          // type = VirtualPath::File
      */
      if (std::filesystem::is_directory(locationAsStr)) return Directory;
      else if (std::filesystem::is_regular_file(locationAsStr)) return File;
      else THROW_HERE("Path '" + locationAsStr + "' is of unknown type");
      return Directory; // To make the compiler happy
    }
    std::string filename() const {
      /*
        Filename of this instance, including the extension.
        Example:
          VirtualPath path("/gitstuff/cslib/cslib.h++");
          std::string filename = path.filename();
          // filename = "cslib.h++"
      */
      return std::filesystem::path(locationAsStr).filename().string();
    }
    VirtualPath parent_path() const {
      /*
        Parent path of this instance.
        Example:
          VirtualPath path("/gitstuff/cslib/cslib.h++");
          VirtualPath parent = path.parent_path();
          // parent = "/gitstuff/cslib" (instance of VirtualPath)
      */
      if (locationAsStr.find_last_of(PATH_SEPARATOR) == std::string::npos) THROW_HERE("Path has no parent");
      return VirtualPath(std::filesystem::path(locationAsStr).parent_path().string());
    }
    uint8_t depth() const {
      /*
        Number of elements that is required to reach
        this element from disk.
        Example:
          VirtualPath path("/gitstuff/cslib/cslib.h++");
          uint8_t depth = path.depth();
          // depth = 2
      */
      return separate(locationAsStr, PATH_SEPARATOR).size() - 1; // -1 for the last element
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
      std::filesystem::file_time_type ftime = std::filesystem::last_write_time(locationAsStr);
      std::chrono::system_clock::time_point timePoint = std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
      std::time_t cftime = std::chrono::system_clock::to_time_t(timePoint);
      struct tm* timeinfo = std::localtime(&cftime);
      return TimeStamp(timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year,
                       timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    }

    void move_to(VirtualPath moveTo) {
      /*
        Move this instance to a new location.
        Example:
          VirtualPath path("/gitstuff/cslib/cslib.h++");
          path.move_to("/archive/cslib.h++");
          // path = "/archive/cslib.h++"
      */
      MAKE_SURE(moveTo.type() == Directory);
      MAKE_SURE(moveTo.locationAsStr != this->locationAsStr);
      std::string willBecome = moveTo.locationAsStr + str(PATH_SEPARATOR) + this->filename();
      MAKE_SURE(!std::filesystem::exists(willBecome));
      std::filesystem::rename(this->locationAsStr, willBecome);
      this->locationAsStr = cslib::VirtualPath(willBecome).locationAsStr; // Apply changes
    }
    VirtualPath copy_to(std::string where) const {
      /*
        Copy this instance to a new location.
        Example:
          VirtualPath path("/gitstuff/cslib/cslib.h++");
          path.copy_to("/shared/cslib.h++");
          // path = "/shared/cslib.h++"
      */
      MAKE_SURE(!where.empty());
      std::string willBecome = std::filesystem::absolute(where).string();
      willBecome += str(PATH_SEPARATOR) + filename();
      MAKE_SURE(!std::filesystem::exists(willBecome));
      std::filesystem::copy(locationAsStr, willBecome);
      return VirtualPath(willBecome);
    }
  };



  class File { public:
    /*
      Child class of VirtualPath that represents a file.
    */

    VirtualPath location; // Evade inheritance hell

    File(std::string where) : location(where, VirtualPath::File) {}

    std::string asStr() const {return location.locationAsStr;}

    std::string content() const {
      /*
        Read the content of the file and return it as a string.
        Example:
          File file("/gitstuff/cslib/cslib.h++");
          std::string content = file.content();
          // content = "Hello World"
        Note:
          - File is opened in binary mode to avoid issues with
            line endings and encoding.
          - File is closed automatically when the function returns.
          - Big ass files (like 200 gbs or more) wouldn't fit into
            the RAM. This function doesn't handle such cases. I'll work on it.
      */
      std::ifstream file(asStr(), std::ios::binary);
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
      return std::filesystem::path(asStr()).extension().string();
    }

    size_t size() const {
      /*
        Get the file size in bytes.
        Example:
          File file("/gitstuff/cslib/cslib.h++");
          size_t size = file.size();
          // size = ~400.000 bytes (50000 characters with each 1 byte)
      */
      return std::filesystem::file_size(asStr());
    }
  };



  class Folder { public: // FIXME: All attributes must be changed manually upon changes
    /*
      Child class of VirtualPath that represents a folder and
      everything in it.
    */

    using EntryType = std::variant<Folder, File>;
    std::deque<EntryType> content;
    VirtualPath location;

    Folder(std::string where) : location(where, cslib::VirtualPath::Directory) {
      update();
    }

    std::string asStr() const {return location.locationAsStr;}

    void update() {
      /*
        Update the content of the folder.
        Example:
          Folder folder("/gitstuff");
          folder.update();
          // folder.content() = {File("rl"), File("a36m"), Folder("cslib")}
      */
      content.clear();

      for (std::filesystem::directory_entry entry : std::filesystem::directory_iterator(asStr()))
        if (entry.is_directory())
          content.push_back(Folder(entry.path().string()));
        else if (entry.is_regular_file())
          content.push_back(File(entry.path().string()));
        else THROW_HERE("Bizarre entry type: " + entry.path().string()); // Syslinks and other stuff are not supported

      content.shrink_to_fit();
    }


    // TODO: Search functions
    Folder& search_folder();
    File& search_file();
    Folder& recursive_search_folder();
    File& recursive_search_file();
    std::string tree();
  };



  // Namespaces
  namespace FileParsing {
    /*
      Tools for parsing files and converting them to different formats.
    */

    std::string char_to_intstr(char c) {
      /*
        Takes a char and converts it to the number it
        is on the ASCII table as string.
        Example:
          char c = 'A';
          std::string str = char_to_intstring(c);
          // str = "65"
      */
      return std::to_string(static_cast<int>(c));
    }


    std::string convert_filedata_to_intarray(std::string filePath) {
      /*
        Takes a string and converts it to a string of integers.
        Example:
          std::string filePath = "/file.txt";
          // The file contains "Hi"
          std::string intarray = convert_filedata_to_intarray(filePath);
          // intarray = "{72,105}"
      */

      std::string result = "{";
      for (char c : File(filePath).content())
        result += char_to_intstr(c) + ",";
      if (result.back() == ',') result.pop_back();
      result += "}";
      return result;
    }


    void create_file_from_intarray(std::string filePath, std::string intarray /*ptr to int array*/) {
      /*
        This function takes a string and creates a file from it.
        Example:
          std::string filePath = "/file.txt";
          const char intarray[] = {72,105};
          create_file_from_intarray(filePath, intarray);
          // The file contains "Hi"
      */

      MAKE_SURE(!std::filesystem::exists(filePath));

      std::string fileContentAsStr(intarray);
      fileContentAsStr = fileContentAsStr.substr(0, fileContentAsStr.find(filePath)); // Quick fix for the filePath being at the end of the file

      std::ofstream file(filePath);
      MAKE_SURE(file.is_open());
      MAKE_SURE(file.good());
      file << fileContentAsStr;
    }


    File constexpr_file(std::string headerDefinition, std::string targetFile) {
      /*
        Function reads a file, converts it's content to a string of integers,
        and writes it into an header file with constexpr char headerFileName[].
        Example:
          constexpr_file("my_header", "my_file.txt");
          // my_header.h contains:
          // constexpr char my_header[] = {72,105};
      */

      File constructorOfThisCheckForErrors(targetFile);
      MAKE_SURE(!std::filesystem::exists(headerDefinition + ".h"));

      // Check for illegal characters
      for (char c : headerDefinition)
        switch (c) {
          case 'a': case 'b': case 'c': case 'd': case 'e':
          case 'f': case 'g': case 'h': case 'i': case 'j':
          case 'k': case 'l': case 'm': case 'n': case 'o':
          case 'p': case 'q': case 'r': case 's': case 't':
          case 'u': case 'v': case 'w': case 'x': case 'y':
          case 'z': case 'A': case 'B': case 'C': case 'D':
          case 'E': case 'F': case 'G': case 'H': case 'I':
          case 'J': case 'K': case 'L': case 'M': case 'N':
          case 'O': case 'P': case 'Q': case 'R': case 'S':
          case 'T': case 'U': case 'V': case 'W': case 'X':
          case 'Y': case 'Z': case '_': case '0': case '1':
          case '2': case '3': case '4': case '5': case '6':
          case '7': case '8': case '9':
            break;
          default:
            THROW_HERE("Header definition '" + headerDefinition + "' contains illegal characters");
        }

    std::ofstream headerFile(headerDefinition + ".h");
    MAKE_SURE(headerFile.is_open());
    MAKE_SURE(headerFile.good());

    headerFile << "constexpr char " << headerDefinition << "[] = ";
    headerFile << convert_filedata_to_intarray(targetFile) << ";";

    return File(headerDefinition + ".h");
   }
  };



  namespace TinySTL {
    // The stl is good but sometimes you need something smaller

    template <typename T>
    class Optional { public: // TODO: Look into it further
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



    template <uint8_t N = 4>
    class String { public:
      /*
        Static fixed size string. It can hold up to N characters and
        is only null-terminated when the string isn't full. If it is,
        the size-limit acts as a terminator.
        Example:
          String<2> str("Hi");
          // str = {'H', 'i'}
          String<3> str2("Hi");
          // str2 = {'H', 'i', '\0'}
          String<4> str3("Hi");
          // str3 = {'H', 'i', '\0', 'm' ('m' could have been used
          for something else earlier but ignored after null-termination)}
      */

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
        for (Node node : data)
          if (node.key == key)
            return node.value;
        THROW_HERE("Key not found");
      }
      V& operator[](K key) {
        return at(key);
      }
      bool contains(K key) {
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




  // Unique namespaces
  #if (__has_include("/usr/bin/lrztar"))
  namespace LRZTAR { // TODO: Check https://github.com/ckolivas/lrzip for lrzdir
    /*
      Compress and decompress stuff using LRZip
      Important note:
        At compression, it matters how the path is specified. lrztar will
        also contain all the parent paths of the source directory. So
        if you specify the path as "/home/user/docs", the compressed file
        will contain the parent directories "home" and "user" as empty
        directories as well. Example:
        /home (literally only user directory)
         └ user (also just the docs directory)
            └ docs (contains the files)
              ├ what ever you put in here
              └ ...
        I haven't found a way to change this. If you want to compress a
        directory, make it inside the same directory as the executable.
        This doesn't matter for decompression. Decompression also acts
        differently. To be sure, decompress within the same directory
        as the binary and the compressed file.
    */

    MACRO LRZTAR_VIEW_CLI = "lrzcat \"{FILEPATH}\" | tar -tv";
    MACRO LRZTAR_COMPRESS_CLI = "lrztar -z ";
    MACRO LRZTAR_DECOMPRESS_CLI = "lrztar -d ";
    MACRO LRZTAR_EXTENSION = ".tar.lrz";
    Out lrztarOut("[LRZTAR by cslib]: ", Out::_CYAN);


    void print_lrztar_content(File sourceFile) {
      /*
        Print the content of a tar.lrz file.
        Example:
          print_lrztar_content("my_dir.tar.lrz");
          // Prints the content of my_dir.tar.lrz
      */

      std::string command = format(LRZTAR_VIEW_CLI, {{"FILEPATH", sourceFile.asStr()}}); // lrzcat "my_dir.tar.lrz" | tar -tv
      cslib::sh_call(command);
    }


    File compress(Folder sourceDir) {
      /*
        Compress a directory into a tar.lrz file. A tar.lrz file will be
        created next the executable. The sourceDir will not be deleted.
        Example:
          File compressed = compress("docs");
          // docs.tar.lrz is created in workspace
        Note: If sourceDir is deleted, instances of it will be invalid
        and cause a crash if accessed.
      */

      MAKE_SURE(sourceDir.location.parent_path().locationAsStr == std::filesystem::current_path().string());

      std::string willBecome = sourceDir.location.filename() + LRZTAR_EXTENSION; // docs + .tar.lrz
      MAKE_SURE(!std::filesystem::exists(willBecome));

      cslib::sh_call(str(LRZTAR_COMPRESS_CLI) + " \"" + sourceDir.location.filename() + "\""); // lrztar -z docs

      return File(willBecome);
    }
    File compress(Folder sourceDir, Folder putIn) {
      /*
        Compress a directory into a tar.lrz file. The sourceDir will be deleted.
        Example:
          File compressed = compress_and_move("docs", "/root/backups");
          // docs.tar.lrz is created in workspace and moved to /root/backups
      */

      std::string wouldBecome = putIn.asStr() + VirtualPath::PATH_SEPARATOR + sourceDir.location.filename() + LRZTAR_EXTENSION;
      MAKE_SURE(!std::filesystem::exists(wouldBecome));

      compress(sourceDir).location.move_to(putIn.location);

      return File(wouldBecome); // Check for existence again with constructor
    }


    Folder decompress(File sourceFile) {
      /*
        Decompress a tar.lrz file. Folder is located in workspace
        Example:
          Folder decompressed = decompress("docs.tar.lrz");
          // docs is created in workspace
      */

      MAKE_SURE(sourceFile.location.parent_path().locationAsStr == std::filesystem::current_path().string());

      std::string sourceName = sourceFile.asStr(); // docs.tar.lrz
      MAKE_SURE(sourceName.substr(sourceName.length() - str(LRZTAR_EXTENSION).length()) == LRZTAR_EXTENSION);
      /*
        .extension() wouldn't recognize the .tar in .tar.lrz as part of the extension
      */

      std::string willBecome = sourceName.substr(0, sourceName.length() - str(LRZTAR_EXTENSION).length()); // Remove ".tar.lrz"
      MAKE_SURE(!std::filesystem::exists(willBecome))

      cslib::sh_call(str(LRZTAR_DECOMPRESS_CLI) + " \"" + sourceName + "\""); // lrztar -d "docs.tar.lrz"
      return Folder(willBecome);
    }
    Folder decompress(File sourceFile, Folder putIn) {
      /*
        Decompress a tar.lrz file. The sourceFile will be deleted.
        Example:
          Folder decompressed = decompress_and_move("docs.tar.lrz", "/root/backups");
          // docs is created in workspace and moved to /root/backups
      */

      std::string willBecome = putIn.asStr() + VirtualPath::PATH_SEPARATOR + sourceFile.location.filename().substr(0, sourceFile.location.filename().length() - str(LRZTAR_EXTENSION).length());
      MAKE_SURE(!std::filesystem::exists(willBecome));

      Folder ex = decompress(sourceFile);
      ex.location.move_to(putIn.location);
      return Folder(willBecome);
    }
  };
  #endif



  #if __has_include("httplib.h")
  namespace LocalServer { // [Not tested]
    /*
      A simple local server using the httplib library.
      It can be used to serve files and handle requests.
      Using library:
        github.com/yhirose/cpp-httplib
    */

    MACRO PORT = 8080;
    MACRO CONTENT_TYPE = "text/plain";
    MACRO LISTEN_TO = "0.0.0.0";


    std::function<void(const httplib::Request&, httplib::Response&)> errorHandler;
    std::map<std::string, std::function<void(const httplib::Request&, httplib::Response&)>> postRoutes;
    std::map<std::string, std::function<void(const httplib::Request&, httplib::Response&)>> getRoutes;
    /*
      A map of routes and their corresponding handlers.
      The key is the route and the value is the handler function.
      Example:
        routes["/"] = [](const httplib::Request& req, httplib::Response& res) {
          res.set_content("Hello World", CONTENT_TYPE);
        };
      Keep in mind that after the server is started, new routes are ignored.
      The server will only listen to the routes that were added before.
      Note: the '/' in the route is needed.
    */



    std::map<std::string, std::string> extract_params(std::string reqAsStr) {
      /*
        Extracts the parameters from the request and returns them as a map.
        The key is the parameter name and the value is the parameter value.
      */
      std::map<std::string, std::string> params;
      std::string paramsStr = reqAsStr.substr(reqAsStr.find('?') + 1); // +1 to skip the '?'
      for (std::string param : separate(paramsStr, '&')) {
        std::string key = param.substr(0, param.find('='));
        std::string value = param.substr(param.find('=') + 1); // +1 to skip the '='
        params.insert({key, value});
      }
      return params;
    }



    bool serverIsRunning = false;
    class Server { public:
      // Wrapper around the httplib::Server with RAII


      httplib::Server server;

      Server() {
        if (serverIsRunning)
          THROW_HERE("Server is already running");
        serverIsRunning = true;

        for (auto [route, handler] : postRoutes) {
          cslibOut << "[Server] Adding POST route: '" << route << "'";
          server.Post(route, handler);
        }

        for (auto [route, handler] : getRoutes) {
          cslibOut << "[Server] Adding GET route: '" << route << "'";
          server.Get(route, handler);
        }

        cslibOut << "[Server] Adding error handler";
        if (errorHandler != nullptr)
          server.set_error_handler(errorHandler);
        else
          server.set_error_handler([](const httplib::Request& req, httplib::Response& res) {
            std::string answer = "Existing routes:\n";
            answer += "GET:\n";
            for (auto [route, handler] : getRoutes)
              answer += route + "\n";
            answer += "POST:\n";
            for (auto [route, handler] : postRoutes)
              answer += route + "\n";
            answer += "Error: Route not found\n";
            res.status = 404;
            res.set_content(answer, CONTENT_TYPE);
          });

        cslibOut << "[Server] Listening on '" << LISTEN_TO << "' at port " << PORT;
        server.listen(LISTEN_TO, PORT);
      }
    };



    std::string literally_string(std::string str); // (not implemented yet)
      /*
        Converts the string to what it's literally is. Usuualy
        used for external libraries that need the string in a specific format.
        Example:
          std::string str = "C:\\";
          str = literally_string(str);
          // str = "C:\\\\";
      */
    std::string relatively_string(std::string str); // (not implemented yet)
      /*
        Converts the string to what C++ would interpret it as.
        Example:
          std::string str = "C:\\\\";
          str = relatively_string(str);
          // str = "C:\\";
      */



    char decode(std::string s) {
      /*
        Decodes the character from a URL-safe format.
        Example:
          char c = "%20";
          c = decode(c);
          // c = ' '
      */
      MAKE_SURE(s.length() == 3);
      MAKE_SURE(s.at(0) == '%');

      if (s == "%20") return '\x20'; // Space
      if (s == "%21") return '\x21'; // Exclamation mark
      if (s == "%22") return '\x22'; // Double quote
      if (s == "%23") return '\x23'; // Hash
      if (s == "%24") return '\x24'; // Dollar sign
      if (s == "%25") return '\x25'; // Percent sign
      if (s == "%26") return '\x26'; // Ampersand
      if (s == "%27") return '\x27'; // Single quote
      if (s == "%28") return '\x28'; // Left parenthesis
      if (s == "%29") return '\x29'; // Right parenthesis
      if (s == "%2A") return '\x2A'; // Asterisk
      if (s == "%2B") return '\x2B'; // Plus sign
      if (s == "%2C") return '\x2C'; // Comma
      if (s == "%2D") return '\x2D'; // Minus sign
      if (s == "%2E") return '\x2E'; // Dot
      if (s == "%2F") return '\x2F'; // Slash
      if (s == "%3A") return '\x3A'; // Colon
      if (s == "%3B") return '\x3B'; // Semicolon
      if (s == "%3C") return '\x3C'; // Less than
      if (s == "%3D") return '\x3D'; // Equals sign
      if (s == "%3E") return '\x3E'; // Greater than
      if (s == "%3F") return '\x3F'; // Question mark
      if (s == "%40") return '\x40'; // At sign
      if (s == "%5B") return '\x5B'; // Left square bracket
      if (s == "%5C") return '\x5C'; // Backslash
      if (s == "%5D") return '\x5D'; // Right square bracket
      if (s == "%7B") return '\x7B'; // Left curly brace
      if (s == "%7C") return '\x7C'; // Vertical bar
      if (s == "%7D") return '\x7D'; // Right curly brace
      if (s == "%7E") return '\x7E'; // Tilde

      THROW_HERE("Invalid URL encoding");
      return '\0'; // Invalid character
    }
    std::string encode(char s) {
      /*
        Encodes the character to a URL-safe format.
        Example:
          char c = ' ';
          c = encode(c);
          // c = "%20"
      */
      switch (s) {
        case '\x20': return "%20"; // Space
        case '\x21': return "%21"; // Exclamation mark
        case '\x22': return "%22"; // Double quote
        case '\x23': return "%23"; // Hash
        case '\x24': return "%24"; // Dollar sign
        case '\x25': return "%25"; // Percent sign
        case '\x26': return "%26"; // Ampersand
        case '\x27': return "%27"; // Single quote
        case '\x28': return "%28"; // Left parenthesis
        case '\x29': return "%29"; // Right parenthesis
        case '\x2A': return "%2A"; // Asterisk
        case '\x2B': return "%2B"; // Plus sign
        case '\x2C': return "%2C"; // Comma
        case '\x2D': return "%2D"; // Minus sign
        case '\x2E': return "%2E"; // Dot
        case '\x2F': return "%2F"; // Slash
        case '\x3A': return "%3A"; // Colon
        case '\x3B': return "%3B"; // Semicolon
        case '\x3C': return "%3C"; // Less than
        case '\x3D': return "%3D"; // Equals sign
        case '\x3E': return "%3E"; // Greater than
        case '\x3F': return "%3F"; // Question mark
        case '\x40': return "%40"; // At sign
        case '\x5B': return "%5B"; // Left square bracket
        case '\x5C': return "%5C"; // Backslash
        case '\x5D': return "%5D"; // Right square bracket
        case '\x7B': return "%7B"; // Left curly brace
        case '\x7C': return "%7C"; // Vertical bar
        case '\x7D': return "%7D"; // Right curly brace
        case '\x7E': return "%7E"; // Tilde
        default: THROW_HERE("Invalid character '" + std::string(1, s) + "' for URL encoding");
        return "";
      }
    }
  };
  #endif // __has_include("httplib.h")
};