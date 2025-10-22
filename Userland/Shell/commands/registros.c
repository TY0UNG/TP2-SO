
#include <commands.h>
#include <draw.h>
#include "../lib/str.h"
#include "../lib/time.h"

extern sys_get_reg();

int regs(char ** argv, int argc){
    char *vec = sys_get_reg();   

    if (vec == 0 || vec[0] == '\0') {   
        print("No hay registros guardados.\n");
        return ;
    }
    print(vec);
}