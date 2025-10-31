/*
    Comando que me da lo que tarde en hacerse 
*/

#include <commands.h>
#include "../lib/time.h"

int speed(char ** argv, int argc) {
    
    if(argc<2){
        print("Poner comando");
        return 0;
    }
    
    uint8_t datetime0[6];                               // tiempo inicial
    uint8_t datetimef[6];                               //tiempo final
    uint8_t  dif[6];

    getDateTime(datetime0);
    uint64_t ms0=sys_get_ms();                          // mls iniciales
   
    commandDispatcher(argv[1]);

    getDateTime(datetimef);
    uint64_t msf=sys_get_ms();                          //mls finales
    difTime(datetime0,datetimef,dif);


    msf-=ms0;
    if(msf<0){
     //rerd();       //     reduce un segundo en la hora 
     msf=1-msf;           
    }

    print("El tiempo de procesamiento fue de: ");
    printHR_M_S(dif);
    printDec(msf); print(" ms\n"); 

        
    return 0;
    
}