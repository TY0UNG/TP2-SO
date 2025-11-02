#include <stdint.h>
#include <draw.h>
#include <stddef.h>
#include "../lib/time.h"
#include "inout.h"
#include <sound.h>

//VER TEMA DE PANTALLA 
#define SCREEN_WIDTH 1024                                   
#define SCREEN_HEIGHT 768

#define GRID_SIZE 6
#define DELAY 4 
#define MARGINSUP (GRID_SIZE*20)
// DIRECS
#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

// Colores                           VER 

#define COLOR_P1 0x0080FF  
#define COLOR_P2 0xFF4000  

#define COLOR_BACKGROUND 0x000820 /*0x0a0015*/
#define COLOR_WALL 0x0088AA
#define COLOR_GRID 0x333333

typedef struct {
    int x, y;
    int dir;
    int alive;
    int power;                              // aca poner el boost de velocidad ???
    uint32_t color;
} Player;

// Grid para detectar colisiones
static uint8_t grid[SCREEN_HEIGHT/GRID_SIZE][SCREEN_WIDTH/GRID_SIZE];

void initGrid() {       // Senaliza en  0
    for (int i = 0; i < SCREEN_HEIGHT/GRID_SIZE; i++) {
        for (int j = 0; j < SCREEN_WIDTH/GRID_SIZE; j++) {
            grid[i][j] = 0;
        }
    }
}
////
void inGameMusic(){                 
    
    play_sound(440, 220);   // La4
    play_sound(0,   55);    // Pausa
    play_sound(660, 220);   // Mi5
    play_sound(0,   55);    // Pausa
    play_sound(880, 220);   // La5
    play_sound(0,   55);    // Pausa
    play_sound(660, 220);   // Mi5
    play_sound(0,   55);    // Pausa
    play_sound(523, 220);   // Do5
    play_sound(0,   55);    // Pausa
    play_sound(698, 220);   // Fa5
    play_sound(0,   55);    // Pausa
    play_sound(880, 220);   // La5
    play_sound(0,   55);    // Pausa
    play_sound(440, 220);   // La4
    play_sound(0, 400); 
    
}
void crashSound(){
    clear_audio_buffer();

    play_sound(200, 110);  
    play_sound(0, 55);     
    play_sound(80, 110);   
    play_sound(0, 55);     // Pausa (1 tick)
    play_sound(50, 110);   // Tono bajísimo (2 ticks)

}

void MenuMusic(){

    play_sound(523, 200);   // C5
     play_sound(0, 110);
    play_sound(659, 200);   // E5
     play_sound(0, 110);
    play_sound(784, 250);   // G5
    play_sound(0, 110);
    play_sound(659, 200);   // E5
    play_sound(0, 110);
    play_sound(523, 250);
    play_sound(0, 400);
       

}

////////////// dibuja ////////////////////////////

void drawWalls() {    
    drawFilledRectangle(0, 0, SCREEN_WIDTH, GRID_SIZE, COLOR_WALL);
    drawFilledRectangle(0, SCREEN_HEIGHT - GRID_SIZE, SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_WALL);
    drawFilledRectangle(0, 0, GRID_SIZE, SCREEN_HEIGHT, COLOR_WALL);
    drawFilledRectangle(SCREEN_WIDTH - GRID_SIZE, 0, SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_WALL);
}
void drawPlayer(Player *p) {
    drawFilledRectangle(p->x, p->y, p->x + GRID_SIZE, p->y + GRID_SIZE, p->color);
}

void drawLivesCounter(int *livesP1, int *livesP2, int maxLives) {

    drawFilledRectangle(840, 10, 1010, 40, 0x222222); drawRectangle(840, 10, 1010, 40, 2, COLOR_WALL);
    
    char lvText[26] = "P1=     P2= ";
    lvText[4] = '0' + (maxLives - *livesP1);
    lvText[12] = '0' + (maxLives - *livesP2);
    drawText(850, 20, lvText, 22, 0xAAAAAA);
}

