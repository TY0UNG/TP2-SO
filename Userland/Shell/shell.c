#include <inout.h>
#include <str.h>
#include <commands.h>

extern int  sys_create_process(const char *name, void *entry, int argc, char **argv);
extern int  sys_wait(int pid);
extern void sys_exit(int status);
extern int  sys_set_foreground(int pid);

typedef int (*command_entry_t)(int argc, char **argv);

typedef struct {
    const char *name;
    command_entry_t entry;
} Command;

// Convierte un comando con firma (char**, int) a (int, char**). El wrapper
// también llama sys_exit al final porque el kernel salta directo a este
// entry point: no hay _start envolviéndolo. Si retornáramos por C, el ret
// del wrapper saltaría a basura del stack del child.
#define WRAP_CMD(cmd) \
    static int cmd##_wrap(int argc, char **argv) { \
        int ret = cmd(argv, argc); \
        sys_exit(ret); \
        return 0; /* inalcanzable */ \
    }

WRAP_CMD(help)
WRAP_CMD(echo)
WRAP_CMD(regs)
WRAP_CMD(time)
WRAP_CMD(fps)
WRAP_CMD(show)
WRAP_CMD(speed)
WRAP_CMD(resize)
WRAP_CMD(benchfloat)
WRAP_CMD(benchhw)
WRAP_CMD(benchmem)
WRAP_CMD(benchkbd)
WRAP_CMD(testmm)
WRAP_CMD(meminfo)
WRAP_CMD(tronGame)
WRAP_CMD(bounce)
WRAP_CMD(processList);

static int clear_wrap(int argc, char **argv) {
    (void) argc; (void) argv;
    clear();
    sys_exit(0);
    return 0;
}

static int dividezero_wrap(int argc, char **argv) {
    (void) argc; (void) argv;
    dividezero();
    sys_exit(0);
    return 0;
}

static int invalidop_wrap(int argc, char **argv) {
    (void) argc; (void) argv;
    invalidop();
    sys_exit(0);
    return 0;
}

static const Command commands[] = {
    { "help",       help_wrap       },
    { "clear",      clear_wrap      },
    { "echo",       echo_wrap       },
    { "registers",  regs_wrap       },
    { "time",       time_wrap       },
    { "fps",        fps_wrap        },
    { "show",       show_wrap       },
    { "speed",      speed_wrap      },
    { "resize",     resize_wrap     },
    { "benchfloat", benchfloat_wrap },
    { "benchhw",    benchhw_wrap    },
    { "benchmem",   benchmem_wrap   },
    { "benchkbd",   benchkbd_wrap   },
    { "testmm",     testmm_wrap     },
    { "meminfo",    meminfo_wrap    },
    { "tron",       tronGame_wrap   },
    { "bounce",     bounce_wrap     },
    { "dividezero", dividezero_wrap },
    { "invalidop",  invalidop_wrap  },
    { "ps",         processList_wrap}
};
static const int commands_count = sizeof(commands) / sizeof(commands[0]);

static int show_welcome = 1;    // si se vuelve a llamar a la shell, no se resetea a 1 pues se guarda en section .data

int main(int argc, char **argv) {
    (void) argc; (void) argv;
    char input[256];
    if (show_welcome) {
        println("Bienvenido al sistema operativo.");
        show_welcome = 0;
    }
    while(1) {
        print("OS> ");
        read(input);
        char* argsv[256];
        int argsc = strparse(input, argsv, " ");
        if (argsc == 0) continue;
        commandDispatcher(argsv, argsc);
        println("Saliendo Dispatcher: ");
    }
    
    return 0;
}

int commandDispatcher(char ** argsv, int argsc) {

    println("Entrando Dispatcher: ");
    const char * cmd = argsv[0];
    const char * lastArgCmd = argsv[argsc - 1];

    for (int i = 0; i < commands_count; i++) {

        if (strcmp(cmd, commands[i].name) == 0) {


            int pid = createProcess(commands[i].name, (void *) commands[i].entry, argsc, argsv);
            if (pid <= 0) {
                println("Error al crear proceso");
                return 1;
            }

            if(strcmp(lastArgCmd, "&") != 0){
                // Le cedemos el foreground al hijo para que pueda leer/escribir
                // en la terminal. Cuando termine, el kernel devuelve fg al padre
                // (shell) automaticamente.
                sys_set_foreground(pid);
                sys_wait(pid);
            }
            
            return 1;
        }
        
    }
    println("Comando desconocido. Ejecute 'help' para obtener ayuda.");
    return 1;
}
