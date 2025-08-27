// LICENSE: ☝️🤓

// Including every single header that might ever be needed
#include <condition_variable>
#include <initializer_list>
#include <unordered_map>
#include <unordered_set>
#include <string_view>
#include <stop_token>
#include <filesystem>
#include <functional>
#include <stdexcept>
#include <algorithm>
#include <iostream> // Already contains many libraries
#include <optional>
#include <expected>
#include <utility>
#include <fstream>
#include <sstream>
#include <variant>
#include <cstring>
#include <cstdint>
#include <cstddef>
#include <cctype>
#include <cstdio>
#include <random>
#include <vector>
#include <chrono>
#include <thread>
#include <locale>
#include <array>
#include <deque>
#include <cmath>
#include <list>
#include <map>
#include <set>
#ifdef _WIN32
  #include <windows.h>
  #include <io.h>
  #include <fcntl.h>
#endif


#pragma once
/*
  Include this header file only once pwease. No support for
  linking and stuff.
  Prevent multiple inclusions of this header file
*/

#define CPLUSPLUSERRORMAKER_HELPER(x) #x
#define CPLUSPLUSERRORMAKER(x) CPLUSPLUSERRORMAKER_HELPER(x)
static_assert(__cplusplus >= 202002L, "Requires C++20 or newer. Current version is " CPLUSPLUSERRORMAKER(__cplusplus));




namespace cslib {
  // Jack of all trades (Helper functions and classes)

  // Other
  using cstr = const char *const; // C-style string
  using str_t = std::string;
  using strv_t = std::string_view;
  using byte_t = char;
  /*
    Differentiate between a char as byte and
    a char as character in a string
  */
  template <typename T>
  using cptr = const T *const;
  template <typename T>
  using opt = std::optional<T>;
  using maybe_err_t = str_t;
  template <typename T, typename Or = maybe_err_t>
  using maybe = std::expected<T, Or>;
  #define SHARED inline // Alias inline for shared functions, etc.
  #define MACRO inline constexpr auto // Macros for macro definitions
  #define FIXED inline constexpr // Explicit alternative for MACRO



  str_t to_str(const auto&... _anything) noexcept {
    std::ostringstream oss;
    (oss << ... << _anything);
    return oss.str();
  }



  class any_error : public std::runtime_error { public:
    /*
      Custom error class for cslib
      Example:
        throw cslib::any_error("Something went wrong");
    */
    any_error(size_t _lineInCode, strv_t _funcName, const auto&... _msgs) : std::runtime_error([_lineInCode, &_funcName, &_msgs... ] {
      /*
        Create a custom error message with the given messages.
        Example:
          throw cslib::any_error(__LINE__, "Aye", L"yo", '!', 123, true);
      */
      std::ostringstream oss;
      oss << "cslib::any_error called in workspace " << std::filesystem::current_path() << ' ';
      oss << "on line " << _lineInCode << " in function '" << _funcName << "' because: ";
      ((oss << _msgs), ...);
      oss << std::flush;
      return oss.str();
    }()) {}
  };
  #define cslib_throw_up(...) throw cslib::any_error(__LINE__, __func__, __VA_ARGS__)



  void pause(size_t ms) noexcept {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
  }


  std::unexpected<str_t> unexpect(const auto&... _msgs) noexcept {
    return std::unexpected<str_t>(to_str(_msgs...));
  }



  maybe<str_t, int> sh_call(strv_t _command) noexcept {
    /*
      Non-blocking system call that returns the
      output of the command. Returns exit code
      upon failure.
      Important Note:
        This method is VERY prone to injections
    */
    std::array<char, 128> buffer = {};
    str_t result;
    int exitCode = -1;
    FILE* pipe = popen(_command.data(), "r");
    if (!pipe)
      return std::unexpected<int>(exitCode);
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
      result += buffer.data();
    exitCode = pclose(pipe);
    if (exitCode != 0)
      return std::unexpected<int>(exitCode);
    return result;
  }



