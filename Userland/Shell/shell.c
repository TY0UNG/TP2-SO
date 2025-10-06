#include <inout.h>
#include <str.h>
#include <commands.h>

static int commandDispatcher(char * input);

int main() {
    char input[256];
    println("Bienvenido al sistema operativo.");
    while(1) {
        print("OS> ");
        read(input);
        commandDispatcher(input);
    }
    return 0;
}

int commandDispatcher(char * input) {
    char* argsv[256];
    int argc = strparse(input, argsv, " ");
    char * cmd = argsv[0];
    if (strcmp(cmd, "help") == 0) return help(argsv, argc);
    if (strcmp(cmd, "clear") == 0) {
        clear();
        return 1;
    }
    if (strcmp(cmd, "echo") == 0) return echo(argsv, argc);
    println("Comando desconocido. Ejecute 'help' para obtener ayuda.");
    return 1;
}