#include <commands.h>
#include <draw.h>
#include "../lib/str.h"
#include "../lib/time.h"

int fps(char ** argv, int argc) {
    enableGraphicsMode();

    uint32_t color = 0x00000000;
    uint64_t actual_fps = 3;
    uint64_t fps_sum = 0;
    uint64_t fps_count = 0;

    char fps_buffer[30] = "FPS: ";
    char promedio_buffer[30] = "Promedio: ";

    while(1) {
        clearCanvas();

        uint64_t startMS = getMilisFromBoot();

        drawFillScreen(color);

        parseInt(actual_fps, fps_buffer+5, 25);
        drawText(275, 200, fps_buffer, 100, 0xFFFFFFFF);

        if (fps_count != 0) {
            parseInt(fps_sum / fps_count, promedio_buffer+10, 20);
            drawText(200, 300, promedio_buffer, 100, 0xFFFFFFFF);
        }

        swapBuffers();
        uint64_t finishMS = getMilisFromBoot();
        actual_fps = 1000 / (finishMS - startMS);
        fps_sum += actual_fps;
        fps_count++;
        color+=1000;

        KeyEvent * key = getKey();
        if (key != 0 && key->scancode == 1) {
            break;
        }
    }

    disableGraphicsMode();
    return 0;
}