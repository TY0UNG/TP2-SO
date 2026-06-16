#include <processes.h>
#include <terminal.h>
#include <pipes.h>
#define PRIORITY_COUNT 5
#define STACK_SIZE 1024 * 16 // 16 KB

size_t pid_count = 1;
size_t proccess_count = 0;

Process processes[PROCESSES_LIMIT];

// Quantum (en ticks del timer) por nivel de prioridad. Nivel 0 = mayor
// prioridad => turno mas largo. Todos los procesos listos entran al mismo
// round-robin; la prioridad solo decide cuanta CPU recibe cada uno por vuelta,
// nunca si corre o no. Asi no hay inanicion: todo proceso listo corre en cada
// vuelta de la rueda, aunque sea por un solo tick.
static const int QUANTUM[PRIORITY_COUNT] = { 5, 4, 3, 2, 1 };

// Indice del ultimo proceso elegido. El round-robin reanuda la busqueda en el
// siguiente para repartir la CPU de forma equitativa.
static size_t rr_index = 0;

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

// Round-robin plano sobre todos los procesos listos. La busqueda arranca en el
// siguiente al ultimo elegido (rr_index) y, si no hay otro, vuelve a caer en el
// actual. La prioridad NO interviene aca: solo afecta el quantum (ver QUANTUM).
static Process * selectNextProcess() {
    for (size_t k = 1; k <= PROCESSES_LIMIT; k++) {
        size_t j = (rr_index + k) % PROCESSES_LIMIT;
        Process * p = &processes[j];
        if (p->active && !p->blocked && !p->zombie) {
            rr_index = j;
            return p;
        }
    }
    return NULL;
}

extern void switch_rsp(uint64_t *save, uint64_t load);
extern void switch_to_idle(uint64_t *save, uint64_t idle_rsp);
extern void trampoline();

// Idle: corre sobre el kernel stack cuando no hay ningun proceso listo.
void idle() {
    while (1) _hlt();
}

