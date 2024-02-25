set(antlr4_include
        antlr4/runtime/Cpp/runtime/src
        antlr4/runtime/Cpp/runtime/src/atn
        antlr4/runtime/Cpp/runtime/src/dfa
        antlr4/runtime/Cpp/runtime/src/internal
        antlr4/runtime/Cpp/runtime/src/misc
        antlr4/runtime/Cpp/runtime/src/support
        antlr4/runtime/Cpp/runtime/src/tree
        antlr4/runtime/Cpp/runtime/src/tree/pattern
        antlr4/runtime/Cpp/runtime/src/tree/xpath
)

file(GLOB libantlrcpp_SRC
        "antlr4/runtime/Cpp/runtime/src/*.cpp"
        "antlr4/runtime/Cpp/runtime/src/atn/*.cpp"
        "antlr4/runtime/Cpp/runtime/src/dfa/*.cpp"
        "antlr4/runtime/Cpp/runtime/src/internal/*.cpp"
        "antlr4/runtime/Cpp/runtime/src/misc/*.cpp"
        "antlr4/runtime/Cpp/runtime/src/support/*.cpp"
        "antlr4/runtime/Cpp/runtime/src/tree/*.cpp"
        "antlr4/runtime/Cpp/runtime/src/tree/pattern/*.cpp"
        "antlr4/runtime/Cpp/runtime/src/tree/xpath/*.cpp"
)

add_library(antlr4_shared SHARED ${libantlrcpp_SRC})
target_include_directories(antlr4_shared PUBLIC ${antlr4_include})