  MACRO contains(const auto& _lookIn, const auto& _lookFor) {
    /*
      Checks if element `_lookFor` is in `_lookIn` and
      if so, returns a ptr to the first instance of
      `_lookFor` or nullptr if not found.
      Note:
        No noexcept because we don't know how the
        equal-operator behaves
    */
    for (const auto& item : _lookIn)
      if (item == _lookFor)
        return true;
    return false;
  }
  MACRO have_common(const auto& _cont1, const auto& _cont2) {
    /*
      Checks if any element in `_cont1` is also in `_cont2`
      Note:
        No noexcept because contains() might throw
    */
    for (const auto& item : _cont1)
      if (contains(_cont2, item))
        return true;
    return false;
  }



  maybe<str_t> get_env(strv_t _var) noexcept {
    /*
      Get the value of an environment variable.
    */
    cstr envCStr = getenv(_var.data());
    if (envCStr == NULL)
      return unexpect("Environment variable '", _var, "' not found");
    return str_t(envCStr);
  }



  template <typename T>
  requires std::is_integral_v<T>
  std::vector<T> range(const T& _start, const T& _end) noexcept {
    /*
      Simplified range function that takes two integers
      and returns a vector of integers (first inclusive)
    */
    std::vector<T> result;
    if (_start > _end) // reverse
      for (T i = _start; i > _end; --i)
        result.push_back(i);
    else if (_start < _end) // start to end
      for (T i = _start; i < _end; ++i)
        result.push_back(i);
    else // just start
      result.push_back(_start);
    return result;
  }
  template <typename T>
  requires std::is_integral_v<T>
  std::vector<T> range(const T& end) noexcept {
    return range(T(0), end);
  }
  template <typename T>
  std::vector<T> range(cptr<T> _begin, size_t _count) noexcept {
    /*
      For types with no .end() or specific .begin()
      such as c-strings
    */
    return std::vector<T>(_begin, _begin + _count);
  }



  template <typename Func, typename... Args>
  maybe<std::invoke_result_t<Func, Args...>> retry(const Func& _func, size_t _maxAttempts, const Args&... _args) noexcept {
    /*
      Retry a function up to `retries` times
      Example:
        std::function<int()> func = [] {
          // Do something that might fail
        };
        cslib::retry(func, 3, ...);
    */
    while (_maxAttempts-- > 0) {
      try {
        if constexpr (std::is_void_v<std::invoke_result_t<Func, Args...>>) {
          _func(_args...);
          return {};
        }
        else
          return _func(_args...);
      }
      catch (const std::exception& e) {
        if (!_maxAttempts)
          return unexpect("Function failed after maximum attempts: ", e.what());
      }
      catch (...) {
        if (!_maxAttempts)
          return unexpect("Function failed after maximum attempts: unknown exception");
      }
    }
    return unexpect("Function never invoked because _maxAttempts is 0");
  }



  maybe<std::vector<strv_t>> parse_cli_args(int _argc, cstr _args[]) {
    /*
      Parse command line arguments and return
      them as a vector of strings.
      Note:
        Make sure `_argc` and the `_args`'s size
        align even when nullptr or vector won't
        recognize values
    */
    if (_args == nullptr or _argc <= 0)
      return unexpect("No command line arguments provided");
    return std::vector<strv_t>(_args, _args + _argc); // Includes binary name
  }



  str_t stringify_container(const auto& _vec) {
    /*
      Convert a vector to a string representation.
      Example:
        cslib::stringify_container({1, 2, 3}); // "{1, 2, 3}"
    */
    str_t result = "{";
    for (const auto& item : _vec)
      result += to_str(item) + ", ";
    if (result.length() > 1) { // If there are items
      result.pop_back(); // Remove the last comma
      result.pop_back(); // Remove the last space
    }
    result += "}";
    return result;
  }



