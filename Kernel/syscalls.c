#include <stdint.h>
#include <keyboard.h>
#include <video.h>
#include <interrupts.h>
#include "./drivers/time.h"

void dump_registers();          ////

typedef struct {
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
} Registers;

static int syscall_shutdown(Registers * registers);
static int syscall_write(Registers * registers);
static int syscall_read(Registers * registers);
static int syscall_clear(Registers * registers);
static int syscall_graphics_mode(Registers * registers);
static int syscall_draw_line(Registers * registers);
static int syscall_draw_rectangle(Registers * registers);
static int syscall_draw_filled_rectangle(Registers * registers);
static int syscall_draw_circle(Registers * registers);
static int syscall_draw_filled_circle(Registers * registers);
static int syscall_draw_text(Registers * registers);
static int syscall_clear_canvas(Registers * registers);
static int syscall_swap_buffers(Registers * registers);
static int syscall_time(Registers *registers);
static uint64_t syscall_ms(Registers *registers);
static KeyEvent * syscall_get_key(Registers *registers);
static uint64_t sycall_getRegs(Registers * registers);

uint64_t sysCallDispatcher(Registers * registers) {
    switch ((*registers).rax) {
    case 0:
        return syscall_shutdown(registers);
    case 1:
        return syscall_write(registers);
    case 2:
        return syscall_read(registers);
    case 3:
        return syscall_clear(registers);
    case 4:
        return syscall_graphics_mode(registers);
    case 5:
        return syscall_draw_line(registers);
    case 6:
        return syscall_draw_rectangle(registers);
    case 7:
        return syscall_draw_filled_rectangle(registers);
    case 8:
        return syscall_draw_circle(registers);
    case 9:
        return syscall_draw_filled_circle(registers);
    case 10:
        return syscall_draw_text(registers);
    case 11:
        return syscall_clear_canvas(registers);
    case 12:
        return syscall_swap_buffers(registers);
    case 13:
        return syscall_time(registers);
    case 14:
        return syscall_ms(registers);
    case 15:
        return syscall_get_key(registers);
    case 16:
        return sycall_getRegs(registers);
    default:
        break;
    }
    return 0;
}   

uint64_t sycall_getRegs(Registers * registers){                          /////ver esto 
     return (uint64_t)get_register_dump();
}

/// /////////
/// 


int syscall_write(Registers * registers) {
    selectStyle(registers->rbx == 2 ? 0x04 : 0x0F);
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
    clearTextBuffer();
    
    return 0;
}

int syscall_graphics_mode(Registers * registers) {
    if (registers->rbx) {
        canvasMode();
    } else {
        textMode();
    }
    return 0;
}

typedef struct {
    uint64_t x1, y1, x2, y2;
    uint16_t thickness;
    uint32_t color;
} LineParameters;

typedef struct {
    uint64_t x1, y1, x2, y2;
    uint16_t thickness;
    uint32_t color;
} RectangleParameters;

typedef struct {
    uint64_t x1, y1, x2, y2;
    uint32_t color;
} FilledRectangleParameters;

typedef struct {
    uint64_t x, y;
    uint16_t radius;
    uint16_t thickness;
    uint32_t color;
} CircleParameters;

typedef struct {
    uint64_t x, y;
    uint16_t radius;
    uint32_t color;
} FilledCircleParameters;

typedef struct {
    uint64_t x, y;
    const char* text;
    uint16_t height;
    uint32_t color;
} TextParameters;

int syscall_draw_line(Registers * registers) {
    LineParameters * params = (LineParameters *) registers->rbx;
    drawLine(params->x1, params->y1, params->x2, params->y2, params->thickness, params->color);
    return 0;
}

int syscall_draw_rectangle(Registers * registers) {
    RectangleParameters * params = (RectangleParameters *) registers->rbx;
    drawRectangle(params->x1, params->y1, params->x2, params->y2, params->thickness, params->color);
    return 0;
}

int syscall_draw_filled_rectangle(Registers * registers) {
    FilledRectangleParameters * params = (FilledRectangleParameters *) registers->rbx;
    drawFilledRectangle(params->x1, params->y1, params->x2, params->y2, params->color);
    return 0;
}

int syscall_draw_circle(Registers * registers) {
    CircleParameters * params = (CircleParameters *) registers->rbx;
    drawCircle(params->x, params->y, params->radius, params->thickness, params->color);
    return 0;
}

int syscall_draw_filled_circle(Registers * registers) {
    FilledCircleParameters * params = (FilledCircleParameters *) registers->rbx;
    drawFilledCircle(params->x, params->y, params->radius, params->color);
    return 0;
}

int syscall_draw_text(Registers * registers) {
    TextParameters * params = (TextParameters *) registers->rbx;
    drawText(params->x, params->y, params->text, params->height, params->color);
    return 0;
}

int syscall_clear_canvas(Registers * registers) {
    clearCanvas();
    return 0;
}

static int syscall_swap_buffers(Registers * registers) {
    swapBuffers();
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

int syscall_time(Registers * registers){

    uint8_t* datetime_buffer = (uint8_t*)registers->rbx;
    
    if (datetime_buffer == 0) {
        return -1;  // Error
    }
    
    getTime(datetime_buffer);
    
    return 0;

}

uint64_t syscall_ms(Registers * registers){
    return getMilisFromBoot();
}

static KeyEvent event;

KeyEvent * syscall_get_key(Registers * registers) {
    if (isKeyBufferEmpty()) return 0;
    event = getNextKey();
    return &event;
}