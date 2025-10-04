#include <inout.h>

static int strcmp(const char *a, const char *b){
    while(*a && *a == *b){ a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

#define MAX_LINE 128

static char line[MAX_LINE];

static void run(char * cmd) {
    if(cmd[0]==0) return;
    if(strcmp(cmd,"help")==0){
        print("Comandos: help\n", STDOUT);
    } else {
        print("Comando desconocido\n", STDERR);
    }
}

int main() {
    for(;;){
        print("> ", STDOUT);
        int n = read(line);
        run(line);
    }
    return 0;
}