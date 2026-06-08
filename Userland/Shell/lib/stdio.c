#include <stdarg.h>
#include <stdint.h>
#include "stdio.h"
#include "inout.h"

int printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    const char *p = fmt;
    for (; *p != '\0'; p++) {
        if (*p != '%') {
            printChar(*p);
            continue;
        }
        p++;
        switch (*p) {
            case 'd': {
                int64_t v = va_arg(ap, int);
                if (v < 0) {
                    printChar('-');
                    printDec((uint64_t)(-v));
                } else {
                    printDec((uint64_t) v);
                }
                break;
            }
            case 'u':
                printDec((uint64_t) va_arg(ap, unsigned int));
                break;
            case 'x':
                printHex((uint64_t) va_arg(ap, unsigned int));
                break;
            case 'c':
                printChar((char) va_arg(ap, int));
                break;
            case 's':
                print(va_arg(ap, const char *));
                break;
            case '%':
                printChar('%');
                break;
            case '\0':          // '%' al final del string
                p--;            // que el for corte
                break;
            default:            // especificador desconocido: lo imprimimos tal cual
                printChar('%');
                printChar(*p);
                break;
        }
    }

    va_end(ap);
    return 0;
}
