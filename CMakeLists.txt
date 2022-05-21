cmake_minimum_required(VERSION 3.21)
project(yyn_lang C)

set(CMAKE_C_STANDARD 99)

add_executable(yyn_lang src/main.c src/include/token.h src/include/lexer.h src/lexer.c src/token.c src/include/list.h src/list.c)
