cmake_minimum_required(VERSION 3.16)
project(discord_voice_receive VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler warnings
if(MSVC)
    add_compile_options(/W3 /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find packages
find_package(Threads REQUIRED)
find_package(dpp CONFIG REQUIRED)
find_package(opus CONFIG REQUIRED)

# Executable
add_executable(discord_voice_receive src/main.cpp src/wav_writer.cpp src/logger.cpp src/config.cpp src/stats.cpp)

# Link libraries (modern CMake)
target_link_libraries(discord_voice_receive PRIVATE
    dpp::dpp
    opus::opus
    Threads::Threads
)

target_include_directories(discord_voice_receive PRIVATE ${CMAKE_SOURCE_DIR}/include)
