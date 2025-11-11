# CMakeLists.txt
# Set minimum version required
cmake_minimum_required(VERSION 3.10)

# Define your project name and the C++ standard to use
project(ChessEngine CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------------------------------------------------------
# 1. Gather all source files
# This is a bit of a manual process but is best practice
# to ensure CMake knows exactly what to compile.

set(ENGINE_SOURCES
    src/core/main.cpp
    src/utils/utils.cpp
    src/data/precomputeBitBoards.cpp
    tools/debug.cpp
)



# -------------------------------------------------------------
# 2. Define the Executable (the final program)
add_executable(
    chess_engine 
    ${ENGINE_SOURCES}
)

# -------------------------------------------------------------
# 3. CONFIGURE INCLUDE PATHS (The Key Step)
# This adds the -I flag to the compiler for your subdirectories.
# Use PUBLIC if you expect others to include your headers, PRIVATE 
# for headers only needed internally by this target.

target_include_directories(chess_engine 
    PUBLIC 
        src   # Contains Types.h, etc.
        src/data    # Contains precompute.h
        tools       # Contains debug.h
)