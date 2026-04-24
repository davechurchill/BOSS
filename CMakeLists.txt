cmake_minimum_required(VERSION 3.20)

# Default to Release if no build type is specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

project(BOSS CXX)

# Output all executables to bin/ regardless of config (Debug/Release/etc.)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY         "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── BOSS core static library ──────────────────────────────────────────────────
# Re-run CMake when source files are added or removed.
file(GLOB BOSS_CORE_SOURCES   src/BOSS/*.cpp)
file(GLOB BOSS_SEARCH_SOURCES src/search/*.cpp)

add_library(BOSS STATIC
    ${BOSS_CORE_SOURCES}
    ${BOSS_SEARCH_SOURCES}
)

target_include_directories(BOSS PUBLIC
    src
    src/BOSS
    src/search
    src/json
)

target_compile_definitions(BOSS PUBLIC NOMINMAX)

if(MSVC)
    target_compile_options(BOSS PUBLIC
        /W3
        /wd4996   # suppress 'deprecated' (matches VS project DisableSpecificWarnings)
    )
    target_compile_definitions(BOSS PUBLIC _CONSOLE)
else()
    target_compile_options(BOSS PUBLIC -Wall -Wno-deprecated-declarations)
endif()

# ── BOSS_Test ─────────────────────────────────────────────────────────────────
add_executable(BOSS_Test
    src/test/catch2/catch_amalgamated.cpp
    src/test/main_test.cpp
)

target_include_directories(BOSS_Test PRIVATE src/test)
target_link_libraries(BOSS_Test PRIVATE BOSS)

# ── BOSS_Experiments ──────────────────────────────────────────────────────────
add_executable(BOSS_Experiments
    src/experiments/BOSSExperiments.cpp
    src/experiments/CombatSearchExperiment.cpp
    src/experiments/DFBBSearchExperiment.cpp
    src/experiments/main_experiments.cpp
)

target_include_directories(BOSS_Experiments PRIVATE src/experiments)
target_link_libraries(BOSS_Experiments PRIVATE BOSS)

# ── BOSS_SFML ─────────────────────────────────────────────────────────────────
# Point CMake at your SFML installation via -DSFML_DIR=<path>/lib/cmake/SFML
# or the SFML_DIR environment variable that the VS project uses.
if(DEFINED ENV{SFML_DIR} AND NOT DEFINED SFML_DIR)
    set(SFML_DIR "$ENV{SFML_DIR}/lib/cmake/SFML")
endif()

find_package(SFML 2 QUIET COMPONENTS graphics window system audio network)

if(SFML_FOUND)
    add_executable(BOSS_SFML src/sfml/main_sfml.cpp)
    target_include_directories(BOSS_SFML PRIVATE src/sfml)
    target_link_libraries(BOSS_SFML PRIVATE
        BOSS
        sfml-graphics
        sfml-window
        sfml-system
        sfml-audio
        sfml-network
    )
else()
    message(STATUS "SFML not found -- skipping BOSS_SFML target. "
                   "Set SFML_DIR to your SFML cmake directory to enable it.")
endif()
