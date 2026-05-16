#include <stdint.h>
#include <keyboard.h>
#include <video.h>
#include <interrupts.h>
#include <audio.h>
#include <processes.h>
#include "./drivers/memory.h"
#include "./drivers/time.h"

extern const char * get_register_dump();

typedef struct {
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rdi;
} Registers;

static int syscall_write(Registers * registers);
static int syscall_read(Registers * registers);
static int syscall_clear(Registers * registers);
static int syscall_graphics_mode(Registers * registers);
static int syscall_draw_line(Registers * registers);
static int syscall_draw_rectangle(Registers * registers);
static int syscall_draw_filled_rectangle(Registers * registers);
static int syscall_draw_fill_screen(Registers * registers);
static int syscall_draw_circle(Registers * registers);
static int syscall_draw_filled_circle(Registers * registers);
static int syscall_draw_text(Registers * registers);
static int syscall_clear_canvas(Registers * registers);
static int syscall_swap_buffers(Registers * registers);
static int syscall_time(Registers *registers);
static uint64_t syscall_ms(Registers *registers);
static KeyEvent * syscall_get_key(Registers *registers);
static uint64_t sycall_getRegs(Registers * registers);
static int syscall_set_text_size(Registers * registers);
static int syscall_set_fps_overlay(Registers * registers);
static int syscall_play_sound(Registers *registers);
static int syscall_is_audio_buffer_empty(Registers *registers);
static int syscall_clear_audio_buffer(Registers *registers);
static void * syscall_malloc(Registers *registers);
static int syscall_free(Registers *registers);
static uint64_t syscall_get_total_memory(Registers *registers);
static uint64_t syscall_get_used_memory(Registers *registers);
static uint64_t syscall_create_process(Registers *registers);
static int syscall_exit(Registers *registers);
static int syscall_wait(Registers *registers);
static int syscall_yield(Registers *registers);

uint64_t sysCallDispatcher(Registers * registers) {
    switch ((*registers).rax) {
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
        return (uint64_t)syscall_get_key(registers);
    case 16:
        return sycall_getRegs(registers);
    case 17:
        return syscall_set_text_size(registers);
    case 18:
        return syscall_play_sound(registers);
    case 19:
        return syscall_is_audio_buffer_empty(registers);
    case 20:
        return syscall_clear_audio_buffer(registers);
    case 21:
        return syscall_draw_fill_screen(registers);
    case 22:
        return syscall_set_fps_overlay(registers);
    case 23:
        return (uint64_t) syscall_malloc(registers);
    case 24:
        return syscall_free(registers);
    case 25:
        return syscall_get_total_memory(registers);
    case 26:
        return syscall_get_used_memory(registers);
    case 27:
        return syscall_create_process(registers);
    case 28:
        return syscall_exit(registers);
    case 29:
        return syscall_wait(registers);
    case 30:
        return syscall_yield(registers);
    default:
        break;
    }
    return 0;
}

static uint64_t syscall_create_process(Registers *registers) {
    const char *name = (const char *) registers->rbx;
    void (*entry)() = (void (*)()) registers->rcx;
    // argc en rdx (no se usa directamente; create_process lo recalcula desde argv)
    (void) registers->rdx;
    const char **argv = (const char **) registers->rdi;
    return (uint64_t) create_process(name, entry, argv);
}

static int syscall_exit(Registers *registers) {
    int status = (int) registers->rbx;
    exit_current_process(status);
    return 0;   // no llega
}

static int syscall_wait(Registers *registers) {
    pid_t pid = (pid_t) registers->rbx;
    return wait_pid(pid);
}

static int syscall_yield(Registers *registers) {
    (void) registers;
    yield();
    return 0;
}

static uint64_t sycall_getRegs(Registers * registers){
     return (uint64_t)get_register_dump();
}

static int syscall_write(Registers * registers) {
    selectStyle(registers->rbx == 2 ? 0x04 : 0x0F);
    print((char *) registers->rcx);
    return 1;
}

