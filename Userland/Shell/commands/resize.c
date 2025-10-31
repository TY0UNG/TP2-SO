#include <commands.h>

int resize(char **argsv, int argc) {
    if (argc < 2) {
        print("Usage: resize <size>\n");
        print("Size must be between 12 and 64\n");
        return 1;
    }

    int size = 0;
    char *str = argsv[1];
    while (*str) {
        if (*str < '0' || *str > '9') {
            print("Error: size must be a number\n");
            return 1;
        }
        size = size * 10 + (*str - '0');
        str++;
    }

    if (size < 12 || size > 64) {
        print("Error: size must be between 12 and 64\n");
        return 1;
    }

    setTextSize(size);
    return 0;
}