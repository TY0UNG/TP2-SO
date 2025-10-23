#include "time.h"

extern void sys_get_time(uint8_t* buffer);
extern uint64_t sys_get_ms();

void getDateTime(uint8_t* buffer) {
    sys_get_time(buffer);
}

uint64_t getMilisFromBoot() {
    return sys_get_ms();
}

void sleep(uint64_t ms) {
    uint64_t start = getMilisFromBoot();
    while (getMilisFromBoot() < start + ms);
}

void printTime(uint8_t * datetime){

    print("Fecha: ");
    printHex(datetime[2]);  // Dia
    printChar('/');
    printHex(datetime[1]);  // Mes
    printChar('/');
    printHex(datetime[0]);  // Año
    printChar('\n');
    
    print("Hora:  ");
    printHex((datetime[3] - 3));  // Hora
    printChar(':');
    printHex(datetime[4]);  // Minutos
    printChar(':');
    printHex(datetime[5]);  // Segundos
    printChar('\n');

    return;
}

uint8_t * difTime(uint8_t * Time_0,uint8_t * Time_1){               //falta ver q si sea de dim 6 
    uint8_t dif_T[6];

    int num=0;
   for(int i = 5; i > 0; i--) {    
       
              
        int aux= Time_1[i] - Time_0[i]-num;  
        
        if(num!=0)  num=0;
        if(aux<0){
         aux=0;
         num=aux*-1;
        }
        dif_T[i]=(uint8_t)aux;
    }

    return dif_T;
}
