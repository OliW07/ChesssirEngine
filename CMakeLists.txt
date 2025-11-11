
cmake_minimum_required(VERSION 3.10)


project(ChessEngine CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(ENGINE_SOURCES
    src/core/main.cpp
    src/utils/utils.cpp
    src/data/precomputeBitBoards.cpp
    tools/debug.cpp
)


add_executable(
    chess_engine 
    ${ENGINE_SOURCES}
)


target_include_directories(chess_engine 
    PUBLIC 
        src  
        src/data    
        tools      
)