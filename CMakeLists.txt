cmake_minimum_required(VERSION 3.10)
project(CCompiler VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(extern)

set(EXE "ccomp")

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(SOURCES
    "${SRC_DIR}/main.cpp"
    "${SRC_DIR}/io.cpp"
    "${SRC_DIR}/args.cpp"
    "${SRC_DIR}/common.cpp"
    "${SRC_DIR}/parser.cpp"
)

set(HEADER
    "${SRC_DIR}/io.hpp"
    "${SRC_DIR}/args.hpp"
    "${SRC_DIR}/common.hpp"
    "${SRC_DIR}/preprocessor.hpp"
    "${SRC_DIR}/parser.hpp"
    "${SRC_DIR}/ast.hpp"
    "${SRC_DIR}/visitors/ASTVisitor.hpp"
    "${SRC_DIR}/visitors/ASTBaseVisitor.hpp"
    "extern/jlibc/jc_log.h"
)

set(VISITORS
    printVisitor
    dotVisitor
)

list(TRANSFORM VISITORS PREPEND "${SRC_DIR}/visitors/")

foreach(VISITOR ${VISITORS})
    list(APPEND SOURCES "${VISITOR}.cpp")
    list(APPEND HEADER "${VISITOR}.hpp")
endforeach()

set(AUTO_GENERATED_ANTLR
    src/antlr/CLexer.cpp
    src/antlr/CLexer.h
    src/antlr/CParser.cpp
    src/antlr/CParser.h
)

add_executable(${EXE} ${SOURCES} ${HEADER} ${AUTO_GENERATED_ANTLR})
target_link_libraries(${EXE} antlr4_shared)
target_include_directories(${EXE} PRIVATE "${SRC_DIR}" "extern/jlibc" ${antlr4_include})

# target_compile_options(${EXE} PRIVATE -Wall -Wextra -Wpedantic)
