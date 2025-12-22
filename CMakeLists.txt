
cmake_minimum_required(VERSION 3.10)


project(ChessEngine CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo) 
endif()

set(ENGINE_LIB_SOURCES
    src/core/board.cpp
    src/core/moveGenerator.cpp
    src/core/attackHandler.cpp
    src/core/evaluate.cpp
    src/core/engine.cpp
    src/utils/utils.cpp
    src/utils/fenHelper.cpp
    src/utils/uci.cpp
    src/data/precomputeBitBoards.cpp
    tools/debug.cpp
    tools/tests/perft.cpp
)

add_library(core ${ENGINE_LIB_SOURCES})

target_include_directories(core 
    PUBLIC 
        src  
        src/data  
        src/utils
        src/core  
        tools
        tools/tests      
)

add_executable(
    chess_engine 
    src/core/main.cpp
)
target_link_libraries(chess_engine PRIVATE core)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
# Prevent GTest from overriding your compiler flags (optional but safer)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(unit_tests tools/tests/perftTest.cpp)
target_link_libraries(unit_tests PRIVATE core gtest_main)

include(GoogleTest)
gtest_discover_tests(unit_tests)
