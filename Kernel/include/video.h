#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <lib.h>
#include <stdint.h>
#include <stdbool.h>

// Modo grafico
void canvasMode();
void clearCanvas();
void swapBuffers();
void drawPixel(uint64_t x, uint64_t y, uint32_t color);
void drawLine(uint64_t x1, uint64_t y1, uint64_t x2, uint64_t y2, uint16_t thickness, uint32_t color);
void drawRectangle(uint64_t x1, uint64_t y1, uint64_t x2, uint64_t y2, uint16_t thickness, uint32_t color);
void fillScreen(uint32_t color);
void drawFilledRectangle(uint64_t x1, uint64_t y1, uint64_t x2, uint64_t y2, uint32_t color, bool directWrite);
void drawCircle(uint64_t x, uint64_t y, uint16_t radius, uint16_t thickness, uint32_t color);
void drawFilledCircle(uint64_t x, uint64_t y, uint16_t radius, uint32_t color);
void drawText(uint64_t x, uint64_t y, const char* text, uint16_t height, uint32_t color);

// Helpers que necesita terminal.c para layout / render por celda
void drawStyledChar(uint64_t x, uint64_t y, char c, char style, uint16_t height, bool directWrite);
uint16_t getCharWidth(uint16_t height);
uint16_t getMaxCols(uint16_t height);
uint16_t getMaxRows(uint16_t height);

// Bandera de modo texto vs grafico (la usa terminal.c para decidir si renderiza)
bool isTextMode();
void setTextMode(bool enabled);

void setFpsOverlayEnabled(bool enabled);
uint16_t getScreenWidth();
uint16_t getScreenHeight();

#endif
