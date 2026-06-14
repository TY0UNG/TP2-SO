#include "commands.h"
#include <inout.h>
#include <stdint.h>

#define BUF_SIZE 256

// copia stdin a stdout tal cual lo recibe, hasta EOF
int cat(char **argv, int argc) {
    (void) argv; (void) argc;

    char buf[BUF_SIZE];
    int n;
    while ((n = read_fd(STDIN, buf, BUF_SIZE)) > 0) {
        write_fd(STDOUT, buf, n);
    }
    return 0;
}
