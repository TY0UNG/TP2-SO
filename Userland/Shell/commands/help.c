#include <commands.h>

typedef struct {
    char *name;
    char *description;
    char *category;
} Commands;

Commands command_list[] = {
    // SISTEMA
    {"help", "muestra este listado de ayuda", "  SISTEMA"},
    {"clear", "limpia la pantalla", "  SISTEMA"},

    // INFORMACION DEL SISTEMA
    {"time", "muestra fecha y hora actuales", "  INFORMACION"},
    {"show fps", "habilita o deshabilita el overlay de FPS", "  INFORMACION"},
    {"registers", "muestra los registros del CPU", "  INFORMACION"},

    // BENCHMARKS
    {"fps", "mide cuadros por segundo del render", "  BENCHMARKS"},
    {"speed <comando>", "cronometra la ejecucion de un comando", "  BENCHMARKS"},
    {"benchfloat [s]", "mide divisiones de coma flotante por segundo (1-60s)", "  BENCHMARKS"},
    {"benchhw [s]", "mide lecturas del RTC por segundo (1-60s)", "  BENCHMARKS"},
    {"benchmem [s]", "mide copias de memoria por segundo (1-60s)", "  BENCHMARKS"},
    {"benchkbd [s]", "mide lecturas del teclado por segundo (1-60s)", "  BENCHMARKS"},
    {"meminfo", "muestra memoria total y memoria usada", "  BENCHMARKS"},

    // UTILIDADES
    {"echo <texto>", "imprime los argumentos en pantalla", "  UTILIDADES"},
    {"resize <size>", "cambia el tamaño del texto (12-64)", "  UTILIDADES"},

    // PROCESOS
    {"loop [ms]", "imprime su PID con un saludo cada ms (espera activa)", "  PROCESOS"},
    {"kill <pid>", "termina el proceso con ese PID", "  PROCESOS"},
    {"nice <pid> <prio>", "cambia la prioridad de un proceso (0=alta..4=baja)", "  PROCESOS"},
    {"block <pid>", "alterna un proceso entre bloqueado y listo", "  PROCESOS"},

    // FILTROS (stdin -> stdout)
    {"cat", "imprime stdin tal como lo recibe", "  FILTROS"},
    {"wc", "cuenta la cantidad de lineas del input", "  FILTROS"},
    {"filter", "filtra (quita) las vocales del input", "  FILTROS"},

    // JUEGOS Y DEMOS
    {"tron", "inicia el juego Tron Neon Grid", "  JUEGOS"},
    {"bounce", "muestra la animacion de rebote", "  JUEGOS"},

    // PRUEBAS Y EXCEPCIONES
    {"testmm <bytes> [iter]", "stress test de malloc/free del memory manager del kernel", "  PRUEBAS"},
    {"testsync <n> <use_sem>", "prueba sincronizacion con y sin semaforos (race conditions)", "  PRUEBAS"},
    {"testprocesses <n>", "prueba creacion, kill, block y unblock de n procesos", "  PRUEBAS"},
    {"testprio <max>", "prueba el scheduler con distintas prioridades", "  PRUEBAS"},
    {"mvar <esc> <lec>", "lectores/escritores sobre una MVar (sincronizacion)", "  PRUEBAS"},
    {"dividezero", "genera la excepcion de division por cero", "  PRUEBAS"},
    {"invalidop", "genera la excepcion de instruccion inválida", "  PRUEBAS"},

    {NULL, NULL, NULL}
};

void print_command(const char *name, const char *description) {
    int maxNameWidth = 24; // columna donde empieza el "-"
    int len = 0;
    while (name[len] != '\0') len++;

    print("     "); print(name);
    
    for (int i = len; i < maxNameWidth; i++)
        printChar(' '); 
        
    print(" - ");  println(description);

}

int help(char ** argv, int argc) {
    println("  Lista de comandos disponibles\n");

    println("========= COMANDOS DISPONIBLES ================================");

    char *current = NULL;

    for (int i = 0; command_list[i].name != NULL; i++) {
        if (current == NULL || strcmp(current, command_list[i].category) != 0) {
            current = command_list[i].category;
            printChar('\n');
            println(current);
        }

        print_command(command_list[i].name, command_list[i].description);
    }

    println("\n===============================================================");

    return 0;
}