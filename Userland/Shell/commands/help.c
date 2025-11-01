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
    {"exit", "apaga el sistema", "  SISTEMA"},

    // INFORMACIÓN DEL SISTEMA
    {"time", "muestra fecha y hora actuales", "  INFORMACIÓN"},
    {"registros", "muestra los registros del CPU", "  INFORMACIÓN"},

    // BENCHMARKS
    {"fps", "mide cuadros por segundo del render", "  BENCHMARKS"},
    {"speed <comando>", "cronometra la ejecución de un comando", "  BENCHMARKS"},
    {"benchfloat [ms]", "mide divisiones de coma flotante por segundo", "  BENCHMARKS"},
    {"benchhw [ms]", "mide lecturas del RTC por segundo", "  BENCHMARKS"},
    {"benchmem [ms]", "mide copias de memoria por segundo", "  BENCHMARKS"},
    {"benchkbd [ms]", "mide lecturas del teclado por segundo", "  BENCHMARKS"},

    // UTILIDADES
    {"echo", "imprime los argumentos en pantalla", "  UTILIDADES"},
    {"resize <size>", "cambia el tamaño del texto (12-64)", "  UTILIDADES"},

    // JUEGOS Y DEMOS
    {"tron", "inicia el juego Tron", "  JUEGOS"},
    {"bounce", "muestra la animación de rebote", "  JUEGOS"},

    // PRUEBAS Y EXCEPCIONES
    {"dividezero", "genera la excepción de división por cero", "  PRUEBAS"},
    {"invalidop", "genera la excepción de instrucción inválida", "  PRUEBAS"},

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