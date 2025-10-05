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



static void print_hex(unsigned char c) {
    char hex[] = "0123456789ABCDEF";
    char buffer[3] = {hex[c >> 4], hex[c & 0xF], 0};
    print(buffer, STDOUT);
}

#define MAX_LINE 128
static char line[MAX_LINE];


int main() {
    
    
    for(;;){
        
        print("> ", STDOUT);
        int n = read(line);
        //print(num,STDOUT);
        print(n,STDOUT);
        run(line);
        //num++;
    }

    int test_num = 1;

 


    
    return 0;
}