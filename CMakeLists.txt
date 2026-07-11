cmake_minimum_required(VERSION 3.16)
project(discord_voice_receive VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler warnings
if(MSVC)
    add_compile_options(
        /W3
        /utf-8
        /permissive-
    )
    # Disable some warnings that DPP triggers
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS WIN32_LEAN_AND_MEAN)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unused-parameter
    )
endif()

# Try to use vcpkg if available (Windows)
if(WIN32)
    # Check for vcpkg integration
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg.json")
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain")
        message(STATUS "Using vcpkg toolchain: $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    endif()
    
    # Try to find packages via vcpkg first
    find_package(opus CONFIG)
    find_package(OpenSSL CONFIG)
    find_package(ZLIB CONFIG)
    find_package(dpp CONFIG)
else()
    # Linux/Unix - use pkg-config
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(OPUS REQUIRED opus)
    pkg_check_modules(OPUSFILE REQUIRED opusfile)
    
    # DPP library - FetchContent for simplicity on Linux
    include(FetchContent)
    FetchContent_Declare(
        dpp
        GIT_REPOSITORY https://github.com/brainboxdotcc/DPP.git
        GIT_TAG v10.0.35
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(dpp)
endif()

# Threads (always needed)
find_package(Threads REQUIRED)

# Create executable
add_executable(discord_voice_receive
    src/main.cpp
    src/wav_writer.cpp
    src/logger.cpp
    src/config.cpp
    src/stats.cpp
)

# Include directories
target_include_directories(discord_voice_receive PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Platform-specific includes
if(WIN32)
    if(TARGET opus::opus)
        target_include_directories(discord_voice_receive PRIVATE ${opus_INCLUDE_DIRS})
    endif()
    if(TARGET OpenSSL::SSL)
        target_include_directories(discord_voice_receive PRIVATE ${OPENSSL_INCLUDE_DIR})
    endif()
else()
    target_include_directories(discord_voice_receive PRIVATE
        ${OPUS_INCLUDE_DIRS}
        ${OPUSFILE_INCLUDE_DIRS}
    )
endif()

# Link libraries
target_link_libraries(discord_voice_receive PRIVATE
    Threads::Threads
)

# Platform-specific link libraries
if(WIN32)
    if(TARGET dpp::dpp)
        target_link_libraries(discord_voice_receive PRIVATE dpp::dpp)
    endif()
    if(TARGET opus::opus)
        target_link_libraries(discord_voice_receive PRIVATE opus::opus)
    endif()
    if(TARGET OpenSSL::SSL)
        target_link_libraries(discord_voice_receive PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    endif()
    if(TARGET ZLIB::ZLIB)
        target_link_libraries(discord_voice_receive PRIVATE ZLIB::ZLIB)
    endif()
else()
    target_link_libraries(discord_voice_receive PRIVATE
        dpp
        ${OPUS_LIBRARIES}
        ${OPUSFILE_LIBRARIES}
    )
endif()

# Install target
install(TARGETS discord_voice_receive DESTINATION bin)

# Create directories for build
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/recordings)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/logs)

# Print configuration info
message(STATUS "===========================================")
message(STATUS "Discord Voice Receive - Build Configuration")
message(STATUS "===========================================")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "===========================================")