  MACRO TRIM_WITH = "...";
  FIXED maybe<str_t> shorten_end(strv_t _strsv, size_t _maxLength) noexcept {
    /*
      Example:
        cslib::shorten_end(L"cslib.h++", 6); // "csl..."
    */
    if (_maxLength < strlen(TRIM_WITH))
      return unexpect("maxLength must be at least ", strlen(TRIM_WITH), " ('", TRIM_WITH, "' length)");
    if (_strsv.length() <= _maxLength)
      return str_t(_strsv);
    return str_t(_strsv.substr(0, _maxLength - strlen(TRIM_WITH))) + TRIM_WITH;
  }

  FIXED maybe<str_t> shorten_begin(strv_t _strsv, size_t _maxLength) noexcept {
    /*
      Example:
        cslib::shorten_begin(L"cslib.h++", 6); // "...h++"
    */
    if (_maxLength < strlen(TRIM_WITH))
      return unexpect("maxLength must be at least ", strlen(TRIM_WITH), " ('", TRIM_WITH, "' length)");
    if (_strsv.length() <= _maxLength)
      return str_t(_strsv);
    return str_t(TRIM_WITH) + str_t(_strsv.substr(_strsv.length() - (_maxLength - strlen(TRIM_WITH))));
  }



  std::vector<str_t> separate(strv_t _strsv, strv_t _delimiter) noexcept {
    /*
      Example:
        cslib::separate("John Money", " ") // {"John", "Money"}
    */
    std::vector<str_t> result;
    size_t pos = 0;
    while ((pos = _strsv.find(_delimiter)) != strv_t::npos) {
      result.push_back(str_t(_strsv.substr(0, pos)));
      _strsv.remove_prefix(pos + _delimiter.length());
    }
    if (!_strsv.empty())
      result.push_back(_strsv.data()); // Add the last part
    return result;
  }
  std::vector<str_t> separate(strv_t _strsv, char _delimiter) noexcept {
    return separate(_strsv, to_str(_delimiter));
  }



  template <typename T>
  requires std::is_integral_v<T>
  T roll_dice(T _min, T _max) noexcept {
    /*
      Minimum and maximum value and returns a random
      number between them (inclusive).
      Example:
        cslib::roll_dice(1, 6); // Returns a random number
        between 1 and 6 (inclusive)
    */
    if (_min > _max) std::swap(_min, _max);
    static std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<T> distribution(_min, _max);
    return distribution(generator);
  }



  str_t read_data(std::istream& _inStream) {
    /*
      Read all data from the given istream and return it as a string.
      After reading, the stream is considered empty.
      Throws an error if the stream is not open or in a bad state.
      Note:
        Handling encoding, other states, flags or similar are managed
        by the caller. This function only cleans up its own changes.
    */
    if (!_inStream or !_inStream.good())
      cslib_throw_up("std::istream is not good or in a bad state");
    std::streampos previousPos = _inStream.tellg();
    str_t result{std::istreambuf_iterator<char>(_inStream), std::istreambuf_iterator<char>()};
    _inStream.seekg(previousPos);
    return result;
  }
  void do_io(std::istream& _inStream, std::ostream& _outStream) {
    _outStream << read_data(_inStream) << std::flush;
  }



  class TimeStamp { public:
    /*
      A wrapper around std::chrono
    */
    std::chrono::system_clock::time_point timePoint;
    

    // Contructors and error handling
    TimeStamp() noexcept {timePoint = std::chrono::system_clock::now();}
    TimeStamp(std::chrono::system_clock::time_point _tp) : timePoint(_tp) {}
    TimeStamp(int _hour, int _min, int _sec, int _day, int _month, int _year) {
      /*
        Create a time stamp from the given date and time
        after making sure that the date is valid.
      */
      // Determine date
      std::chrono::year_month_day ymd{
        std::chrono::year(_year),
        std::chrono::month(_month),
        std::chrono::day(_day)
      };
      if (!ymd.ok())
        cslib_throw_up("Invalid date: ", _day, "-", _month, "-", _year);
      // Determine time
      if ((_hour >= 24 or _hour < 0) or (_min >= 60 or _min < 0) or (_sec >= 60 or _sec < 0))
        cslib_throw_up("Invalid time: ", _hour, ":", _min, ":", _sec);
      std::chrono::hh_mm_ss hms{
        std::chrono::hours(_hour) +
        std::chrono::minutes(_min) +
        std::chrono::seconds(_sec)
      };
      // Combine date and time into a time point
      timePoint = std::chrono::system_clock::time_point(
        std::chrono::sys_days(ymd).time_since_epoch() + hms.to_duration()
      );
    }


