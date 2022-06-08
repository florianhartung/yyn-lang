#include <stdio.h>
#include <malloc.h>
#include "include/yyn.h"
#include <windows.h>

int main(int argc, char** argv) {
    if (argc <= 1) {
        fprintf(stderr, "No yyn source file given!");
        exit(1);
    }


    char* out_filename;
    if(argc == 3) {
        out_filename = argv[2];
    } else {
        out_filename = calloc(strlen(argv[1]) + 5, sizeof(char));
        strcpy(out_filename, argv[1]);
        strcat(out_filename, ".asm");
    }

    yyn_compile_file(argv[1], out_filename);
    return 0;
}
