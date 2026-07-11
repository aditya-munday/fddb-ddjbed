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
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS WIN32_LEAN_AND_MEAN)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unused-parameter
    )
endif()

# ============================================================
# vcpkg Integration
# ============================================================
if(WIN32)
    # Find vcpkg toolchain file
    if(DEFINED ENV{VCPKG_ROOT})
        file(TO_CMAKE_PATH "$ENV{VCPKG_ROOT}" VCPKG_ROOT_PATH)
        set(TOOLCHAIN_FILE "${VCPKG_ROOT_PATH}/scripts/buildsystems/vcpkg.cmake")
        if(EXISTS "${TOOLCHAIN_FILE}")
            set(CMAKE_TOOLCHAIN_FILE "${TOOLCHAIN_FILE}" CACHE FILEPATH "Vcpkg toolchain")
            message(STATUS "Using vcpkg toolchain from: ${TOOLCHAIN_FILE}")
        endif()
    endif()
    
    # vcpkg packages
    find_package(opus CONFIG)
    find_package(OpenSSL CONFIG)
    find_package(ZLIB CONFIG)
    find_package(nlohmann_json CONFIG)
    find_package(dpp CONFIG)
else()
    # Linux/Unix - use pkg-config
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(OPUS REQUIRED opus)
    pkg_check_modules(OPUSFILE REQUIRED opusfile)
    
    # DPP library via FetchContent
    include(FetchContent)
    FetchContent_Declare(
        dpp
        GIT_REPOSITORY https://github.com/brainboxdotcc/DPP.git
        GIT_TAG v10.0.35
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(dpp)
endif()

# Threads
find_package(Threads REQUIRED)

# ============================================================
# Executable
# ============================================================
add_executable(discord_voice_receive
    src/main.cpp
    src/wav_writer.cpp
    src/logger.cpp
    src/config.cpp
    src/stats.cpp
)

target_include_directories(discord_voice_receive PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# ============================================================
# Link Libraries
# ============================================================
target_link_libraries(discord_voice_receive PRIVATE
    Threads::Threads
)

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

# Create output directories
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/recordings)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/logs)

# Install target
install(TARGETS discord_voice_receive DESTINATION bin)