    str_t as_str() const noexcept {
      /*
        Convert the time point to (almost) ISO 8601
        in format HH:MM:SS DD-MM-YYYY)..
      */
      std::time_t time = std::chrono::system_clock::to_time_t(timePoint);
      return (std::ostringstream() << std::put_time(std::gmtime(&time), "%H:%M:%S %d-%m-%Y")).str();
    }
    size_t year() const noexcept {
      auto ymd = std::chrono::year_month_day(std::chrono::floor<std::chrono::days>(timePoint));
      return size_t(int(ymd.year()));
    }
    size_t month() const noexcept {
      auto ymd = std::chrono::year_month_day(std::chrono::floor<std::chrono::days>(timePoint));
      return size_t(unsigned(ymd.month()));
    }
    size_t day() const noexcept {
      auto ymd = std::chrono::year_month_day(std::chrono::floor<std::chrono::days>(timePoint));
      return size_t(unsigned(ymd.day()));
    }
    size_t hour() const noexcept {
      auto day_point = std::chrono::floor<std::chrono::days>(timePoint);
      auto time_since_midnight = timePoint - day_point;
      auto hms = std::chrono::hh_mm_ss(time_since_midnight);
      return size_t(hms.hours().count());
    }
    size_t minute() const noexcept {
      auto day_point = std::chrono::floor<std::chrono::days>(timePoint);
      auto time_since_midnight = timePoint - day_point;
      auto hms = std::chrono::hh_mm_ss(time_since_midnight);
      return size_t(hms.minutes().count());
    }
    size_t second() const noexcept {
      auto day_point = std::chrono::floor<std::chrono::days>(timePoint);
      auto time_since_midnight = timePoint - day_point;
      auto hms = std::chrono::hh_mm_ss(time_since_midnight);
      return size_t(hms.seconds().count());
    }
  };



  MACRO Bold = "\033[1m";
  MACRO Underline = "\033[4m";
  MACRO Italic = "\033[3m";
  MACRO Reverse = "\033[7m";
  MACRO Hidden = "\033[8m";
  MACRO Black = "\033[30m";
  MACRO Red = "\033[31m";
  MACRO Green = "\033[32m";
  MACRO Yellow = "\033[33m";
  MACRO Blue = "\033[34m";
  MACRO Magenta = "\033[35m";
  MACRO Cyan = "\033[36m";
  MACRO White = "\033[37m";
  MACRO Reset = "\033[0m";
  class Out { public:
    /*
      Print to console with color and optionally into an existing file.
      Usage:
        cslib::Out error("Error: ", cslib::Out::Color::RED);
        error << "Something went wrong";
    */
    std::ostream& outTo;
    str_t prefix;
    Out() = default;
    Out(std::ostream& _outTo) : outTo(_outTo) {
      prefix = "";
    }
    Out(std::ostream& _outTo, strv_t _prefsv = "", strv_t _color = "") : outTo(_outTo) {
      std::ostringstream prefixStream;
      prefixStream << _color << _prefsv;
      if (!_prefsv.empty())
        prefixStream << " ";
      if (!_color.empty())
        prefixStream << Reset;
      prefix = prefixStream.str();
    }
    std::ostream& operator<<(const auto& _msg) const noexcept {
      return outTo << '[' << TimeStamp().as_str() << ']' << prefix << _msg;
    }
    std::ostream& operator<<(auto&& _msg) const noexcept {
      return outTo << '[' << TimeStamp().as_str() << ']' << prefix << std::forward<decltype(_msg)>(_msg);
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
    Benchmark() noexcept {
      startTime = std::chrono::high_resolution_clock::now();
    }
    void reset() noexcept {
      startTime = std::chrono::high_resolution_clock::now();
    }
    double elapsed_ns() noexcept {
      return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
    }
    double elapsed_ms() noexcept {
      return elapsed_ns() / 1'000'000.0f; // Convert nanoseconds to milliseconds
    }
    double elapsed_us() noexcept {
      return elapsed_ns() / 1'000.0f; // Convert nanoseconds to microseconds
    }
  };



