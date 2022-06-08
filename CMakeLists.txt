cmake_minimum_required(VERSION 3.21)
project(yyn_lang C)

set(CMAKE_C_STANDARD 99)

add_executable(yyn_lang src/main.c src/include/token.h src/include/lexer.h src/lexer.c src/token.c src/include/list.h src/list.c src/include/yyn.h src/yyn.c src/include/util.h src/include/ast.h src/ast.c src/include/parser.h src/parser.c src/include/string_builder.h src/string_builder.c src/include/cg_nasm_win32.h src/cg_nasm_win32.c)
