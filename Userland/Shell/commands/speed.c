#include <commands.h>
#include "../lib/time.h"

int speed(char ** argsv, int argsc) {
    if(argsc<2){
        print("Uso incorrecto. Uso correcto: speed <comando>\n");
        return 1;
    }

    uint64_t initialTime = getMilisFromBoot();
   
    commandDispatcher(argsv + 1, argsc - 1);

    uint64_t diff = getMilisFromBoot() - initialTime;

    print("El tiempo de procesamiento fue de: ");
    printDec(diff / 60000); print(" minutos, ");
    printDec((diff % 60000) / 1000); print(" segundos y ");
    printDec(diff % 1000); print(" milisegundos.\n");
    return 0;
}