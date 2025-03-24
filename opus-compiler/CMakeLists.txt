# Minimum CMake version required is selected to be 3.16
# Since the project doesn't require advanced features, it would be a safe and widely supported choice
cmake_minimum_required(VERSION 3.20)

# Project name and version, referring "Programming Requirements", it states that
# "The project must be implemented in C, which offers low-level control and is well-suited for understanding
# "how language processors work."
project(Opus LANGUAGES C)

# C17 standard provides a balance between modern features and various C compilers support
# With minor bug fixes and no new features comparing with C11
set(CMAKE_C_STANDARD 17)

# Add include directory for the header files (.h)
include_directories(opus-lexer/includes opus-parser/includes opus-analyzer/includes)

# Phase 1 Lexer (lexer.c) has been added to the executable list for debugging purpose
add_executable(Opus main.c opus-lexer/src/lexer.c opus-parser/src/parser.c opus-analyzer/src/analyzer.c)

# LSP 'clangd' relies on compile_commands.json to locate header files
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