  MACRO& PATH_SEPARATOR = std::filesystem::path::preferred_separator;
  class File;
  class Folder;
  class BizarreRoad;
  using road_t = std::variant<File, Folder, BizarreRoad>;
  class Road { public:
    /*
      Abstract base class for filesystem entries.
      Represents a path in the filesystem and provides
      methods to manage it.
    */
    std::filesystem::path isAt; // Covers move and copy semantics


    // Path management
    std::string str() const noexcept { return isAt.string(); }
    std::string name() const noexcept { return isAt.filename().string(); }
    std::filesystem::file_type type() const {
      return std::filesystem::status(isAt).type();
    }
    std::chrono::system_clock::time_point last_modified() const noexcept {
      auto ftime = std::filesystem::last_write_time(isAt);
      return std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
    }
    size_t depth() const noexcept {
      /*
        Example:
          Road path("/gitstuff/cslib/cslib.h++");
          size_t depth = path.depth();
          // depth = 3 (because there are 3 directories before the file)
      */
      return separate(this->str(), to_str(PATH_SEPARATOR)).size() - 1;
    }
    maybe<Folder> operator[](size_t _index) const noexcept;


    maybe<void> rename_self_to(strv_t _newName) noexcept {
      /*
        Rename the entry to a new name.
        Note:
          The new name must not contain any path separators.
      */
      if (_newName.find(PATH_SEPARATOR) != std::string::npos)
        return unexpect("New name '", _newName, "' contains path separators");
      std::filesystem::path newPath = isAt.parent_path() / _newName;
      if (std::filesystem::exists(newPath))
        return unexpect("Path ", newPath, " already exists");
      std::filesystem::rename(isAt, newPath);
      isAt = newPath; // Update the path
    }


    bool operator==(const Road& _other) const noexcept { return this->isAt == _other.isAt; }
    bool operator!=(const Road& _other) const noexcept { return !(*this == _other); }
    bool operator==(const std::filesystem::path& _other) const noexcept { return this->isAt == _other; }
    bool operator!=(const std::filesystem::path& _other) const noexcept { return !(*this == _other); }
    // Implicitly converts wide (c-)strings to std::filesystem::path 


    // Transform into stl
    operator std::string() const noexcept { return this->str(); }
    operator std::filesystem::path() const noexcept { return this->isAt; }
    operator std::filesystem::path&() noexcept { return this->isAt; }
    operator const std::filesystem::path&() const noexcept { return this->isAt; }
    operator std::filesystem::path*() noexcept { return &this->isAt; }
    operator std::filesystem::path*() const noexcept { return const_cast<std::filesystem::path*>(&this->isAt); } // casted immutable
    friend std::ostream& operator<<(std::ostream& _out, const Road& _entry) noexcept {
      return _out << _entry.str();
    }


    protected: Road(const std::filesystem::path& _where) {
      if (_where.empty())
        cslib_throw_up("Path empty");
      isAt = std::filesystem::canonical(_where);
    } // Abstract class can't be instantiated
  };



  class BizarreRoad : public Road { public:
    /*
      Specifically for files that don't fit
      into the usual categories such as
      symbolic links or pipes
    */

