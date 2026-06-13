#include "commands.h"
#include <inout.h>
#include <stdint.h>

#define BUF_SIZE 256

// cuenta la cantidad de lineas del input (numero de '\n'), hasta EOF.
int wc(char **argv, int argc) {
    (void) argv; (void) argc;

    char buf[BUF_SIZE];
    int n;
    uint64_t lines = 0;
    while ((n = read_fd(STDIN, buf, BUF_SIZE)) > 0) {
        for (int i = 0; i < n; i++)
            if (buf[i] == '\n')
                lines++;
    }

    printDec(lines);
    print("\n");
    return 0;
}
