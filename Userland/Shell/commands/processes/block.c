#include <commands.h>
#include <test_util.h>   // satoi
#include <inout.h>
#include <stdint.h>

extern int sys_toggle_block(int pid);

int block(char **argv, int argc) {
    if (argc != 2) {
        println("Uso: block <pid>");
        return 1;
    }
    int pid = satoi(argv[1]);
    if (pid <= 0) {
        println("block: pid invalido");
        return 1;
    }
    int state = sys_toggle_block(pid);
    if (state < 0) {
        print("block: no existe el proceso ");
        printDec((uint64_t) pid);
        print("\n");
        return 1;
    }
    print("block: proceso ");
    printDec((uint64_t) pid);
    println(state ? " -> bloqueado" : " -> listo");
    return 0;
}
