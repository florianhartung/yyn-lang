#include <stdio.h>
#include <malloc.h>
#include "include/yyn.h"

int main(int argc, char** argv) {
    if (argc <= 1) {
        fprintf(stderr, "No yyn source file given!");
        exit(1);
    }

    yyn_compile_file(argv[1]);
    return 0;
}
