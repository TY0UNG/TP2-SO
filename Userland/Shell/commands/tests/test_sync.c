#include "commands.h"
#include "../lib/test_util.h"
#include <stdint.h>

// Test de sincronizacion (analogo a test_mm): lanza pares de procesos que
// incrementan/decrementan una variable compartida. Con semaforo el resultado
// final debe ser 0; sin semaforo la race condition lo deja en cualquier valor.

#define SEM_ID "sem_testsync"
#define TOTAL_PAIR_PROCESSES 2

extern int  sys_create_process(const char *name, void *entry, int argc, char **argv);
extern int  sys_wait(int pid);
extern void sys_exit(int status);
extern void sys_yield(void);
extern int  sys_sem_open(const char *name, int initialValue);
extern int  sys_sem_wait(const char *name);
extern int  sys_sem_post(const char *name);
extern int  sys_sem_close(const char *name);

// Memoria compartida: todos los procesos corren la misma imagen del shell, asi
// que esta global vive una sola vez y la ven todos.
static int64_t global;

static void printSignedDec(int64_t v) {
    if (v < 0) {
        print("-");
        printDec((uint64_t)(-v));
    } else {
        printDec((uint64_t)v);
    }
}

static void slowInc(int64_t *p, int64_t inc) {
    uint64_t aux = *p;
    if (GetUniform(100) < 30)
        sys_yield();    // hace la race condition muy probable
    aux += inc;
    *p = aux;
}

static uint64_t my_process_inc(uint64_t argc, char *argv[]) {
    int64_t n, inc, use_sem;

    if (argc != 3) return (uint64_t)-1;
    if ((n = satoi(argv[0])) <= 0) return (uint64_t)-1;
    if ((inc = satoi(argv[1])) == 0) return (uint64_t)-1;
    if ((use_sem = satoi(argv[2])) < 0) return (uint64_t)-1;

    if (use_sem)
        if (sys_sem_open(SEM_ID, 1) < 0) {
            println("testsync: ERROR al abrir el semaforo");
            return (uint64_t)-1;
        }

    for (int64_t i = 0; i < n; i++) {
        if (use_sem) sys_sem_wait(SEM_ID);
        slowInc(&global, inc);
        if (use_sem) sys_sem_post(SEM_ID);
    }

    if (use_sem) sys_sem_close(SEM_ID);

    return 0;
}

// Entry point de los hijos: el kernel salta directo aca (no hay _start), asi
// que hay que terminar con sys_exit.
static int my_process_inc_entry(int argc, char **argv) {
    sys_exit((int) my_process_inc((uint64_t) argc, argv));
    return 0;   // inalcanzable
}

static void run_test_sync(char *n_str, char *sem_str) {
    uint64_t pids[2 * TOTAL_PAIR_PROCESSES];

    char *argvDec[] = { n_str, "-1", sem_str, NULL };
    char *argvInc[] = { n_str, "1",  sem_str, NULL };

    global = 0;

    for (uint64_t i = 0; i < TOTAL_PAIR_PROCESSES; i++) {
        pids[i] =
            sys_create_process("test_dec", (void *) my_process_inc_entry, 3, argvDec);
        pids[i + TOTAL_PAIR_PROCESSES] =
            sys_create_process("test_inc", (void *) my_process_inc_entry, 3, argvInc);
    }

    for (uint64_t i = 0; i < 2 * TOTAL_PAIR_PROCESSES; i++) {
        if ((int) pids[i] > 0) sys_wait((int) pids[i]);
    }

    print("Final value: ");
    printSignedDec(global);
    println("");
}

int testsync(char **argv, int argc) {
    if (argc != 3) {
        println("Uso: testsync <n> <use_sem>");
        println("  n        iteraciones por proceso (entero positivo)");
        println("  use_sem  1 usa semaforo (final 0), 0 sin semaforo (race)");
        return 1;
    }
    if (satoi(argv[1]) <= 0) {
        println("testsync: n debe ser un entero positivo");
        return 1;
    }
    if (satoi(argv[2]) < 0) {
        println("testsync: use_sem debe ser 0 o 1");
        return 1;
    }

    run_test_sync(argv[1], argv[2]);
    return 0;
}
