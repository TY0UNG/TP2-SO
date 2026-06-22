#include "commands.h"
#include "../lib/time.h"
#include "../lib/test_util.h"
#include <inout.h>
#include <stdint.h>

extern uint64_t sys_getpid(void);

#define DEFAULT_INTERVAL_MS 1000

// Imprime su PID repetitivamente con un cierto intervalo.
int loop(char **argv, int argc) {
    uint64_t interval = DEFAULT_INTERVAL_MS;
    if (argc >= 2) {
        int v = satoi(argv[1]);
        if (v <= 0) {
            println("Uso: loop [intervalo_ms]  (intervalo entero positivo)");
            return 1;
        }
        interval = (uint64_t) v;
    }

    uint64_t pid = sys_getpid();

    while (1) {
        printDec(pid);
        print(" ");

        // Espera activa: gira sobre el reloj sin ceder la CPU (no bloqueante).
        //uint64_t start = getMilisFromBoot();
        int i = 100000000;
        while (i-- != 0);
    }

    return 0;   // inalcanzable
}
