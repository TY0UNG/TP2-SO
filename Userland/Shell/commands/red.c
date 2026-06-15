#include "commands.h"
#include <inout.h>
#include <stdint.h>

#define BUF_SIZE 256
#define RED_STYLE 0x0C  // rojo claro sobre negro (ver getHexColor)

// copia stdin a stdout pintado de rojo, hasta EOF.
int red(char **argv, int argc) {
    (void) argv; (void) argc;

    selectStyle(RED_STYLE);   // todo lo que escriba este proceso sale en rojo

    char buf[BUF_SIZE];
    int n;
    while ((n = read_fd(STDIN, buf, BUF_SIZE)) > 0) {
        write_fd(STDOUT, buf, n);
    }
    return 0;
}
