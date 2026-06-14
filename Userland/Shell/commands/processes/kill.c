#include <commands.h>
#include <test_util.h>
#include <inout.h>
#include <stdint.h>

extern void sys_kill(int pid);

int kill(char **argv, int argc) {
    if (argc != 2) {
        println("Uso: kill <pid>");
        return 1;
    }
    int pid = satoi(argv[1]);
    if (pid <= 0) {
        println("kill: pid invalido");
        return 1;
    }
    sys_kill(pid);
    print("kill: senal enviada al proceso ");
    printDec((uint64_t) pid);
    print("\n");
    return 0;
}
