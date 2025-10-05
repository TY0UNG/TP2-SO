#include <inout.h>
#include <str.h>
#include <commands.h>

static int commandDispatcher(char * command);

int main() {
    char command[256];
    println("Bienvenido al sistema operativo.");
    while(1) {
        print("OS> ");
        read(command);
        commandDispatcher(command);
    }
    return 0;
}

int commandDispatcher(char * command) {
    if (strcmp(command, "help") == 0) return help();
    println("Comando desconocido. Ejecute 'help' para obtener ayuda.");
    return 1;
}