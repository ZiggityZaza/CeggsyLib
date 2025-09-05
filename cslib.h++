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
  namespace stdfs = std::filesystem;
  #define MACRO inline constexpr auto // Macros for macro definitions
  #define FIXED inline constexpr // Explicit alternative for MACRO
  MACRO IS_WINDOWS = [] {
    #ifdef _WIN32
      return true;
    #else
      return false;
    #endif
  }();



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
      oss << "cslib::any_error called in workspace " << stdfs::current_path() << ' ';
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
    else // Nothing
      return std::vector<T>();
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



  maybe<str_t> read_data(std::istream& _inStream) noexcept {
    /*
      Read all data from the given istream and return it as a string.
      After reading, the stream is considered empty.
      Throws an error if the stream is not open or in a bad state.
      Note:
        Handling encoding, other states, flags or similar are managed
        by the caller. This function only cleans up its own changes.
    */
    if (!_inStream or !_inStream.good())
      return unexpect("std::istream is not good or in a bad state");
    std::streampos previousPos = _inStream.tellg();
    str_t result{std::istreambuf_iterator<char>(_inStream), std::istreambuf_iterator<char>()};
    _inStream.seekg(previousPos);
    return result;
  }
  maybe<void> do_io(std::istream& _inStream, std::ostream& _outStream) noexcept {
    maybe<str_t> data = read_data(_inStream);
    if (!data)
      return unexpect("Failed to read data: ", data.error());
    _outStream << *data << std::flush;
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



  stdfs::path where_is_path_really(stdfs::path _path) {
    /*
      Returns the actual path of the entry on disk
      but won't resolve symlinks.
    */
    if (stdfs::exists(_path) and stdfs::is_symlink(_path))
      return stdfs::read_symlink(_path);
    return stdfs::canonical(_path);
  }
  MACRO& PATH_SEPARATOR = stdfs::path::preferred_separator;
  class File;
  class Folder;
  class BizarreRoad;
  using road_t = std::variant<File, Folder, BizarreRoad>;
  class Road { public:
    /*
      Abstract base class for filesystem entries.
      Represents a path in the filesystem and provides
      methods to manage it.
      Note:
        Copying/Moving/changing the object won't change
        the represented entry on the disk.
    */
    stdfs::path isAt; // Covers move and copy semantics


    // Path management
    std::string str() const noexcept { return isAt.string(); }
    std::string name() const noexcept { return isAt.filename().string(); }
    stdfs::file_type type() const {
      return stdfs::status(isAt).type();
    }
    std::chrono::system_clock::time_point last_modified() const noexcept {
      auto ftime = stdfs::last_write_time(isAt);
      return std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - stdfs::file_time_type::clock::now() + std::chrono::system_clock::now());
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


    [[nodiscard]] maybe<void> rename_self_to(strv_t _newName) noexcept {
      /*
        Rename the entry to a new name.
        Note:
          No path separators allowed as parent directories
          are not created
      */
      if (_newName.find(PATH_SEPARATOR) != std::string::npos)
        return unexpect("Filename can't be moved with this function (previous: '", str(), "', new: '", _newName, "')");
      const stdfs::path newPath = isAt.parent_path() / _newName;
      if (stdfs::exists(newPath))
        return unexpect("Path ", newPath, " already exists");
      stdfs::rename(isAt, newPath);
      isAt = newPath; // Update the path
      return {}; // "Illegal instruction" error if left out
    }

    
    bool operator==(Road _other) const noexcept { return this->isAt == _other.isAt; }
    bool operator!=(Road _other) const noexcept { return !(*this == _other); }
    bool operator==(stdfs::path _other) const noexcept { return this->isAt == _other; }
    bool operator!=(stdfs::path _other) const noexcept { return !(*this == _other); }
    // Implicitly converts wide (c-)strings to stdfs::path 


    // Transform into stl
    operator std::string() const noexcept { return this->str(); }
    operator stdfs::path() const noexcept { return this->isAt; }
    operator stdfs::path&() noexcept { return this->isAt; }
    operator const stdfs::path&() const noexcept { return this->isAt; }
    operator stdfs::path*() noexcept { return &this->isAt; }
    operator stdfs::path*() const noexcept { return const_cast<stdfs::path*>(&this->isAt); } // casted immutable
    friend std::ostream& operator<<(std::ostream& _out, const Road& _entry) noexcept {
      return _out << _entry.str();
    }


    // Abstract class shouldn't be instantiated
    protected: Road() = default;
    protected: Road(stdfs::path _where) {
      /*
        Resolves the actual path for it's kids
      */
      if (_where.empty())
        cslib_throw_up("Path empty");
      isAt = where_is_path_really(_where);
    }

    static Road create_self(stdfs::path _where) {
      /*
        Creates a new instance of Road without
        allowing global access of constructor
        directly. Additional step to construct
        should be taken to ensure proper
        initialization
      */
      return Road(_where);
    }
  };



  class BizarreRoad : public Road { public:
    /*
      Specifically for files that don't fit into
      the usual categories such as symbolic links
      or pipes.
    */
    BizarreRoad() = default;
    BizarreRoad(stdfs::path _where) : Road(_where) {
      if (!stdfs::exists(isAt))
        cslib_throw_up("Path '", isAt, "' does not exist");
      if (this->type() == stdfs::file_type::regular or
          this->type() == stdfs::file_type::directory)
        cslib_throw_up("Path '", isAt, "' is not a bizarre file");
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
    Folder(stdfs::path _where, bool _createIfNotExists = false) : Road([_where, _createIfNotExists] {
      if (_createIfNotExists and !stdfs::exists(_where))
        stdfs::create_directory(_where);
      if (!stdfs::is_directory(_where))
        cslib_throw_up("Path '", _where.string(), "' is not a directory");
      return _where;
    }()) {}


    stdfs::path operator/(stdfs::path _other) const noexcept {
      return this->isAt / _other;
    }


    std::vector<road_t> list() const noexcept;


    std::vector<Road> untyped_list() const noexcept {
      std::vector<Road> result;
      for (const stdfs::directory_entry& entry : stdfs::directory_iterator(isAt))
        result.emplace_back(Road::create_self(entry.path()));
      return result;
    }


    maybe<road_t> find(strv_t _path) const noexcept;


    [[nodiscard]] maybe<void> move_self_into(Folder _parentDict) noexcept {
      /*
        Important note:
          Upon moving, subfolder and file objects will still
          point to the old location.
      */
      if (stdfs::exists(_parentDict / name()))
        return unexpect("Path ", isAt, " already exists in folder ", _parentDict);
      stdfs::rename(isAt, _parentDict / name());
      isAt = _parentDict / name();
      return {};
    }


    [[nodiscard]] maybe<Folder> copy_self_into(Folder _parentDict) const noexcept {
      if (stdfs::exists(_parentDict / name()))
        return unexpect("Path '", str(), "' already exists in folder '", _parentDict.str(), "'");
      stdfs::copy(isAt, _parentDict / name(), stdfs::copy_options::recursive);
      return Folder(_parentDict / name());
    }


    [[nodiscard]] maybe<void> copy_content_into(Folder _otherDir, stdfs::copy_options _options) const noexcept {
      /*
        Copying self with custom options for extra
        control
      */
      stdfs::copy(isAt, _otherDir, _options);
      return {};
    }
  };



  class File : public Road { public:
    /*
      Child class of RouteToFile that represents a file.
      Example:
        File file("/root/story.txt");
        str_t content = file.read_text();
        // content = "Around 50 years ago, a group of people..."
    */
    File() = default;
    File(stdfs::path _where, bool _createIfNotExists = false) : Road([_where, _createIfNotExists] {
      if (_createIfNotExists and !stdfs::exists(_where))
        std::ofstream(_where) << "";
      if (!stdfs::is_regular_file(_where))
        cslib_throw_up("Path ", _where, " is not a regular file");
      return _where;
    }()) {}


    [[nodiscard]] maybe<std::ifstream> reach_in(std::ios::openmode mode) const noexcept {
      std::ifstream file(isAt, mode);
      if (!file.is_open() or !file.good())
        return unexpect("Couldn't in-stream (read) '", str(), "'");
      return file;
    }
    [[nodiscard]] maybe<std::ofstream> reach_out(std::ios::openmode mode) const noexcept {
      std::ofstream file(isAt, mode);
      if (!file.is_open() or !file.good())
        return unexpect("Couldn't out-stream (write) '", str(), "'");
      return file;
    }

    maybe<str_t> read_text() const noexcept {
      maybe<std::ifstream> file(reach_in(std::ios::in));
      if (!file) return unexpect(file.error());
      return str_t((std::istreambuf_iterator<char>(*file)), std::istreambuf_iterator<char>());
    }
    maybe<void> edit_text(const auto&... _newTexts) const noexcept {
      maybe<std::ofstream> file(reach_out(std::ios::out | std::ios::trunc));
      if (!file) return unexpect(file.error());
      ((*file << _newTexts), ...);
      return {};
    }


    [[nodiscard]] maybe<std::vector<byte_t>> read_binary() const noexcept {
      /*
        Streambufs for byte-by-byte reading
        for raw data reading.
      */
      maybe<std::ifstream> file(reach_in(std::ios::binary));
      if (!file) return unexpect(file.error());
      return std::vector<byte_t> {
        std::istreambuf_iterator<byte_t>(*file),
        std::istreambuf_iterator<byte_t>()
      };
    }
    [[nodiscard]] maybe<void> edit_binary(const auto *const _newData, size_t _len) const {
      maybe<std::ofstream> file(reach_out(std::ios::binary | std::ios::trunc));
      if (!file) return unexpect(file.error());
      file->write(reinterpret_cast<const char *const>(_newData), _len);
      return {};
    }


    std::string extension() const noexcept {return isAt.extension().string();}
    size_t bytes() const noexcept {return stdfs::file_size(isAt);}


    [[nodiscard]] maybe<void> move_self_into(Folder _newLocation) noexcept {
      if (stdfs::exists(_newLocation / name()))
        return unexpect("Path ", name(), " already exists in folder ", _newLocation.str());
      stdfs::rename(isAt, _newLocation / name());
      isAt = _newLocation / name(); // Update the path
      return {};
    }


    [[nodiscard]] File copy_self_into(Folder _newLocation, stdfs::copy_options _options = stdfs::copy_options::none) const noexcept {
      /*
        Copying self with custom options for extra
        control
      */
      stdfs::copy_file(isAt, _newLocation / name(), _options);
      return File(_newLocation / name());
    }
  };



  // Implementations for Road stuff due to constructors not being defined yet
  std::vector<road_t> Folder::list() const noexcept {
    /*
      List all entries in the folder.
      Returns a vector of Path, File and Folder objects.
    */
    std::vector<road_t> result;
    for (stdfs::directory_entry entry : stdfs::directory_iterator(isAt))
      switch (entry.status().type()) {
        case stdfs::file_type::regular:
          result.emplace_back(File(entry.path()));
          break;
        case stdfs::file_type::directory:
          result.emplace_back(Folder(entry.path()));
          break;
        default: // other types (symlinks, sockets, etc.)
          result.emplace_back(BizarreRoad(entry.path()));
          break;
      }
    return result;
  }
  maybe<road_t> Folder::find(strv_t _lookFor) const noexcept {
    /*
      Check if the folder contains a file or folder with
      the given relative path. If it does, return the
      corresponding Road object.
      Note:
        Path MUST be relative
      Example:
        if (maybe<opt<road_t>> road = folder.has("subfolder/subfile.txt"))
          if (*road)
            // Do something with the road
    */
    if (_lookFor.empty())
      return unexpect("Path is empty");
    if constexpr (IS_WINDOWS) {
      if (_lookFor.at(2) == PATH_SEPARATOR and _lookFor.at(1) == ':') // C:\ (c = 0, : = 1, \ = 2)
        return unexpect("Path is absolute");
    }
    else {
      if (_lookFor.at(0) == PATH_SEPARATOR)
        return unexpect("Path is absolute");
    }
    if (stdfs::exists(isAt / _lookFor))
      switch (stdfs::status(isAt / _lookFor).type()) {
        case stdfs::file_type::regular:
          return File(isAt / _lookFor);
        case stdfs::file_type::directory:
          return Folder(isAt / _lookFor);
        default:
          return BizarreRoad(isAt / _lookFor); // Other types (symlinks, sockets, etc.)
      }
    return unexpect("Couldn't find '", _lookFor, "' in here '", str(), "'");
  }
  maybe<Folder> Road::operator[](size_t _index) const noexcept {
    /*
      Find parent paths by layer
      Example:
        Folder f("/root/projects/folder");
        Folder root = f[0];
        Folder parent = f[f.depth()-1];
    */
    stdfs::path parent = isAt;
    if (_index >= depth())
      return unexpect("Trying to reach (", _index, ") fruther than path '", str(), "' would allow (", depth(), ")");
    for ([[maybe_unused]] size_t _ : range(depth() - _index))
      parent = parent.parent_path();
    return Folder(parent);
  }



  MACRO SCRAMBLE_LEN = 5; // 59^n possible combinations
  static_assert(SCRAMBLE_LEN > 0, "SCRAMBLE_LEN must be greater than 0 so that temporary filenames can be generated");
  str_t scramble_filename() noexcept {
    /*
      Generate a random filename with a length of `SCRAMBLE_LEN`
      characters. The filename consists of uppercase and lowercase
      letters and digits.
      Note:
        If all possible names are exhausted, the calling code
        will be stuck in an infinite loop.
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
      while (stdfs::exists(stdfs::temp_directory_path() / tempFileName))
        tempFileName = "cslibTempFile_" + scramble_filename() + ".tmp";
      return stdfs::temp_directory_path() / tempFileName;
    }(), true) {}
    ~TempFile() noexcept {
      if (stdfs::exists(isAt))
        stdfs::remove(isAt);
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
      while (stdfs::exists(stdfs::temp_directory_path() / tempFolderName))
        tempFolderName = "cslibTempFolder_" + scramble_filename();
      return stdfs::temp_directory_path() / tempFolderName;
    }(), true) {}
    ~TempFolder() noexcept {
      if (stdfs::exists(isAt))
        stdfs::remove_all(isAt);
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
  MACRO highest_value_of() noexcept {
    /*
      Get the highest possible value that
      the type T can represent.
    */
    if constexpr (std::is_integral_v<T>) return std::numeric_limits<T>::max();
    else if constexpr (std::is_floating_point_v<T>) return std::numeric_limits<T>::infinity();
    cslib_throw_up("Unsupported type for highest_value_of (impossible)");
  }
  template <typename T>
  requires std::is_arithmetic_v<T>
  MACRO lowest_value_of() noexcept {
    /*
      Get the lowest possible value that
      the type T can represent.
    */
    if constexpr (std::is_integral_v<T>) return std::numeric_limits<T>::lowest();
    else if constexpr (std::is_floating_point_v<T>) return -std::numeric_limits<T>::infinity();
    cslib_throw_up("Unsupported type for lowest_value_of (impossible)");
  }



  /*
    'I know what I am doing' functions to avoid repetitive
    error handling code
  */
  template <typename T, typename... VTs>
  FIXED const maybe<T> get(const std::variant<VTs...>& _variant) noexcept {
    if (!std::holds_alternative<T>(_variant))
      return unexpect("Expected variant type ", typeid(T).name(), " but got ", _variant.index());
    return std::get<T>(_variant);
  }
  template <typename T, typename... VTs>
  FIXED maybe<T> get(const std::variant<VTs...>& _variant) noexcept {
    if (!std::holds_alternative<T>(_variant))
      return unexpect("Expected variant type ", typeid(T).name(), " but got ", _variant.index());
    return std::get<T>(_variant);
  }
  template <typename T, typename... VTs>
  FIXED bool holds(const std::variant<VTs...>& _variant) noexcept {
    return std::holds_alternative<T>(_variant);
  }
  template <typename T, typename _>
  FIXED T get(const std::expected<T, _>& _expected) noexcept {
    /*
      If you are certain that the expected contains
      a value, use this to save code and have the
      safety of throwing an error if it doesn't.
    */
    if (!_expected)
      cslib_throw_up("Expected value but got error: ", _expected.error());
    return _expected.value();
  }



  template <typename To = int>
  requires std::is_arithmetic_v<To>
  FIXED maybe<To> to_number(const auto& _number) noexcept {
    /*
      Gives an additional layer of safety when
      converting numbers with different sizes.
      For example when passing the size of a
      container (size_t) to a function that
      expects an int.
    */
    static_assert(std::is_arithmetic_v<decltype(_number)>, "Passing number type invalid");
    static_assert(std::is_arithmetic_v<To>, "Returning number type invalid");
    static_assert(!std::is_same_v<decltype(_number), To>, "Conversion between same types is not necessary");
    if (_number < lowest_value_of<To>() or _number > highest_value_of<To>())
      return unexpect("Number ", _number, " is higher/lower than ", lowest_value_of<To>(), "/", highest_value_of<To>());
    return To(_number);
  }
} // namespace cslib