static int syscall_read(Registers * registers) {
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

static int syscall_play_sound(Registers *registers) {
    uint16_t frequency = (uint16_t)registers->rbx;
    uint16_t duration = (uint16_t)registers->rcx;
    play_sound(frequency, duration);
    return 0;
}

static int syscall_is_audio_buffer_empty(Registers *registers) {
    return isAudioBufferEmpty() ? 1 :0;
}

static int syscall_clear_audio_buffer(Registers *registers) {
    clearAudioBuffer();
    return 0;
}



static int syscall_clear(Registers * registers) {
    clearTextBuffer();  
    return 0;
}

static int syscall_graphics_mode(Registers * registers) {
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
    bool directWrite;
} FilledRectangleParameters;

typedef struct {
    uint32_t color;
} FillScreenParameters;

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

static int syscall_draw_line(Registers * registers) {
    LineParameters * params = (LineParameters *) registers->rbx;
    drawLine(params->x1, params->y1, params->x2, params->y2, params->thickness, params->color);
    return 0;
}

static int syscall_draw_rectangle(Registers * registers) {
    RectangleParameters * params = (RectangleParameters *) registers->rbx;
    drawRectangle(params->x1, params->y1, params->x2, params->y2, params->thickness, params->color);
    return 0;
}

static int syscall_draw_filled_rectangle(Registers * registers) {
    FilledRectangleParameters * params = (FilledRectangleParameters *) registers->rbx;
    drawFilledRectangle(params->x1, params->y1, params->x2, params->y2, params->color, params->directWrite);
    return 0;
}

static int syscall_draw_fill_screen(Registers * registers) {
    FillScreenParameters * params = (FillScreenParameters *) registers->rbx;
    fillScreen(params->color);
    return 0;
}

static int syscall_draw_circle(Registers * registers) {
    CircleParameters * params = (CircleParameters *) registers->rbx;
    drawCircle(params->x, params->y, params->radius, params->thickness, params->color);
    return 0;
}

static int syscall_draw_filled_circle(Registers * registers) {
    FilledCircleParameters * params = (FilledCircleParameters *) registers->rbx;
    drawFilledCircle(params->x, params->y, params->radius, params->color);
    return 0;
}

static int syscall_draw_text(Registers * registers) {
    TextParameters * params = (TextParameters *) registers->rbx;
    drawText(params->x, params->y, params->text, params->height, params->color);
    return 0;
}

static int syscall_clear_canvas(Registers * registers) {
    clearCanvas();
    return 0;
}

static int syscall_swap_buffers(Registers * registers) {
    swapBuffers();
    return 0;
}

static int syscall_time(Registers * registers){
    DateTime * datetime_buffer = (DateTime *) registers->rbx;
    
    if (datetime_buffer == 0) {
        return -1;  // Error
    }
    
    getTime(datetime_buffer);
    
    return 0;
}

static uint64_t syscall_ms(Registers * registers){
    return getMilisFromBoot();
}

static KeyEvent event;

static KeyEvent * syscall_get_key(Registers * registers) {
    if (isKeyBufferEmpty()) return 0;
    event = getNextKey();
    return &event;
}

static int syscall_set_text_size(Registers * registers) {
    uint16_t height = (uint16_t)registers->rbx;
    setTextSize(height);
    return 0;
}

static int syscall_set_fps_overlay(Registers * registers) {
    setFpsOverlayEnabled(registers->rbx != 0);
    return 0;
}

static void * syscall_malloc(Registers * registers) {
    return malloc((size_t) registers->rbx);
}


static int syscall_free(Registers * registers) {
    free((void *) registers->rbx);
    return 0;
}

static uint64_t syscall_get_total_memory(Registers *registers) {
    return (uint64_t) getTotalMemory();
}

static uint64_t syscall_get_used_memory(Registers *registers) {
    return (uint64_t) getUsedMemory();
}