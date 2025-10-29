#include <stdint.h>
#include <draw.h>
#include <stddef.h>
#include "../lib/time.h"
#include "inout.h"

//VER TEMA DE PANTALLA 
#define SCREEN_WIDTH 1024                                   
#define SCREEN_HEIGHT 768

#define GRID_SIZE 4
#define SPEED 50 

// DIRECS
#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

// Colores                           VER 
#define COLOR_P1 0x00FFFF
#define COLOR_P2 0xFF00FF
#define COLOR_BACKGROUND 0x000000
#define COLOR_WALL 0xFFFFFF

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

////////////// dibuja ////////////////////////////

void drawWalls() {    
    drawFilledRectangle(0, 0, SCREEN_WIDTH, GRID_SIZE, COLOR_WALL);
    drawFilledRectangle(0, SCREEN_HEIGHT - GRID_SIZE, SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_WALL);
    drawFilledRectangle(0, 0, GRID_SIZE, SCREEN_HEIGHT, COLOR_WALL);
    drawFilledRectangle(SCREEN_WIDTH - GRID_SIZE, 0, SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_WALL);
}
void drawPlayer(Player *p) {
    int gx = p->x / GRID_SIZE, gy = p->y / GRID_SIZE;

    if (gx >= 0 && gx < SCREEN_WIDTH/GRID_SIZE && 
        gy >= 0 && gy < SCREEN_HEIGHT/GRID_SIZE) {
        grid[gy][gx] = (p->color == COLOR_P1) ? 1 : 2;
    }
    drawFilledRectangle(p->x, p->y, p->x + GRID_SIZE, p->y + GRID_SIZE, p->color);
}

void drawLivesCounter(int *livesP1, int *livesP2, int maxLives) {

    drawFilledRectangle(840, 10, 1010, 40, 0x222222); drawRectangle(840, 10, 1010, 40, 2, COLOR_WALL);
    
    char lvText[26] = "P1=     P2= ";
    lvText[4] = '0' + (maxLives - *livesP1);
    lvText[12] = '0' + (maxLives - *livesP2);
    drawText(850, 20, lvText, 22, 0xAAAAAA);
}

//////////////////////////////////////////////////////////////////////

