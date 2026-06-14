#include <semaphores.h>
#include <processes.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_SEMAPHORES   64
// Cada proceso se encola a lo sumo una vez (dedup en is_waiting), asi que con
// PROCESSES_LIMIT la cola nunca desborda.
#define MAX_SEM_WAITERS  PROCESSES_LIMIT
#define SEM_NAME_MAX     32

// Estructura con la informacion de cada semaforo: nombre (su id), el entero
// del semaforo y la cola de procesos esperando.
typedef struct semaphore_t {
    bool   in_use;                       // entrada de la tabla ocupada
    char   name[SEM_NAME_MAX];           // nombre: identifica al semaforo
    int    value;                        // entero del semaforo
    int    openers;                      // procesos que lo tienen abierto

    pid_t  waiters[MAX_SEM_WAITERS];     // cola FIFO de procesos bloqueados
    size_t waiter_head;
    size_t waiter_tail;
    size_t waiter_count;
} semaphore_t;

extern Process processes[];
extern size_t actual_index;

// Primitivas de exclusion mutua implementadas con TSL (XCHG) + irqsave en
// assembler. enter_region devuelve los RFLAGS previos; leave_region los
// restaura.
extern uint64_t enter_region(volatile uint8_t * lock);
extern void leave_region(volatile uint8_t * lock, uint64_t flags);

static semaphore_t sem_table[MAX_SEMAPHORES];

// Un unico lock protege la tabla de semaforos. Los tramos criticos son cortos
// (modificar un contador, encolar/desencolar un waiter), por lo que el
// busy-wait de enter_region es aceptable.
static volatile uint8_t sem_lock = 0;

static int sem_strcmp(const char * a, const char * b) {
    while (*a != 0 && *a == *b) {
        a++;
        b++;
    }
    return (int)(uint8_t)*a - (int)(uint8_t)*b;
}

static void sem_strncpy(char * dest, const char * src, size_t n) {
    size_t i = 0;
    while (i < n - 1 && src[i] != 0) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = 0;
}

// Devuelve el indice del semaforo con ese nombre, o -1 si no existe.
// Debe llamarse con sem_lock tomado.
static int find_by_name(const char * name) {
    for (int i = 0; i < MAX_SEMAPHORES; i++) {
        if (sem_table[i].in_use && sem_strcmp(sem_table[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

static int find_free_slot(void) {
    for (int i = 0; i < MAX_SEMAPHORES; i++) {
        if (!sem_table[i].in_use) return i;
    }
    return -1;
}

static void reset_waiters(semaphore_t * s) {
    s->waiter_head = 0;
    s->waiter_tail = 0;
    s->waiter_count = 0;
}

static bool is_waiting(semaphore_t * s, pid_t pid) {
    for (size_t i = 0, idx = s->waiter_head; i < s->waiter_count; i++) {
        if (s->waiters[idx] == pid) return true;
        idx = (idx + 1) % MAX_SEM_WAITERS;
    }
    return false;
}

static void enqueue_waiter(semaphore_t * s, pid_t pid) {
    if (s->waiter_count >= MAX_SEM_WAITERS) return;
    if (is_waiting(s, pid)) return;
    s->waiters[s->waiter_tail] = pid;
    s->waiter_tail = (s->waiter_tail + 1) % MAX_SEM_WAITERS;
    s->waiter_count++;
}

static bool dequeue_waiter(semaphore_t * s, pid_t * out) {
    if (s->waiter_count == 0) return false;
    *out = s->waiters[s->waiter_head];
    s->waiter_head = (s->waiter_head + 1) % MAX_SEM_WAITERS;
    s->waiter_count--;
    return true;
}

// Inicializa la entrada con nombre, valor y un unico opener. Asume sem_lock.
static void init_slot(int id, const char * name, int initialValue) {
    semaphore_t * s = &sem_table[id];
    s->in_use = true;
    sem_strncpy(s->name, name, SEM_NAME_MAX);
    s->value = initialValue;
    s->openers = 1;
    reset_waiters(s);
}

int sem_init(const char * name, int initialValue) {
    if (name == NULL || name[0] == 0) return -1;

    uint64_t flags = enter_region(&sem_lock);

    if (find_by_name(name) >= 0) {
        // Ya existe: sem_init no debe re-crearlo.
        leave_region(&sem_lock, flags);
        return -1;
    }

    int id = find_free_slot();
    if (id < 0) {
        leave_region(&sem_lock, flags);
        return -1;
    }

    init_slot(id, name, initialValue);
    leave_region(&sem_lock, flags);
    return 0;
}

int sem_open(const char * name, int initialValue) {
    if (name == NULL || name[0] == 0) return -1;

    uint64_t flags = enter_region(&sem_lock);

    int id = find_by_name(name);
    if (id >= 0) {
        // Ya existe: solo lo abrimos otra vez.
        sem_table[id].openers++;
        leave_region(&sem_lock, flags);
        return 0;
    }

    id = find_free_slot();
    if (id < 0) {
        leave_region(&sem_lock, flags);
        return -1;
    }

    init_slot(id, name, initialValue);
    leave_region(&sem_lock, flags);
    return 0;
}

int sem_wait(const char * name) {
    if (name == NULL) return -1;

    uint64_t flags = enter_region(&sem_lock);
    int id = find_by_name(name);
    if (id < 0) {
        leave_region(&sem_lock, flags);
        return -1;
    }
    semaphore_t * s = &sem_table[id];

    // Mientras no haya recurso disponible, bloquearse y reintentar al despertar.
    while (s->value <= 0) {
        enqueue_waiter(s, get_actual_pid());
        processes[actual_index].blocked = true;
        processes[actual_index].wait_reason = WAIT_SEM;
        leave_region(&sem_lock, flags);
        scheduler();            // cede la CPU hasta que un sem_post lo despierte
        flags = enter_region(&sem_lock);
        // El semaforo podria haberse cerrado mientras estabamos bloqueados.
        if (!s->in_use) {
            leave_region(&sem_lock, flags);
            return -1;
        }
    }

    s->value--;
    leave_region(&sem_lock, flags);
    return 0;
}

int sem_post(const char * name) {
    if (name == NULL) return -1;

    uint64_t flags = enter_region(&sem_lock);
    int id = find_by_name(name);
    if (id < 0) {
        leave_region(&sem_lock, flags);
        return -1;
    }
    semaphore_t * s = &sem_table[id];

    s->value++;

    pid_t pid;
    if (dequeue_waiter(s, &pid)) {
        unblock_process(pid);
    }

    leave_region(&sem_lock, flags);
    return 0;
}

int sem_close(const char * name) {
    if (name == NULL) return -1;

    uint64_t flags = enter_region(&sem_lock);
    int id = find_by_name(name);
    if (id < 0) {
        leave_region(&sem_lock, flags);
        return -1;
    }
    semaphore_t * s = &sem_table[id];

    if (s->openers > 0) s->openers--;
    if (s->openers == 0) {
        s->in_use = false;
        reset_waiters(s);
    }

    leave_region(&sem_lock, flags);
    return 0;
}
