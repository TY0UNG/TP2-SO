#include <commands.h>
#include <inout.h>
#include <stdint.h>

extern void sys_set_fps_overlay(uint8_t enable);

int show(char ** argv, int argc) {
    if (argc < 2 || strcmp(argv[1], "fps") != 0) {
        println("Uso: show fps [on|off]");
        return 1;
    }

    bool enable = true;
    if (argc >= 3) {
        if (strcmp(argv[2], "on") == 0) {
            enable = true;
        } else if (strcmp(argv[2], "off") == 0) {
            enable = false;
        } else {
            println("Uso: show fps [on|off]");
            return 1;
        }
    }

    sys_set_fps_overlay(enable ? 1 : 0);
    println(enable ? "Overlay de FPS habilitado." : "Overlay de FPS deshabilitado.");
    return 0;
}
