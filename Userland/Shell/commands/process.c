#include <commands.h>

typedef enum {
    WAIT_NONE = 0,
    WAIT_PID,        // bloqueado en wait_pid esperando que termine un hijo
    WAIT_PIPE,       // bloqueado leyendo/escribiendo un pipe (incluye stdin)
    WAIT_SEM,        // bloqueado en sem_wait esperando un semaforo
} wait_reason_t;


typedef struct {
    size_t pid;
    size_t parent_pid;
    bool active;
    bool blocked;
    bool zombie;
    int priority;
    char name[32];
    wait_reason_t wait_reason;
    char * stack_base;
    char * stack_memory;
} ProcessInfo;


extern int sys_get_process_list(ProcessInfo * buffer, int max_count);

int processList(char ** argv, int argc){
    
    ProcessInfo processes[64];

    int count = sys_get_process_list(processes, 64);

    if (count < 0) {
        println("Error obteniendo procesos\n");
        return 1;
    }

    println("PID PPID STATE NAME WAIT_REAZON BASE_POINTER STACK");

    for (int i = 0; i < count; i++) {

        const char *state =
            processes[i].zombie  ? "ZOMBIE " :
            processes[i].blocked ? "BLOCKED" :
                                   "READY  ";

        const char * wait_reazon = 
            processes[i].wait_reason == WAIT_NONE ? "WAIT_NONE" : 
            processes[i].wait_reason == WAIT_PID ? "WAIT_PID " : 
            processes[i].wait_reason == WAIT_PIPE ? "WAIT_PIPE" : 
            processes[i].wait_reason == WAIT_SEM ? "WAIT_SEM " : "";

        printDec(processes[i].pid);
        print("  ");
        printDec(processes[i].parent_pid);
        print("  ");
        print(state);
        print("  ");
        print(processes[i].name);
        print("  ");
        print(wait_reazon);
        print("  ");
        printHex((uint64_t)processes[i].stack_base);
        print("  ");
        printHex((uint64_t)processes[i].stack_memory);
        println(" ");
    }

    return 0;
}