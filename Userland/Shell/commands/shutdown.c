#include <commands.h>

extern void sys_shutdown();

int shutdown(char ** argv, int argc) {
    sys_shutdown();
    return 0;
}