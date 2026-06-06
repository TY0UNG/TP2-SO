#include <processes.h>
#include <terminal.h>
#define PROCESSES_LIMIT 128
#define PRIORITY_COUNT 5
#define STACK_SIZE 1024 * 16 // 16 KB

size_t pid_count = 1;
size_t proccess_count = 0;

Process processes[PROCESSES_LIMIT];

typedef struct PrioritySlot {
    Process * process;
    bool deleted;
} PrioritySlot;

PrioritySlot priority_lists[PRIORITY_COUNT][PROCESSES_LIMIT] = { NULL };
size_t priority_index[PRIORITY_COUNT] = { 0 };

pid_t actual_pid = 0;
size_t actual_index = -1;

size_t actual_process_start_time = 0;

// Tope del kernel stack sobre el que corre idle cuando no hay proceso listo.
// Se captura una vez en start_idle(), antes del primer context switch.
uint64_t kernel_rsp = 0;

extern uint64_t get_rsp();
extern void set_rsp(uint64_t);
extern void halt();
extern void _hlt();
extern void _sti();

static void modify_process_priority(Process * process, int new_priority);

static int getNextFreeSlot() {
    for (int i = 0; i < PROCESSES_LIMIT; i++) {
        if (processes[i].active == false) return i;
    }
    return -1;
}

static int getNextPriorityFreeSlot(int priority) {
    for (int i = 0; i < PROCESSES_LIMIT; i++) {
        if (priority_lists[priority][i].process == NULL ||
            priority_lists[priority][i].deleted ||
            priority_lists[priority][i].process->active == false) 
            return i;
    }
    return -1;
}

static size_t calculateArgc(const char **args) {
    if (!args) return 0;
    size_t argc = 0;
    while(args[argc] != NULL) {
        argc++;
    }
    return argc;
}

static int getIndex(pid_t pid) {
    for (int i = 0; i < PROCESSES_LIMIT; i++) {
        if (processes[i].pid == pid) return i;
    }
    return -1;
}

void initializeScheduler() {

}

static Process * selectNextProcess() {
    for (int i = 0; i < PRIORITY_COUNT; i++) {
        size_t start = priority_index[i];
        for (size_t k = 0; k < PROCESSES_LIMIT; k++) {
            size_t j = (start + k) % PROCESSES_LIMIT;
            if (priority_lists[i][j].deleted) continue;
            if (priority_lists[i][j].process == NULL) continue;
            Process *p = priority_lists[i][j].process;
            if (p->active && !p->blocked && !p->zombie) {
                priority_index[i] = (j + 1) % PROCESSES_LIMIT;
                return priority_lists[i][j].process;
            }
        }
    }
    return NULL;
}

extern void switch_rsp(uint64_t *save, uint64_t load);
extern void switch_to_idle(uint64_t *save, uint64_t idle_rsp);
extern void trampoline();

// Idle: corre sobre el kernel stack cuando no hay ningun proceso listo.
// Hace hlt hasta que el proximo timer tick interrumpa y el scheduler switchee
// a un proceso real. No retorna.
void idle() {
    while (1) _hlt();
}

void scheduler() {
    // Si había un proceso corriendo, guardar su RSP (lo dejó el asm en saved_stack_ptr)
    // y ajustar prioridad segun cuanto corrió.
    if (actual_index != (size_t)-1 && processes[actual_index].active) {
        size_t millis = getMilisFromBoot() - actual_process_start_time;
        if (millis > 40 && processes[actual_index].priority < PRIORITY_COUNT - 1) {
            modify_process_priority(&processes[actual_index], processes[actual_index].priority + 1);
        } else if (processes[actual_index].priority > 0 && millis < 10) {
            modify_process_priority(&processes[actual_index], processes[actual_index].priority - 1);
        }
    }

    Process * nextProcess = selectNextProcess();
    if (nextProcess == NULL) {
        // Durante el boot temprano el timer ya puede estar tickeando (el audio
        // del bootAnimation habilita interrupciones) y todavia no hay ningun
        // proceso ni kernel_rsp capturado. Ahi no hay idle al que saltar:
        // simplemente volvemos y dejamos seguir el boot, como antes.
        if (kernel_rsp == 0) {
            actual_pid = 0;
            actual_index = (size_t)-1;
            return;
        }
        size_t old_idx = actual_index;
        actual_pid = 0;
        actual_index = (size_t)-1;
        if (old_idx != (size_t)-1 && processes[old_idx].active) {
            switch_to_idle(&processes[old_idx].rsp, kernel_rsp);
        } else {
            uint64_t dummy;
            switch_to_idle(&dummy, kernel_rsp);
        }
        return;
    }

    size_t old_idx = actual_index;

    // Si el scheduler eligió el mismo proceso que estaba corriendo, no hay
    // que switchear (el IRQ handler hará popState/iretq y el proceso continúa).
    if (nextProcess->index == old_idx) {
        actual_process_start_time = getMilisFromBoot();
        return;
    }

    actual_pid = nextProcess->pid;
    actual_index = nextProcess->index;
    actual_process_start_time = getMilisFromBoot();

    if (old_idx == (size_t)-1) {
        // No hay current al que guardarle el rsp (primer boot o veniamos de un exit). Se descarta.
        uint64_t dummy;
        switch_rsp(&dummy, nextProcess->rsp);
    } else {
        switch_rsp(&processes[old_idx].rsp, nextProcess->rsp);
    }
}

