/*
    Comando que me da lo que tarde en hacerse 
*/

#include <commands.h>
#include "../lib/time.h"

//extern uint8_t * difTime(uint8_t * Time_0,uint8_t * Time_1);
extern int commandDispatcher(char * input);

int speed(char ** argv, int argc) {
    
    if(argc<2){
        print("Poner comando");
        return 0;
    }
    
    uint8_t datetime0[6];                               // tiempo inicial
    uint8_t datetimef[6];                               //tiempo final

    getDateTime(datetime0);
    uint64_t ms0=sys_get_ms();                          // mls iniciales

   
    commandDispatcher(argv[1]);

    getDateTime(datetimef);
    uint64_t msf=sys_get_ms();                          //mls finales
    uint8_t * dif=difTime(datetime0,datetimef);

   
    msf-=ms0;
    if(msf<0){
     //rerd();       //     reduce un segundo en la hora 
     msf=1-msf;           
    }
    println("El tiempo de procesamiento fue de:");
    printTime(dif);

    print("MS: ");
    printDec(msf);
    printChar('\n');

    
    return 0;
    


   
}