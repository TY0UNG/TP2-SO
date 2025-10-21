#include <inout.h>
#include <str.h>
#include <commands.h>

static int show_welcome = 1;    // si se vuelve a llamar a la shell, no se resetea a 1 pues se guarda en section .data

static int commandDispatcher(char * input);

int main() {
    char input[256];
    if (show_welcome) {
        println("Bienvenido al sistema operativo.");
        show_welcome = 0;
    }
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

    /* Para probar las excepciones, borrenlo cuando no se necesite más hacer los testeos */
    if (strcmp(cmd, "dividezero") == 0) dividezero();
    if (strcmp(cmd, "invalidop") == 0) invalidop();
    /* Fin de prueba de excepciones */

    if (strcmp(cmd, "exit") == 0) shutdown(argsv, argc);
    if (strcmp(cmd, "help") == 0) return help(argsv, argc);
    if (strcmp(cmd, "clear") == 0) {
        clear();
        return 1;
    }
    if (strcmp(cmd, "tron") == 0) return tron(argsv, argc); 
    if (strcmp(cmd, "bounce") == 0) return bounce(argsv, argc);
    if (strcmp(cmd, "echo") == 0) return echo(argsv, argc);
    if (strcmp(input, "registros") == 0) regs(argsv, argc);
    if (strcmp(cmd, "time") == 0) return time(argsv, argc);
    if (strcmp(cmd, "fps") == 0) return fps(argsv, argc);
    println("Comando desconocido. Ejecute 'help' para obtener ayuda.");
    return 1;
}