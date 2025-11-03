#include <stdint.h>
#include <draw.h>

extern int sys_graphics_mode(int enabled);
extern int sys_draw_line(void* params);
extern int sys_draw_rectangle(void* params);
extern int sys_draw_filled_rectangle(void* params);
extern int sys_draw_fill_screen(void* params);
extern int sys_draw_circle(void* params);
extern int sys_draw_filled_circle(void* params);
extern int sys_draw_text(void* params);
extern int sys_clear_canvas();
extern int sys_swap_buffers();

void enableGraphicsMode() {
    sys_graphics_mode(1);
}

void disableGraphicsMode() {
    sys_graphics_mode(0);
}

void drawLine(uint64_t x1, uint64_t y1, uint64_t x2, uint64_t y2, uint16_t thickness, uint32_t color) {
    LineParameters params = {
        .x1 = x1, 
        .y1 = y1, 
        .x2 = x2, 
        .y2 = y2, 
        .thickness = thickness, 
        .color = color
    };
    sys_draw_line(&params);
}

void drawRectangle(uint64_t x1, uint64_t y1, uint64_t x2, uint64_t y2, uint16_t thickness, uint32_t color) {
    RectangleParameters params = {
        .x1 = x1, 
        .y1 = y1, 
        .x2 = x2, 
        .y2 = y2, 
        .thickness = thickness, 
        .color = color
    };
    sys_draw_rectangle(&params);
}

void drawFilledRectangle(uint64_t x1, uint64_t y1, uint64_t x2, uint64_t y2, uint32_t color, bool directWrite) {
    FilledRectangleParameters params = {
        .x1 = x1, 
        .y1 = y1, 
        .x2 = x2, 
        .y2 = y2, 
        .color = color,
        .directWrite = directWrite
    };
    sys_draw_filled_rectangle(&params);
}

void drawFillScreen(uint32_t color) {
    FillScreenParameters params = {
        .color = color
    };
    sys_draw_fill_screen(&params);
}

void drawCircle(uint64_t x_center, uint64_t y_center, uint16_t radius, uint16_t thickness, uint32_t color) {
    CircleParameters params = {
        .x = x_center, 
        .y = y_center, 
        .radius = radius, 
        .thickness = thickness, 
        .color = color
    };
    sys_draw_circle(&params);
}

void drawFilledCircle(uint64_t x_center, uint64_t y_center, uint16_t radius, uint32_t color) {
    FilledCircleParameters params = {
        .x = x_center, 
        .y = y_center, 
        .radius = radius, 
        .color = color
    };
    sys_draw_filled_circle(&params);
}

void drawText(uint64_t x, uint64_t y, const char* text, uint16_t height, uint32_t color) {
    TextParameters params = {
        .x = x, 
        .y = y, 
        .text = text, 
        .height = height, 
        .color = color
    };
    sys_draw_text(&params);
}


void drawTextCentered(const char* text, uint64_t y, uint16_t height, uint32_t color,const int SCREEN_WIDTH) {                  
    if (!text) return;

    uint16_t char_width = 8;
    uint16_t char_height = 16;

    // Scale factor que usa drawText
    float scale_factor = (float)height / (float)char_height;
    if (scale_factor < 1.0f) scale_factor = 1.0f;

    // Ancho total del texto en píxeles
    float text_width = strlen(text) * char_width * scale_factor;

    // Coordenada X para centrar en pantalla
    uint64_t x = (SCREEN_WIDTH - (uint64_t)text_width) / 2;

    drawText(x, y, text, height, color);
}

void clearCanvas() {
    sys_clear_canvas();
}

void swapBuffers() {
    sys_swap_buffers();
}