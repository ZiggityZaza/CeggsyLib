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
#include <thread>
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
  using wstrsv_t = std::wstring_view;
  #define SHARED inline // Alias inline for shared functions, etc.
  #define MACRO inline constexpr auto // Macros for macro definitions
  #define FIXED inline constexpr // Explicit alternative for MACRO



  // Defined beforehand to avoid circular dependencies
  template <typename... Args>
  std::runtime_error up_impl(size_t line, Args&&... messages) {
    /*
      Create a custom runtime error with the given messages.
      Example:
        #define up(...) up_impl(__LINE__, __VA_ARGS__)
        if (1 == 2)
          throw up("Hello", "World", 123, L"Wide string");
    */
    wstr_t message;
    ((message += to_wstr(std::forward<Args>(messages))), ...);
    std::wcout << L"\033[1m" << L"\033[31m" << L"Error: " << message << L"\033[0m" << std::endl;
    std::filesystem::path currentPath = std::filesystem::current_path();
    return std::runtime_error("std::runtime_error called from line " + std::to_string(line) + " in workspace '" + currentPath.string() + "'");
  }
  #define throw_up(...) throw up_impl(__LINE__, __VA_ARGS__)



  // Find the correct way to convert T to string
  template <typename T>
  std::string to_ptrstr(T* value) {
    /*
      Convert a pointer to a cool looking
      pointer address string.
      Example:
        cslib::to_str(&value) // "0x123"
    */
    if (value == nullptr)
      return "nullptr"; // Handle null pointers
    std::ostringstream oss;
    oss << "0x" << std::hex << reinterpret_cast<std::uintptr_t>(value);
    return oss.str();
  }
  MACRO to_str(char value) {
    return std::string(1, value); // Convert char to string
  }
  MACRO to_str(wchar_t value) {
    return std::string(1, static_cast<char>(value)); // Convert wchar_t to string
  }
  MACRO to_str(std::string_view value) {
    /*
      Also for const char*
    */
    return std::string(value.data(), value.data() + value.size());
  }
  MACRO to_str(wstrsv_t value) {
    /*
      Also for const wchar_t*
      Note: This is a lossy conversion, as wchar_t can represent characters
      that cannot be represented in a single byte string.
      Use with caution, as it may lead to data loss.
    */
    return std::string(value.data(), value.data() + value.size());
  }
  template <std::integral T>
  MACRO to_str(T value) {
    return std::to_string(value);
  }
  template <std::floating_point T>
  MACRO to_str(T value) {
    return std::to_string(value);
  }

  // Find the correct way to convert T to wide string
  template <typename T>
  MACRO to_ptrwstr(T* value) {
    /*
      Convert a pointer to a cool looking
      pointer address string.
      Example:
        cslib::to_wstr(&value) // "0x123"
    */
    if (value == nullptr)
      return L"nullptr"; // Handle null pointers
    std::wostringstream oss;
    oss << L"0x" << std::hex << reinterpret_cast<std::uintptr_t>(value);
    return oss.str();
  }
  MACRO to_wstr(char value) {
    return wstr_t(1, static_cast<wchar_t>(value));
  }
  MACRO to_wstr(wchar_t value) {
    return wstr_t(1, value);
  }
  MACRO to_wstr(std::string_view value) {
    /*
      Also for const char*
    */
    return wstr_t(value.data(), value.data() + value.size());
  }
  MACRO to_wstr(wstrsv_t value) {
    /*
      Also for const wchar_t*
    */
    return wstr_t(value.data(), value.data() + value.size());
  }
  template <std::integral T>
  MACRO to_wstr(T value) {
    return std::to_wstring(value);
  }
  template <std::floating_point T>
  MACRO to_wstr(T value) {
    return std::to_wstring(value);
  }



  // Pillar namespace
  namespace TinySTL {
    // The stl is good but sometimes you need something smaller

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

      void wipe_clean() {
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
      T* begin() { return data; }
      T* end() { return data + size; }
      const T* begin() const { return data; }
      const T* end() const { return data + size; }
      void increment_capacity() {
        T* new_data = new T[++capacity];
        size = -1;
        for (T& ownData : *this)
          new_data[++size] = ownData;
        delete[] data;
        data = new_data;
      }
      void push_back(T&& value) {
        if (size == capacity)
          increment_capacity();
        data[++size] = std::move(value);
      }
      void push_back(const T& value) {
        if (size == capacity)
          increment_capacity();
        data[++size] = value;
      }
      template <typename... Args>
      void emplace_back(Args&&... args) {
        if (size == capacity)
          increment_capacity();
        data[++size] = T(std::forward<Args>(args)...);
      }
      void pop_back() {
        if (size == 0)
          throw_up("Cannot pop from an empty vector at ", to_ptrstr(this));
        --size;
      }
      ~Vector() {
        if (data != nullptr)
          delete[] data;
      }
      T& operator[](uint32_t index) {
        if (index >= size)
          throw_up("Index out of bounds for Vector at ", to_ptrstr(this), ": ", index, " >= ", size);
        return data[index];
      }
      bool operator==(const Vector<T>& other) const {
        if (size != other.size)
          return false;
        for (uint32_t i = 0; i < size; ++i)
          if (data[i] != other.data[i])
            return false;
        return true;
      }
      bool operator!=(const Vector<T>& other) const {
        return !(*this == other);
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
      operator std::vector<T>*() { // Pointer
        std::vector<T> vec;
        vec.reserve(size);
        for (T& item : *this)
          vec.push_back(std::move(item));
        return new std::vector<T>(std::move(vec));
      }
      operator std::vector<T>*() const { // Const pointer
        std::vector<T> vec;
        vec.reserve(size);
        for (const T& item : *this)
          vec.push_back(item);
        return new std::vector<T>(std::move(vec));
      }

      // Standard constructors
      Vector() : data(nullptr), size(0), capacity(0) {}
      Vector(uint32_t initialCapacity) : size(0), capacity(initialCapacity) {
        if (initialCapacity == 0)
          throw_up("Vector capacity must be greater than 0");
        data = new T[initialCapacity];
      }
      Vector(std::initializer_list<T> initList) : size(initList.size()), capacity(initList.size()) {
        // Copies whatever is in the initializer list
        if (initList.size() == 0)
          throw_up("Vector initializer list cannot be empty");
        data = new T[initList.size()];
        uint32_t index = -1;
        for (const T& item : initList)
          data[++index] = item;
      }
      Vector(const std::vector<T>& vec) : size(vec.size()), capacity(vec.capacity()) {
        if (vec.empty())
          throw_up("Vector cannot be initialized from an empty std::vector");
        data = new T[vec.size()];
        uint32_t index = -1;
        for (const T& item : vec)
          data[++index] = item;
      }
      Vector(std::vector<T>&& vec) : size(vec.size()), capacity(vec.capacity()) {
        if (vec.empty())
          throw_up("Vector cannot be initialized from an empty std::vector");
        data = new T[vec.size()];
        uint32_t index = -1;
        for (T& item : vec)
          data[++index] = item;
      }

      // Copy operations
      Vector(const Vector& other) : size(other.size), capacity(other.capacity) {
        if (other.data == nullptr)
          throw_up("Cannot copy from an empty vector at ", to_ptrstr(&other));
        data = new T[capacity];
        uint32_t index = -1;
        for (const T& item : other)
          data[++index] = item;
      }
      Vector& operator=(const Vector& other) {
        if (this != &other) {
          this->wipe_clean();
          size = other.size;
          capacity = other.capacity;
          if (other.data == nullptr)
            throw_up("Cannot copy from an empty vector at ", to_ptrstr(&other));
          data = new T[capacity];
          uint32_t index = -1;
          for (const T& item : other)
            data[++index] = item;
        }
        return *this;
      }
      // Move operations
      Vector(Vector&& other) noexcept : data(other.data), size(other.size), capacity(other.capacity) {
        other.data = nullptr;
        other.size = 0;
        other.capacity = 0;
      }
      Vector& operator=(Vector&& other) noexcept {
        if (this != &other) {
          delete[] data;
          data = other.data;
          size = other.size;
          capacity = other.capacity;
          // No need to call wipe_clean() here

          other.data = nullptr;
          other.size = 0;
          other.capacity = 0;
        }
        return *this;
      }
    };



    template <size_t N, typename char_t = wchar_t>
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

      char_t data[N] = {0};

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
      constexpr char_t* begin() { return data; }
      constexpr char_t* end() { return data + length(); }
      constexpr const char_t* begin() const { return data; }
      constexpr const char_t* end() const { return data + length(); }
      constexpr void append(const char_t& c) {
        size_t len = length();
        if (len >= N)
          throw_up("String (", to_ptrstr(this), ") containing '", to_str(*this), "' capacity exceeded: ", N);
        data[len] = c;
        if (len + 1 < N)
          data[len + 1] = 0;
      }
      constexpr void wipe_clean() {
        for (size_t i = 0; i < N; ++i)
          data[i] = 0;
      }
      constexpr char_t& at(size_t index) {
        if (index >= N)
          throw_up("Index out of bounds for String at ", to_ptrstr(this), ": ", index, " >= ", N);
        return data[index];
      }
      constexpr bool operator==(const String& other) const {
        if (length() != other.length())
          return false;
        for (size_t i = 0; i < length(); ++i)
          if (data[i] != other.data[i])
            return false;
        return true;
      }
      constexpr bool operator!=(const String& other) const {
        return !(*this == other);
      }

      // Transform into stl (lossy conversion for chars)
      constexpr operator std::string() const { return std::string(data, data + length()); }
      constexpr operator std::string&&() { return std::move(std::string(*this)); }
      constexpr operator std::string*() { return new std::string(*this); }
      constexpr operator std::string*() const { return new std::string(*this); }
      constexpr operator std::wstring() const { return std::wstring(data, data + length()); }
      constexpr operator std::wstring&&() { return std::move(std::wstring(*this)); }
      constexpr operator std::wstring*() { return new std::wstring(*this); }
      constexpr operator std::wstring*() const { return new std::wstring(*this); }
      constexpr friend std::ostream& operator<<(std::ostream& os, const String& str) {
        /*
          Print the string to an output stream.
          Example:
            std::cout << cslib::String<5>("Hello") << std::endl;
        */
        return os << std::string(str.data, str.data + str.length());
      }
      constexpr friend std::wostream& operator<<(std::wostream& os, const String& str) {
        /*
          Print the wide string to an output stream.
          Example:
            std::wcout << cslib::String<5>(L"Hello") << std::endl;
        */
        return os << std::wstring(str.data, str.data + str.length());
      }

      // Standard constructors
      constexpr String() = default;
      constexpr String(std::string_view otherStr) {
        /*
          Initialize the string with a std::string_view.
          Note:
            This is a lossy conversion, as char_t can represent characters
            that cannot be represented in a single byte string.
            Use with caution, as it may lead to data loss.
        */
        if (otherStr.size() >= N)
          throw_up("Constructing string view '", to_str(otherStr), "' with size ", otherStr.size(), " exceeds capacity ", N);
        size_t index = -1;
        for (char c : otherStr)
          data[++index] = char_t(c);
        if (index + 1 < N)
          data[++index] = 0;
      }
      constexpr String(wstrsv_t otherWstr) {
        if (otherWstr.size() >= N)
          throw_up("Constructing string view '", to_str(otherWstr), "' with size ", otherWstr.size(), " exceeds capacity ", N);
        size_t index = -1;
        for (wchar_t c : otherWstr)
          data[++index] = char_t(c);
        if (index + 1 < N)
          data[++index] = 0;
      }
      constexpr String(const char_t* otherStr) {
        size_t index = -1;
        while (otherStr[++index] != 0 and index < N)
          data[index] = otherStr[index];
        if (index + 1 < N)
          data[++index] = 0;
      }

      // Copy operations
      constexpr String(const String& other) {
        if (other.length() >= N)
          throw_up("Copying string '", to_str(std::string(other)), "' with size ", other.length(), " exceeds capacity ", N);
        size_t index = -1;
        for (const char_t& c : other)
          data[++index] = c;
        if (index + 1 < N)
          data[++index] = 0;
      }
      constexpr String& operator=(const String& other) {
        if (this != &other) {
          this->wipe_clean();
          if (other.length() >= N)
            throw_up("Copying string '", to_str(std::string(other)), "' with size ", other.length(), " exceeds capacity ", N);
          size_t index = -1;
          for (const char_t& c : other)
            data[++index] = c;
          if (index + 1 < N)
            data[++index] = 0;
        }
        return *this;
      }
      // Move operations
      String(String&& other) noexcept {
        for (size_t i = 0; i < N; ++i)
          this->data[i] = std::move(other.data[i]);
      }
      String& operator=(String&& other) noexcept {
        if (this != &other) {
          this->wipe_clean();
          for (size_t i = 0; i < N; ++i)
            this->data[i] = std::move(other.data[i]);
        }
        return *this;
      }
    };



    template <typename K, typename V>
    class Map { public:
      /*
        A simple map implementation using a vector of key-value pairs.
        Note:
          This is not a hash map, so it is not as efficient as std::unordered_map.
          It is meant for small maps where memory is preferred over speed.
      */

      struct Node {
        K key;
        V value;
      };

      Vector<Node> data;

      std::pair<K, V>* begin() { return reinterpret_cast<std::pair<K, V>*>(data.begin()); }
      std::pair<K, V>* end() { return reinterpret_cast<std::pair<K, V>*>(data.end()); }
      const std::pair<K, V>* begin() const { return reinterpret_cast<const std::pair<K, V>*>(data.begin()); }
      const std::pair<K, V>* end() const { return reinterpret_cast<const std::pair<K, V>*>(data.end()); }
      void wipe_clean() {
        data.wipe_clean();
      }
      void insert(const K& key, const V& value) {
        for (Node& node : data) {
          if (node.key == key) {
            node.value = value;
            return;
          }
        }
        data.push_back({key, value});
      }
      void insert(K&& key, V&& value) {
        for (Node& node : data) {
          if (node.key == key) {
            node.value = std::move(value);
            return;
          }
        }
        data.push_back({std::move(key), std::move(value)});
      }
      void insert(const std::pair<K, V>& pair) {
        insert(pair.first, pair.second);
      }
      void insert(std::pair<K, V>&& pair) {
        insert(std::move(pair.first), std::move(pair.second));
      }
      bool has_key(const K& key) const {
        for (const Node& node : data)
          if (node.key == key)
            return true;
        return false;
      }
      bool has_key(K&& key) const {
        for (const Node& node : data)
          if (node.key == key)
            return true;
        return false;
      }
      bool has_value(const V& value) const {
        for (const Node& node : data)
          if (node.value == value)
            return true;
        return false;
      }
      bool has_value(V&& value) const {
        for (const Node& node : data)
          if (node.value == value)
            return true;
        return false;
      }
      V& operator[](const K& key) {
        for (Node& node : data)
          if (node.key == key)
            return node.value;
        data.push_back({key, V()});
        return data.back().value;
      }
      V& operator[](K&& key) {
        for (Node& node : data)
          if (node.key == key)
            return node.value;
        data.push_back({std::move(key), V()});
        return data.back().value;
      }
      std::vector<K> keys() const {
        /*
          Returns a vector of all keys in the map.
        */
        std::vector<K> keys;
        for (const Node& node : data)
          keys.push_back(node.key);
        return keys;
      }

      // Transform into stl
      operator std::unordered_map<K, V>() const { // Copy
        std::unordered_map<K, V> map;
        for (const Node& node : data)
          map[node.key] = node.value;
        return map;
      }
      operator std::unordered_map<K, V>&&() { // Move
        std::unordered_map<K, V> map;
        for (Node& node : data)
          map[std::move(node.key)] = std::move(node.value);
        return std::move(map);
      }
      operator std::unordered_map<K, V>*() { // Pointer
        std::unordered_map<K, V> map;
        for (Node& node : data)
          map[std::move(node.key)] = std::move(node.value);
        return new std::unordered_map<K, V>(std::move(map));
      }
      operator std::unordered_map<K, V>*() const { // Const pointer
        std::unordered_map<K, V> map;
        for (const Node& node : data)
          map[node.key] = node.value;
        return new std::unordered_map<K, V>(std::move(map));
      }
      operator std::map<K, V>() const { // Copy
        std::map<K, V> map;
        for (const Node& node : data)
          map[node.key] = node.value;
        return map;
      }
      operator std::map<K, V>&&() { // Move
        std::map<K, V> map;
        for (Node& node : data)
          map[std::move(node.key)] = std::move(node.value);
        return std::move(map);
      }
      operator std::map<K, V>*() { // Pointer
        std::map<K, V> map;
        for (Node& node : data)
          map[std::move(node.key)] = std::move(node.value);
        return new std::map<K, V>(std::move(map));
      }
      operator std::map<K, V>*() const { // Const pointer
        std::map<K, V> map;
        for (const Node& node : data)
          map[node.key] = node.value;
        return new std::map<K, V>(std::move(map));
      }

      // Standard constructors
      Map() = default;
      Map(std::initializer_list<std::pair<K, V>> initList) {
        for (const auto& pair : initList)
          insert(pair.first, pair.second);
      }
      Map(const std::unordered_map<K, V>& map) {
        for (const auto& pair : map)
          insert(pair.first, pair.second);
      }
      Map(std::unordered_map<K, V>&& map) {
        for (auto& pair : map)
          insert(std::move(pair.first), std::move(pair.second));
      }
      Map(const std::map<K, V>& map) {
        for (const auto& pair : map)
          insert(pair.first, pair.second);
      }
      Map(std::map<K, V>&& map) {
        for (auto& pair : map)
          insert(std::move(pair.first), std::move(pair.second));
      }
      Map(const Map<K, V>& other) : data(other.data) {}
      Map<K, V>& operator=(const Map<K, V>& other) {
        if (this != &other) {
          this->wipe_clean();
          data = other.data;
        }
        return *this;
      }
      Map(Map<K, V>&& other) noexcept : data(std::move(other.data)) {
        other.data.wipe_clean();
      }
      Map<K, V>& operator=(Map<K, V>&& other) noexcept {
        if (this != &other) {
          this->wipe_clean();
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



  template <typename T>
  void print(const T& msg) {
    std::wcout << msg << std::flush;
  }
  template <typename T>
  void print(T&& msg) {
    std::wcout << std::forward<T>(msg) << std::flush;
  }

  template <typename T>
  void println(const T& msg) {
    std::wcout << msg << std::endl;
  }
  template <typename T>
  void println(T&& msg) {
    std::wcout << std::forward<T>(msg) << std::endl;
  }



  void sh_call(std::string_view command) {
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



  std::string get_env(std::string_view var) {
    /*
      Get the value of an environment variable.
    */
    const char *const envCStr = getenv(var.data());
    if (envCStr == NULL)
      throw_up("Environment variable '", var, "' not found");
    return std::string(envCStr);
  }



  TinySTL::Vector<int> range(int start, int end) {
    /*
      Simplified range function that takes two integers
      and returns a vector of integers (inclusive)
    */
    TinySTL::Vector<int> result;
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
  TinySTL::Vector<int> range(int end) {
    return range(0, end);
  }
  TinySTL::Vector<size_t> range(size_t start, size_t end) {
    TinySTL::Vector<size_t> result;
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
  TinySTL::Vector<size_t> range(size_t end) {
    return range(size_t(0), end);
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
          throw_up("Function ", to_ptrstr(&target), " failed after ", retries, " retries: ", e.what());
        }
      } catch (...) {
        // Catch all other exceptions
        if (tried == retries - 1) {
          throw_up("Function ", to_ptrstr(&target), " failed after ", retries, " retries: Unknown exception");
        }
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }
    return T(); // This line is unreachable but keeps compiler happy
  }



  TinySTL::Vector<std::string_view> parse_cli_args(int argc, const char *const argv[]) {
    /*
      Parse command line arguments and return them as a
      vector of strings.
      Note:
        The first argument is the program name, so we skip it
    */
    TinySTL::Vector<std::string_view> args;
    if (argc <= 1)
      return args; // No arguments provided
    for (int i : range(1, argc - 1))
      args.emplace_back(argv[i]);
    return args;
  }



  FIXED wstrsv_t TRIM_WITH = L"...";
  wstr_t shorten_end(wstrsv_t wstrsv, size_t maxLength) {
    /*
      Trim and content of `TRIM_WITH` to the end of the string
      if it exceeds `maxLength`.
      Example:
        cslib::shorten_end(L"cslib.h++", 3);
        // is "c..."
    */
    if (maxLength < TRIM_WITH.length())
      throw_up("maxLength must be greater than or equal to the length of TRIM_WITH (", TRIM_WITH.length(), ')');
    wstr_t wstr(wstrsv);
    if (wstr.length() > maxLength)
      return wstr.substr(0, maxLength - TRIM_WITH.length()) + TRIM_WITH.data();
    return wstr;
  }
  wstr_t shorten_begin(wstrsv_t wstrsv, size_t maxLength) {
    /*
      Trim and content of `TRIM_WITH` to the beginning of the string
      if it exceeds `maxLength`.
      Example:
        cslib::shorten_begin(L"cslib.h++", 3);
        // is "...h++"
    */
    if (maxLength < TRIM_WITH.length())
      throw_up("maxLength must be greater than or equal to the length of TRIM_WITH (", TRIM_WITH.length(), ')');
    wstr_t wstr(wstrsv);
    if (wstr.length() > maxLength)
      return TRIM_WITH.data() + wstr.substr(wstr.length() - (maxLength - TRIM_WITH.length()));
    return wstr;
  }



  wstr_t upper(wstrsv_t wstrsv) {
    /*
      Converts it to uppercase.
      Example:
        to_upper("csLib.h++");
        // is "CSLIB.H++"
    */
    wstr_t str(wstrsv);
    return std::transform(str.begin(), str.end(), str.begin(), ::toupper), str;
  }
  void upper_ref(wstr_t& wstr) {
    std::transform(wstr.begin(), wstr.end(), wstr.begin(), ::toupper);
  }

  wstr_t lower(wstrsv_t wstrsv) {
    /*
      Converts it to lowercase.
      Example:
        to_lower("csLib.h++");
        // is "cslib.h++"
    */
    wstr_t str(wstrsv);
    return std::transform(str.begin(), str.end(), str.begin(), ::tolower), str;
  }
  void lower_ref(wstr_t& wstr) {
    std::transform(wstr.begin(), wstr.end(), wstr.begin(), ::tolower);
  }



  TinySTL::Vector<wstr_t> separate(wstrsv_t wstrsv, wchar_t delimiter) {
    /*
      Same as above, but for wide strings.
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



  wstr_t input() {
    wstr_t input;
    std::getline(std::wcin, input);
    return input;
  }



  // Get the first available UTF-8 locale
  FIXED std::array<std::string_view, 6> utf8_locales = {
    "en_US.UTF-8",
    "en_US.utf8",
    "C.UTF-8",
    "POSIX.UTF-8",
    "C.utf8",
    "POSIX.utf8"
  };
  void enable_wchar_io() {
    // Set all io-streaming globally to UTF-8 encoding

    std::locale utf8_locale;
    for (std::string_view locale_name : utf8_locales) {
      try {
        utf8_locale = std::locale(locale_name.data());
      } catch (const std::runtime_error&) {
        // Ignore the exception, try the next locale
        std::wcerr << L"[⚠️] Failed to set locale: " << locale_name.data() << L". Trying next...\n";
      }
    }
    if (utf8_locale.name().empty())
      throw_up("Failed to find a suitable UTF-8 locale. Ensure your system supports UTF-8 locales");

    std::locale::global(utf8_locale);
    std::wcout.imbue(utf8_locale);
    std::wcin.imbue(utf8_locale);
    std::wclog.imbue(utf8_locale);
    std::wcerr.imbue(utf8_locale);

    std::wcout << L"[⛓️‍💥] Console initialized with UTF-8 encoding.\n";
  }



  // Classes
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
    Out(wstrsv_t wprefsv, wstrsv_t color = L"") {
      prefix = color;
      prefix += wprefsv;
      prefix += Reset;
      prefix += L" ";
    }
    Out(std::string_view prefsv, std::string_view color = "") {
      prefix = to_wstr(color);
      prefix += to_wstr(prefsv);
      prefix += Reset;
      prefix += L" ";
    }
    template <typename T>
    std::wostream& operator<<(const T& msg) const {
      /*
        Print to console with the given prefix
        Example:
          cslib::Out out(L"Info: ", GREEN);
          out << "This is an info message" << std::endl;
      */
      std::wcout << prefix << msg << std::flush;
      return std::wcout;
    }
    template <typename T>
    std::wostream& operator<<(T&& msg) const {
      /*
        Print to console with the given prefix
        Example:
          cslib::Out out(L"Info: ", GREEN);
          out << "This is an info message" << std::endl;
      */
      std::wcout << prefix << std::forward<T>(msg) << std::flush;
      return std::wcout;
    }
  };



  class TimeStamp { public:
    // A wrapper around std::chrono that I have control over
    std::chrono::system_clock::time_point timePoint;
    TimeStamp() {update();}
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
    void update() {
      timePoint = std::chrono::system_clock::now();
    }
    wstr_t asWstr() const {
      /*
        Convert the time point to (lighter form of) ISO 8601
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
  void sleep_until(const TimeStamp& untilPoint) {
    // Sleep until the given time point.
    if (untilPoint.timePoint <= std::chrono::system_clock::now()) {
      throw_up("Cannot sleep until a time point in the past: ", TimeStamp().asWstr(), " (current time: ", untilPoint.asWstr(), ')');
    }
    std::this_thread::sleep_until(untilPoint.timePoint);
  }



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
  constexpr wstrsv_t fs_type_to_str(std::filesystem::file_type type) {
    switch (type) {
      case std::filesystem::file_type::regular: return L"regular file";
      case std::filesystem::file_type::directory: return L"directory";
      case std::filesystem::file_type::symlink: return L"symbolic link";
      case std::filesystem::file_type::block: return L"block device";
      case std::filesystem::file_type::character: return L"character device";
      case std::filesystem::file_type::fifo: return L"FIFO (named pipe)";
      case std::filesystem::file_type::socket: return L"socket";
      case std::filesystem::file_type::unknown: return L"unknown file type";
      default: return L"failed to determine file type";
    }
  }
  class VirtualPath { public:
    // Wrapper around std::filesystem::path

    std::filesystem::path isAt; // Covers move and copy semantics

    void assert_valid() const {
      if (!std::filesystem::exists(isAt))
        throw_up("VirtualPath ", to_ptrstr(this), " isn't initialized");
    }
    std::filesystem::file_type type() const {
      assert_valid();
      return std::filesystem::status(isAt).type();
    }
    VirtualPath parent() const {
      /*
        Returns the parent path of the path.
        Example:
          VirtualPath path("/gitstuff/cslib/cslib.h++");
          VirtualPath parent = path.parent();
          // parent = "/gitstuff/cslib"
      */
      assert_valid();
      if (isAt.parent_path().empty())
        throw_up("VirtualPath '", to_ptrstr(this), "' has somehow no parent");
      return VirtualPath(isAt.parent_path().wstring(), std::filesystem::file_type::directory);
    }
    size_t depth() const {
      /*
        Returns the depth of the path.
        Example:
          VirtualPath path("/gitstuff/cslib/cslib.h++");
          size_t depth = path.depth();
          // depth = 2 (because there are 2 directories before the file)
      */
      assert_valid();
      return separate(isAt.wstring(), PATH_DELIMITER).size - 1; // -1 for the last element
    }
    bool operator==(const VirtualPath& other) const { return this->isAt == other.isAt; }
    bool operator!=(const VirtualPath& other) const { return !(*this == other); }
    bool operator==(wstrsv_t other) const { return this->isAt == std::filesystem::path(other); }
    bool operator!=(wstrsv_t other) const { return !(*this == other); }
    bool operator==(std::string_view other) const { return this->isAt == std::filesystem::path(other);}
    bool operator!=(std::string_view other) const { return !(*this == other); }
    bool operator==(const std::filesystem::path& other) const { return this->isAt == other; }
    bool operator!=(const std::filesystem::path& other) const { return !(*this == other); }

    // Transform into stl
    operator wstr_t() const { return this->isAt.wstring(); }
    operator std::string() const { return this->isAt.string(); }
    operator std::filesystem::path() const { return this->isAt; }
    operator std::filesystem::path&() { return this->isAt; }
    operator std::filesystem::path*() { return &this->isAt; }
    operator std::filesystem::path*() const { return const_cast<std::filesystem::path*>(&this->isAt); } // const_cast is safe here

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
      TimeStamp ts(std::chrono::system_clock::from_time_t(std::mktime(timeinfo)));
      return ts;
    }
    void move_to(const VirtualPath& moveTo) {
      // Move this instance to a new location and apply changes.
      this->assert_valid();
      moveTo.assert_valid();
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
      this->assert_valid();
      targetDict.assert_valid();
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

    // Constructors
    VirtualPath() = default;
    VirtualPath(wstrsv_t where) : isAt(std::filesystem::canonical(where.data())) {}
    /*
      Constructor that takes a string and checks if it's a valid path.
      Notes:
        - If where is relative, it will be converted to an absolute path.
        - If where is empty, you will crash.
    */
    VirtualPath(wstrsv_t where, std::filesystem::file_type shouldBe) : VirtualPath(where) {
      if (this->type() != shouldBe)
        throw_up("Path '", to_ptrstr(this), "' should be of type '", fs_type_to_str(shouldBe), "', but is actually of type '", fs_type_to_str(this->type()), '\'');
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
    File(wstrsv_t where) : is(where, std::filesystem::file_type::regular) {}

    wstr_t content(std::ios_base::openmode openMode = std::ios::in) const {
      /*
        Read the content of the file and return it as a string.
        Note:
          - No error-handling for files larger than available memory
      */
      is.assert_valid();
      std::wifstream file(is.isAt, openMode);
      if (!file.is_open())
        throw_up("Failed to open file '", is.isAt.wstring(), '\'');
      if (!file.good())
        throw_up("Failed to read file '", is.isAt.wstring(), '\'');
      return wstr_t((std::istreambuf_iterator<wchar_t>(file)), std::istreambuf_iterator<wchar_t>());
    }
    wstr_t wstr() const {
      is.assert_valid();
      return is.isAt.wstring();
    }
    wstr_t extension() const {
      is.assert_valid();
      return is.isAt.extension().wstring();
    }
    size_t bytes() const {
      is.assert_valid();
      return std::filesystem::file_size(is.isAt);
    }
  };



  MACRO TEMP_FILE_HEAD = "cslibTempFile_";
  MACRO TEMP_FILE_TAIL = ".tmp";
  MACRO TEMP_FILE_NAME_LEN = 200 - (std::strlen(TEMP_FILE_HEAD) + std::strlen(TEMP_FILE_TAIL)); // Freebuffer of 55
  class TempFile { public:
    /*
      A temporary file that is created in the system's temporary directory.
      It will be deleted when the object is destroyed.
      Example:
        TempFile tempFile;
        tempFile.write("Hello World");
        std::string content = tempFile.read();
        // content = "Hello World"
    */
    File file;

    TempFile() {
      /*
        Create a temporary file in the system's temporary directory.
        The file will be deleted when the object is destroyed.
        File name is generated based on random characters
      */
      Folder tempDir(std::filesystem::temp_directory_path().wstring());
      std::string randomName;
      for (size_t i : range(TEMP_FILE_NAME_LEN)) {
        switch (roll_dice(0, 2)) {
          case 0: randomName += wchar_t(roll_dice(L'A', L'Z')); break; // Uppercase letter
          case 1: randomName += wchar_t(roll_dice(L'a', L'z')); break; // Lowercase letter
          case 2: randomName += wchar_t(roll_dice(L'0', L'9')); break; // Digit
        }
      }
      std::string tempFileName = TEMP_FILE_HEAD + randomName + TEMP_FILE_TAIL;
      file = File(tempDir.wstr() + PATH_DELIMITER + to_wstr(tempFileName));
    }
    ~TempFile() {
      if (!std::filesystem::exists(file.is.isAt)) {
        std::wcerr << L"[⚠️] Temporary file '" << file.is.isAt.wstring() << L"' was deleted before cleanup.\n";
        return;
      }
      std::filesystem::remove(file.is.isAt);
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
    Folder(wstrsv_t where) : is(where, std::filesystem::file_type::directory) {update();}
    wstr_t wstr() const {
      is.assert_valid();
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
      is.assert_valid();
      return contains(content, item);
    }
    bool has(const File& item) const {
      is.assert_valid();
      return contains(content, item.is);
    }
    bool has(const Folder& item) const {
      is.assert_valid();
      return contains(content, item.is);
    }
    void update() {
      is.assert_valid();
      content.clear();
      for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(is.isAt))
        content.push_back(VirtualPath(entry.path().wstring()));
      content.shrink_to_fit();
    }
  };
} // namespace cslib