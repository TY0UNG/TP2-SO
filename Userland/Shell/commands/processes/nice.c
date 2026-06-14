#include <commands.h>
#include <test_util.h>   // satoi
#include <inout.h>
#include <stdint.h>

extern void sys_nice(int pid, int priority);

#define PRIORITY_LEVELS 5

int nice(char **argv, int argc) {
    if (argc != 3) {
        println("Uso: nice <pid> <prioridad>   (prioridad 0=alta .. 4=baja)");
        return 1;
    }
    int pid = satoi(argv[1]);
    int prio = satoi(argv[2]);
    if (pid <= 0) {
        println("nice: pid invalido");
        return 1;
    }
    if (prio < 0 || prio >= PRIORITY_LEVELS) {
        println("nice: prioridad fuera de rango (0 a 4)");
        return 1;
    }
    sys_nice(pid, prio);
    print("nice: proceso ");
    printDec((uint64_t) pid);
    print(" -> prioridad ");
    printDec((uint64_t) prio);
    print("\n");
    return 0;
}
