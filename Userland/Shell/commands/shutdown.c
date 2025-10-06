#include <commands.h>

extern void sys_shutdown();

int  shutdown() {
    sys_shutdown();
    return 0;
}