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

void startText(){
    clearCanvas();
    drawTextCentered( "Iniciando juego...",200, 25, 0x00FF00,SCREEN_WIDTH);
    swapBuffers();
    sleep(2);   
}



int tron(char **argv, int argc) {
    enableGraphicsMode();

    int mode = menu();

    // --- INICIALIZAR JUEGO ---

    startText();           

    
    uint64_t last_time = getMilisFromBoot();

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
        drawTextCentered(mode == 1 ? "Modo 1 Jugador" : "Modo 2 Jugadores", 100,25, 0x00FFFF,SCREEN_WIDTH);
        
        //drawTextCentered(" lslslsls...",400, 25, 0x00FF00);
        swapBuffers();
    }

    disableGraphicsMode();
    return 0;
}
   