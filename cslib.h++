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
#include <fstream>
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


#if __cplusplus < 202002L
  #error "Requires C++ >= 20"
#endif




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
  MACRO to_str(const wchar_t *const _cwstr) {
    size_t len = 0;
    while (_cwstr[len] != 0)
      ++len;
    return std::string(_cwstr, _cwstr + len);
  }
  MACRO to_str(const wstr_t& _wstr) {return std::string(_wstr.begin(), _wstr.end());}
  MACRO to_str(const wstrv_t& _wstrv) {return std::string(_wstrv.begin(), _wstrv.end());}
  MACRO to_str(const auto& _anything) {
    std::ostringstream oss;
    oss << _anything;
    return oss.str();
  }

  MACRO to_wstr(const char *const _cstr) {return std::wstring(_cstr, _cstr + std::strlen(_cstr));}
  MACRO to_wstr(const str_t& _str) {return std::wstring(_str.begin(), _str.end());}
  MACRO to_wstr(const strv_t& _strv) {return std::wstring(_strv.begin(), _strv.end());}
  MACRO to_wstr(const auto& _anything) {
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
          throw up("Hello", "World", 123, L"Wide string");
    */
    wstr_t message;
    ((message += to_wstr(std::forward<_args>(_msgs))), ...);
    std::wcout << L"\033[1m" << L"\033[31m" << L"Error: " << message << L"\033[0m" << std::endl;
    std::filesystem::path currentPath = std::filesystem::current_path();
    return std::runtime_error("std::runtime_error called from line " + std::to_string(_lineInCode) + " in workspace '" + currentPath.string() + "'");
  }
  #define throw_up(...) throw up_impl(__LINE__, __VA_ARGS__)



  // Pillar namespace
  namespace TinySTL {
    // The stl is good but sometimes you need something smaller

    template <size_t N>
    class String { public:
      /*
        Static fixed size string. It can hold up to `N` characters and is
        only null-terminated when cslib::String's capacity isn't maxed.
        If it is, the size-limit acts as a terminator.
      */
      static_assert(N > 0, "String size must be greater than 0");
      wchar_t data[N] = {0};


      // Access functions
      constexpr size_t length() const {
        /*
          Returns the length of the string.
          Note:
            This is not the capacity, but the actual length of the string.
        */
        size_t len = 0;
        while (data[len] != 0 and len < N)
          ++len;
        return len;
      }
      constexpr wchar_t* begin() { return data; }
      constexpr wchar_t* end() { return data + length(); }
      constexpr const wchar_t* begin() const { return data; }
      constexpr const wchar_t* end() const { return data + length(); }


      // String management
      constexpr void append(wchar_t _c) {
        size_t len = length();
        if (len >= N)
          throw_up("Capacity exceeded: ", N);
        data[len] = _c;
        if (len + 1 < N)
          data[len + 1] = 0;
      }

      constexpr void append(wstrv_t _wstrv) {
        size_t currentPos = length();
        if (currentPos + _wstrv.length() >= N)
          throw_up("Appending string exceeds capacity (current: ", currentPos, ", appending: ", _wstrv.length(), ")");
        for (wchar_t c : _wstrv)
          data[currentPos++] = c;
      }

      constexpr void wipe_clean() {
        for (size_t i = 0; i < N; ++i)
          data[i] = 0;
      }

      constexpr wchar_t at(size_t _i) {
        if (_i >= N)
          throw_up("Index out of bounds: ", _i, " >= ", N);
        return data[_i];
      }

      constexpr bool operator==(const String& _other) const {
        if (length() != _other.length())
          return false;
        for (size_t i = 0; i < length(); ++i)
          if (data[i] != _other.data[i])
            return false;
        return true;
      }
      constexpr bool operator!=(const String& _other) const {
        return !(*this == _other);
      }
      constexpr bool operator==(wstrv_t _wstrv) const {
        if (length() != _wstrv.length())
          return false;
        size_t index = -1;
        for (wchar_t c : _wstrv)
          if (data[++index] != c)
            return false;
        return true;
      }


      // Transform into stl (lossy conversion for byte differences)
      constexpr operator str_t() const { return str_t(data, data + length()); }
      constexpr operator str_t&&() { return std::move(str_t(*this)); }
      constexpr operator wstr_t() const { return std::wstring(data, data + length()); }
      constexpr operator wstr_t&&() { return std::move(std::wstring(*this)); }
      constexpr friend std::ostream& operator<<(std::ostream& _os, const String& _ourSelf) {
        return _os << str_t(_ourSelf.begin(), _ourSelf.end());
      }
      constexpr friend std::wostream& operator<<(std::wostream& _wos, const String& _ourSelf) {
        return _wos << wstr_t(_ourSelf.begin(), _ourSelf.end());
      }


      // Standard constructors
      constexpr String() = default;

      constexpr String(strv_t _strv) {
        if (_strv.size() >= N)
          throw_up("Constructing wide string view with size ", _strv.size(), " exceeds capacity ", N);
        size_t index = -1;
        for (char c : _strv)
          data[++index] = static_cast<wchar_t>(c);
        if (index + 1 < N)
          data[++index] = 0;
      }

      constexpr String(wstrv_t _wstrv) {
        if (_wstrv.size() >= N)
          throw_up("Constructing wide string view with size ", _wstrv.size(), " exceeds capacity ", N);
        size_t index = -1;
        for (wchar_t c : _wstrv)
          data[++index] = c;
        if (index + 1 < N)
          data[++index] = 0;
      }


      // Move/Copy operations
      constexpr String(const String& _other) {
        if (_other.data == 0)
          throw_up("Can't copy empty string");
        size_t index = -1;
        for (wchar_t c : _other)
          data[++index] = c;
      }
      constexpr String& operator=(const String& _other) {
        if (this != &_other) {
          wipe_clean();
          size_t index = -1;
          for (wchar_t c : _other)
            data[++index] = c;
        }
        return *this;
      }

      constexpr String(String&& _other) noexcept {
        if (_other.data == 0)
          throw_up("Can't move empty string");
        size_t index = -1;
        for (wchar_t c : _other)
          data[++index] = c;
        _other.wipe_clean();
      }
      constexpr String& operator=(String&& _other) noexcept {
        if (this != &_other) {
          wipe_clean();
          size_t index = -1;
          for (wchar_t c : _other)
            data[++index] = c;
          _other.wipe_clean();
        }
        return *this;
      }
    };

  
  
    template <typename T>
    class Vector { public:
      /*
        Same as std::vector but smaller and less features.
        Note:
          The size and capacity are 4 bytes each, when padding
          is done, memory-usage isn't doubled
      */
      T* data;
      uint32_t size;
      uint32_t capacity;


      // Vector handling
      T* begin() { return data; }
      T* end() { return data + size; }
      const T* begin() const { return data; }
      const T* end() const { return data + size; }

      void wipe_clean() noexcept {
        /*
          Call destructor on all elements and
          and reset the vector
        */
        if (data == nullptr)
          return;
        delete[] data;
        data = nullptr;
        size = 0;
        capacity = 0;
      }

      void increment_capacity() {
        T* new_data = new T[++capacity];
        size = -1;
        for (T& ownData : *this)
          new_data[++size] = ownData;
        delete[] data;
        data = new_data;
      }

      void push_back(T&& _new_v) {
        if (size == capacity)
          increment_capacity();
        data[++size] = std::move(_new_v);
      }
      void push_back(const T& _new_v) {
        if (size == capacity)
          increment_capacity();
        data[++size] = _new_v;
      }

      template <typename... Args>
      void emplace_back(Args&&... _args) {
        if (size == capacity)
          increment_capacity();
        data[++size] = T(std::forward<Args>(_args)...);
      }

      T pop_back() {
        if (size == 0)
          throw_up("Can't pop from empty");
        return std::move(data[--size]);
      }

      T& operator[](uint32_t _atIndex) {
        if (_atIndex >= size)
          throw_up("Index out of bounds: ", _atIndex, " >= ", size);
        return data[_atIndex];
      }

      bool operator==(const Vector<T>& _other) const {
        if (size != _other.size)
          return false;
        for (uint32_t i = 0; i < size; ++i)
          if (data[i] != _other.data[i])
            return false;
        return true;
      }
      bool operator!=(const Vector<T>& _other) const {
        return !(*this == _other);
      }

      bool operator==(std::initializer_list<T> _initList) const {
        if (size != _initList.size())
          return false;
        uint32_t index = -1;
        for (const T& item : _initList)
          if (data[++index] != item)
            return false;
        return true;
      }
      bool operator!=(std::initializer_list<T> _initList) const {
        return !(*this == _initList);
      }


      // Transform into stl
      operator std::vector<T>() const { // Copy
        std::vector<T> vec;
        vec.reserve(size);
        for (const T& item : *this)
          vec.push_back(item);
        return vec;
      }
      operator std::vector<T>&&() { // Move
        std::vector<T> vec;
        vec.reserve(size);
        for (T& item : *this)
          vec.push_back(std::move(item));
        return std::move(vec);
      }
  

      // Standard constructors
      ~Vector() {
        if (data != nullptr)
          delete[] data;
      }

      Vector() : data(nullptr), size(0), capacity(0) {}

      Vector(std::initializer_list<T> _initList) : size(_initList.size()), capacity(_initList.size()) {
        // Copies whatever is in the initializer list
        if (_initList.size() == 0)
          throw_up("Vector initializer list cannot be empty");
        data = new T[_initList.size()];
        uint32_t index = -1;
        for (const T& item : _initList)
          data[++index] = item;
      }

      Vector(const std::vector<T>& _vec) : size(_vec.size()), capacity(_vec.capacity()) {
        if (_vec.empty())
          throw_up("Vector cannot be initialized from an empty std::vector");
        data = new T[_vec.size()];
        uint32_t index = -1;
        for (const T& item : _vec)
          data[++index] = item;
      }


      // Move/Copy operations
      Vector(const Vector& _other) : size(_other.size), capacity(_other.capacity) {
        if (_other.data == nullptr)
          throw_up("Cannot copy from an empty vector at ", to_str(&_other));
        data = new T[capacity];
        uint32_t index = -1;
        for (const T& item : _other)
          data[++index] = item;
      }
      Vector& operator=(const Vector& _other) {
        if (this != &_other) {
          wipe_clean();
          size = _other.size;
          capacity = _other.capacity;
          if (_other.data == nullptr)
            throw_up("Cannot copy from an empty vector at ", to_str(&_other));
          data = new T[capacity];
          uint32_t index = -1;
          for (const T& item : _other)
            data[++index] = item;
        }
        return *this;
      }
      Vector(Vector&& _other) noexcept : data(_other.data), size(_other.size), capacity(_other.capacity) {
        _other.data = nullptr;
        _other.size = 0;
        _other.capacity = 0;
      }
      Vector& operator=(Vector&& _other) noexcept {
        if (this != &_other) {
          delete[] data;
          data = _other.data;
          size = _other.size;
          capacity = _other.capacity;
          _other.data = nullptr;
          _other.size = 0;
          _other.capacity = 0;
        }
        return *this;
      }
    };



    template <typename K, typename V>
    class Map { public:
      /*
        A simple map implementation using a vector of key-value pairs.
      */

      using Node_t = std::pair<K, V>;
      Vector<Node_t> data;

      Node_t* begin() { return data.data(); }
      Node_t* end() { return data.data() + data.size(); }
      const Node_t* begin() const { return data.data(); }
      const Node_t* end() const { return data.data() + data.size(); }


      // Map management
      void insert(const K& _key, const V& _value) {
        for (const Node_t& node : data)
          if (node.first == _key) {
            node.second = _value;
            return;
          }
        data.push_back({_key, _value});
      }
      void insert(K&& _key, V&& _value) {
        for (Node_t& node : data)
          if (node.first == _key) {
            node.value = std::move(_value);
            return;
          }
        data.push_back({std::move(_key), std::move(_value)});
      }
      void insert(const Node_t& _pair) {
        insert(_pair.first, _pair.second);
      }
      void insert(std::pair<K, V>&& _pair) {
        insert(std::move(_pair.first), std::move(_pair.second));
      }

      bool has_key(const K& _key) const {
        for (const Node_t& node : data)
          if (node.first == _key)
            return true;
        return false;
      }
      bool has_key(K&& _key) const {
        for (const Node_t& node : data)
          if (node.first == _key)
            return true;
        return false;
      }
      bool has_value(const V& _value) const {
        for (const Node_t& node : data)
          if (node.second == _value)
            return true;
        return false;
      }
      bool has_value(V&& _value) const {
        for (const Node_t& node : data)
          if (node.second == _value)
            return true;
        return false;
      }

      V& operator[](const K& _key) {
        for (Node_t& node : data)
          if (node.first == _key)
            return node.second;
        data.push_back({_key, V()});
        return data.back().second;
      }
      V& operator[](K&& _key) {
        for (Node_t& node : data)
          if (node.first == _key)
            return node.second;
        data.push_back({std::move(_key), V()});
        return data.back().second;
      }


      // Transform into stl
      operator std::unordered_map<K, V>() const { // Copy
        std::unordered_map<K, V> map;
        for (const Node_t& node : data)
          map[node.first] = node.second;
        return map;
      }
      operator std::unordered_map<K, V>&&() { // Move
        std::unordered_map<K, V> map;
        for (Node_t& node : data)
          map[std::move(node.first)] = std::move(node.second);
        return std::move(map);
      }
      operator std::map<K, V>() const { // Copy
        std::map<K, V> map;
        for (const Node_t& node : data)
          map[node.first] = node.second;
        return map;
      }
      operator std::map<K, V>&&() { // Move
        std::map<K, V> map;
        for (Node_t& node : data)
          map[std::move(node.first)] = std::move(node.second);
        return std::move(map);
      }


      // Standard constructors
      Map() = default;

      Map(std::initializer_list<std::pair<K, V>> _initList) {
        for (const Node_t& pair : _initList)
          insert(pair.first, pair.second);
      }

      Map(const std::unordered_map<K, V>& _map) {
        for (const Node_t& pair : _map)
          insert(pair.first, pair.second);
      }

      Map(const std::map<K, V>& _map) {
        for (const Node_t& pair : _map)
          insert(pair.first, pair.second);
      }


      // Move/Copy operations
      Map(const Map<K, V>& _other) : data(_other.data) {}

      Map<K, V>& operator=(const Map<K, V>& _other) {
        if (this != &_other) {
          this->data.wipe_clean();
          data = _other.data;
        }
        return *this;
      }
      Map(Map<K, V>&& other) noexcept : data(std::move(other.data)) {
        other.data.wipe_clean();
      }
      Map<K, V>& operator=(Map<K, V>&& other) noexcept {
        if (this != &other) {
          this->data.wipe_clean();
          data = std::move(other.data);
          other.data.wipe_clean();
        }
        return *this;
      }
    };
  };



  // Functions
  void pause(size_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
  }



  void print(const auto& _msg) {
    // Print and flush after
    std::wcout << _msg << std::flush;
  }
  void println(const auto& _msg) {
    // Print and flush after with newline
    std::wcout << _msg << std::endl;
  }
  void print(const auto&& _msg) {
    std::wcout << std::forward<decltype(_msg)>(_msg) << std::flush;
  }
  void println(const auto&& _msg) {
    std::wcout << std::forward<decltype(_msg)>(_msg) << std::endl;
  }



  void sh_call(strv_t command) {
    /*
      Blocking system call
    */
    if (system(command.data()) != 0)  
      throw_up("Failed to execute command: ", command);
  }



  void clear_console() {
    #ifdef _WIN32
      sh_call("cls");
    #else
      sh_call("clear");
    #endif
  }



  template <typename Key, typename Container>
  bool contains(Container& lookIn, Key& lookFor) {
    /*
      does `container` contain `key`
    */
    return std::find(lookIn.begin(), lookIn.end(), lookFor) != lookIn.end();
  }
  template <typename Containers>
  bool have_something_common(Containers& c1, Containers& c2) {
    /*
      do `c1` and `c2` contain similar keys
    */
    for (auto item : c1)
      if (contains(c2, item))
        return true;
    return false;
  }



  str_t get_env(strv_t var) {
    /*
      Get the value of an environment variable.
    */
    const char *const envCStr = getenv(var.data());
    if (envCStr == NULL)
      throw_up("Environment variable '", var, "' not found");
    return str_t(envCStr);
  }



  template <typename T>
  requires std::is_integral_v<T>
  TinySTL::Vector<T> range(T start, T end) {
    /*
      Simplified range function that takes two integers
      and returns a vector of integers (inclusive)
    */
    TinySTL::Vector<T> result;
    if (start > end) // reverse
      for (T i = start; i >= end; --i)
        result.push_back(i);
    else if (start < end) // start to end
      for (T i = start; i <= end; ++i)
        result.push_back(i);
    else // just start
      result.push_back(start);
    return result;
  }
  template <typename T>
  requires std::is_integral_v<T>
  TinySTL::Vector<T> range(T end) {
    return range(T(0), end);
  }



  template <typename T>
  T retry(const std::function<T()>& target, size_t retries, size_t delay = 0) {
    /*
      Retry a function up to `retries` times with a delay
      of `delay` milliseconds between each retry.
      Note:
        No lambda support
      Example:
        std::function<void()> func = []() {
          // Do something that might fail
        };
        cslib::retry(func, 3);
    */
    if (retries == 0)
      throw_up("Retries must be greater than 0");
    for (size_t tried : range(retries)) {
      try {
        return target();
      } catch (const std::exception& e) {
        if (tried == retries - 1) {
          throw_up("Function ", to_str(&target), " failed after ", retries, " retries: ", e.what());
        }
      } catch (...) {
        // Catch all other exceptions
        if (tried == retries - 1) {
          throw_up("Function ", to_str(&target), " failed after ", retries, " retries: Unknown exception");
        }
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }
    return T(); // This line is unreachable but keeps compiler happy
  }



  TinySTL::Vector<strv_t> parse_cli_args(int argc, const char *const argv[]) {
    /*
      Parse command line arguments and return them as a
      vector of strings.
      Note:
        The first argument is the program name, so we skip it
    */
    TinySTL::Vector<strv_t> args;
    if (argc <= 1)
      return args; // No arguments provided
    for (int i : range(1, argc - 1))
      args.emplace_back(argv[i]);
    return args;
  }



  FIXED wstrv_t TRIM_WITH = L"...";
  MACRO shorten_end(wstrv_t wstrsv, size_t maxLength) {
    /*
      Example:
        cslib::shorten_end(L"cslib.h++", 6); // "csl..."
    */
    if (maxLength < TRIM_WITH.length())
      throw_up("maxLength must be at least ", TRIM_WITH.length(), " (TRIM_WITH length)");
    if (wstrsv.length() <= maxLength)
      return wstr_t(wstrsv);
    return wstr_t(wstrsv.substr(0, maxLength - TRIM_WITH.length())) + TRIM_WITH.data();
  }

  MACRO shorten_begin(wstrv_t wstrsv, size_t maxLength) {
    /*
      Example:
        cslib::shorten_begin(L"cslib.h++", 6); // "...h++"
    */
    if (maxLength < TRIM_WITH.length())
      throw_up("maxLength must be at least ", TRIM_WITH.length(), " (TRIM_WITH length)");
    if (wstrsv.length() <= maxLength)
      return wstr_t(wstrsv);
    return wstr_t(TRIM_WITH) + wstr_t(wstrsv.substr(wstrsv.length() - (maxLength - TRIM_WITH.length())));
  }



  MACRO upper(wstrv_t wstrsv) {
    /*
      Example:
        cslib::upper(L"csLib.h++"); // "CSLIB.H++"
    */
    wstr_t str(wstrsv);
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
  }
  MACRO upper_ref(wstr_t& wstr) {
    std::transform(wstr.begin(), wstr.end(), wstr.begin(), ::toupper);
  }

  MACRO lower(wstrv_t wstrsv) {
    /*
      Example:
        cslib::lower(L"csLib.h++"); // "cslib.h++"
    */
    wstr_t str(wstrsv);
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
  }
  MACRO lower_ref(wstr_t& wstr) {
    std::transform(wstr.begin(), wstr.end(), wstr.begin(), ::tolower);
  }



  TinySTL::Vector<wstr_t> separate(wstrv_t wstrsv, wchar_t delimiter) {
    /*
      Example:
        cslib::separate(L"Hello World", ' ') // {"Hello", "World"}
    */
    wstr_t str(wstrsv);
    TinySTL::Vector<wstr_t> result;
    wstr_t temp;

    if (str.empty() or delimiter == L'\0')
      return result;

    for (wchar_t c : str) {
      if (c == delimiter)
        result.push_back(std::move(temp));
      else
        temp += c;
    }

    result.push_back(temp);
    return result;
  }



  size_t roll_dice(size_t min, size_t max) {
    /*
      Minimum and maximum value and returns a random
      number between them (inclusive).
    */
    if (min > max) std::swap(min, max);

    static thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<size_t> distribution(min, max);
    return distribution(generator);
  }



  wstr_t do_io(std::wistream& winStream) {
    wstr_t input;
    std::getline(winStream, input);
    return input;
  }
  void do_io(std::wistream& winStream, std::wostream& woutStream) {
    woutStream << do_io(winStream) << std::flush;
  }
  str_t do_io(std::istream& inStream) {
    str_t input;
    std::getline(inStream, input);
    return input;
  }
  void do_io(std::istream& inStream, std::ostream& outStream) {
    outStream << do_io(inStream) << std::flush;
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
    wstr_t prefix;
    Out() = default;
    Out(wstrv_t wprefsv = L"", wstrv_t color = L"") {
      prefix = color;
      prefix += wprefsv;
      if (!color.empty())
        prefix += Reset;
      prefix += L" ";
    }
    std::wostream& operator<<(const auto& msg) {
      std::wcout << prefix << msg;
      return std::wcout;
    }
    std::wostream& operator<<(auto&& msg) {
      std::wcout << prefix << std::forward<decltype(msg)>(msg);
      return std::wcout;
    }
  };



  class TimeStamp { public:
    /*
      A wrapper around std::chrono that I have control over
    */
    std::chrono::system_clock::time_point timePoint;
    

    // Contructors and error handling
    TimeStamp() {timePoint = std::chrono::system_clock::now();}
    TimeStamp(std::chrono::system_clock::time_point tp) : timePoint(tp) {}
    TimeStamp(uint sec, uint min, uint hour, uint day, uint month, uint year) {
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
        throw_up("Invalid date: ", year, "-", month, "-", day);
      // Determine time
      if (hour >= 24 or min >= 60 or sec >= 60)
        throw_up("Invalid time: ", hour, ":", min, ":", sec);
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


    // Getters
    wstr_t asWstr() const {
      /*
        Convert the time point to (almost) ISO 8601
        in format YYYY-MM-DD HH:MM:SS)..
      */
      std::time_t time = std::chrono::system_clock::to_time_t(timePoint);
      return (std::wstringstream() << std::put_time(std::gmtime(&time), L"%Y-%m-%d %H:%M:%S")).str();
    }
    uint get_year() const {
      auto ymd = std::chrono::year_month_day(std::chrono::floor<std::chrono::days>(timePoint));
      return uint(int(ymd.year()));
    }
    uint get_month() const {
      auto ymd = std::chrono::year_month_day(std::chrono::floor<std::chrono::days>(timePoint));
      return uint(ymd.month());
    }
    uint get_day() const {
      auto ymd = std::chrono::year_month_day(std::chrono::floor<std::chrono::days>(timePoint));
      return uint(ymd.day());
    }
    uint get_hour() const {
      auto day_point = std::chrono::floor<std::chrono::days>(timePoint);
      auto time_since_midnight = timePoint - day_point;
      auto hms = std::chrono::hh_mm_ss(time_since_midnight);
      return uint(hms.hours().count());
    }
    uint get_minute() const {
      auto day_point = std::chrono::floor<std::chrono::days>(timePoint);
      auto time_since_midnight = timePoint - day_point;
      auto hms = std::chrono::hh_mm_ss(time_since_midnight);
      return uint(hms.minutes().count());
    }
    uint get_second() const {
      auto day_point = std::chrono::floor<std::chrono::days>(timePoint);
      auto time_since_midnight = timePoint - day_point;
      auto hms = std::chrono::hh_mm_ss(time_since_midnight);
      return uint(hms.seconds().count());
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




  #ifdef _WIN32
    MACRO PATH_DELIMITER = L'\\';
  #else
    MACRO PATH_DELIMITER = L'/';
  #endif
  class VirtualPath { public:
    /*
      Wrapper around std::filesystem::path
    */
    std::filesystem::path isAt; // Covers move and copy semantics


    // Path management
    std::filesystem::file_type type() const {
      return std::filesystem::status(isAt).type();
    }

    std::chrono::system_clock::time_point last_modified_tp() const {
      auto ftime = std::filesystem::last_write_time(isAt);
      return std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
    }

    VirtualPath parent() const {
      /*
        Example:
          VirtualPath path("/gitstuff/cslib/cslib.h++");
          VirtualPath parent = path.parent();
          // parent = "/gitstuff/cslib"
      */
      if (isAt.parent_path().empty())
        throw_up("VirtualPath '", to_str(this), "' has somehow no parent");
      return VirtualPath(isAt.parent_path().wstring(), std::filesystem::file_type::directory);
    }

    size_t depth() const {
      /*
        Example:
          VirtualPath path("/gitstuff/cslib/cslib.h++");
          size_t depth = path.depth();
          // depth = 2 (because there are 2 directories before the file)
      */
      return separate(isAt.wstring(), PATH_DELIMITER).size - 1; // -1 for the last element
    }

    bool operator==(const VirtualPath& other) const { return this->isAt == other.isAt; }
    bool operator!=(const VirtualPath& other) const { return !(*this == other); }
    bool operator==(wstrv_t other) const { return this->isAt == std::filesystem::path(other); }
    bool operator!=(wstrv_t other) const { return !(*this == other); }
    bool operator==(strv_t other) const { return this->isAt == std::filesystem::path(other);}
    bool operator!=(strv_t other) const { return !(*this == other); }
    bool operator==(const std::filesystem::path& other) const { return this->isAt == other; }
    bool operator!=(const std::filesystem::path& other) const { return !(*this == other); }


    // Transform into stl
    operator wstr_t() const { return this->isAt.wstring(); }
    operator std::filesystem::path() const { return this->isAt; }
    operator std::filesystem::path&() { return this->isAt; }
    operator std::filesystem::path*() { return &this->isAt; }
    operator std::filesystem::path*() const { return const_cast<std::filesystem::path*>(&this->isAt); } // const_cast is safe here


    // Constructors
    VirtualPath() = default;
    VirtualPath(wstrv_t where) : isAt(std::filesystem::canonical(where.data())) {}
    /*
      Constructor that takes a string and checks if it's a valid path.
      Notes:
        - If where is relative, it will be converted to an absolute path.
        - If where is empty, you will crash.
    */
    VirtualPath(wstrv_t where, std::filesystem::file_type shouldBe) : VirtualPath(where) {
      if (this->type() != shouldBe)
        throw_up("Path '", where, "' initialized with unexpected file type");
    }


    // Complicated methods
    void move_to(const VirtualPath& moveTo) {
      if (moveTo.type() != std::filesystem::file_type::directory)
        throw_up("Target path '", moveTo.isAt.wstring(), "' is not a directory (this: '", this->isAt.wstring(), "')");
      if (moveTo == *this)
        throw_up("Cannot move to the same path: ", this->isAt.wstring());
      std::filesystem::path willBecome = moveTo.isAt / this->isAt.filename();
      if (std::filesystem::exists(willBecome))
        throw_up("Target path '", willBecome.wstring(), "' already exists (this: '", this->isAt.wstring(), "')");
      std::filesystem::rename(this->isAt, willBecome);
      this->isAt = VirtualPath(willBecome.wstring()).isAt; // Apply changes
    }

    VirtualPath copy_into(const VirtualPath& targetDict) const {
      /*
        Copies this instance to a new location and returns
        a new VirtualPath instance pointing to the copied file.
      */
      if (targetDict.type() != std::filesystem::file_type::directory)
        throw_up("Target path '", targetDict.isAt.wstring(), "' is not a directory (this: '", this->isAt.wstring(), "')");
      if (targetDict == *this)
        throw_up("Cannot copy to the same path: ", this->isAt.wstring());
      std::filesystem::path willBecome = targetDict.isAt / this->isAt.filename();
      if (std::filesystem::exists(willBecome))
        throw_up("Target path '", willBecome.wstring(), "' already exists (this: '", this->isAt.wstring(), "')");
      std::filesystem::copy(this->isAt, willBecome);
      return VirtualPath(willBecome.wstring(), this->type());
    }
  };



  class File { public:
    /*
      Child class of VirtualPath that represents a file.
      Example:
        File file("/gitstuff/cslib/cslib.h++");
        str_t content = file.content();
        // content = "Hello World"
    */
    VirtualPath is; // Composition over inheritance

    File() = default;
    File(wstrv_t where) : is(where, std::filesystem::file_type::regular) {}

    wstr_t content(std::ios_base::openmode openMode = std::ios::in) const {
      /*
        Read the content of the file and return it as a string.
        Note:
          - No error-handling for files larger than available memory
      */
      std::wifstream file(is.isAt, openMode);
      if (!file.is_open())
        throw_up("Failed to open file '", is.isAt.wstring(), '\'');
      if (!file.good())
        throw_up("Failed to read file '", is.isAt.wstring(), '\'');
      return wstr_t((std::istreambuf_iterator<wchar_t>(file)), std::istreambuf_iterator<wchar_t>());
    }
    wstr_t wstr() const {return is.isAt.wstring();}
    wstr_t extension() const {return is.isAt.extension().wstring();}
    size_t bytes() const {return std::filesystem::file_size(is.isAt);}
  };



  class Folder { public:
    /*
      Child class of VirtualPath that represents a folder and
      everything in it.
    */

    std::vector<VirtualPath> content;
    VirtualPath is;

    void update() {
      content.clear();
      for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(is.isAt))
        content.push_back(VirtualPath(entry.path().wstring()));
      content.shrink_to_fit();
    }

    Folder() = default;
    Folder(wstrv_t where) : is(where, std::filesystem::file_type::directory) {update();}
    wstr_t wstr() const {
      return is.isAt.wstring();
    }

    bool has(const VirtualPath& item) const {
      /*
        Check if the folder contains the given item.
        Example:
          Folder folder("/gitstuff/cslib");
          VirtualPath item("/gitstuff/cslib/cslib.h++");
          bool exists = folder.has(item);
          // exists = true
      */
      return contains(content, item);
    }
    bool has(const File& item) const {
      Folder shouldBe(this->wstr());
      return contains(shouldBe.content, item.is);
    }
    bool has(const Folder& item) const {
      Folder shouldBe(this->wstr());
      return contains(shouldBe.content, item.is);
    }
  };



  MACRO TEMP_FILE_HEAD = "cslibTempFile_";
  MACRO TEMP_FILE_TAIL = ".tmp";
  MACRO TEMP_FILE_NAME_LEN = 200 - (std::strlen(TEMP_FILE_HEAD) + std::strlen(TEMP_FILE_TAIL)); // Freebuffer of 55
  class TempFile { public:
    /*
      A temporary file that is created in the system's temporary directory.
      It will be deleted when the object is destroyed.
    */
    File file;
    TempFile() {
      Folder tempDir(std::filesystem::temp_directory_path().wstring());
      str_t randomName;
      for (size_t i : range(TEMP_FILE_NAME_LEN))
        switch (roll_dice(0, 2)) {
          case 0: randomName += wchar_t(roll_dice('A', 'Z')); break; // Uppercase letter
          case 1: randomName += wchar_t(roll_dice('a', 'z')); break; // Lowercase letter
          case 2: randomName += wchar_t(roll_dice('0', '9')); break; // Digit
        }
      str_t tempFileName = TEMP_FILE_HEAD + randomName + TEMP_FILE_TAIL;
      file = File(tempDir.wstr() + to_wstr(PATH_DELIMITER) + to_wstr(tempFileName));
    }
    ~TempFile() {
      if (std::filesystem::exists(file.is.isAt))
        std::filesystem::remove(file.is.isAt);
    }
  };
} // namespace cslib