    BizarreRoad() = default;
    BizarreRoad(const std::filesystem::path& _where) : Road(_where) {
      if (!std::filesystem::exists(_where))
        cslib_throw_up("Path ", _where, " does not exist");
      if (this->type() == std::filesystem::file_type::regular or
          this->type() == std::filesystem::file_type::directory)
        cslib_throw_up("Path ", _where, " is not a bizarre file");
    }
  };



  class Folder : public Road { public:
    /*
      Child class of Path that represents a folder.
      Example:
        Folder folder("/gitstuff/cslib");
        if (folder.has(Road("/gitstuff/cslib/cslib.h++")))
          std::wcout << "Folder contains the file\n";
    */

    Folder() = default;
    Folder(const std::filesystem::path& _where, bool _createIfNotExists = false) : Road(_where) {
      if (_createIfNotExists and !std::filesystem::exists(_where))
        std::filesystem::create_directory(_where);
      else if (!std::filesystem::is_directory(_where))
        cslib_throw_up("Path ", _where, " is not a directory");
    }


    std::vector<road_t> list() const noexcept;


    opt<road_t> has(strv_t _path) const noexcept;


    maybe<void> move_self_into(Folder& _newLocation) noexcept {
      /*
        Important note:
          Upon moving, subfolders and files objects
          will still point to the old location.
      */
      if (std::filesystem::exists(_newLocation.isAt / isAt.filename()))
        return unexpect("Path ", isAt, " already exists in folder ", _newLocation.isAt);
      std::filesystem::rename(isAt, _newLocation.isAt / isAt.filename());
      isAt = _newLocation.isAt / isAt.filename();
    }


    Folder copy_self_into(Folder& _newLocation, std::filesystem::copy_options _options = std::filesystem::copy_options::recursive) const noexcept {
      std::filesystem::copy(isAt, _newLocation.isAt / isAt.filename(), _options);
      return Folder(_newLocation.isAt / isAt.filename());
    }
  };



  class File : public Road { public:
    /*
      Child class of RouteToFile that represents a file.
      Example:
        File file("/gitstuff/cslib/cslib.h++");
        str_t content = file.content();
        // content = "Around 50 years ago, a group of people..."
    */

    File() = default;
    File(const std::filesystem::path& _where, bool _createIfNotExists = false) : Road(_where) {
      if (_createIfNotExists and !std::filesystem::exists(_where))
        std::ofstream file(_where);
      if (!std::filesystem::is_regular_file(_where))
        cslib_throw_up("Path ", _where, " is not a regular file");
    }
    File(const Road& _where, bool _createIfNotExists) : File(_where.isAt, _createIfNotExists) {}


    std::ifstream reach_in(std::ios::openmode mode) const {
      std::ifstream file(isAt, mode);
      if (!file.is_open() or !file.good())
        cslib_throw_up("Failed to open file ", isAt);
      return file;
    }
    std::ofstream reach_out(std::ios::openmode mode) const {
      std::ofstream file(isAt, mode);
      if (!file.is_open() or !file.good())
        cslib_throw_up("Failed to open file ", isAt);
      return file;
    }

