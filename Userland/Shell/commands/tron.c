#include <stdint.h>
#include <draw.h>
#include <stddef.h>
#include "../lib/time.h"
#include "inout.h"

void sleep(uint64_t ms) {
    uint64_t start = getMilisFromBoot();
    while (getMilisFromBoot() < start + ms);
}

/**
 * Anima un círculo que se comporta como una pelota rebotando
 * con gravedad y pérdida de energía (restitución).
 */
int tron(char ** argv, int argc) {
    enableGraphicsMode();
    // --- Configuración de la pantalla y el objeto ---
    const uint32_t SCREEN_WIDTH = 1024;
    const uint32_t SCREEN_HEIGHT = 768;
    const uint32_t COLOR = 0x00FFFF; // Color cian

    // --- NUEVO: Constantes de física ---
    const float GRAVITY = 980.0f;     // Aceleración hacia abajo en píxeles/segundo^2
    const float RESTITUTION = 0.85f;  // Factor de "rebote" (85%). 1.0 es un rebote perfecto.
    const float FRICTION = 0.999f;    // Fricción ligera para que pierda velocidad horizontal

    // Propiedades del círculo
    float x = 100.0f;
    float y = 100.0f;
    float radius = 20.0f;
    
    // Velocidad inicial
    float vx = 350.0f;
    float vy = 0.0f;

    // --- Inicialización del temporizador para Delta Time ---
    uint64_t last_time = getMilisFromBoot();

    // --- Bucle principal de la animación ---
    while (1) {
        // 1. CALCULAR DELTA TIME
        // ---------------------------------------------------------------------
        uint64_t current_time = getMilisFromBoot();
        float delta_time = (float)(current_time - last_time) / 1000.0f;
        last_time = current_time;

        // 2. ACTUALIZAR LA FÍSICA (VELOCIDAD Y POSICIÓN)
        // ---------------------------------------------------------------------
        
        // a. APLICAR ACELERACIONES (GRAVEDAD Y FRICCIÓN)
        // La gravedad solo afecta la velocidad vertical.
        vy += GRAVITY * delta_time;
        // La fricción afecta la velocidad horizontal.
        vx *= FRICTION;

        // b. ACTUALIZAR POSICIÓN
        // La posición se actualiza con la nueva velocidad.
        x += vx * delta_time;
        y += vy * delta_time;

        // 3. DETECTAR COLISIONES Y APLICAR REBOTE CON PÉRDIDA DE ENERGÍA
        // ---------------------------------------------------------------------
        // Colisión con borde izquierdo o derecho
        if (x - radius < 0) {
            x = radius;
            vx = -vx * RESTITUTION; // Invierte y reduce velocidad
        } else if (x + radius > SCREEN_WIDTH) {
            x = SCREEN_WIDTH - radius;
            vx = -vx * RESTITUTION;
        }

        // Colisión con el suelo (borde inferior)
        if (y + radius > SCREEN_HEIGHT) {
            y = SCREEN_HEIGHT - radius; // Corrige la posición
            vy = -vy * RESTITUTION;     // Invierte y reduce la velocidad vertical
        }
        
        // Colisión con el techo (borde superior)
        if (y - radius < 0) {
            y = radius;
            vy = -vy * RESTITUTION;
        }

        KeyEvent * key = getKey();

        if (key != NULL && key->scancode == 1) {
            break;
        }

        // 4. RENDERIZAR EL FOTOGRAMA
        // ---------------------------------------------------------------------
        drawFilledCircle((uint64_t)x, (uint64_t)y, (uint16_t)radius, COLOR);
        swapBuffers();
        drawFilledCircle((uint64_t)x, (uint64_t)y, (uint16_t)radius, 0);
    }

    disableGraphicsMode();
    return 0;
}