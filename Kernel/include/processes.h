#ifndef PROCESSES_H
#define PROCESSES_H

#include <time.h>
#include <memory.h>
#include <lib.h>
#include <files.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_FDS 16

// Razon por la que un proceso esta bloqueado. Permite despertar selectivamente:
// p. ej. al cambiar el foreground solo se despierta a quien espera la terminal,
// sin tocar a los que esperan un hijo (wait_pid) o un pipe.
typedef enum {
    WAIT_NONE = 0,
    WAIT_PID,        // bloqueado en wait_pid esperando que termine un hijo
    WAIT_PIPE,       // bloqueado leyendo/escribiendo un pipe (incluye stdin)
    WAIT_SEM,        // bloqueado en sem_wait esperando un semaforo
} wait_reason_t;

typedef struct Process {
    bool active;
    bool blocked;
    bool zombie;        // termino pero todavia no fue reapeado por wait
    wait_reason_t wait_reason;  // por que esta blocked (valido solo si blocked)
    int exit_status;
    size_t pid;
    size_t parent_pid;
    size_t index;
    size_t waiting_for_pid;  // pid que este proceso esta esperando, 0 si ninguno
    int priority;
    uint64_t rsp;
    char * stack_base;
    char * stack_memory;
    char name[32];
    void (*entry_point)();
    file_t * fds[MAX_FDS];
    // Write end del pipe de stdin (fds[0] es el read end). Lo usa el hilo de
    // terminal para inyectar la entrada cocida al foreground. NULL si no tiene.
    file_t * stdin_writer;
} Process;

typedef struct {
    size_t pid;
    size_t parent_pid;
    bool active;
    bool blocked;
    bool zombie;
    int priority;
    char name[32];
    wait_reason_t wait_reason;
} ProcessInfo;

typedef size_t pid_t;

void initializeScheduler();
void scheduler();

// Tope del kernel stack sobre el que corre idle cuando no hay proceso listo.
// Hay que capturarlo (con get_rsp) ANTES de habilitar interrupciones en el
// boot: apenas se hace sti, el primer timer tick switchea al primer proceso y
// abandona el stack de main, asi que cualquier captura posterior no corre.
extern uint64_t kernel_rsp;

// Corre idle sobre el kernel stack hasta que el scheduler tenga un proceso
// listo (hace hlt). No retorna.
void idle();

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
// Bloquea el proceso actual con una razon de espera y cede la CPU.
void block_current(wait_reason_t reason);
// Despierta a pid solo si esta bloqueado por la razon dada. Devuelve true si lo
// desperto. Lo usa la terminal para despertar al nuevo foreground sin pisar a
// procesos bloqueados por otra causa.
bool unblock_if_reason(size_t pid, wait_reason_t reason);
void sleep_process(size_t pid, size_t milliseconds);
void wait_for_process(size_t pid_to_block, size_t pid_to_wait);
void yield();

Process * get_processes();
size_t get_actual_pid();
Process get_actual_process();

// Write end del stdin del proceso pid, o NULL si no existe / no esta vivo.
// Lo usa el hilo de terminal para entregar la entrada al foreground.
file_t * process_stdin_writer(pid_t pid);

int get_processesInfo(ProcessInfo *buffer, int max_count);

#endif