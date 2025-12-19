
cmake_minimum_required(VERSION 3.10)


project(ChessEngine CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type to Debug for debugging if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")


set(ENGINE_SOURCES
    src/core/main.cpp
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


add_executable(
    chess_engine 
    ${ENGINE_SOURCES}
)


target_include_directories(chess_engine 
    PUBLIC 
        src  
        src/data  
        src/utils
        src/core  
        tools
        tools/tests      
)
