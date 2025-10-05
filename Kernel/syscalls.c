#include <stdint.h>
#include <keyboard.h>
#include <screen.h>

typedef struct {
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
} Registers;

static int syscall_write(Registers * registers);
static int syscall_read(Registers * registers);

int sysCallDispatcher(Registers * registers) {
    switch ((*registers).rax) {
    case 1:
        return syscall_write(registers);
    case 2:
        return syscall_read(registers);
    default:
        break;
    }
    return 0;
}

int syscall_write(Registers * registers) {
    ncSetStyle(registers->rbx == 2 ? 0x04 : 0x0F);
    ncPrint((char *) registers->rcx);
    return 1;
}

int syscall_read(Registers * registers) {
    char * input = (char *) registers->rbx;
    uint8_t size = 0;
    const uint8_t MAX = 255;

    _sti();                                                             ///

    for(;;){
        //if(isKeyBufferEmpty()) continue;
        if(isKeyBufferEmpty()) _hlt();                                  ////

        KeyEvent event = getNextKey();

        // Ignorar liberaciones y teclas sin ascii (modificadores)
        if(event.is_release || event.ascii == 0)
            continue;

        if(event.ascii == '\n'){
            input[size] = 0;
            ncPrint("\n");
            return size;
        } else if(event.ascii == '\b'){
            if(size > 0){
                size--;
                // borrar en pantalla: mover cursor atrás, espacio, atrás
                ncPrint("\b \b");
            }
        } else {
            if(size < MAX){
                input[size++] = event.ascii;
                char tmp[2] = { event.ascii, 0 };
                ncPrint(tmp);
            }
        }
    }
}
