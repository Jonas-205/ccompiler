#!/bin/bash

set -e

# Check if antlr is there
./extern/get_antlr.sh

# Generate Antlr Cpp files
mkdir -p src/antlr
if [[ src/C.g4 -nt src/antlr/CParser.h ]]; then
    echo "Generating Antlr Cpp files"
    cd src
    java -jar ../extern/antlr.jar -Dlanguage=Cpp -o antlr/ -no-listener -no-visitor -lib antlr C.g4
    cd ..
else
    echo "Antlr Cpp files are up to date"
fi

# Check Formatting
clang-format src/*.cpp src/*.hpp -i

# Configure cmake
BUILD_DIR=build
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=1
ln -sf "$BUILD_DIR"/compile_commands.json ..

# Compile
make -j 16

# Run
cd ..
clear
./"$BUILD_DIR"/ccomp examples/structs.c --dot foo.dot

# dot file
dot foo.dot -Tpdf > foo.pdf