typedef struct NewProcessStackFrame {
    // 6 callee-saved que switch_rsp va a popear
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t rbx;
    uint64_t rbp;
    // Return address que el ret de switch_rsp consume -> trampoline
    uint64_t return_address;
    // Lo que el trampoline popea (orden: pop rsi, pop rdi)
    uint64_t rsi;
    uint64_t rdi;
    // iretq frame
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} NewProcessStackFrame;

pid_t create_process(const char* name, void (*entry_point)(), const char ** args) {
    int index = getNextFreeSlot();
    if (index == -1) return 0;

    if (strlen(name) >= 32) return 0;
    strcpy(processes[index].name, name);

    // Calculamos tamaños de argumentos
    size_t argc = calculateArgc(args);
    size_t string_space = 0;
    for(size_t i = 0; i < argc; i++) {
        string_space += strlen(args[i]) + 1;
    }
    // Espacio para los punteros de los argumentos + 1 para el NULL
    size_t pointers_space = (argc + 1) * sizeof(char*);
    // Espacio para frame de iretq (ss, rsp, rflags, cs, rip)
    size_t frame_space = sizeof(NewProcessStackFrame);

    // Setup de stack
    processes[index].stack_memory = malloc(STACK_SIZE);
    uint64_t sp = (uint64_t) processes[index].stack_memory + STACK_SIZE - 1;

    // En el espacio asignado guardamos argumentos y los punteros a argumentos.
    sp -= (string_space + pointers_space);
    sp &= ~0xF; // Alinear a 16 bytes

    processes[index].stack_base = (char *) sp;

    char **argv_array = (char **)sp;
    char *string_dest = (char *)(sp + pointers_space); 

    for (size_t i = 0; i < argc; i++) {
        argv_array[i] = string_dest;
        strcpy(string_dest, args[i]);
        string_dest += strlen(args[i]) + 1;
    }
    argv_array[argc] = NULL; // Ultimo argumento es NULL

    NewProcessStackFrame * frame = (NewProcessStackFrame *) (sp - frame_space);
    frame->r15 = frame->r14 = frame->r13 = frame->r12 = 0;
    frame->rbx = frame->rbp = 0;
    frame->return_address = (uint64_t) trampoline;
    frame->rdi = argc;
    frame->rsi = (uint64_t) argv_array;
    frame->rip = (uint64_t) entry_point;
    frame->rsp = sp;
    frame->cs = 0x08;
    frame->rflags = 0x202;
    frame->ss = 0x00;

    processes[index].index = index;
    processes[index].pid = pid_count++;
    processes[index].parent_pid = actual_pid;
    processes[index].entry_point = entry_point;
    processes[index].active = true;
    processes[index].blocked = false;
    processes[index].zombie = false;
    processes[index].exit_status = 0;
    processes[index].waiting_for_pid = 0;
    processes[index].rsp = (uint64_t) frame;

    // fds heredados: 0=stdin, 1=stdout, 2=stderr -> todos a la terminal global.
    file_t * term = get_terminal_fd();
    for (int i = 0; i < MAX_FDS; i++) processes[index].fds[i] = NULL;
    if (term != NULL) {
        processes[index].fds[0] = term;
        processes[index].fds[1] = term;
        processes[index].fds[2] = term;
        term->ref_count += 3;
    }

    int priority_slot = getNextPriorityFreeSlot(0);
    priority_lists[0][priority_slot].process = &processes[index];
    priority_lists[0][priority_slot].deleted = false;
    processes[index].priority = 0;

    proccess_count++;

    return processes[index].pid;
}

// Saca al proceso de las priority_lists. Se llama desde kill_process /
// exit_current_process.
static void remove_from_priority_lists(Process * process) {
    for (int p = 0; p < PRIORITY_COUNT; p++) {
        for (int j = 0; j < PROCESSES_LIMIT; j++) {
            if (priority_lists[p][j].process == process) {
                priority_lists[p][j].deleted = true;
            }
        }
    }
}

static void close_all_fds(Process * p) {
    for (int i = 0; i < MAX_FDS; i++) {
        file_t * f = p->fds[i];
        if (f != NULL && f->ops != NULL && f->ops->close != NULL) {
            f->ops->close(f);
        }
        p->fds[i] = NULL;
    }
}

// Libera completamente el slot. Asume que la memoria de stack ya no se usa.
// active=false marca el slot como reutilizable.
static void reap(int index) {
    if (processes[index].stack_memory) {
        free(processes[index].stack_memory);
        processes[index].stack_memory = NULL;
    }
    processes[index].active = false;
    processes[index].zombie = false;
    processes[index].pid = 0;
}

