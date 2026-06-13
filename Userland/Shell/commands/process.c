#include <commands.h>

typedef struct {
    size_t pid;
    size_t parent_pid;
    bool active;
    bool blocked;
    bool zombie;
    int priority;
    char name[32];
} ProcessInfo;

extern int sys_get_process_list(ProcessInfo * buffer, int max_count);

int processList(char ** argv, int argc){
    
    ProcessInfo processes[64];

    int count = sys_get_process_list(processes, 64);

    if (count < 0) {
        println("Error obteniendo procesos\n");
        return 1;
    }

    println("PID PPID STATE NAME");

    for (int i = 0; i < count; i++) {

        const char *state =
            processes[i].zombie  ? "ZOMBIE" :
            processes[i].blocked ? "BLOCKED" :
                                   "READY";

        printDec(processes[i].pid);
        print("  ");
        printDec(processes[i].parent_pid);
        print("  ");
        print(state);
        print("  ");
        println(processes[i].name);
    }

    return 0;
}