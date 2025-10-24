#include <commands.h>

typedef struct {
    char *name;
    char *description;
    char *category;
} Commands;

Commands command_list[] = {
    // SISTEMA
    {"help", " display online manual documentation pages", "  SISTEM"},
    {"exit", " close down the system at a given time", "  SISTEM"},
    // OTRAS FUNC
    {"time", " Muestra fecha y hora actual", "  DATA SIST."},
    {"registros", " Muestra registros del CPU", "  DATA SIST."},
    //BENCHMARKING
    {"fps", " Muestra frames por segundo", "  BENCHMARKING"},
    {"speed <COMAND>", " show processing time of command", "  BENCHMARKING"},
    // UTILIDADES
    {"echo", " write arguments to the standard output", "  UTILITIES"},
    // JUEGO
    {"tron", " Juego Tron", "  GAME"},
    {"bounce", " Animacion rebotando", "  GAME"},
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
    println("  Lista de comandos:\n help - Obtiene la lista de comandos\n");

    println("========= COMANDOS DISPONIBLES ================================");
    
    char *current = NULL;
    
    for (int i = 0; command_list[i].name != NULL; i++) {
        // Si cambia de categ
        if (current == NULL || strcmp(current, command_list[i].category) != 0) {
            current = command_list[i].category;
            printChar('\n');  println(current);
        }
        
         print_command(command_list[i].name, command_list[i].description);
    }
    println("\n===============================================================");

    return 0;
}