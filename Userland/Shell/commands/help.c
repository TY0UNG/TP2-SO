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
    {"testmm <bytes>", "stress test de malloc/free en el memory manager del kernel", "  BENCHMARKS"},
    {"meminfo", "muestra memoria total y memoria usada", "  BENCHMARKS"},

    // UTILIDADES
    {"echo", "imprime los argumentos en pantalla", "  UTILIDADES"},
    {"resize <size>", "cambia el tamaño del texto (12-64)", "  UTILIDADES"},

    // JUEGOS Y DEMOS
    {"tron", "inicia el juego Tron Neon Grid", "  JUEGOS"},
    {"bounce", "muestra la animacion de rebote", "  JUEGOS"},

    // PRUEBAS Y EXCEPCIONES
    {"dividezero", "genera la excepcion de division por cero", "  PRUEBAS"},
    {"invalidop", "genera la excepcion de instruccion inválida", "  PRUEBAS"},

    {NULL, NULL, NULL}
};

void print_command(const char *name, const char *description) {
    int maxNameWidth = 20; // columna donde empieza el "-"
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