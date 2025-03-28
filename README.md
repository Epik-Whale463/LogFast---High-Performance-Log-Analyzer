# LogFast - A High-Performance Log Generator and Parser

A C++ application that demonstrates high-performance log generation and parsing using modern C++17 features and multi-threading.

## Features
- **High-Performance**: Utilizes modern C++17 multithreading capabilities for maximum performance
- **Auto-Configuration**: Automatically detects optimal thread count based on hardware
- **Scalable**: Efficiently handles millions of log entries with minimal memory footprint
- **Customizable**: Command-line options for log count, thread count and output file
- **Flexible**: Generates random log entries with different severity levels (INFO, WARNING, ERROR, DEBUG)
- **Detailed Analytics**: Provides comprehensive performance statistics and log level distribution
- **Thread Pool**: Uses a custom thread pool for optimal workload distribution
- **Buffer Management**: Implements efficient buffered I/O to minimize disk operations

## Requirements
- CMake (3.10 or higher)
- C++ compiler with C++17 support (e.g., MSVC, GCC, Clang)
- Standard threading library support

## How to Build and Run (Windows)

### Using Visual Studio

1. Create a build directory:
```
mkdir build
cd build
```

2. Run CMake to generate Visual Studio project files:
```
cmake ..
```

3. Build the project:
```
cmake --build . --config Release
```

4. Run the program with defaults:
```
.\Release\logfast.exe
```

5. Run with custom parameters (optional):
```
.\Release\logfast.exe [log_count] [thread_count] [filename]
```

### Rebuilding after code changes

After modifying source files, you only need to rebuild the project:
```
cmake --build . --config Release
```

## How to Build and Run (Linux/macOS)

1. Create a build directory:
```
mkdir build && cd build
```

2. Run CMake:
```
cmake .. -DCMAKE_BUILD_TYPE=Release
```

3. Compile:
```
make
```

4. Run the program:
```
./logfast [log_count] [thread_count] [filename]
```

## Performance Tuning

The application automatically selects the optimal number of threads based on your hardware. For best performance:

1. Use Release build configuration
2. Run with a log count appropriate for your system (default: 100,000)
3. For extremely large log files, consider adjusting thread count manually

## Project Structure
- `src/` - Source code files
  - `main.cpp` - Main entry point and statistics reporting
  - `log_generator.cpp/h` - Multithreaded log generation with thread pool
  - `log_parser.cpp/h` - Parallel log parsing with statistics collection
- `logs/` - Directory where logs are stored

## Architecture
The application uses a producer-consumer architecture with thread pools for both log generation and parsing. It implements custom thread-safe queues and lock-free operations where possible to maximize throughput.