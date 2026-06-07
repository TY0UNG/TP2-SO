#include <commands.h>
#include <draw.h>
#include "../lib/str.h"
#include "../lib/time.h"

int fps(char ** argv, int argc) {
    enableGraphicsMode();
    set_terminal_mode(TERM_RAW);   // teclas crudas via getKey(), sin echo

    uint32_t color = 0x00000000;
    uint64_t actual_fps = 0;
    uint64_t fps_sum = 0;
    uint64_t fps_count = 0;

    char fps_buffer[30] = "FPS: ";
    char promedio_buffer[30] = "Promedio: ";

    // Use RTC to update FPS once per second
    DateTime dt;
    getDateTime(&dt);
    uint8_t prev_second = dt.seconds;
    uint64_t frames_this_second = 0;

    while (1) {
        clearCanvas();

        drawFillScreen(color);

        // Display last computed fps (updated when RTC second changes)
        parseInt(actual_fps, fps_buffer + 5, 25);
        drawText(275, 200, fps_buffer, 100, 0xFFFFFFFF);

        if (fps_count != 0) {
            parseInt(fps_sum / fps_count, promedio_buffer + 10, 20);
            drawText(200, 300, promedio_buffer, 100, 0xFFFFFFFF);
        }

        swapBuffers();

        // One more frame was rendered in the current second
        frames_this_second++;

        // Check RTC seconds; when it changes, commit FPS for the finished second
        getDateTime(&dt);
        if (dt.seconds != prev_second) {
            actual_fps = frames_this_second;
            fps_sum += actual_fps;
            fps_count++;
            frames_this_second = 0;
            prev_second = dt.seconds;
        }

        color += 1000;

        KeyEvent *key = getKey();
        if (key != 0 && key->scancode == 1) {
            break;
        }
    }

    set_terminal_mode(TERM_COOKED);
    disableGraphicsMode();
    return 0;
}