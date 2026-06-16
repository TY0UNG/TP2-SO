#include "commands.h"
#include "../lib/utils.h"
#include "../lib/test_util.h"
#include <inout.h>
#include <stdint.h>

extern int  sys_create_process(const char *name, void *entry, int argc, char **argv);
extern void sys_exit(int status);
extern int  sys_sem_open(const char *name, int initialValue);
extern int  sys_sem_init(const char *name, int initialValue);
extern int  sys_sem_wait(const char *name);
extern int  sys_sem_post(const char *name);

#define SEM_EMPTY "mvar_empty"
#define SEM_FULL  "mvar_full"

#define MAX_WRITERS 26   // 'A' .. 'Z'
#define MAX_READERS 16

// Como todos los procesos comparten la imagen del shell, esta variable global vive una sola vez y la ven todos.
static volatile char mvar_value;

// Paleta de colores (byte de estilo VGA: FG en el nibble bajo) para identificar a cada lector.
static const char reader_colors[MAX_READERS] = {
    0x0C, 0x0A, 0x0B, 0x0E, 0x0D, 0x09, 0x0F, 0x06,
    0x0C, 0x0A, 0x0B, 0x0E, 0x0D, 0x09, 0x0F, 0x06
};

static void busy_wait_random(void) {
    volatile uint64_t sink = 0;
    uint32_t spins = GetUniform(40000) + 1000;
    for (uint32_t i = 0; i < spins; i++) {
        sink += i;
    }
}

// ----------------------------- escritor -----------------------------

static uint64_t writer_loop(char value) {
    if (sys_sem_open(SEM_EMPTY, 1) < 0 || sys_sem_open(SEM_FULL, 0) < 0)
        return (uint64_t) -1;

    while (1) {
        busy_wait_random();
        sys_sem_wait(SEM_EMPTY);   // espera a que la MVar este vacia
        mvar_value = value;        // deposita su valor unico
        sys_sem_post(SEM_FULL);    // avisa que hay algo para leer
    }
    return 0;   // inalcanzable
}

static int writer_entry(int argc, char **argv) {
    char value = (argc >= 1 && argv[0] != 0) ? argv[0][0] : '?';
    sys_exit((int) writer_loop(value));
    return 0;   // inalcanzable
}

// ----------------------------- lector -------------------------------

static uint64_t reader_loop(char style) {
    if (sys_sem_open(SEM_EMPTY, 1) < 0 || sys_sem_open(SEM_FULL, 0) < 0)
        return (uint64_t) -1;

    selectStyle(style);            // color identificador del lector (una sola vez)

    while (1) {
        busy_wait_random();
        sys_sem_wait(SEM_FULL);    // espera a que haya un valor
        char c = mvar_value;       // lo consume
        char str[2] = { c, 0 };
        print(str);                // lo imprime con su color
        sys_sem_post(SEM_EMPTY);   // libera la MVar para el proximo escritor
    }
    return 0;   // inalcanzable
}

static int reader_entry(int argc, char **argv) {
    char style = (argc >= 1 && argv[0] != 0) ? (char) satoi(argv[0]) : 0x0F;
    sys_exit((int) reader_loop(style));
    return 0;   // inalcanzable
}

// ----------------------------- comando ------------------------------

int mvar(char **argv, int argc) {
    if (argc != 3) {
        println("Uso: mvar <escritores> <lectores>");
        return 1;
    }

    int writers = satoi(argv[1]);
    int readers = satoi(argv[2]);

    if (writers <= 0 || readers <= 0) {
        println("mvar: escritores y lectores deben ser enteros positivos");
        return 1;
    }
    if (writers > MAX_WRITERS) {
        println("mvar: demasiados escritores (max 26)");
        return 1;
    }
    if (readers > MAX_READERS) {
        println("mvar: demasiados lectores (max 16)");
        return 1;
    }

    // Crear (o reinicializar) los semaforos a su estado inicial antes de lanzar los hijos
    if (sys_sem_init(SEM_EMPTY, 1) < 0 || sys_sem_init(SEM_FULL, 0) < 0) {
        println("mvar: error al abrir los semaforos");
        return 1;
    }

    mvar_value = 0;

    print("mvar: escritores ");
    for (int i = 0; i < writers; i++) {
        char vbuf[2] = { (char) ('A' + i), 0 };
        char *wargv[] = { vbuf, 0 };
        int pid = sys_create_process("mvar_writer", (void *) writer_entry, 1, wargv);
        print(vbuf);
        print("(pid ");
        printDec((uint64_t) pid);
        print(") ");
    }
    print("\nmvar: lectores ");
    for (int i = 0; i < readers; i++) {
        char sbuf[6];
        parseInt((int) reader_colors[i], sbuf, sizeof(sbuf));
        char *rargv[] = { sbuf, 0 };
        int pid = sys_create_process("mvar_reader", (void *) reader_entry, 1, rargv);
        print("(pid ");
        printDec((uint64_t) pid);
        print(") ");
    }
    print("\n");

    // El proceso principal termina de inmediato; los lectores/escritores siguen corriendo en background.
    return 0;
}