int checkCollision(Player *p) {
    int gx = p->x / GRID_SIZE; int gy = p->y / GRID_SIZE;
    
    // bordes
    if (p->x < GRID_SIZE || p->x >= SCREEN_WIDTH - GRID_SIZE ||p->y < GRID_SIZE || p->y >= SCREEN_HEIGHT - GRID_SIZE) {
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
    if (next_x < GRID_SIZE || next_x >= SCREEN_WIDTH - GRID_SIZE ||next_y < GRID_SIZE || next_y >= SCREEN_HEIGHT - GRID_SIZE ||(gx >= 0 && gx < SCREEN_WIDTH/GRID_SIZE && gy >= 0 && gy < SCREEN_HEIGHT/GRID_SIZE && grid[gy][gx] != 0)) {
        
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


void  waitForContinue() {                    
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
    Player p2 = {SCREEN_WIDTH/4, SCREEN_HEIGHT/2, RIGHT, 1, COLOR_P2};
    Player p1 = {3*SCREEN_WIDTH/4, SCREEN_HEIGHT/2, LEFT, 1, COLOR_P1};
    
    uint64_t last_move = getMilisFromBoot();
    
    while (1) {
        KeyEvent *key = getKey();
        
        // Control P1 
        if (key != NULL) {
            if (key->scancode == 72 && p1.dir != DOWN) p1.dir = UP;             //  Arriba
            else if (key->scancode == 80 && p1.dir != UP) p1.dir = DOWN;        //  Abajo
            else if (key->scancode == 75 && p1.dir != RIGHT) p1.dir = LEFT;     // Izqu
            else if (key->scancode == 77 && p1.dir != LEFT) p1.dir = RIGHT;     //  derch
            
            // Control P2 
            if (mode == 2) {
                if (key->ascii == 'w' && p2.dir != DOWN) p2.dir = UP;
                else if (key->ascii == 's' && p2.dir != UP) p2.dir = DOWN;
                else if (key->ascii == 'a' && p2.dir != RIGHT) p2.dir = LEFT;
                else if (key->ascii == 'd' && p2.dir != LEFT) p2.dir = RIGHT;
            }
            
            if (key->scancode == 1) return 0;  // ESC
            if (key->scancode == 14) return 1; // DEL
        }
        
        uint64_t current_time = getMilisFromBoot();
        if (current_time - last_move >= SPEED) {
            last_move = current_time;
            
            // Actualizar maquina 
            if (mode == 1) {
                updateMachine(&p2, &p1);
            }
            
            // Mueve players 
            if (p1.alive) movePlayer(&p1);
            if (p2.alive) movePlayer(&p2);
            
            int col1 = 0, col2 = 0;
            if (p1.alive) col1 = checkCollision(&p1);
            if (p2.alive) col2 = checkCollision(&p2);
            
            // Si hay choque 
            if (col1 || col2) {                                     
    
                if (col1 && !col2) {
                    (*livesP1)++;
                } else if (col2 && !col1) {
                    
                    (*livesP2)++;
                }
                // OBS: Si chocan ambos, no se suma nada
                
                clearCanvas();
                drawTextCentered(mode == 1 ? "Modo 1 Jugador" : "Modo 2 Jugadores", 100, 25, 0x00FFFF, SCREEN_WIDTH);
                char livesText[50];
                if (mode == 1) {
                    drawTextCentered("Player", SCREEN_HEIGHT/2 - 100, 25, COLOR_P1, SCREEN_WIDTH);
                    
                } else {
                    drawTextCentered("Player 1", SCREEN_HEIGHT/2 - 100, 25, COLOR_P1, SCREEN_WIDTH);
                    drawTextCentered("Player 2", SCREEN_HEIGHT/2 - 50, 25, COLOR_P2, SCREEN_WIDTH);
                }
                //ve vidas 
                if (*livesP1 >= maxLives || *livesP2 >= maxLives) {
                    clearCanvas();

                    if (*livesP2 >= maxLives) {
                        drawTextCentered(mode == 1 ? "GANASTE!" :"Player 1 gana!", 100, 30, COLOR_P1, SCREEN_WIDTH);
                    }
                    else {
                        drawTextCentered(mode == 1 ? "CPU gana!" : "Player 2 gana!", 100, 30, COLOR_P2, SCREEN_WIDTH);
                    }
                }

                waitForContinue();
                return ;                            //
            }
        
            clearCanvas();
            drawWalls();
            
            // Dibujar lineas
            for (int i = 0; i < SCREEN_HEIGHT/GRID_SIZE; i++) {
                for (int j = 0; j < SCREEN_WIDTH/GRID_SIZE; j++) {
                    if (grid[i][j] == 1) {
                        drawFilledRectangle(j*GRID_SIZE, i*GRID_SIZE, j*GRID_SIZE + GRID_SIZE, i*GRID_SIZE + GRID_SIZE, COLOR_P1);
                    } else if (grid[i][j] == 2) {
                        drawFilledRectangle(j*GRID_SIZE, i*GRID_SIZE, j*GRID_SIZE + GRID_SIZE, i*GRID_SIZE + GRID_SIZE, COLOR_P2);
                    }
                }
            }
            
            if (p1.alive) drawPlayer(&p1);
            if (p2.alive) drawPlayer(&p2);
            
            drawText(10, 10, "DEL: Menu | ESC: Salir", 23, 0xAAAAAA);
            drawLivesCounter(livesP1, livesP2,  maxLives);
            
            drawTextCentered(mode == 1 ? "Modo 1 Jugador" : "Modo 2 Jugadores", 100, 25, 0x00FFFF, SCREEN_WIDTH);
            swapBuffers();
        }
    }
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

   if (mode != 0) {
        startText();
        int maxLives = 3, livesP1 = 0, livesP2 = 0, flag=0;

        while (1) {
            KeyEvent *key = getKey();
            if (key != NULL && key->scancode == 1) break; // ESC
            
            if (key != NULL && key->scancode == 14) { // DEL
                mode = menu();
                if (mode == 0) break;
                livesP1 = 0; livesP2 = 0;
                startText();
            }
            
            if (livesP1 >= maxLives || livesP2 >= maxLives) {
                livesP1 = 0; livesP2 = 0;
            }
            
            flag = playRound(mode, &livesP1, &livesP2, maxLives);

            //ver de no repetir code  y de en caso de DEL            
            if(flag==0) break;
        }
    }
    }
    disableGraphicsMode();
    return 0;
}

void check(){       // Aca lo q haria es para ver el tema de dDEL ESC para salir y ahorrar codido 
   
    while(1){
        KeyEvent *key = getKey();
            if (key != NULL && key->scancode == 1) break; // ESC
                
            if (key != NULL && key->scancode == 14) { // DEL
            }           // ver bien 
    }


}