void drawTable(int mode ,int *livesP1, int *livesP2, int maxLives){
    drawWalls();
    drawLivesCounter(livesP1, livesP2,  maxLives);
    drawText(10, 20, "DEL: Menu ", 23, 0xAAAAAA);
    drawTextCentered(mode == 1 ? "Modo 1 Jugador" : "Modo 2 Jugadores", MARGINSUP - 40 , 25, 0x00FFFF, SCREEN_WIDTH);               //ver
    drawFilledRectangle(0, MARGINSUP - GRID_SIZE, SCREEN_WIDTH, MARGINSUP, COLOR_WALL);                                             //VER 
    
    // Fondo dentro de tablero 
    drawFilledRectangle(GRID_SIZE, MARGINSUP, SCREEN_WIDTH - GRID_SIZE, SCREEN_HEIGHT - GRID_SIZE, COLOR_BACKGROUND);               //
    
    for (int x = GRID_SIZE; x <= SCREEN_WIDTH - GRID_SIZE; x += GRID_SIZE*6) {
        drawFilledRectangle(x, MARGINSUP, x + 1, SCREEN_HEIGHT - GRID_SIZE, COLOR_GRID);
    }
    for (int y = MARGINSUP; y <= SCREEN_HEIGHT - GRID_SIZE; y += GRID_SIZE*6) {
        drawFilledRectangle(GRID_SIZE, y, SCREEN_WIDTH - GRID_SIZE, y + 1, COLOR_GRID);
    }

}
//////////////////////////////////////////////////////////////////////

int checkCollision(Player *p) {
    int gx = p->x / GRID_SIZE; int gy = p->y / GRID_SIZE;
    
    // bordes           
    if (p->x < GRID_SIZE || p->x >= SCREEN_WIDTH - GRID_SIZE ||p->y < MARGINSUP || p->y >= SCREEN_HEIGHT - GRID_SIZE) {           //ver
        return 1;
    }
    // Contrincantes   
    if (gx >= 0 && gx < SCREEN_WIDTH/GRID_SIZE &&  gy >= 0 && gy < SCREEN_HEIGHT/GRID_SIZE) {
        if (grid[gy][gx] != 0) return 1;
    }
    return 0;
}

void movePlayer(Player *p) {
   
    switch (p->dir) {
        case UP:    p->y -= GRID_SIZE; break;
        case DOWN:  p->y += GRID_SIZE; break;
        case LEFT:  p->x -= GRID_SIZE; break;
        case RIGHT: p->x += GRID_SIZE; break;
    }
}

//Ver temas de switch 
void updateMachine(Player *mach, Player *player) {                          /// cambiar                     
    int next_x = mach->x, next_y = mach->y;
    
    switch (mach->dir) {
        case UP:    next_y -= GRID_SIZE; break;
        case DOWN:  next_y += GRID_SIZE; break;
        case LEFT:  next_x -= GRID_SIZE; break;
        case RIGHT: next_x += GRID_SIZE; break;
    }

    int gx = next_x / GRID_SIZE, gy = next_y / GRID_SIZE;
    
    // ve obst 
    if (next_x < GRID_SIZE || next_x >= SCREEN_WIDTH - GRID_SIZE ||next_y < MARGINSUP || next_y >= SCREEN_HEIGHT - GRID_SIZE ||(gx >= 0 && gx < SCREEN_WIDTH/GRID_SIZE && gy >= 0 && gy < SCREEN_HEIGHT/GRID_SIZE && grid[gy][gx] != 0)) {
        
        // Intentar girar a la derecha o izquierda
        int new_dirs[2] = {(mach->dir + 1) % 4, (mach->dir + 3) % 4};
        
        for (int i = 0; i < 2; i++) {
            int test_dir = new_dirs[i];
            int test_x = mach->x;
            int test_y = mach->y;
            
            switch (test_dir) {
                case UP:    test_y -= GRID_SIZE; break;
                case DOWN:  test_y += GRID_SIZE; break;
                case LEFT:  test_x -= GRID_SIZE; break;
                case RIGHT: test_x += GRID_SIZE; break;
            }
            
            int tgx = test_x / GRID_SIZE, tgy = test_y / GRID_SIZE;
            
            if (test_x >= GRID_SIZE && test_x < SCREEN_WIDTH - GRID_SIZE &&test_y >= GRID_SIZE && test_y < SCREEN_HEIGHT - GRID_SIZE &&(tgx < 0 || tgx >= SCREEN_WIDTH/GRID_SIZE || tgy < 0 || tgy >= SCREEN_HEIGHT/GRID_SIZE || grid[tgy][tgx] == 0)) {
                mach->dir = test_dir;
                break;
            }
        }
    }
}


