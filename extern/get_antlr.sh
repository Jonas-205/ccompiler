#!/bin/bash

set -e

ORIG_DIR=$(pwd)
FILE_DIR=$(dirname $(readlink -f $0))
cd "$FILE_DIR"

# antlr
if [ ! -f "antlr.jar" ]; then
    wget https://www.antlr.org/download/antlr-4.13.0-complete.jar
    mv antlr-*.jar antlr.jar
fi
