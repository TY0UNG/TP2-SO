#include <stdint.h>
#include "./process.h"

extern int sys_create_process(const char* name, int (*entry)(char**, int), int argc , char ** argv);

//Realiza una syscall un proceso, retorna si PID. 
int createProcess(const char* name, int (*entry)(char**, int), int argc , char ** argv){
    return sys_create_process(name, entry , argc, argv);
}