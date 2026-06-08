#include "commands.h"
#include <stdio.h>
#include <stdint.h>
#include "../lib/test_util.h"

// Wrappers a las syscalls de este SO.
extern int  sys_create_process(const char *name, void *entry, int argc, char **argv);
extern int  sys_wait(int pid);
extern int  sys_kill(int pid);
extern int  sys_block(int pid);
extern int  sys_unblock(int pid);

// Este SO crea procesos por puntero a funcion, no por nombre como la catedra.
// Mapeamos los nombres que usan los tests a su entry point.
static int64_t my_create_process(char *name, int argc, char *argv[]) {
    void *entry = 0;
    if (strcmp(name, "endless_loop") == 0)
        entry = (void *) endless_loop;
    else if (strcmp(name, "endless_loop_print") == 0)
        entry = (void *) endless_loop_print;
    if (entry == 0)
        return -1;
    return sys_create_process(name, entry, argc, argv);
}

static int64_t my_kill(int64_t pid)    { return sys_kill((int) pid); }
static int64_t my_block(int64_t pid)   { return sys_block((int) pid); }
static int64_t my_unblock(int64_t pid) { return sys_unblock((int) pid); }
static int64_t my_wait(int64_t pid)    { return sys_wait((int) pid); }

// ======================= test de la catedra (verbatim) =======================

enum State { RUNNING,
             BLOCKED,
             KILLED };

typedef struct P_rq {
  int32_t pid;
  enum State state;
} p_rq;

int64_t test_processes(uint64_t argc, char *argv[]) {
  uint8_t rq;
  uint8_t alive = 0;
  uint8_t action;
  uint64_t max_processes;
  char *argvAux[] = {0};

  if (argc != 1)
    return -1;

  if ((max_processes = satoi(argv[0])) <= 0)
    return -1;

  p_rq p_rqs[max_processes];

  while (1) {

    // Create max_processes processes
    for (rq = 0; rq < max_processes; rq++) {
      p_rqs[rq].pid = my_create_process("endless_loop", 0, argvAux);

      if (p_rqs[rq].pid == -1) {
        printf("test_processes: ERROR creating process\n");
        return -1;
      } else {
        p_rqs[rq].state = RUNNING;
        alive++;
      }
    }

    // Randomly kills, blocks or unblocks processes until every one has been killed
    while (alive > 0) {

      for (rq = 0; rq < max_processes; rq++) {
        action = GetUniform(100) % 2;

        switch (action) {
          case 0:
            if (p_rqs[rq].state == RUNNING || p_rqs[rq].state == BLOCKED) {
              if (my_kill(p_rqs[rq].pid) == -1) {
                printf("test_processes: ERROR killing process\n");
                return -1;
              }
              p_rqs[rq].state = KILLED;
              my_wait(p_rqs[rq].pid);
              alive--;
            }
            break;

          case 1:
            if (p_rqs[rq].state == RUNNING) {
              if (my_block(p_rqs[rq].pid) == -1) {
                printf("test_processes: ERROR blocking process\n");
                return -1;
              }
              p_rqs[rq].state = BLOCKED;
            }
            break;
        }
      }

      // Randomly unblocks processes
      for (rq = 0; rq < max_processes; rq++)
        if (p_rqs[rq].state == BLOCKED && GetUniform(100) % 2) {
          if (my_unblock(p_rqs[rq].pid) == -1) {
            printf("test_processes: ERROR unblocking process\n");
            return -1;
          }
          p_rqs[rq].state = RUNNING;
        }
    }
  }
}

// ============================= comando del shell =============================

int testprocesses(char **argv, int argc) {
    if (argc != 2) {
        printf("Uso: testprocesses <max_processes>\n");
        return 1;
    }
    char *args[] = { argv[1] };
    test_processes(1, args);   // stress test infinito (no retorna salvo error)
    return 0;
}
