#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <stdint.h>
#include <stdbool.h>

void canvas_mode();
void clear_canvas();
void draw_pixel(uint64_t x, uint64_t y, uint32_t color);
void draw_line(uint64_t x1, uint64_t y1, uint64_t x2, uint64_t y2, uint32_t color);
void draw_rectangle(uint64_t x, uint64_t y, uint16_t width, uint16_t height, uint32_t color);
void draw_circle(uint64_t x, uint64_t y, uint16_t radius, uint32_t color);
void draw_text(uint64_t x, uint64_t y, const char* text, uint32_t color);

void text_mode();
void clear_text_buffer();
void select_style(char style);
void print(const char* text);
void printChar(char c);
void deleteChar();

#endif