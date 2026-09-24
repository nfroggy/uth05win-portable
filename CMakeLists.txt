cmake_minimum_required(VERSION 3.24)
project(uth05win LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(SDL3 3.2 REQUIRED CONFIG)
find_package(OpenGL REQUIRED)
find_package(Freetype REQUIRED)
find_package(Fontconfig REQUIRED)
find_package(Iconv REQUIRED)

# PMDWin's portable decoder; no SDL2 player or Windows DLL is built.
set(PMDMINI_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/pmdmini")
if(NOT EXISTS "${PMDMINI_SOURCE_DIR}/src/pmdwin/pmdwin.cpp")
    message(FATAL_ERROR
        "The pmdmini submodule is missing.\n"
        "Run: git submodule update --init --recursive")
endif()
file(GLOB PMD_SOURCES CONFIGURE_DEPENDS
    "${PMDMINI_SOURCE_DIR}/src/fmgen/*.cpp"
    "${PMDMINI_SOURCE_DIR}/src/pmdwin/*.cpp")
add_library(pmd_decoder STATIC ${PMD_SOURCES})
target_include_directories(pmd_decoder SYSTEM PUBLIC
    "${PMDMINI_SOURCE_DIR}/src/fmgen" "${PMDMINI_SOURCE_DIR}/src/pmdwin")
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(pmd_decoder PRIVATE -fpermissive -Wno-write-strings)
endif()

file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS "Game/*.cpp" "gameLib/*.cpp")
add_library(uth05_game STATIC ${GAME_SOURCES})
add_executable(uth05win main.cpp)
target_link_libraries(uth05win PRIVATE uth05_game)
target_link_libraries(uth05_game PUBLIC SDL3::SDL3 OpenGL::GL OpenGL::GLU
    Freetype::Freetype Fontconfig::Fontconfig Iconv::Iconv pmd_decoder)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Legacy resource readers use unaligned type-punning on little-endian data.
    target_compile_options(uth05_game PRIVATE -fno-strict-aliasing -Wno-write-strings)
endif()

option(UTH05_BUILD_TESTS "Build the asset-dependent graphics/audio/gameplay smoke test" OFF)
if(UTH05_BUILD_TESTS)
    add_executable(port_smoke tests/port_smoke.cpp)
    target_include_directories(port_smoke PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(port_smoke PRIVATE uth05_game)
endif()
