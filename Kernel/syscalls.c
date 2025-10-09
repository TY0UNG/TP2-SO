#include <stdint.h>
#include <keyboard.h>
#include <video.h>
#include <interrupts.h>

typedef struct {
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
} Registers;

static int syscall_write(Registers * registers);
static int syscall_read(Registers * registers);
static int syscall_clear(Registers * registers);
static int  syscall_shutdown(Registers * registers);

int sysCallDispatcher(Registers * registers) {
    switch ((*registers).rax) {
    case 0:
        return syscall_shutdown(registers);
    case 1:
        return syscall_write(registers);
    case 2:
        return syscall_read(registers);
    case 3:
        return  syscall_clear(registers);
    default:
        break;
    }
    return 0;
}

int syscall_write(Registers * registers) {
    select_style(registers->rbx == 2 ? 0x04 : 0x0F);
    print((char *) registers->rcx);
    return 1;
}

int syscall_read(Registers * registers) {
    char * input = (char *) registers->rbx;
    uint8_t size = 0;

    _sti();

    while(1) {
        if (!isKeyBufferEmpty()) {
            KeyEvent event = getNextKey();
                if (!event.is_release) {
                    if(event.ascii == '\n') {
                        input[size] = 0;
                        print("\n");
                        return size;
                    } else if(event.ascii == '\b'){
                        if(size > 0){
                            size--;
                            deleteChar();
                        }
                    } else if (event.printable) {
                        input[size++] = event.ascii;
                        printChar(event.ascii);
                    }
            }
        }
    }
}

int syscall_clear(Registers * registers) {
    clear_text_buffer();  
    return 0;
}

int   syscall_shutdown(Registers * registers){
    syscall_clear(registers);

    print("Apagando....");
    //poner un tiempo  antes de apagar
    /*
    
    */ 


    __asm__ volatile ("outw %0, %1" : : "a"((uint16_t)0x2000), "Nd"((uint16_t)0x604));   // solo funciona en QEMU    //ver bien 
    /*|--> mov ax, 0x2000    ; Poner valor en AX
           mov dx, 0x604     ; Poner puerto en DX
           out dx, ax        ; Enviar AX al puerto DX  
    */

   //aca seria maquina fisica 



    //si no apago 
    print("NO SE PUDO APAGAR");
    while(1) {
        _hlt();
    }

    return 0;
}