int menu(){
     
    // MENU
    clearCanvas();
    

    drawTextCentered("=== TRON GAME ===", 200, 28, 0x00FFFF,SCREEN_WIDTH);
    drawTextCentered("Presiona 1 para un jugador", 300, 20, 0xFFFFFF,SCREEN_WIDTH);
    drawTextCentered("Presiona 2 para dos jugadores", 340, 20, 0xFFFFFF,SCREEN_WIDTH);
    drawTextCentered("Presiona ESC para salir", 400, 20, 0xAAAAAA,SCREEN_WIDTH);

   swapBuffers();
   clear_audio_buffer();

   MenuMusic();

    while (1) {
        if (is_audio_buffer_empty()) MenuMusic(); // vuelve a encolar la secuencia de notas
        
        KeyEvent *key = getKey();
        if (key != NULL) {
            
            if (key->scancode == 2) { // Tecla '1'
                return  1;
            } else if (key->scancode == 3) { // Tecla '2'
                    return 2;
            } else if (key->scancode == 1) { // ESC
                 return 0;
            }

        }
    }

}


void  waitForContinue() {           
    drawWalls();         
    drawTextCentered("Presiona ENTER para continuar", SCREEN_HEIGHT/2 + 100, 20, 0xFFFF00, SCREEN_WIDTH);
    swapBuffers();
    
    while (1) {
        KeyEvent *key = getKey();
        if (key != NULL) {
            if (key->scancode == 1) { // ESC
                return ;
            }
            if (key->scancode == 28/*ver bien el numero */) { // ENTER
                break;
            }
        }
    }
   
}