    str_t read_text() const {
      std::ifstream file(reach_in(std::ios::in));
      return str_t((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }
    void edit_text(const auto& _newText) const {
      reach_out(std::ios::out) << _newText;
    }


    std::vector<byte_t> read_binary() const {
      /*
        Streambufs for byte-by-byte reading
        for raw data reading.
      */
      std::ifstream file(reach_in(std::ios::binary));
      return std::vector<byte_t> {
        std::istreambuf_iterator<byte_t>(file),
        std::istreambuf_iterator<byte_t>()
      };
    }
    void edit_binary(const auto *const _newData, size_t _len) const {
      std::ofstream file(reach_out(std::ios::binary | std::ios::trunc));
      file.write(reinterpret_cast<const char *const>(_newData), _len);
      if (!file.good())
        cslib_throw_up("Failed to write into file ", isAt);
    }


    std::string extension() const noexcept {return isAt.extension().string();}
    size_t bytes() const noexcept {return std::filesystem::file_size(isAt);}


    maybe<void> move_self_into(Folder& _newLocation) noexcept {
      if (std::filesystem::exists(_newLocation / isAt.filename()))
        return unexpect("Path ", isAt, " already exists in folder ", _newLocation.isAt);
      std::filesystem::rename(isAt, _newLocation / isAt.filename());
      isAt = _newLocation / isAt.filename(); // Update the path
    }


    File copy_self_into(Folder& _newLocation, std::filesystem::copy_options _options = std::filesystem::copy_options::none) const noexcept {
      std::filesystem::copy(isAt, _newLocation / isAt.filename(), _options);
      return File(_newLocation / isAt.filename());
    }
  };



  // Implementations for Road stuff due to constructors not being defined yet
  std::vector<road_t> Folder::list() const noexcept {
    /*
      List all entries in the folder.
      Returns a vector of Path, File and Folder objects.
    */
    std::vector<road_t> result;
    for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(isAt))
      switch (entry.status().type()) {
        case std::filesystem::file_type::regular:
          result.emplace_back(File(entry.path()));
          break;
        case std::filesystem::file_type::directory:
          result.emplace_back(Folder(entry.path()));
          break;
        default: // other types (symlinks, sockets, etc.)
          result.emplace_back(BizarreRoad(entry.path()));
          break;
      }
    return result;
  }
  opt<road_t> Folder::has(strv_t _lookFor) const noexcept {
    /*
      Check if the folder contains a file or folder with
      the given relative path. If it does, return the
      corresponding Road object.
      Example:
        if (opt<road_t> road = folder.has("subfolder/subfile.txt"))
          // Do something with the road
    */
    if (std::filesystem::exists(isAt / _lookFor))
      switch (std::filesystem::status(isAt / _lookFor).type()) {
        case std::filesystem::file_type::regular:
          return File(isAt / _lookFor);
        case std::filesystem::file_type::directory:
          return Folder(isAt / _lookFor);
        default:
          return BizarreRoad(isAt / _lookFor); // Other types (symlinks, sockets, etc.)
      }
    return std::nullopt;
  }
  maybe<Folder> Road::operator[](size_t _index) const noexcept {
    /*
      Find parent paths by layer
      Example:
        Folder f("/root/projects/folder");
        Folder root = f[0];
        Folder self = f[f.depth()];
        Folder parent = f[f.depth()-1];
    */
    str_t pathAsStr;
    std::vector<str_t> paths = separate("C:"+this->str(), PATH_SEPARATOR);
    if (paths.size() <= _index)
      return unexpect("Index ", _index, " out of bounds (max ", paths.size(), ") for path ", isAt);
    for (size_t pos = 0; pos <= _index; ++pos)
      pathAsStr += paths[pos] + PATH_SEPARATOR;
    return Folder(pathAsStr);
  }



  MACRO SCRAMBLE_LEN = 2; // n^59 possible combinations hehe
  str_t scramble_filename() noexcept {
    /*
      Generate a random filename with a length of `SCRAMBLE_LEN`
      characters. The filename consists of uppercase and lowercase
      letters and digits.
    */
    std::ostringstream randomName;
    for ([[maybe_unused]] auto _ : range(SCRAMBLE_LEN))
      switch (roll_dice(0, 2)) {
        case 0: randomName << roll_dice('A', 'Z'); break; // Uppercase letter
        case 1: randomName << roll_dice('a', 'z'); break; // Lowercase letter
        case 2: randomName << roll_dice('0', '9'); break; // Digit
        default: randomName << '?'; // Shouldn't happen tho
      }
    return randomName.str();
  }