void scheduler() {
    Process * nextProcess = selectNextProcess();
    if (nextProcess == NULL) {
        // Durante el boot temprano el timer ya puede estar tickeando (el audio
        // del bootAnimation habilita interrupciones) y todavia no hay ningun
        // proceso ni kernel_rsp capturado.
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

    // Recarga el quantum del proceso elegido segun su prioridad.
    nextProcess->quantum_left = QUANTUM[nextProcess->priority];

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

// Preempcion por quantum.
void timer_tick() {
    if (actual_index != (size_t)-1) {
        Process * cur = &processes[actual_index];
        if (cur->active && !cur->blocked && !cur->zombie && cur->quantum_left > 1) {
            cur->quantum_left--;
            return;   // todavia tiene turno: sigue corriendo
        }
    }
    scheduler();
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
    processes[index].parent_pid = get_actual_pid();
    processes[index].entry_point = entry_point;
    processes[index].active = true;
    processes[index].blocked = false;
    processes[index].wait_reason = WAIT_NONE;
    processes[index].zombie = false;
    processes[index].exit_status = 0;
    processes[index].waiting_for_pid = 0;
    processes[index].rsp = (uint64_t) frame;
    processes[index].killable = true;
    processes[index].style = 0x0F;   // blanco sobre negro por defecto

    // fds: 0=stdin, 1=stdout, 2=stderr apuntan por defecto a la terminal global.
    // La terminal hace de stdin del teclado via su file-op read (line discipline
    // en contexto del lector). La redireccion con '|' reemplaza fds[0] por el
    // read end de un pipe despues de crear el proceso.
    for (int i = 0; i < MAX_FDS; i++) processes[index].fds[i] = NULL;
    processes[index].stdin_writer = NULL;

    file_t * term = get_terminal_fd();
    if (term != NULL) {
        processes[index].fds[0] = term;
        processes[index].fds[1] = term;
        processes[index].fds[2] = term;
        term->ref_count += 3;
    }

    processes[index].priority = 0;
    processes[index].quantum_left = QUANTUM[0];

    proccess_count++;

    return processes[index].pid;
}

static void close_all_fds(Process * p) {
    for (int i = 0; i < MAX_FDS; i++) {
        file_t * f = p->fds[i];
        if (f != NULL && f->ops != NULL && f->ops->close != NULL) {
            f->ops->close(f);
        }
        p->fds[i] = NULL;
    }
    // Cerrar tambien el write end de stdin (no es un fd visible al proceso).
    if (p->stdin_writer != NULL && p->stdin_writer->ops != NULL &&
        p->stdin_writer->ops->close != NULL) {
        p->stdin_writer->ops->close(p->stdin_writer);
    }
    p->stdin_writer = NULL;
}

file_t * process_stdin_writer(pid_t pid) {
    int idx = getIndex(pid);
    if (idx < 0) return NULL;
    if (!processes[idx].active || processes[idx].zombie) return NULL;
    return processes[idx].stdin_writer;
}

void set_killable(pid_t pid, bool value) {
    int idx = getIndex(pid);
    if (idx >= 0) processes[idx].killable = value;
}

bool process_is_killable(pid_t pid) {
    int idx = getIndex(pid);
    if (idx < 0) return false;
    return processes[idx].killable;
}

char get_current_style(void) {
    if (actual_index == (size_t)-1) return 0x0F;   // sin proceso (boot): default
    return processes[actual_index].style;
}

void set_current_style(char style) {
    if (actual_index == (size_t)-1) return;
    processes[actual_index].style = style;
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

// Cosecha los zombies huerfanos (cuyo padre ya no esta vivo): nadie va a
// llamar wait_pid por ellos, asi que los liberamos para no fugar slots
static void harvest_orphans(void) {
    for (int i = 0; i < PROCESSES_LIMIT; i++) {
        if ((size_t)i == actual_index) continue;
        if (!processes[i].active || !processes[i].zombie) continue;
        size_t parent = processes[i].parent_pid;
        int pidx = (parent != 0) ? getIndex(parent) : -1;
        bool parent_alive = (pidx >= 0 && processes[pidx].active && !processes[pidx].zombie);
        if (!parent_alive) reap(i);
    }
}

// Lógica común de terminación
static void terminate_process(int idx, int status) {
    size_t the_pid = processes[idx].pid;

    processes[idx].exit_status = status;
    // Zombie: el slot sigue ocupado (active=true) hasta que wait lo reapee,
    // pero ya no es schedulable (selectNextProcess ignora a los zombie).
    processes[idx].zombie = true;
    proccess_count--;

    // Si era el foreground, el padre lo toma. Si el padre no existe o no esta
    // activo, fg queda en 0 (cualquiera lo puede tomar despues).
    if (the_pid == get_foreground_pid()) {
        size_t parent = processes[idx].parent_pid;
        int parent_idx = (parent != 0) ? getIndex(parent) : -1;
        if (parent_idx >= 0 && processes[parent_idx].active && !processes[parent_idx].zombie) {
            set_foreground_pid(parent);   // pid del padre, NO el indice
        } else {
            set_foreground_pid(0);
        }
    }

    close_all_fds(&processes[idx]);

    // Reparent
    for (int i = 0; i < PROCESSES_LIMIT; i++) {
        if (processes[i].active && processes[i].parent_pid == the_pid) {
            processes[i].parent_pid = 0;
        }
    }

    // Despertar a cualquier proceso esperando por este pid.
    for (int i = 0; i < PROCESSES_LIMIT; i++) {
        if (processes[i].active && processes[i].blocked &&
            processes[i].waiting_for_pid == the_pid) {
            processes[i].blocked = false;
            processes[i].wait_reason = WAIT_NONE;
            processes[i].waiting_for_pid = 0;
        }
    }

    // Cosechar zombies huerfanos. Incluye a los hijos recien reparentados
    harvest_orphans();
}

void kill_process(pid_t pid) {
    int index = getIndex(pid);
    if (index < 0 || !processes[index].active || processes[index].zombie) return;

    terminate_process(index, -1);

    // Si nos estamos matando a nosotros mismos no hay que volver al proceso muerto: marcamos que no hay proceso actual para que el scheduler no guarde nuestro RSP
    if ((size_t)index == actual_index) {
        actual_pid = 0;
        actual_index = (size_t)-1;
        scheduler();
        // Salvavidas. El scheduler no debería retornar
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
    // Salvavidas. Scheduler no deberia retornar
    while (1);
}

int wait_pid(pid_t pid) {
    
    int idx = getIndex(pid);
    
    
    // Solo el padre puede esperar/cosechar a un proceso.
    if (processes[idx].parent_pid != get_actual_pid()) return -1;
    
    if (idx < 0 || !processes[idx].active) return 0;

    while (!processes[idx].zombie) {
        processes[actual_index].waiting_for_pid = pid;
        processes[actual_index].blocked = true;
        processes[actual_index].wait_reason = WAIT_PID;
        scheduler();
        // Al despertar revalidamos: el slot pudo cambiar mientras dormiamos.
        idx = getIndex(pid);
        if (idx < 0 || !processes[idx].active) return -1;
    }

    int status = processes[idx].exit_status;

    reap(idx);
    return status;
}

void yield() {
    scheduler();
}

// Fija el nivel de prioridad del proceso (clamp a 0..PRIORITY_COUNT-1).
// Nivel 0 = mayor prioridad => quantum mas largo. El nuevo peso toma efecto en
// el proximo turno; recargamos el quantum para que aplique cuanto antes.
static void modify_process_priority(Process * process, int new_priority) {
    if (process == NULL) return;
    if (new_priority < 0) new_priority = 0;
    if (new_priority >= PRIORITY_COUNT) new_priority = PRIORITY_COUNT - 1;
    if (process->priority == new_priority) return;

    process->priority = new_priority;
    process->quantum_left = QUANTUM[new_priority];
}

void modify_process_priority_by_pid(size_t pid, int new_priority) {
    int idx = getIndex(pid);
    if (idx >= 0 && processes[idx].active) {
        modify_process_priority(&processes[idx], new_priority);
    }
}

void block_process(size_t pid) {
    int idx = getIndex(pid);
    if (idx >= 0) processes[idx].blocked = true;
}

void unblock_process(size_t pid) {
    int idx = getIndex(pid);
    if (idx >= 0) {
        processes[idx].blocked = false;
        processes[idx].wait_reason = WAIT_NONE;
    }
}

// Alterna el estado del proceso entre bloqueado y listo (para el comando block).
// Devuelve el nuevo estado: 1 = bloqueado, 0 = listo, -1 = pid inexistente.
int toggle_block_process(size_t pid) {
    int idx = getIndex(pid);
    if (idx < 0) return -1;
    if (processes[idx].blocked) {
        processes[idx].blocked = false;
        processes[idx].wait_reason = WAIT_NONE;
        return 0;
    }
    processes[idx].blocked = true;
    return 1;
}

void block_current(wait_reason_t reason) {
    if (actual_index == (size_t)-1) return;
    processes[actual_index].blocked = true;
    processes[actual_index].wait_reason = reason;
    scheduler();
}

bool unblock_if_reason(size_t pid, wait_reason_t reason) {
    int idx = getIndex(pid);
    if (idx < 0) return false;
    if (!processes[idx].blocked || processes[idx].wait_reason != reason) return false;
    processes[idx].blocked = false;
    processes[idx].wait_reason = WAIT_NONE;
    return true;
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
        
        if (processes[i].active){

            buffer[count].pid = processes[i].pid;
            buffer[count].parent_pid = processes[i].parent_pid;
            buffer[count].active = processes[i].active;
            buffer[count].blocked = processes[i].blocked;
            buffer[count].zombie = processes[i].zombie;
            buffer[count].priority = processes[i].priority;
            buffer[count].wait_reason = processes[i].wait_reason;
            buffer[count].stack_base = processes[i].stack_base;
            buffer[count].stack_memory = processes[i].stack_memory;

            strcpy(buffer[count].name, processes[i].name);
            
            count++;
        }
    }

    return count;
}

int replace_process_fd(int indice, file_t * fd, int pid){
    
    if(indice < 0 || indice >= MAX_FDS) return 0;

    processes[getIndex(pid)].fds[indice] = fd;
    
    return 1;
}