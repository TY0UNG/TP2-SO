#include "commands.h"
#include <inout.h>
#include <stdint.h>

#define BUF_SIZE 256

static int is_vowel(char c) {
    switch (c) {
        case 'a': case 'e': case 'i': case 'o': case 'u':
        case 'A': case 'E': case 'I': case 'O': case 'U':
            return 1;
        default:
            return 0;
    }
}

// copia stdin a stdout quitando las vocales, hasta EOF.
int filter(char **argv, int argc) {
    (void) argv; (void) argc;

    char in[BUF_SIZE];
    char out[BUF_SIZE];
    int n;
    while ((n = read_fd(STDIN, in, BUF_SIZE)) > 0) {
        int o = 0;
        for (int i = 0; i < n; i++)
            if (!is_vowel(in[i]))
                out[o++] = in[i];
        if (o > 0)
            write_fd(STDOUT, out, o);
    }
    return 0;
}
