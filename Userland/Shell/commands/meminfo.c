#include <commands.h>
#include "../lib/memory.h"

int meminfo(char **argv, int argc) {
    (void)argv;
    (void)argc;

    size_t total = getTotalMemory();
    size_t used = getUsedMemory();

    print("Memoria total: ");
    printDec(total);
    println(" bytes");

    print("Memoria usada: ");
    printDec(used);
    println(" bytes");

    return 0;
}