cmake_minimum_required(VERSION 3.16)
project(threadforge CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Generate compile_commands.json so clangd / VSCode resolve includes correctly.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Strict warnings (mirrors the project .vscode settings).
add_compile_options(
    -Wall -Wextra -Wpedantic
    -Wshadow -Wformat=2
    -Wcast-align -Wnull-dereference
)

# Always need pthread on Linux for std::thread; on macOS clang it's a no-op
# but harmless.
find_package(Threads REQUIRED)

add_executable(threadforge
    src/main.cpp
    src/Task.cpp
    src/Worker.cpp
    src/ThreadPool.cpp
)

target_include_directories(threadforge PRIVATE include)
target_link_libraries(threadforge PRIVATE Threads::Threads)
