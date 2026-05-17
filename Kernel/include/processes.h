#ifndef PROCESSES_H
#define PROCESSES_H

#include <time.h>
#include <memory.h>
#include <lib.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct Process {
    bool active;
    bool blocked;
    bool zombie;        // termino pero todavia no fue reapeado por wait
    int exit_status;
    size_t pid;
    size_t index;
    size_t waiting_for_pid;  // pid que este proceso esta esperando, 0 si ninguno
    int priority;
    uint64_t rsp;
    char * stack_base;
    char * stack_memory;
    char name[32];
    void (*entry_point)();
} Process;

typedef size_t pid_t;

void initializeScheduler();
void scheduler();

// Devuelve el pid del nuevo proceso, o 0 si falla.
pid_t create_process(const char* name, void (*entry_point)(), const char** args);
void kill_process(size_t pid);
// Marca el proceso actual como zombie con el exit_status dado. No retorna.
void exit_current_process(int status);
// Bloquea hasta que el proceso pid termine. Devuelve su exit_status.
int wait_pid(pid_t pid);
// Cede CPU al scheduler. Si no hay otro proceso listo, vuelve enseguida.
void yield_current();
void modify_process_priority_by_pid(pid_t pid, int new_priority);
void block_process(size_t pid);
void unblock_process(size_t pid);
void sleep_process(size_t pid, size_t milliseconds);
void wait_for_process(size_t pid_to_block, size_t pid_to_wait);
void yield();

Process * get_processes();
size_t get_actual_pid();
Process get_actual_process();

#endif