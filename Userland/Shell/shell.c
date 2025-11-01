#include <inout.h>
#include <str.h>
#include <commands.h>

static int show_welcome = 1;    // si se vuelve a llamar a la shell, no se resetea a 1 pues se guarda en section .data

int main() {
    char input[256];
    if (show_welcome) {
        println("Bienvenido al sistema operativo.");
        show_welcome = 0;
    }
    while(1) {
        print("OS> ");
        read(input);
        char* argsv[256];
        int argc = strparse(input, argsv, " ");
        commandDispatcher(argsv, argc);
    }
    return 0;
}

int commandDispatcher(char ** argsv, int argsc) {
    char * cmd = argsv[0];
    if (strcmp(cmd, "help") == 0) return help(argsv, argsc);
    if (strcmp(cmd, "clear") == 0) {
        clear();
        return 1;
    }
    if (strcmp(cmd, "echo") == 0) return echo(argsv, argsc);
    if (strcmp(cmd, "exit") == 0) shutdown(argsv, argsc);
    
    if (strcmp(cmd, "registros") == 0) return regs(argsv, argsc);
    if (strcmp(cmd, "time") == 0) return time(argsv, argsc);
    if (strcmp(cmd, "fps") == 0) return fps(argsv, argsc);
    if (strcmp(cmd, "show") == 0) return show(argsv, argsc);
    if (strcmp(cmd, "speed") == 0) return speed(argsv, argsc);
    if (strcmp(cmd, "resize") == 0) return resize(argsv, argsc);
    if (strcmp(cmd, "benchfloat") == 0) return benchfloat(argsv, argsc);
    if (strcmp(cmd, "benchhw") == 0) return benchhw(argsv, argsc);
    if (strcmp(cmd, "benchmem") == 0) return benchmem(argsv, argsc);
    if (strcmp(cmd, "benchkbd") == 0) return benchkbd(argsv, argsc);
    
    if (strcmp(cmd, "tron") == 0) return tron(argsv, argsc);
    if (strcmp(cmd, "bounce") == 0) return bounce(argsv, argsc);
    

    if (strcmp(cmd, "dividezero") == 0) dividezero();
    if (strcmp(cmd, "invalidop") == 0) invalidop();

    println("Comando desconocido. Ejecute 'help' para obtener ayuda.");
    return 1;
}