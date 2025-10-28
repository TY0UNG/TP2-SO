#include <stdint.h>
#include <draw.h>
#include <stddef.h>
#include "../lib/time.h"
#include "inout.h"

//VER TEMA DE PANTALLA 
#define SCREEN_WIDTH 1024                                   
#define SCREEN_HEIGHT 768



int menu(){
    int mode = 0; // 1 jugador o 2 jugadores 

    // MENU
    clearCanvas();

   
    drawTextCentered("=== TRON GAME ===", 200, 28, 0x00FFFF,SCREEN_WIDTH);
    drawTextCentered("Presiona 1 para un jugador", 300, 20, 0xFFFFFF,SCREEN_WIDTH);
    drawTextCentered("Presiona 2 para dos jugadores", 340, 20, 0xFFFFFF,SCREEN_WIDTH);
    drawTextCentered("Presiona ESC para salir", 400, 20, 0xAAAAAA,SCREEN_WIDTH);

    swapBuffers();


   
        while (mode == 0) {
            KeyEvent *key = getKey();
            if (key != NULL) {
                if (key->scancode == 2) { // Tecla '1'
                    mode = 1;
                } else if (key->scancode == 3) { // Tecla '2'
                    mode = 2;
                } else if (key->scancode == 1) { // ESC
                    disableGraphicsMode();
                    return 0;
                }
            }
        }

    return mode;
}


int  waitForContinue() {                    // 1 si di salir 
    clearCanvas();
    drawTextCentered("Presiona ENTER para continuar", SCREEN_HEIGHT/2 + 50, 20, 0xFFFF00, SCREEN_WIDTH);
    swapBuffers();
    
    while (1) {
        KeyEvent *key = getKey();
        if (key != NULL) {
            if (key->scancode == 1) { // ESC
                return 1;
            }
            if (key->scancode == 28/*ver bien el numero */) { // ENTER
                break;
            }
        }
    }
    return 0;
}

void startText(){
    clearCanvas();
    drawTextCentered( "Iniciando juego...",200, 25, 0x00FF00,SCREEN_WIDTH);
    swapBuffers();
    sleep(9);   
}



int tron(char **argv, int argc) {
    enableGraphicsMode();

    int mode = menu();
    

    // --- INICIALIZAR JUEGO ---


    
    
    uint64_t last_time = getMilisFromBoot();
    if(mode!=0){
    startText();
    while (1) {
        KeyEvent *key = getKey();
        if (key != NULL && key->scancode == 1) { // ESC para salir
            break;
        }
         if (key != NULL && key->scancode == 14) { // delete para volver a menu  ver si hacer por ascii key->ascii == '\b' o 
            mode = menu();
            if (mode == 0) break;
            startText();
        }


        
        clearCanvas();
        //Se juega con el modo q eligio 
        int maxLives=3;
        int flag=0;                         //0 para continuar 1 para terminar 

        for(int livesP1=0 ,livesP2=0; 1&&flag==0 ; ){// esto podria ser una funcion aparte    //el p2 es puede ser la maquina                           
            drawText(10, 10, "DEL para volver a menu",19, 0xAAAAAA);
            drawTextCentered(mode == 1 ? "Modo 1 Jugador" : "Modo 2 Jugadores", 100,25, 0x00FFFF,SCREEN_WIDTH);
            
            // IDEA 

            //se arma arma el juego, si es de un player se juega con flechas y si es d dos p2 a w d
            
            /*
            se hace lectura de teclado 

            //-------Si chocan ---------

            if(){                               //aca si se impacta solo el p1      
                livesP1++;
            }
              if(){                             //aca si se impacta solo el p2
                livesP2++;
            }
            
            // obs si chocan a la vez no se suna 

            //si alguno choca vamos a una especie de menu pera dar  enter para continuar 
              
                flag= waitForContinue();            

            }
                    
            */
            swapBuffers();

        }
        
    }
    }

    disableGraphicsMode();
    return 0;
}
   