#include <inout.h>
#include <str.h>
#include <commands.h>

extern int  sys_create_process(const char *name, void *entry, int argc, char **argv);
extern int  sys_wait(int pid);
extern void sys_exit(int status);
extern int  sys_set_foreground(int pid);
extern int  sys_create_pipe(file_t ** fd);
extern int sys_replace_process_fd(int indice, file_t * fd, int pid);

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
WRAP_CMD(testsync)
WRAP_CMD(testprocesses)
WRAP_CMD(testprio)
WRAP_CMD(meminfo)
WRAP_CMD(mvar)
WRAP_CMD(loop)
WRAP_CMD(kill)
WRAP_CMD(nice)
WRAP_CMD(block)
WRAP_CMD(cat)
WRAP_CMD(wc)
WRAP_CMD(filter)
WRAP_CMD(red)
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
    { "testsync",   testsync_wrap   },
    { "testprocesses", testprocesses_wrap },
    { "testprio",   testprio_wrap   },
    { "meminfo",    meminfo_wrap    },
    { "mvar",       mvar_wrap       },
    { "loop",       loop_wrap       },
    { "kill",       kill_wrap       },
    { "nice",       nice_wrap       },
    { "block",      block_wrap      },
    { "cat",        cat_wrap        },
    { "wc",         wc_wrap         },
    { "filter",     filter_wrap     },
    { "red",        red_wrap        },
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
    }
    
    return 0;
}

int commandDispatcher(char ** argsv, int argsc) {



    char ** left_argsv = argsv;

    const char * left_cmd = argsv[0];
    const char * left_lastArgCmd = argsv[argsc - 1];
    
    int left_argsc = argsc; 

    char **right_argsv = NULL;

    char *right_cmd = NULL;

    int right_argsc; 

    int pipePos = -1;

    // Buscar si hay pipe
    for (int i = 0; i < argsc; i++) {
        if (strcmp(argsv[i], "|") == 0) {
            pipePos = i;
            break;
        }
    }

    if(pipePos >= 1){
        left_argsc = pipePos;
        right_argsc = (argsc - 1) - pipePos;

        left_lastArgCmd = argsv[pipePos - 1];

        right_argsv = argsv + (pipePos + 1);
        right_cmd = right_argsv[0];

        // El kernel recalcula argc recorriendo argv hasta NULL (ignora el argc
        // que le pasamos), asi que cortamos el array izquierdo en el pipe.
        argsv[pipePos] = NULL;

    }

    file_t * fd[2];

    int command_right_index = -1;
    


    if(right_cmd != NULL){
        for(int i = 0; i < commands_count; i++){
            if (strcmp(right_cmd, commands[i].name) == 0 ){
                command_right_index = i;
            }
        }
        if(command_right_index == -1){    
            println("Comando desconocido. Ejecute 'help' para obtener ayuda.");
            return 1;
        }
    }
    for (int i = 0; i < commands_count; i++) {
        
        if (strcmp(left_cmd, commands[i].name) == 0 ){

            int pid_r = -1;
            if(pipePos != -1){
                sys_create_pipe(fd);
                pid_r = sys_create_process(commands[command_right_index].name, (void *) commands[command_right_index].entry, right_argsc, right_argsv);
                sys_replace_process_fd(0, fd[0], pid_r);
            }

            int pid_l = sys_create_process(commands[i].name, (void *) commands[i].entry, left_argsc, left_argsv);
            if (pid_l <= 0) {
                println("Error al crear proceso");
                return 1;
            }

            if(right_cmd != NULL){
                sys_replace_process_fd(1, fd[1], pid_l);
            }

            if(strcmp(left_lastArgCmd, "&") != 0){
                // Le cedemos el foreground al hijo para que pueda leer/escribir
                // en la terminal. Cuando termine, el kernel devuelve fg al padre
                // (shell) automaticamente.
                sys_set_foreground(pid_l);

                sys_wait(pid_l);
                if(pid_r != -1){
                    sys_wait(pid_r);
                }
            }

            return 1;
        }
    }
        
    
    println("Comando desconocido. Ejecute 'help' para obtener ayuda.");
    return 1;
}
