
cmake_minimum_required(VERSION 3.10)


project(ChessEngine CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -gdwarf-4")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Zi")
    #fix msvc warnings
    add_compile_options(/W4)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo) 
endif()

add_library(core 
    src/core/board.cpp
    src/core/moveGenerator.cpp
    src/core/moveHandler.cpp
    src/core/attackHandler.cpp
    src/core/transpositionTable.cpp
    src/core/evaluate.cpp
    src/core/engine.cpp
    src/utils/utils.cpp
    src/utils/fenHelper.cpp
    src/utils/uci.cpp
    src/utils/log.cpp
    src/data/precomputeBitBoards.cpp
    src/data/zobrist.cpp
    tools/debug.cpp
    tools/perft.cpp
)

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

    add_executable(
        unit_tests 
        tools/tests/perftTest.cpp
        tools/tests/searchTest.cpp
        tools/tests/drawTest.cpp
        tools/tests/zobristTest.cpp
        tools/tests/TTTest.cpp
        tools/tests/evalTest.cpp
        tools/tests/moveOrderTest.cpp
        
    )
    target_link_libraries(unit_tests PRIVATE core gtest_main)


    include(GoogleTest)
  