int playRound(int mode, int *livesP1, int *livesP2, int maxLives) {                
    
    initGrid();
    Player p2 = {SCREEN_WIDTH/4, SCREEN_HEIGHT/2, RIGHT, 1, 0,COLOR_P2};
    Player p1 = {3*SCREEN_WIDTH/4, SCREEN_HEIGHT/2, LEFT, 1, 0,COLOR_P1};
    
    uint64_t last_move = getMilisFromBoot();
    clearCanvas();   clear_audio_buffer();
    drawTable(mode,livesP1, livesP2,  maxLives);

    inGameMusic();

    while (1) {
        if (is_audio_buffer_empty()) inGameMusic();
        KeyEvent *key = getKey();
        
        // Control P1 
        if (key != NULL) {                                                                                          // se puede hacer generico 
            
            if (key->scancode == 72 && p1.dir != DOWN) p1.dir = UP;             //  Arriba
            else if (key->scancode == 80 && p1.dir != UP) p1.dir = DOWN;        //  Abajo
            else if (key->scancode == 75 && p1.dir != RIGHT) p1.dir = LEFT;     // Izqu
            else if (key->scancode == 77 && p1.dir != LEFT) p1.dir = RIGHT;     //  derch
            else if (key->ascii == 'm' ) p1.power = 3;                          // Speedboost 
            
            // Control P2 
            if (mode == 2) {
                if (key->ascii == 'w' && p2.dir != DOWN) p2.dir = UP;
                else if (key->ascii == 's' && p2.dir != UP) p2.dir = DOWN;
                else if (key->ascii == 'a' && p2.dir != RIGHT) p2.dir = LEFT;
                else if (key->ascii == 'd' && p2.dir != LEFT) p2.dir = RIGHT;
                else if (key->ascii == 'q' ) p2.power = 3;                          // Speedboost 
                 
            }
            
            if (key->scancode == 1) return 0;  // ESC                   ///sacar 
            if (key->scancode == 14) return 0; // DEL
        }
        
        uint64_t current_time = getMilisFromBoot();
        if (current_time - last_move >= DELAY) {
            last_move = current_time;
            if (mode == 1) {
                updateMachine(&p2, &p1);
            }

        int col1 = 0, col2 = 0;
            
    
    // Mover P1 
    if (p1.alive) {
        int moves_p1 = (p1.power > 0) ? 5 : 1;
        
        for (int i = 0; i < moves_p1; i++) {
            movePlayer(&p1);
            
            // PRIMERO verificar colisión (antes de marcar)
            if (checkCollision(&p1)) {
                col1 = 1;
                break;
            }
            //marcar en grid
            //Para que este alineado 
             p1.x = (p1.x / GRID_SIZE) * GRID_SIZE;   p1.y = (p1.y / GRID_SIZE) * GRID_SIZE;
            int gx = p1.x / GRID_SIZE;  int gy = p1.y / GRID_SIZE;
            if (gx >= 0 && gx < SCREEN_WIDTH/GRID_SIZE && 
                gy >= 0 && gy < SCREEN_HEIGHT/GRID_SIZE) {
                grid[gy][gx] = 1;
            }

             drawPlayer(&p1);
            
        }
        
        if (p1.power > 0) p1.power = 0;
    }

    // Mover P2 con power boost
    if (p2.alive) {                                                     //HACER GENERIC
        int moves_p2 = (p2.power > 0) ? 3 : 1;
        
        for (int i = 0; i < moves_p2; i++) {
            movePlayer(&p2);
            if (checkCollision(&p2)) {
                col2 = 1;
                break;
            }
             //Para que este alineado 
             p2.x = (p2.x / GRID_SIZE) * GRID_SIZE;   p2.y = (p2.y / GRID_SIZE) * GRID_SIZE;
            int gx = p2.x / GRID_SIZE;   int gy = p2.y / GRID_SIZE;

            if (gx >= 0 && gx < SCREEN_WIDTH/GRID_SIZE && gy >= 0 && gy < SCREEN_HEIGHT/GRID_SIZE) {
                grid[gy][gx] = 2;
            }
            drawPlayer(&p2);
        }
        
        if (p2.power > 0) p2.power = 0;                             //ver
    }


    // Si hay choque 
    if (col1 || col2) {
        crashSound();

        if (col1 && !col2) {
            (*livesP2)++;
        } else if (col2 && !col1) {
            (*livesP1)++;
        }
        // OBS: Si chocan ambos, no se suma nada
        
        
        //ve vidas 
        if (*livesP1 >= maxLives || *livesP2 >= maxLives) {
            clearCanvas();
            if (*livesP1 >= maxLives) {
                drawTextCentered(mode == 1 ? "GANASTE!" :"Player 1 gana!", 100, 30, COLOR_P1, SCREEN_WIDTH);
            }
            else {
                drawTextCentered(mode == 1 ? "CPU gana!" : "Player 2 gana!", 100, 30, COLOR_P2, SCREEN_WIDTH);
            }
        } 
        
        //waitForContinue();
        drawTextCentered("Presiona ESC para continuar o DEl para volver  menu ", SCREEN_HEIGHT/2 + 100, 30, 0xFFFF00, SCREEN_WIDTH);
        swapBuffers();
        return check();
    }
    
        swapBuffers();
            
        }
    }
}

void startText(){
    clearCanvas();
    drawTextCentered( "Iniciando juego...",200, 25, 0x00FF00,SCREEN_WIDTH);
    swapBuffers();
    sleep(3);   
}

int tron(char **argv, int argc) {
    enableGraphicsMode();
    
    // --- INICIALIZAR JUEGO ---
    
    uint64_t last_time = getMilisFromBoot();
   
    int maxLives = 3, livesP1 = 0, livesP2 = 0, flag=0;
    int mode;

    while ( ( mode=menu() )!=0) {
        startText();   
        while(playRound(mode, &livesP1, &livesP2, maxLives)){

            if (livesP1 >= maxLives || livesP2 >= maxLives) {
                    livesP1 = 0; livesP2 = 0;
            }
        }
    }

    clear_audio_buffer();
    disableGraphicsMode();
    return 0;
}

int check(){       // Aca lo q haria es para ver el tema de dDEL ESC para salir y ahorrar codido 
   
    while(1){
        KeyEvent *key = getKey();

            if (key != NULL && key->scancode == 1)return 1 ;                   // enter toca ver por ahor pongo esc
            
                
            if (key != NULL && key->scancode == 14) return 0;// DEL
                   // ver bien 
    }


}