// Lógica común de terminación. Deja al proceso en idx como zombie (el slot
// sigue activo hasta que wait lo reapee), lo saca de la run queue, cede el
// foreground al padre si correspondía y despierta a quien lo esté esperando.
// No toca actual_index ni llama al scheduler: eso lo hace cada caller según
// si el proceso terminado era o no el que está corriendo.
static void terminate_process(int idx, int status) {
    size_t the_pid = processes[idx].pid;

    processes[idx].exit_status = status;
    // Zombie: el slot sigue ocupado (active=true) hasta que wait lo reapee,
    // pero ya no es schedulable -> lo sacamos de las priority_lists.
    processes[idx].zombie = true;
    remove_from_priority_lists(&processes[idx]);
    proccess_count--;

    // Si era el foreground, el padre lo toma. Si el padre no existe o no esta
    // activo, fg queda en 0 (cualquiera lo puede tomar despues).
    if (the_pid == get_foreground_pid()) {
        size_t parent = processes[idx].parent_pid;
        int parent_idx = (parent != 0) ? getIndex(parent) : -1;
        if (parent_idx >= 0 && processes[parent_idx].active && !processes[parent_idx].zombie) {
            
            print("IM RETUNING IN THE FATHER! | ");
            printDec(parent_idx);
            print("\n");
            set_foreground_pid(parent_idx);
        } else {
             print("IM RETUNING IN 0! \n");
            set_foreground_pid(0);
        }
    }

    close_all_fds(&processes[idx]);

    // Despertar a cualquier proceso esperando por este pid.
    for (int i = 0; i < PROCESSES_LIMIT; i++) {
        if (processes[i].active && processes[i].blocked &&
            processes[i].waiting_for_pid == the_pid) {
            processes[i].blocked = false;
            processes[i].waiting_for_pid = -1;
        }
    }
}

void kill_process(pid_t pid) {
    int index = getIndex(pid);
    if (index < 0 || !processes[index].active || processes[index].zombie) return;

    terminate_process(index, -1);

    // Si nos estamos matando a nosotros mismos (p. ej. una excepción mata al
    // proceso en ejecución), no hay que volver al proceso muerto: marcamos que
    // no hay proceso actual para que el scheduler no guarde nuestro RSP, cedemos
    // y nunca volvemos.
    if ((size_t)index == actual_index) {
        actual_pid = 0;
        actual_index = (size_t)-1;
        scheduler();
        // Salvavidas: el scheduler no debería retornar acá (ya no estamos en la
        // run queue).
        while (1);
    }
}

void exit_current_process(int status) {
    if (actual_index == (size_t)-1) return;

    terminate_process(actual_index, status);

    // Marcamos que no hay proceso actual para que el scheduler no guarde
    // nuestro RSP. Cedemos y nunca volvemos.
    actual_pid = 0;
    actual_index = (size_t)-1;

    scheduler();
    // Salvavidas: si por algun motivo el scheduler retorna acá (no debería,
    // ya no estamos en la run queue), nos colgamos.
    while (1);
}

int wait_pid(pid_t pid) {
    
    int idx = getIndex(pid);
    
    if (idx < 0 || !processes[idx].active) return 0;

    while (!processes[idx].zombie) {
        processes[actual_index].waiting_for_pid = pid;
        processes[actual_index].blocked = true;
        scheduler();
    }

    int status = processes[idx].exit_status;

    reap(idx);
    return status;
}

void yield() {
    scheduler();
}

static void modify_process_priority(Process * process, int new_priority) {

}

void modify_process_priority_by_pid(size_t pid, int new_priority) {

}

void block_process(size_t pid) {
    int idx = getIndex(pid);
    if (idx >= 0) processes[idx].blocked = true;
}

void unblock_process(size_t pid) {
    int idx = getIndex(pid);
    if (idx >= 0) processes[idx].blocked = false;
}

void sleep_process(size_t pid, size_t milliseconds) {

}

void wait_for_process(size_t pid_to_block, size_t pid_to_wait) {

}


Process * get_processes() {
    return processes;
}

size_t get_actual_pid() {
    return actual_pid;
}

Process get_actual_process() {
    return processes[actual_index];
}

int get_processesInfo(ProcessInfo *buffer, int max_count) {

    if (buffer == NULL || max_count <= 0)
        return -1;

    int count = 0;

    for (int i = 0; i < PROCESSES_LIMIT && count < max_count; i++) {
        if (!processes[i].active)
            continue;

        buffer[count].pid = processes[i].pid;
        buffer[count].parent_pid = processes[i].parent_pid;
        buffer[count].active = processes[i].active;
        buffer[count].blocked = processes[i].blocked;
        buffer[count].zombie = processes[i].zombie;
        buffer[count].priority = processes[i].priority;

        strcpy(buffer[count].name, processes[i].name);

        count++;
    }

    return count;
}