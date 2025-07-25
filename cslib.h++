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
  using wstr_t = std::wstring;
  using str_t = std::string;
  using wstrv_t = std::wstring_view;
  using strv_t = std::string_view;
  #define SHARED inline // Alias inline for shared functions, etc.
  #define MACRO inline constexpr auto // Macros for macro definitions
  #define FIXED inline constexpr // Explicit alternative for MACRO


  // Defined beforehand to avoid circular dependencies
  MACRO to_str(wchar_t _wchar) { return std::string(1, static_cast<char>(_wchar)); }
  MACRO to_str(const wchar_t *const _cwstr) {
    size_t len = 0;
    while (_cwstr[len] != 0) // No constexpr for std::wclen
      ++len;
    return std::string(_cwstr, _cwstr + len);
  }
  MACRO to_str(const wstr_t& _wstr) {return std::string(_wstr.begin(), _wstr.end());}
  MACRO to_str(wstrv_t _wstrv) {return std::string(_wstrv.begin(), _wstrv.end());}
  str_t to_str(const auto& _anything) {
    std::ostringstream oss;
    oss << _anything;
    return oss.str();
  }

  MACRO to_wstr(char _char) { return std::wstring(1, static_cast<wchar_t>(_char)); }
  MACRO to_wstr(const char *const _cstr) {return std::wstring(_cstr, _cstr + std::strlen(_cstr));}
  MACRO to_wstr(const str_t& _str) {return std::wstring(_str.begin(), _str.end());}
  MACRO to_wstr(const strv_t& _strv) {return std::wstring(_strv.begin(), _strv.end());}
  wstr_t to_wstr(const auto& _anything) {
    std::wostringstream woss;
    woss << _anything;
    return woss.str();
  }



  template <typename... _args>
  std::runtime_error up_impl(size_t _lineInCode, _args&&... _msgs) {
    /*
      Create a custom runtime error with the given messages.
      Example:
        #define up(...) up_impl(__LINE__, __VA_ARGS__)
        if (1 == 2)
          throw up("Aye", L"yo", '!', 123);
    */
    std::ostringstream oss;
    oss << "\033[1m" << "\033[31m" << "Error: ";
    ((oss << to_str(std::forward<_args>(_msgs))), ...);
    oss << "\033[0m" << std::flush;
    std::filesystem::path currentPath = std::filesystem::current_path();
    return std::runtime_error("std::runtime_error called from line " + std::to_string(_lineInCode) + " in workspace '" + currentPath.string() + "' because: " + oss.str());
  }
  #define throw_up(...) throw cslib::up_impl(__LINE__, __VA_ARGS__)



  void pause(size_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
  }



  MACRO wstrlen(wstrv_t _wstrv) {
    return _wstrv.length();
  }



  MACRO upper_chr(wchar_t _wchar) {
    return (_wchar >= L'a' and _wchar <= L'z') ? (_wchar - (L'a' - L'A')) : _wchar;
  }
  MACRO upper_str(wstrv_t _wstrv) {
    std::wstring result(_wstrv.data());
    for (wchar_t& c : result)
      c = upper_chr(c);
    return result;
  }
  MACRO lower_chr(wchar_t _wchar) {
    return (_wchar >= L'A' and _wchar <= L'Z') ? (_wchar + (L'a' - L'A')) : _wchar;
  }
  MACRO lower_str(wstrv_t _wstrv) {
    std::wstring result(_wstrv.data());
    for (wchar_t& c : result)
      c = lower_chr(c);
    return result;
  }



  void sh_call(strv_t _command) {
    /*
      Blocking system call
    */
    if (system(_command.data()) != 0)  
      throw_up("Failed to execute command: '", _command, "'");
  }



  void clear_console() {
    #ifdef _WIN32
      sh_call("cls");
    #else
      sh_call("clear");
    #endif
  }



  MACRO contains(const auto& _lookIn, const auto& _lookFor) {
    /*
      does `container` contain `key`
    */
    return std::find(_lookIn.begin(), _lookIn.end(), _lookFor) != _lookIn.end();
  }
  MACRO have_something_common(const auto& _cont1, const auto& _cont2) {
    /*
      do `c1` and `c2` contain similar keys
    */
    for (const auto& item : _cont1)
      if (contains(_cont2, item))
        return true;
    return false;
  }



  MACRO recrusive_lookup(const auto& _lookIn, const auto& _lookFor) {
    /*
      Check if `container` contains `key` or if it contains
      a container that contains `key`
    */
    for (const auto& item : _lookIn) {
      if (item == _lookFor)
        return true;
      if constexpr (std::is_same_v<decltype(item), decltype(_lookIn)>)
        if (recrusive_lookup(item, _lookFor))
          return true;
    }
    return false;
  }



  str_t get_env(strv_t _var) {
    /*
      Get the value of an environment variable.
    */
    const char *const envCStr = getenv(_var.data());
    if (envCStr == NULL)
      throw_up("Environment variable '", _var, "' not found");
    return str_t(envCStr);
  }



  template <typename T>
  requires std::is_integral_v<T>
  std::vector<T> range(const T& _start, const T& _end) {
    /*
      Simplified range function that takes two integers
      and returns a vector of integers (inclusive)
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
  std::vector<T> range(const T& end) {
    return range(T(0), end);
  }



  auto retry(const auto& _func, size_t _maxAttempts, size_t _waitTimeMs) {
    /*
      Retry a function up to `retries` times with a delay
      of `delay` milliseconds between each retry.
      Example:
        std::function<void()> func = []() {
          // Do something that might fail
        };
        cslib::retry(func, 3);
    */
    while (_maxAttempts-- > 0) {
      try {
        static_assert(std::is_invocable_v<decltype(_func)>, "Function must be invocable");
        return _func();
      }
      catch (const std::runtime_error& e) {
        if (_maxAttempts == 0)
          throw_up("Function failed after maximum attempts: ", e.what());
        std::this_thread::sleep_for(std::chrono::milliseconds(_waitTimeMs)); // Wait before retrying
      }
    }
    throw_up("Function failed after maximum attempts");
  }



  std::vector<strv_t> parse_cli_args(int _argc, const char *const _args[]) {
    /*
      Parse command line arguments and return them as a
      vector of strings.
      Note:
        The first argument is the program name, so we skip it
    */
    if (_args == nullptr or _argc <= 0)
      throw_up("No command line arguments provided");
    std::vector<strv_t> args(_args, _args + _argc); // Includes binary name
    args.erase(args.begin()); // Remove the first argument (program name)
    return args;
  }



  FIXED wstrv_t TRIM_WITH = L"...";
  MACRO shorten_end(wstrv_t _wstrsv, size_t _maxLength) {
    /*
      Example:
        cslib::shorten_end(L"cslib.h++", 6); // "csl..."
    */
    if (_maxLength < TRIM_WITH.length())
      throw_up("maxLength must be at least ", TRIM_WITH.length(), " (TRIM_WITH length)");
    if (_wstrsv.length() <= _maxLength)
      return wstr_t(_wstrsv);
    return wstr_t(_wstrsv.substr(0, _maxLength - TRIM_WITH.length())) + TRIM_WITH.data();
  }

  MACRO shorten_begin(wstrv_t _wstrsv, size_t _maxLength) {
    /*
      Example:
        cslib::shorten_begin(L"cslib.h++", 6); // "...h++"
    */
    if (_maxLength < TRIM_WITH.length())
      throw_up("maxLength must be at least ", TRIM_WITH.length(), " (TRIM_WITH length)");
    if (_wstrsv.length() <= _maxLength)
      return wstr_t(_wstrsv);
    return wstr_t(TRIM_WITH) + wstr_t(_wstrsv.substr(_wstrsv.length() - (_maxLength - TRIM_WITH.length())));
  }



  std::vector<wstr_t> separate(wstrv_t _wstrsv, wchar_t _delimiter) {
    /*
      Example:
        cslib::separate(L"John Money", ' ') // {"John", "Money"}
    */
    wstr_t str(_wstrsv);
    std::vector<wstr_t> result;
    wstr_t temp;
    if (str.empty() or _delimiter == L'\0')
      return result;
    for (wchar_t c : str) {
      if (c == _delimiter)
        result.push_back(std::move(temp));
      else
        temp += c;
    }
    result.push_back(temp);
    return result;
  }



  template <typename T>
  requires std::is_integral_v<T>
  T roll_dice(T _min, T _max) {
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



  wstr_t read_wdata(std::wistream& _winStream) {
    /*
      Read all data from the given wistream and return it as a wstring.
      After reading, the stream is considered empty.
      Throws an error if the stream is not open or in a bad state.
      Note:
        Handling encoding, other states, flags or similar are managed
        by the caller. This function only cleans up its own changes.
    */
    if (!_winStream or !_winStream.good())
      throw_up("std::wistream is not good or in a bad state");
    const std::wstreampos previousPos = _winStream.tellg();
    std::wstring result{std::istreambuf_iterator<wchar_t>(_winStream), std::istreambuf_iterator<wchar_t>()};
    _winStream.seekg(previousPos);
    return result;
  }
  void do_io(std::wistream& _winStream, std::wostream& _woutStream) {
    _woutStream << read_wdata(_winStream) << std::flush;
  }



  // Get the first available UTF-8 locale
  FIXED std::array<strv_t, 6> POSSIBLE_UTF8_LOCALES = {
    "en_US.UTF-8",
    "en_US.utf8",
    "C.UTF-8",
    "POSIX.UTF-8",
    "C.utf8",
    "POSIX.utf8"
  };
  SHARED bool isWcharIOEnabled = false;
  void enable_wchar_io() {
    /*
      Set all io-streaming globally to UTF-8 encoding
    */

    if (isWcharIOEnabled) {
      std::wcout << L"[🧌] Console already initialized with UTF-8 encoding.\n";
      return;
    }

    std::locale utf8Locale;
    for (strv_t locale_name : POSSIBLE_UTF8_LOCALES) {
      try {
        utf8Locale = std::locale(locale_name.data());
      }
      catch (const std::runtime_error&) {}
      // Ignore the exception, try the next locale
    }
    if (utf8Locale.name().empty())
      throw_up("Failed to find a suitable UTF-8 locale. Ensure your system supports UTF-8 locales");

    std::locale::global(utf8Locale);
    std::wcout.imbue(utf8Locale);
    std::wcin.imbue(utf8Locale);
    std::wclog.imbue(utf8Locale);
    std::wcerr.imbue(utf8Locale);

    std::wcout << L"[🧌] Console initialized with UTF-8 encoding.\n";
    isWcharIOEnabled = true;
  }



  MACRO Bold = L"\033[1m";
  MACRO Underline = L"\033[4m";
  MACRO Italic = L"\033[3m";
  MACRO Reverse = L"\033[7m";
  MACRO Hidden = L"\033[8m";
  MACRO Black = L"\033[30m";
  MACRO Red = L"\033[31m";
  MACRO Green = L"\033[32m";
  MACRO Yellow = L"\033[33m";
  MACRO Blue = L"\033[34m";
  MACRO Magenta = L"\033[35m";
  MACRO Cyan = L"\033[36m";
  MACRO White = L"\033[37m";
  MACRO Reset = L"\033[0m";
  class Out { public:
    /*
      Print to console with color and optionally into an existing file.
      Usage:
        cslib::Out error("Error: ", cslib::Out::Color::RED);
        error << "Something went wrong";
    */
    std::wostream& outTo;
    wstr_t prefix;
    Out() = default;
    Out(std::wostream& _outTo) : outTo(_outTo) {
      if (!isWcharIOEnabled)
        enable_wchar_io();
      prefix = L"";
    }
    Out(std::wostream& _outTo, wstrv_t _prefsv = L"", wstrv_t _color = L"") : outTo(_outTo) {
      std::wostringstream prefixStream;
      prefixStream << _color << _prefsv;
      if (!_prefsv.empty())
        prefixStream << " ";
      if (!_color.empty())
        prefixStream << Reset;
      prefix = prefixStream.str();
    }
    std::wostream& operator<<(const auto& _msg) {
      outTo << prefix << _msg;
      return outTo;
    }
    std::wostream& operator<<(auto&& _msg) {
      outTo << prefix << std::forward<decltype(_msg)>(_msg);
      return outTo;
    }
  };



  class TimeStamp { public:
    /*
      A wrapper around std::chrono
    */
    std::chrono::system_clock::time_point timePoint;
    

    // Contructors and error handling
    TimeStamp() {timePoint = std::chrono::system_clock::now();}
    TimeStamp(std::chrono::system_clock::time_point _tp) : timePoint(_tp) {}
    TimeStamp(int _sec, int _min, int _hour, int _day, int _month, int _year) {
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
        throw_up("Invalid date: ", _day, "-", _month, "-", _year);
      // Determine time
      if (_hour >= 24 or _min >= 60 or _sec >= 60)
        throw_up("Invalid time: ", _hour, ":", _min, ":", _sec);
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

    wstr_t as_wstr() const {
      /*
        Convert the time point to (almost) ISO 8601
        in format HH:MM:SS DD-MM-YYYY)..
      */
      std::time_t time = std::chrono::system_clock::to_time_t(timePoint);
      return (std::wstringstream() << std::put_time(std::gmtime(&time), L"%H:%M:%S %d-%m-%Y")).str();
    }
    size_t year() const {
      auto ymd = std::chrono::year_month_day(std::chrono::floor<std::chrono::days>(timePoint));
      return size_t(int(ymd.year()));
    }
    size_t month() const {
      auto ymd = std::chrono::year_month_day(std::chrono::floor<std::chrono::days>(timePoint));
      return size_t(unsigned(ymd.month()));
    }
    size_t day() const {
      auto ymd = std::chrono::year_month_day(std::chrono::floor<std::chrono::days>(timePoint));
      return size_t(unsigned(ymd.day()));
    }
    size_t hour() const {
      auto day_point = std::chrono::floor<std::chrono::days>(timePoint);
      auto time_since_midnight = timePoint - day_point;
      auto hms = std::chrono::hh_mm_ss(time_since_midnight);
      return size_t(hms.hours().count());
    }
    size_t minute() const {
      auto day_point = std::chrono::floor<std::chrono::days>(timePoint);
      auto time_since_midnight = timePoint - day_point;
      auto hms = std::chrono::hh_mm_ss(time_since_midnight);
      return size_t(hms.minutes().count());
    }
    size_t second() const {
      auto day_point = std::chrono::floor<std::chrono::days>(timePoint);
      auto time_since_midnight = timePoint - day_point;
      auto hms = std::chrono::hh_mm_ss(time_since_midnight);
      return size_t(hms.seconds().count());
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
    void reset() {
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



  class EntryRoute { public:
    /*
      Represents a file or a folder in the filesystem.
      Provides methods to manage paths, check types, and
      retrieve metadata like last modified time.
    */
    std::filesystem::path isAt; // Covers move and copy semantics


    // Path management
    wstr_t wstr() const { return isAt.wstring(); }
    std::filesystem::file_type type() const {
      return std::filesystem::status(isAt).type();
    }
    std::chrono::system_clock::time_point last_modified() const {
      auto ftime = std::filesystem::last_write_time(isAt);
      return std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
    }
    EntryRoute parent() const {
      return EntryRoute(isAt.parent_path(), std::filesystem::file_type::directory);
    }
    size_t depth() const {
      /*
        Example:
          RouteToFile path("/gitstuff/cslib/cslib.h++");
          size_t depth = path.depth();
          // depth = 3 (because there are 3 directories before the file)
      */
      return separate(isAt.wstring(), std::filesystem::path::preferred_separator).size() - 1;
    }

    // Disk operations
    void delete_self() {
      if (!std::filesystem::exists(isAt))
        throw_up("Path ", isAt, " does not exist");
      if (type() == std::filesystem::file_type::directory)
        std::filesystem::remove_all(isAt);
      else
        std::filesystem::remove(isAt);
      if (std::filesystem::exists(isAt))
        throw_up("Failed to remove path ", isAt);
      isAt.clear();
    }
    void move_self_into(const EntryRoute& _newLocation) {
      if (std::filesystem::exists(_newLocation.isAt / isAt.filename()))
        throw_up("Path ", isAt, " already exists in folder ", _newLocation.isAt);
      if (_newLocation.type() != std::filesystem::file_type::directory)
        throw_up("New location ", _newLocation.isAt, " is not a directory");
      std::filesystem::rename(isAt, _newLocation.isAt / isAt.filename());
      isAt = _newLocation.isAt / isAt.filename(); // Update the path
    }
    EntryRoute copy_self_into(const EntryRoute& _newLocation, std::filesystem::copy_options _options = std::filesystem::copy_options::recursive) const {
      /*
        Copy the file or folder to a new location and return a new EntryRoute object.
        Note:
          - The new location must not already contain a file or folder with the same name.
      */
      if (std::filesystem::exists(_newLocation.isAt / isAt.filename()))
        throw_up("Path ", isAt, " already exists in folder ", _newLocation.isAt);
      if (_newLocation.type() != std::filesystem::file_type::directory)
        throw_up("New location ", _newLocation.isAt, " is not a directory");
      std::filesystem::copy(isAt, _newLocation.isAt / isAt.filename(), _options);
      return EntryRoute(_newLocation.isAt / isAt.filename());
    }


    bool operator==(const EntryRoute& _other) const { return this->isAt == _other.isAt; }
    bool operator!=(const EntryRoute& _other) const { return !(*this == _other); }
    bool operator==(const std::filesystem::path& _other) const { return this->isAt == _other; }
    bool operator!=(const std::filesystem::path& _other) const { return !(*this == _other); }
    // Implicitly converts wide (c-)strings to std::filesystem::path 

    // Transform into stl
    operator wstr_t() const { return this->isAt.wstring(); }
    operator std::filesystem::path() const { return this->isAt; }
    operator std::filesystem::path&() { return this->isAt; }
    operator const std::filesystem::path&() const { return this->isAt; }
    operator std::filesystem::path*() { return &this->isAt; }
    operator std::filesystem::path*() const { return const_cast<std::filesystem::path*>(&this->isAt); } // const_cast is safe here


    EntryRoute() = default;
    EntryRoute(const std::filesystem::path& _fsp) : isAt(std::filesystem::canonical(_fsp)) {}
    EntryRoute(const std::filesystem::path& _fsp, const std::filesystem::file_type _shouldBe) : EntryRoute(_fsp) {
      if (this->type() != _shouldBe)
        throw_up("Path ", isAt, " initialized with unexpected file type");
    }
  };



  class Folder : public EntryRoute { public:
    /*
      Child class of RouteToFile that represents a folder and
      everything in it.
    */
    std::vector<EntryRoute> content;

    void update() {
      content.clear();
      for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(isAt))
        content.push_back(EntryRoute(entry.path()));
      content.shrink_to_fit();
    }

    Folder() = default;
    Folder(const std::filesystem::path& _where, bool _createIfNotExists = false) {
      if (_createIfNotExists and !std::filesystem::exists(isAt))
        std::filesystem::create_directory(isAt);
      update();
    }

    bool has(const EntryRoute& _item) const {
      /*
        Check if the folder contains the given item.
        Example:
          Folder folder("/gitstuff/cslib");
          RouteToFile item("/gitstuff/cslib/cslib.h++");
          bool exists = folder.has(item);
          // exists = true
      */
      return contains(content, _item);
    }
  };



  class File : public EntryRoute { public:
    /*
      Child class of RouteToFile that represents a file.
      Example:
        File file("/gitstuff/cslib/cslib.h++");
        str_t content = file.content();
        // content = "Around 50 years ago, a group of people..."
    */

    File() = default;
    File(const std::filesystem::path& _where, bool _createIfNotExists = false) {
      if (_createIfNotExists and !std::filesystem::exists(isAt)) {
        std::wofstream file(isAt);
        if (!file.is_open())
          throw_up("Failed to create file ", isAt);
      }
      isAt = std::filesystem::canonical(_where);
    }

    wstr_t content(std::ios_base::openmode _openMode = std::ios::in) const {
      /*
        Read the content of the file and return it as a string.
        Note:
          - No error-handling for files larger than available memory
      */
      std::wifstream file(isAt, _openMode);
      if (!file.is_open() or !file.good())
        throw_up("Failed to read file ", isAt);
      return wstr_t((std::istreambuf_iterator<wchar_t>(file)), std::istreambuf_iterator<wchar_t>());
    }
    void edit(const auto& _newContent, std::ios_base::openmode _openMode = std::ios::out) const {
      /*
        Write the given content to the file.
        Note:
          - No error-handling for files larger than available memory
      */
      std::wofstream file(isAt, _openMode);
      if (!file.is_open() or !file.good())
        throw_up("Failed to access file ", isAt);
      file << _newContent;
    }
    wstr_t extension() const {return isAt.extension().wstring();}
    size_t bytes() const {return std::filesystem::file_size(isAt);}
  };



  class TempFile : public File { public:
    /*
      Create a temporary file with a random name in the system's
      temporary directory. The name is generated by rolling dice
      to create a random string of letters and digits.
      Note:
        Using a lambda function to ensure constness of the file
        and to avoid possibly pointing to a file that shouldn't
        be deleted.
    */
    bool keep = false;

    TempFile() {
      wstr_t randomName;
      for ([[maybe_unused]] auto _ : range(64))
        switch (roll_dice(0, 2)) {
          case 0: randomName += wchar_t(roll_dice('A', 'Z')); break; // Uppercase letter
          case 1: randomName += wchar_t(roll_dice('a', 'z')); break; // Lowercase letter
          case 2: randomName += wchar_t(roll_dice('0', '9')); break; // Digit
        }
      wstr_t tempFileName = L"cslibTempFile_" + randomName + L".tmp";
      isAt = File(std::filesystem::temp_directory_path() / tempFileName, true).isAt; // Create the file
    }
    TempFile(const std::filesystem::path& _createAs) {
      if (std::filesystem::exists(_createAs))
        throw_up("File ", _createAs.filename(), " already exists in folder ", _createAs.parent_path());
      isAt = File(_createAs, true).isAt; // Create the file
    }
    ~TempFile() {
      if (std::filesystem::exists(isAt) and !keep)
        std::filesystem::remove(isAt);
    }
  };
  TempFile& make_runtime_file() {
    /*
      Return a reference to a temporary file that is created
      at runtime. The file is created in a static deque to
      ensure that it is not deleted until the program exits.
      Note:
        This function is not thread-safe and should only be
        called from the main thread.
    */
    static std::deque<TempFile> runtimeFiles;
    runtimeFiles.emplace_back();
    return runtimeFiles.back();
  }



  class TempFolder : public Folder { public:
    /*
      Create a temporary folder with a random name in the system's
      temporary directory. The name is generated by rolling dice
      to create a random string of letters and digits.
    */
    bool keep = false;

    TempFolder() {
      wstr_t randomName;
      for ([[maybe_unused]] auto _ : range(64))
        switch (roll_dice(0, 2)) {
          case 0: randomName += wchar_t(roll_dice('A', 'Z')); break; // Uppercase letter
          case 1: randomName += wchar_t(roll_dice('a', 'z')); break; // Lowercase letter
          case 2: randomName += wchar_t(roll_dice('0', '9')); break; // Digit
        }
      wstr_t tempFolderName = L"cslibTempFolder_" + randomName;
      isAt = Folder(std::filesystem::temp_directory_path() / tempFolderName, true).isAt;
    }
    TempFolder(const std::filesystem::path& _createAs) {
      /*
        Create a temporary folder in the given path.
        Note:
          If the folder already exists, it will throw an error.
      */
      if (std::filesystem::exists(_createAs))
        throw_up("Folder ", _createAs.filename(), " already exists in folder ", _createAs.parent_path());
      isAt = Folder(_createAs, true).isAt;
    }
    ~TempFolder() {
      if (std::filesystem::exists(isAt) and !keep)
        std::filesystem::remove_all(isAt);
    }
  };
  Folder& make_runtime_folder() {
    /*
      Return a reference to a temporary folder that is created
      at runtime. The folder is created in a static deque to
      ensure that it is not deleted until the program exits.
      Note:
        This function is not thread-safe and should only be
        called from the main thread.
    */
    static std::deque<TempFolder> runtimeFolders;
    runtimeFolders.emplace_back();
    return runtimeFolders.back();
  }
} // namespace cslib