  class TempFile : public File { public:
    /*
      Create a temporary file with a random name in the system's
      temporary directory. The name is generated by rolling dice
      to create a random string of letters and digits.
    */
    TempFile() : File([] {
      str_t tempFileName = "cslibTempFile_" + scramble_filename() + ".tmp";
      // Ensure the file does not already exist
      while (std::filesystem::exists(std::filesystem::temp_directory_path() / tempFileName))
        tempFileName = "cslibTempFile_" + scramble_filename() + ".tmp";
      return std::filesystem::temp_directory_path() / tempFileName;
    }(), true) {}
    ~TempFile() noexcept {
      if (std::filesystem::exists(isAt))
        std::filesystem::remove(isAt);
    }


    TempFile(const TempFile&) = delete;
    TempFile& operator=(const TempFile&) = delete;
  };



  class TempFolder : public Folder { public:
    /*
      Create a temporary folder with a random name in the system's
      temporary directory. The name is generated by rolling dice
      to create a random string of letters and digits.
      Note:
        Destructor also takes all files in the folder
        into account and deletes them.
    */
    TempFolder() : Folder([] {
      str_t tempFolderName = "cslibTempFolder_" + scramble_filename();
      while (std::filesystem::exists(std::filesystem::temp_directory_path() / tempFolderName))
        tempFolderName = "cslibTempFolder_" + scramble_filename();
      return std::filesystem::temp_directory_path() / tempFolderName;
    }(), true) {}
    ~TempFolder() noexcept {
      if (std::filesystem::exists(isAt))
        std::filesystem::remove_all(isAt);
    }


    TempFolder(const TempFolder&) = delete;
    TempFolder& operator=(const TempFolder&) = delete;
  };



  std::expected<str_t, int> wget(strv_t _url) noexcept {
    /*
      Download the content of the given URL using wget
      and return it as a string. If the download fails,
      an error code will be returned.
      Note:
        This function requires wget to be installed on the system.
        It will throw an error if wget is not found or fails to execute.
    */
    return sh_call("wget -q -O - " + str_t(_url.data())); // -q = quiet mode, -O - = output to stdout
  }



  template <typename T>
  requires std::is_arithmetic_v<T>
  MACRO highest_value_of() {
    /*
      Get the highest possible value that
      the type T can represent.
    */
    if constexpr (std::is_integral_v<T>) return std::numeric_limits<T>::max();
    else if constexpr (std::is_floating_point_v<T>) return std::numeric_limits<T>::infinity();
    cslib_throw_up("Unsupported type for highest_value_of");
  }
  template <typename T>
  requires std::is_arithmetic_v<T>
  MACRO lowest_value_of() {
    /*
      Get the lowest possible value that
      the type T can represent.
    */
    if constexpr (std::is_integral_v<T>) return std::numeric_limits<T>::lowest();
    else if constexpr (std::is_floating_point_v<T>) return -std::numeric_limits<T>::infinity();
    cslib_throw_up("Unsupported type for lowest_value_of");
  }



  template <typename T>
  FIXED T& grab(const auto& _variant) noexcept {
    if (!std::holds_alternative<T>(_variant))
      return unexpect("Expected variant type ", typeid(T).name(), " but got ", _variant.index());
    return std::get<T>(_variant);
  }
  template <typename T>
  MACRO holds(const auto& _variant) {
    return std::holds_alternative<T>(_variant);
  }



  template <typename To = int>
  FIXED maybe<To> to_int(const auto _number) noexcept {
    /*
      Gives an additional layer of safety when
      converting numbers with different sizes.
    */
    static_assert(std::is_arithmetic_v<decltype(_number)>, "Passing number type invalid");
    static_assert(std::is_arithmetic_v<To>, "Returning number type invalid");
    static_assert(!std::is_same_v<decltype(_number), To>, "Conversion between same types is not necessary");
    if (_number < lowest_value_of<To>() or _number > highest_value_of<To>())
      return unexpect("Number ", _number, " is higher/lower than ", lowest_value_of<To>(), "/", highest_value_of<To>());
    return To(_number);
  }
} // namespace cslib