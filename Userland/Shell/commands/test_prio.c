#include "commands.h"
#include <stdint.h>
#include <stdio.h>
#include "../lib/test_util.h"

// Wrappers a las syscalls de este SO.
extern int      sys_create_process(const char *name, void *entry, int argc, char **argv);
extern int      sys_wait(int pid);
extern int      sys_block(int pid);
extern int      sys_unblock(int pid);
extern void     sys_nice(int pid, int priority);
extern uint64_t sys_getpid(void);
extern void     sys_exit(int status);

#define TOTAL_PROCESSES 3

// Prioridades
#define LOWEST 2
#define MEDIUM 1
#define HIGHEST 0

// una funcion void quem NO llama sys_exit.
static void zero_to_max(void);
static void zero_to_max_entry(int argc, char **argv) {
    (void) argc; (void) argv;
    zero_to_max();
    sys_exit(0);
}

static int64_t my_create_process(char *name, int argc, char *argv[]) {
    void *entry = 0;
    if (strcmp(name, "zero_to_max") == 0)
        entry = (void *) zero_to_max_entry;
    if (entry == 0)
        return -1;
    return sys_create_process(name, entry, argc, argv);
}

static int64_t my_wait(int64_t pid)               { return sys_wait((int) pid); }
static int64_t my_block(int64_t pid)              { return sys_block((int) pid); }
static int64_t my_unblock(int64_t pid)            { return sys_unblock((int) pid); }
static void    my_nice(int64_t pid, int64_t prio) { sys_nice((int) pid, (int) prio); }
static int64_t my_getpid(void)                    { return (int64_t) sys_getpid(); }

// ======================= test de la catedra =======================

int64_t prio[TOTAL_PROCESSES] = {LOWEST, MEDIUM, HIGHEST};

uint64_t max_value = 0;

static void zero_to_max(void) {
  uint64_t value = 0;

  while (value++ != max_value);

  printf("PROCESS %d DONE!\n", my_getpid());
}

uint64_t test_prio(uint64_t argc, char *argv[]) {
  int64_t pids[TOTAL_PROCESSES];
  char *ztm_argv[] = {0};
  uint64_t i;

  if (argc != 1)
    return -1;

  if ((max_value = satoi(argv[0])) <= 0)
    return -1;

  printf("SAME PRIORITY...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    pids[i] = my_create_process("zero_to_max", 0, ztm_argv);

  // Expect to see them finish at the same time

  for (i = 0; i < TOTAL_PROCESSES; i++)
    my_wait(pids[i]);

  printf("SAME PRIORITY, THEN CHANGE IT...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++) {
    pids[i] = my_create_process("zero_to_max", 0, ztm_argv);
    my_nice(pids[i], prio[i]);
    printf("  PROCESS %d NEW PRIORITY: %d\n", pids[i], prio[i]);
  }

  // Expect the priorities to take effect

  for (i = 0; i < TOTAL_PROCESSES; i++)
    my_wait(pids[i]);

  printf("SAME PRIORITY, THEN CHANGE IT WHILE BLOCKED...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++) {
    pids[i] = my_create_process("zero_to_max", 0, ztm_argv);
    my_block(pids[i]);
    my_nice(pids[i], prio[i]);
    printf("  PROCESS %d NEW PRIORITY: %d\n", pids[i], prio[i]);
  }

  for (i = 0; i < TOTAL_PROCESSES; i++)
    my_unblock(pids[i]);

  // Expect the priorities to take effect

  for (i = 0; i < TOTAL_PROCESSES; i++)
    my_wait(pids[i]);

  return 0;
}

// ============================= comando del shell =============================

int testprio(char **argv, int argc) {
    if (argc != 2) {
        printf("Uso: testprio <max_value>\n");
        return 1;
    }
    char *args[] = { argv[1] };
    return (int) test_prio(1, args);
}
