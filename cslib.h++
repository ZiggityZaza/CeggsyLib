// LICENSE: ☝️🤓

// Including every single header that might ever be needed
#include <initializer_list>
#include <filesystem>
#include <functional>
#include <expected>
#include <fstream>
#include <cstring>
#include <sstream>
#include <variant>
#include <utility>
#include <ranges>
#include <random>
#include <vector>
#include <chrono>
#include <thread>
#include <future>
#include <mutex>
#include <array>


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
  using str_t = std::string;
  using strv_t = std::string_view;
  using byte_t = char; /*
    Differentiate between a char as byte and
    a char as character in a string */
  template <typename T>
  using sptr = std::shared_ptr<T>;
  template <typename T>
  using uptr = std::unique_ptr<T>;
  template <typename T, typename Or = std::exception_ptr> // Contains error message
  using maybe = std::expected<T, Or>;
  namespace stdfs = std::filesystem;
  #define MACRO inline constexpr auto // Macros for macro definitions
  MACRO IS_WINDOWS = [] {
    #ifdef _WIN32
      return true;
    #else
      return false;
    #endif
  }();



  template <typename... Streamable>
  str_t to_str(Streamable&&... streamable) noexcept {
    std::ostringstream oss;
    (oss << ... << std::forward<Streamable>(streamable));
    return oss.str();
  }



  void pause(size_t ms) noexcept {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
  }



  template <typename Ex>
  std::unexpected<std::exception_ptr> unexpect(auto&&... _because) noexcept {
    return std::unexpected<std::exception_ptr>{
      std::make_exception_ptr(Ex(to_str(std::forward<decltype(_because)>(_because)...)))
    };
  }



  maybe<str_t, int> sh_call(strv_t command) noexcept {
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
    FILE* pipe = popen(command.data(), "r");
    if (!pipe)
      return std::unexpected<int>(exitCode);
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
      result += buffer.data();
    exitCode = pclose(pipe);
    if (exitCode != 0)
      return std::unexpected<int>(exitCode);
    return result;
  }



  inline constexpr bool contains(const auto& lookIn, const auto& lookFor) {
    /*
      Checks if element `_lookFor` is in `_lookIn` and
      if so, returns a ptr to the first instance of
      `_lookFor` or nullptr if not found.
      Note:
        No noexcept because we don't know how the
        equal-operator behaves
    */
    for (const auto& item : lookIn)
      if (item == lookFor)
        return true;
    return false;
  }
  inline constexpr bool have_common(const auto& cont1, const auto& cont2) {
    /*
      Checks if any element in `_cont1` is also in `_cont2`
      Note:
        No noexcept because contains() might throw
    */
    for (const auto& item : cont1)
      if (contains(cont2, item))
        return true;
    return false;
  }



  template <typename T>
  [[nodiscard]] T fake_return(strv_t errMsg = "Reached supposedly unreachable code in cslib::fake_return") {
    /*
      Keep the compiler shut up about missing return
      in a function that doesn't return or returns
      in a different way which the compiler can't
      detect.
      Note:
        This function should never be reached.
    */
    throw std::logic_error(errMsg.data());
    std::unreachable();
  }



  std::vector<int> range(int start, int end) noexcept {
    /*
      Simplified range function that takes two integers
      and returns a vector of integers (first inclusive)
    */
    std::vector<int> result;
    if (start > end) // reverse
      for (int i = start; i > end; --i)
        result.push_back(i);
    else if (start < end) // start to end
      for (int i = start; i < end; ++i)
        result.push_back(i);
    else // Nothing
      return std::vector<int>();
    return result;
  }
  std::vector<int> range(int end) noexcept {
    return range(0, end);
  }



  template <typename F, typename... Args>
  requires std::invocable<F, Args...>
  maybe<std::invoke_result_t<F, Args...>> retry(F&& f, size_t maxAttempts, Args&&... args) noexcept {
    /*
      Retry a function up to `maxAttempts` times
      Example:
        auto func = [] {
          // Do something that might fail
        };
        cslib::retry(func, 3, ...);
    */
    while (maxAttempts-- > 0) {
      try {
        if constexpr (std::is_void_v<std::invoke_result_t<F, Args...>>) {
          std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
          return {};
        }
        else
          return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
      }
      catch (...) {
        if (maxAttempts == 0)
          return std::unexpected(std::current_exception());
      }
    }
    return std::unexpected(std::current_exception());
  }



  maybe<std::vector<str_t>> parse_cli_args(int argc, const char *const args[]) noexcept {
    /*
      Parse command line arguments and return
      them as a vector of strings.
      Note:
        Make sure `_argc` and the `_args`'s size
        align even when nullptr or vector won't
        recognize values
    */
    if (args == nullptr or argc <= 0)
      return unexpect<std::invalid_argument>("No proper command line arguments provided");
    return std::vector<str_t>(args, args + argc); // Includes binary name
  }



  str_t stringify_container(const auto& vec) {
    /*
      Convert a vector to a string representation.
      Note:
        No noexcept because we don't know if something
        internally would throw
      Example:
        cslib::stringify_container({1, 2, 3}); // "{1, 2, 3}"
    */
    str_t result = "{";
    for (const auto& item : vec)
      result += to_str(item) + ", ";
    if (result.length() > 1) { // If there are items
      result.pop_back(); // Remove the last comma
      result.pop_back(); // Remove the last space
    }
    result += "}";
    return result;
  }



  MACRO TRIM_WITH = "...";
  inline constexpr maybe<str_t> rtrim(strv_t strsv, size_t maxLength) noexcept {
    /*
      Example:
        rtrim("cslib.h++", 6); // "csl..."
    */
    if (maxLength < std::strlen(TRIM_WITH))
      return unexpect<std::invalid_argument>("maxLength must be at least ", std::strlen(TRIM_WITH), " ('", TRIM_WITH, "' length)");
    if (strsv.length() <= maxLength)
      return str_t(strsv);
    return str_t(strsv.substr(0, maxLength - std::strlen(TRIM_WITH))) + TRIM_WITH;
  }

  inline constexpr maybe<str_t> ltrim(strv_t strsv, size_t maxLength) noexcept {
    /*
      Example:
        ltrim("cslib.h++", 6); // "...h++"
    */
    if (maxLength < std::strlen(TRIM_WITH))
      return unexpect<std::invalid_argument>("maxLength must be at least ", std::strlen(TRIM_WITH), " ('", TRIM_WITH, "' length)");
    if (strsv.length() <= maxLength)
      return str_t(strsv);
    return str_t(TRIM_WITH) + str_t(strsv.substr(strsv.length() - (maxLength - std::strlen(TRIM_WITH))));
  }



  std::vector<str_t> separate(strv_t strv, strv_t delimiter = "") noexcept {
    std::vector<str_t> tokens;
    for (auto&& part : strv | std::views::split(delimiter))
        tokens.emplace_back(part.begin(), part.end());
    return tokens;
  }



  int roll_dice(int min, int max) noexcept {
    /*
      Minimum and maximum value and returns a random
      number between them (inclusive).
      Example:
        cslib::roll_dice(1, 6); // Returns a random number
        between 1 and 6 (inclusive)
    */
    if (min > max)
      std::swap(min, max);
    static std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(generator);
  }



  maybe<str_t> read_data(std::istream& inStream) noexcept {
    /*
      Read all data from the given istream and return it as a string.
      After reading, the stream is considered empty.
      Throws an error if the stream is not open or in a bad state.
      Note:
        Handling encoding, other states, flags or similar are managed
        by the caller. This function only cleans up its own changes.
    */
    const std::ios::iostate oldIoEx = inStream.exceptions();
    inStream.exceptions(std::ios::badbit); // Enable exceptions for fail and bad bits
    try {
      std::streampos previousPos = inStream.tellg();
      str_t result{std::istreambuf_iterator<char>(inStream), std::istreambuf_iterator<char>()};
      if (previousPos != -1)
        inStream.seekg(previousPos);
      inStream.exceptions(oldIoEx);
      return result;
    }
    catch (...) {
      inStream.exceptions(oldIoEx);
      return std::unexpected(std::current_exception());
    }
    std::unreachable();
  }
  maybe<void> do_io(std::istream& inStream, std::ostream& outStream) noexcept {
    maybe<str_t> data = read_data(inStream);
    if (!data)
      return std::unexpected(data.error());
    const std::ios::iostate oldIoEx = inStream.exceptions();
    outStream.exceptions(std::ios::badbit);
    try {
      outStream << *data << std::flush;
      inStream.exceptions(oldIoEx);
    }
    catch (...) {
      outStream.exceptions(oldIoEx);
      return std::unexpected(std::current_exception());
    }
    return {};
  }



  class TimeStamp { public:
    /*
      A wrapper around std::chrono
    */
    std::chrono::system_clock::time_point timePoint;
    

    // Contructors and error handling
    TimeStamp() noexcept {timePoint = std::chrono::system_clock::now();}
    TimeStamp(std::chrono::system_clock::time_point tp) : timePoint(tp) {}
    TimeStamp(int hour, int min, int sec, int day, int month, int year) {
      /*
        Create a time stamp from the given date and time
        after making sure that the date is valid.
      */
      // Determine date
      std::chrono::year_month_day ymd{
        std::chrono::year(year),
        std::chrono::month(month),
        std::chrono::day(day)
      };
      if (!ymd.ok())
        throw std::logic_error(to_str("Invalid date: ", day, "-", month, "-", year));
      // Determine time
      if ((hour >= 24 or hour < 0) or (min >= 60 or min < 0) or (sec >= 60 or sec < 0))
        throw std::logic_error(to_str("Invalid time: ", hour, ":", min, ":", sec));
      std::chrono::hh_mm_ss hms{
        std::chrono::hours(hour) +
        std::chrono::minutes(min) +
        std::chrono::seconds(sec)
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
    Out(std::ostream& outTo_) : outTo(outTo_) {
      prefix = "";
    }
    Out(std::ostream& outTo_, strv_t prefsv = "", strv_t color = "") : outTo(outTo_) {
      std::ostringstream prefixStream;
      prefixStream << color << prefsv;
      if (!prefsv.empty())
        prefixStream << " ";
      if (!color.empty())
        prefixStream << Reset;
      prefix = prefixStream.str();
    }
    std::ostream& operator<<(const auto& msg) const noexcept {
      return outTo << '[' << TimeStamp().as_str() << ']' << prefix << msg;
    }
    std::ostream& operator<<(auto&& msg) const noexcept {
      return outTo << '[' << TimeStamp().as_str() << ']' << prefix << std::forward<decltype(msg)>(msg);
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



  maybe<stdfs::path> where_is_path_really(stdfs::path path) noexcept {
    /*
      Returns the actual path of the entry on disk
      but won't resolve symlinks.
    */
    try {
      if (stdfs::exists(path) and stdfs::is_symlink(path))
        return stdfs::read_symlink(path);
      return stdfs::canonical(path);
    }
    catch (...) {
      return std::unexpected(std::current_exception());
    }
  }
  MACRO PATH_SEPARATOR = IS_WINDOWS ? '\\' : '/';
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
    str_t str() const noexcept { return isAt.string(); }
    str_t name() const noexcept { return isAt.filename().string(); }
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
    maybe<Folder> operator[](size_t index) const noexcept;


    [[nodiscard]] maybe<void> rename_self_to(strv_t newName) noexcept {
      /*
        Rename the entry to a new name.
        Note:
          No path separators allowed as parent directories
          are not created
      */
      if (newName.find(PATH_SEPARATOR) != str_t::npos)
        return unexpect<std::invalid_argument>("Filename can't be moved with this function (previous: '", str(), "', new: '", newName, "')");
      const stdfs::path newPath = isAt.parent_path() / newName;
      if (newName.empty())
        return unexpect<std::invalid_argument>("Can't rename '", str(), "' to an empty name");
      if (stdfs::exists(isAt.parent_path() / newName))
        return unexpect<std::runtime_error>("File already exists: '", newPath, "'");
      try {
        stdfs::rename(isAt, newPath);
      } catch (...) {
        return std::unexpected(std::current_exception());
      }
      isAt = newPath; // Update the path
      return {};
    }

    
    bool operator==(Road other) const noexcept { return this->isAt == other.isAt; }
    bool operator!=(Road other) const noexcept { return !(*this == other); }
    bool operator==(stdfs::path other) const noexcept { return this->isAt == other; }
    bool operator!=(stdfs::path other) const noexcept { return !(*this == other); }
    operator bool() const noexcept { return stdfs::exists(isAt); } // Check if path exists
    // Implicitly converts wide (c-)strings to stdfs::path 


    // Transform into stl
    operator str_t() const noexcept { return this->str(); }
    operator stdfs::path() const noexcept { return this->isAt; }
    operator stdfs::path&() noexcept { return this->isAt; }
    operator const stdfs::path&() const noexcept { return this->isAt; }
    operator stdfs::path*() noexcept { return &this->isAt; }
    operator stdfs::path*() const noexcept { return const_cast<stdfs::path*>(&this->isAt); } // casted immutable
    friend std::ostream& operator<<(std::ostream& out, const Road& entry) noexcept {
      return out << '\'' << entry.str() << '\'';
    }


    // Abstract class shouldn't be instantiated
    protected: Road() = default;
    protected: Road(stdfs::path where) {
      /*
        Resolves the actual path for it's kids
      */
      if (where.empty())
        throw std::invalid_argument("Path empty");
      maybe<stdfs::path> at = where_is_path_really(where);
      if (!at.has_value())
        throw std::runtime_error("Couldn't resolve path of '" + where.string() + "'");
      isAt = *at;
    }

    static maybe<Road> create_self(stdfs::path where) {
      /*
        Creates a new instance of Road without
        allowing global access of constructor
        directly. Additional step to construct
        should be taken to ensure proper
        initialization
      */
      try {
        return Road(where);
      } catch (...) {
        return std::unexpected(std::current_exception());
      }
    }
  };



  class BizarreRoad : public Road { public:
    /*
      Specifically for files that don't fit into
      the usual categories such as symbolic links
      or pipes.
    */
    BizarreRoad() = default;
    BizarreRoad(stdfs::path where) : Road(where) {
      if (!stdfs::exists(isAt))
        throw std::invalid_argument("Path '" + isAt.string() + "' does not exist");
      if (this->type() == stdfs::file_type::regular or
          this->type() == stdfs::file_type::directory)
        throw std::runtime_error("Path '" + isAt.string() + "' is not a bizarre file");
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
    File(stdfs::path where, bool createIfNotExists = false) : Road([where, createIfNotExists] {
      if (createIfNotExists and !stdfs::exists(where))
        std::ofstream(where) << "";
      if (!stdfs::is_regular_file(where))
        throw std::invalid_argument("Path '" + where.string() + "' is not a regular file");
      return where;
    }()) {}


    maybe<str_t> read_text() const noexcept {
      std::ifstream file(isAt, std::ios::in);
      const std::ios_base::iostate oldIoEx = file.exceptions();
      file.exceptions(std::ios_base::failbit | std::ios_base::badbit);
      try {
        const str_t content = str_t((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.exceptions(oldIoEx);
        return content;
      } catch (...) {
        file.exceptions(oldIoEx);
        return std::unexpected(std::current_exception());
      }
    }
    maybe<void> edit_text(strv_t newText) const noexcept {
      std::ofstream file(isAt, std::ios::out | std::ios::trunc);
      const std::ios_base::iostate oldIoEx = file.exceptions();
      try {
        file << newText;
        file.exceptions(oldIoEx);
      } catch (...) {
        file.exceptions(oldIoEx);
        return std::unexpected(std::current_exception());
      }
      return {};
    }


    [[nodiscard]] maybe<std::vector<byte_t>> read_binary() const noexcept {
      /*
        Streambufs for byte-by-byte reading
        for raw data reading.
      */
      std::ifstream file(isAt, std::ios::binary);
      const std::ios_base::iostate oldIoEx = file.exceptions();
      try {
        const std::vector<byte_t> result = std::vector<byte_t> {
        std::istreambuf_iterator<byte_t>(file),
        std::istreambuf_iterator<byte_t>() };
        file.exceptions(oldIoEx);
        return result;
      } catch(...) {
        file.exceptions(oldIoEx);
        return std::unexpected(std::current_exception());
      }
    }
    [[nodiscard]] maybe<void> edit_binary(const void *const newData, size_t len) const {
      std::ofstream file(isAt, std::ios::binary | std::ios::trunc);
      const std::ios_base::iostate oldIoEx = file.exceptions();
      try {
        file.write(reinterpret_cast<const char *const>(newData), len);
        file.exceptions(oldIoEx);
      } catch(...) {
        file.exceptions(oldIoEx);
        return std::unexpected(std::current_exception());
      }
      return {};
    }


    str_t extension() const noexcept {return isAt.extension().string();}
    size_t bytes() const noexcept {return stdfs::file_size(isAt);}


    [[nodiscard]] maybe<void> move_self_into(Folder newLocation) noexcept;


    [[nodiscard]] maybe<File> copy_self_into(Folder newLocation, stdfs::copy_options options = stdfs::copy_options::none) const noexcept;
  };



  class Folder : public Road { public:
    /*
      Child class of Path that represents a folder.
      Example:
        Folder folder("/gitstuff/cslib");
        if (folder.has(Road("/gitstuff/cslib/cslib.h++")))
          std::cout << "Folder contains the file\n";
    */
    Folder() = default;
    Folder(stdfs::path where, bool createIfNotExists = false) : Road([where, createIfNotExists] {
      if (createIfNotExists and !stdfs::exists(where))
        stdfs::create_directory(where);
      if (!stdfs::is_directory(where))
        throw std::invalid_argument("Path '" + where.string() + "' is not a directory");
      return where;
    }()) {}


    stdfs::path operator/(stdfs::path other) const noexcept {
      return this->isAt / other;
    }


    std::vector<road_t> list() const noexcept {
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


    std::vector<Road> untyped_list() const noexcept {
      std::vector<Road> result;
      for (const stdfs::directory_entry& entry : stdfs::directory_iterator(isAt))
        result.emplace_back(Road::create_self(entry.path()).value());
      return result;
    }


    maybe<road_t> find(strv_t lookFor) const noexcept {
      /*
        Check if the folder contains a file or folder with
        the given relative path. If it does, return the
        corresponding Road object.
        Note:
          Path MUST be relative
        Note 2:
          Using [] operator instead of .at() to avoid exceptions
          and to allow checking for existence
      */
      if (lookFor.empty())
        return unexpect<std::invalid_argument>("Path is empty");
      if constexpr (IS_WINDOWS) {
        if (lookFor[2] == PATH_SEPARATOR and lookFor[1] == ':') // C:\ (c = 0, : = 1, \ = 2)
          return unexpect<std::invalid_argument>("Path starts with a drive letter and a path separator, thus is absolute");
      }
      else {
        if (lookFor[0] == PATH_SEPARATOR)
          return unexpect<std::invalid_argument>("Path starts with a path separator, thus is absolute");
      }
      if (stdfs::exists(isAt / lookFor))
        switch (stdfs::status(isAt / lookFor).type()) {
          case stdfs::file_type::regular: return File(isAt / lookFor);
          case stdfs::file_type::directory: return Folder(isAt / lookFor);
          default: return BizarreRoad(isAt / lookFor); // Other types (symlinks, sockets, etc.)
        }
      return unexpect<std::runtime_error>("Couldn't find '", lookFor, "' in here '", str(), "'");
    }
    maybe<Road> untyped_find(strv_t path) const noexcept {
      maybe<road_t> road = find(path);
      if (!road)
        return std::unexpected(road.error());
      // Since std::visit wont work
      if (std::holds_alternative<File>(*road))
        return Road::create_self(std::get<File>(*road).isAt).value();
      else if (std::holds_alternative<Folder>(*road))
        return Road::create_self(std::get<Folder>(*road).isAt).value();
      else if (std::holds_alternative<BizarreRoad>(*road))
        return Road::create_self(std::get<BizarreRoad>(*road).isAt).value();
      return fake_return<maybe<Road>>();
    }

    maybe<Road> untyped_find(Road path) const noexcept {
      maybe<Road> result = untyped_find(path.name());
      if (!result)
        return std::unexpected(result.error());
      if (path.type() != result.value().type())
        return unexpect<std::runtime_error>("'", path.name(), "' exists in '", this->str(), "' but they are of different types");
      return result;
    }
    maybe<road_t> find(Road path) const noexcept {
      maybe<Road> result = untyped_find(path);
      if (!result)
        return std::unexpected(result.error());
      switch (result.value().type()) {
        case stdfs::file_type::regular: return File(*result);
        case stdfs::file_type::directory: return Folder(*result);
        default: return BizarreRoad(*result);
      }
    }


    [[nodiscard]] maybe<void> move_self_into(Folder parentDict) noexcept {
      /*
        Important note:
          Upon moving, subfolder and file objects will still
          point to the old location.
      */
      if (stdfs::exists(parentDict / name()))
        return unexpect<std::invalid_argument>("Path ", isAt, " already exists in folder ", parentDict);
      stdfs::rename(isAt, parentDict / name());
      isAt = parentDict / name();
      return {};
    }


    [[nodiscard]] maybe<Folder> copy_self_into(Folder parentDict) const noexcept {
      if (stdfs::exists(parentDict / name()))
        return unexpect<std::invalid_argument>("Path '", str(), "' already exists in folder '", parentDict.str(), "'");
      stdfs::copy(isAt, parentDict / name(), stdfs::copy_options::recursive);
      return Folder(parentDict / name());
    }


    [[nodiscard]] maybe<void> copy_content_into(Folder otherDir, stdfs::copy_options options) const noexcept {
      /*
        Copying self with custom options for extra
        control
      */
      try {
        stdfs::copy(isAt, otherDir, options);
      } catch (...) {
        return std::unexpected(std::current_exception());
      }
      return {};
    }
  };



  // Implementations for Road stuff due to constructors not being defined yet
  maybe<Folder> Road::operator[](size_t index) const noexcept {
    /*
      Find parent paths by layer
      Example:
        Folder f("/root/projects/folder");
        Folder root = f[0];
        Folder parent = f[f.depth()-1];
    */
    stdfs::path parent = isAt;
    if (index >= depth())
      return unexpect<std::invalid_argument>("Trying to reach (", index, ") fruther than path '", str(), "' would allow (", depth(), ")");
    for ([[maybe_unused]] size_t _ : range(depth() - index))
      parent = parent.parent_path();
    return Folder(parent);
  }
  [[nodiscard]] maybe<void> File::move_self_into(Folder newLocation) noexcept {
    if (stdfs::exists(newLocation / name()))
      return unexpect<std::invalid_argument>("File '", name(), "' already exists in folder ", newLocation.str());
    stdfs::rename(isAt, newLocation / name());
    isAt = newLocation / name(); // Update the path
    return {};
  }
  [[nodiscard]] maybe<File> File::copy_self_into(Folder newLocation, stdfs::copy_options options) const noexcept {
    /*
      Copying self with custom options for extra
      control
    */
    stdfs::copy_file(isAt, newLocation / name(), options);
    if (!stdfs::exists(newLocation / name()))
      return unexpect<std::runtime_error>("Failed to copy file to '", (newLocation / name()).string(), "'");
    return File(newLocation / name());
  }



  MACRO SCRAMBLE_LEN = 5ULL; // 59^n possible combinations
  static_assert(SCRAMBLE_LEN > 0, "SCRAMBLE_LEN must be greater than 0 so that temporary filenames can be generated");
  str_t scramble_filename(size_t len = SCRAMBLE_LEN) noexcept {
    /*
      Generate a random filename with a length of `SCRAMBLE_LEN`
      characters. The filename consists of uppercase and lowercase
      letters and digits. If `len` is specified, make sure it's
      above 0
      Note:
        If all possible names are exhausted, the calling code
        will be stuck in an infinite loop.
      Example:
        A possible output could be "aB3cX..."
    */
    std::ostringstream randomName;
    for ([[maybe_unused]] auto _ : range(len))
      switch (roll_dice(0, 2)) {
        case 0: randomName << char(roll_dice('A', 'Z')); break; // Uppercase letter
        case 1: randomName << char(roll_dice('a', 'z')); break; // Lowercase letter
        case 2: randomName << char(roll_dice('0', '9')); break; // Digit
        default: randomName << fake_return<decltype(_)>();
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



  std::expected<str_t, int> get(str_t url, str_t params = "-fsSL") noexcept {
    /*
      Download the content of the given URL using wget
      and return it as a string. If the download fails,
      an error code will be returned.
      Note:
        This function requires curl to be installed
      Important note:
        Just like the sh_call function itself, this
        function is VERY injection prone. No direct
        user input recommended
    */
    return sh_call("curl " + params + " " + url); 
  }



  template <typename T>
  requires (std::is_integral_v<T> or std::is_floating_point_v<T>)
  inline constexpr T highest_value_of() noexcept {
    /*
      Get the highest possible value that
      the type T can represent.
    */
    return std::numeric_limits<T>::max();
  }
  template <typename T>
  requires (std::is_integral_v<T> or std::is_floating_point_v<T>)
  inline constexpr T lowest_value_of() noexcept {
    /*
      Get the lowest possible value that
      the type T can represent.
    */
    return std::numeric_limits<T>::lowest();
  }



  /*
    'I know what I am doing' functions to avoid repetitive
    error handling code with safety-nets
  */
  template <typename T, typename... VTs>
  inline constexpr const T get(const std::variant<VTs...>& variant) {
    if (!std::holds_alternative<T>(variant))
      throw std::invalid_argument("Expected variant type (id) ", typeid(T).name(), " but got (index) ", variant.index());
    return std::get<T>(variant);
  }
  template <typename T, typename... VTs>
  inline constexpr T get(const std::variant<VTs...>& variant) {
    if (!std::holds_alternative<T>(variant))
      throw std::invalid_argument("Expected variant type (id) ", typeid(T).name(), " but got (index) ", variant.index());
    return std::get<T>(variant);
  }
  template <typename T, typename... VTs>
  inline constexpr bool holds(const std::variant<VTs...>& variant) noexcept {
    return std::holds_alternative<T>(variant);
  }
  template <typename T, typename _>
  inline constexpr T get(const std::expected<T, _>& expected) noexcept {
    /*
      If you are certain that the expected contains
      a value, use this to save code and have the
      safety of throwing an error if it doesn't.
    */
    if (!expected)
      throw std::invalid_argument("Expected value but got error");
    return expected.value();
  }



  template <typename To = int, typename From>
  requires (std::is_arithmetic_v<From> and std::is_arithmetic_v<To>)
  inline constexpr maybe<To> to_number(const From& number) noexcept {
    /*
      Gives an additional layer of safety when
      converting numbers with different sizes.
      For example when passing the size of a
      container (size_t) to a function that
      expects an int.
    */
    static_assert(!std::is_same_v<From, To>, "Conversion between same types is not necessary");
    if (number < lowest_value_of<To>() or number > highest_value_of<To>())
      return unexpect("Number ", number, " is higher/lower than ", lowest_value_of<To>(), "/", highest_value_of<To>());
    return To(number);
  }



  template <typename F, typename... Args>
  requires (std::is_invocable_v<F, Args...>)
  [[nodiscard]] std::future<std::invoke_result_t<F, Args...>> do_in_parallel(F&& f, Args&&... args) {
    /*
      Note:
        If the result is discarded or not stored in a l
        variable, the calling line in thread will block
        until do_in_parallel is done. This is the exact
        opposite of async calls.
        Bad Example:
          do_in_parallel([] {while (true) {}} // std::future wants to destruct right here
          // Never reached because std::future can't destruct while async function is running
        Good example:
          {
            [[maybe_unused]] std::future<void> _ = do_in_parallel(find_and_delete_file, "\**", "badfile.txt");
            std::cout << "While we wait, tell me more about yourself!"
            // ...
          } // Only when going out of scope, destructor of _ might block if function isn't done
    */
    return std::async(std::launch::async, std::forward<F>(f), std::forward<Args>(args)...);
  }



  template <typename T>
  inline constexpr bool between(T&& val, T&& atLeast, T&& atMost) noexcept {
    // Inclusive to allow 0 in unsigned types
    if (atLeast > atMost)
      std::swap(atLeast, atMost);
    return val >= atLeast and val <= atMost;
  }
} // namespace cslib