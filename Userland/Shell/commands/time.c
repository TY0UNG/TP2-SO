#include <commands.h>
#include <stdint.h>
#include <inout.h>
#include "../lib/time.h"

int time(char ** argv, int argc) {
    uint8_t datetime[6];  
    getDateTime(datetime);

    printTime(datetime);

    print("MS: ");
    printDec(getMilisFromBoot());
    printChar('\n');
    
    return 0;
}







