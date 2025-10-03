# <b><i>C</i></b>elestiarcanum <b><i>S</i></b>ymphony Library 🧌
A comprehensive single-header utility library that provides convenient wrappers and helpers for common programming tasks. Just include and use, no linking required!

#
## Usage Examples

### String Operations
```cpp
using namespace cslib;

// Convert various types to string
str_t text = to_str("Hello ", 42, " world!");

// Trim strings
str_t long_text = "This is a very long string";
str_t trimmed = rtrim(long_text, 10);  // "This is..."

// Split strings
std::vector<str_t> parts = separate("a,b,c", ",");  // {"a", "b", "c"}
```

### Filesystem Operations
```cpp
// Work with files
File config("/etc/config.txt");
str_t content = config.read_text();
config.edit_text("new content");

// Navigate folders
Folder project("/home/user/project");
if (std::optional<Road> file = project.has("README.md"))
    std::cout << *file << "exists";

// Create temporary files (auto-deleted)
{
    TempFile temp;
    temp.edit_text("temporary data");
    // File automatically deleted when temp goes out of scope
}
```

### Colored Console Output
```cpp
Out error(std::cerr, "ERROR:", Red);
error << "Something went wrong!";
// Outputs: [timestamp] ERROR: Something went wrong! (in red)

Out info(std::cout, "INFO:", Green);
info << "Operation successful";
```

### Benchmarking
```cpp
Benchmark timer;
// ... do some work ...
std::cout << "Elapsed: " << timer.elapsed_ms() << " ms\n";
```

### Parallel Execution
```cpp
std::future<T> future = do_in_parallel([]() {
    // Long running task
    return compute_result();
});
// Do other work...
T result = future.get();
```

### System Commands
```cpp
std::expected<str_t, int> result = sh_call("ls -la");
if (result)
    std::cout << "Output: " << result.value();
else
    std::cout << "Command failed with code: " << result.error();
```

### HTTP Downloads
```cpp
std::expected<str_t, int> response = get("https://api.example.com/data");
if (response)
    // Process response.value()
```

#
## Features
### 🎯 Core Utilities
- **String Operations**: Conversion helpers, trimming, splitting, and formatting
- **Container Utilities**: Stringify containers, check element presence, find common elements
- **Random Number Generation**: Simple dice-roll style random numbers
- **Benchmarking**: High-resolution time measurement for performance testing
- **Retry Mechanism**: Automatic retry logic for fallible operations
### 📁 Filesystem Abstractions
- **Road/File/Folder Classes**: Object-oriented filesystem navigation
- **Temporary Files/Folders**: RAII-managed temporary filesystem entries
- **Path Operations**: Cross-platform path handling with automatic OS detection
### 🔧 System & I/O
- **Shell Command Execution**: Run system commands and capture output
- **Stream Configuration Guard**: RAII-based stream state preservation
- **Colored Console Output**: Built-in ANSI color support for terminal output
- **Web Downloads**: Simple HTTP GET requests via curl
### ⏰ Time & Date
- **TimeStamp Class**: Convenient wrapper around std::chrono
- **Date/Time Formatting**: ISO 8601-style formatting and component extraction
### 🚀 Concurrency
- **Parallel Execution**: Simple async task launching with futures
- **Thread Utilities**: Sleep/pause functions

#
## Requirements
- C++20 or newer compiler
- Standard library support for:
  - `<filesystem>`
  - `<ranges>`
  - `<expected>` (C++23 feature)
- Optional: `curl` for web download functionality

#
## Installation
Simply include the header file in your project:
```cpp
#include "cslib.h++"
```
The library uses `#pragma once` and compile-time C++ version checking to ensure compatibility.


#
## Design Philosophy
- **Single Header**: No build configuration needed
- **Modern C++**: Leverages C++20 features extensively
- **Cross-Platform**: Automatic detection of Windows vs Unix-like systems
- **RAII Everywhere**: Resource management through constructors/destructors
- **No External Dependencies**: Core functionality uses only the standard library

#
## Type Aliases

The library provides convenient type aliases:
- `str_t` → `std::string`
- `strv_t` → `std::string_view`
- `sptr<T>` → `std::shared_ptr<T>`
- `uptr<T>` → `std::unique_ptr<T>`
- `stdfs` → `std::filesystem`

#
## Platform-Specific Features
The library automatically detects the platform:
- `IS_WINDOWS` - Compile-time constant for platform detection
- `PATH_SEPARATOR` - Automatically set to `\` on Windows, `/` on Unix

#
## Notes & Warnings
⚠️ **Security Considerations**:
- `sh_call()` and `get()` functions are vulnerable to command injection - never use with untrusted input
- The library includes extensive exception handling but not all functions are `noexcept`

⚠️ **Experimental Features**:
- Uses C++23's `std::expected` - may require newer compilers
- Some features require external tools (curl for web downloads)

#
## Error Handling
The library uses a mix of error handling strategies:
- Exceptions for critical errors (file not found, invalid arguments)
- `std::expected` for operations that commonly fail (shell commands, web requests)
- `std::optional` for queries that may have no result

#
## License
MIT

#
## Contributing
Go wild

#
## Known Quirks
- Heavy template usage may increase compile times