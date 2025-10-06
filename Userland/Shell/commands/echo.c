#include <commands.h>

int echo(char ** argsv, int argc) {
    for (int i = 1; i < argc; i++){
        print(argsv[i]);
        print(" ");
    }
    print("\n");
    return 0;
}