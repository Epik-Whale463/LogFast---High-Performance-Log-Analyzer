# LogFast - A C++ Log Generator and Parser

A simple C++ application that demonstrates log generation and parsing.

## Features
- Generates random log entries with different severity levels (INFO, WARNING, ERROR, DEBUG)
- Parses and displays the log content

## Requirements
- CMake (3.10 or higher)
- C++ compiler with C++17 support (e.g., MSVC, GCC, Clang)

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
cmake --build . --config Debug
```

4. Run the program:
```
.\Debug\logfast.exe
```

### Rebuilding after code changes

After modifying source files, you only need to rebuild the project:
```
cmake --build . --config Debug
```

## How to Build and Run (Linux/macOS)

1. Create a build directory:
```
mkdir build && cd build
```

2. Run CMake:
```
cmake ..
```

3. Compile:
```
make
```

4. Run the program:
```
./logfast
```

## Project Structure
- `src/` - Source code files
  - `main.cpp` - Main entry point
  - `log_generator.cpp/h` - Log generation functionality
  - `log_parser.cpp/h` - Log parsing functionality
- `logs/` - Directory where logs are stored