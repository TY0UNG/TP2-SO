#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <lib.h>
#include <stdint.h>
#include <stdbool.h>

void canvasMode();
void clearCanvas();
void drawPixel(uint64_t x, uint64_t y, uint32_t color);
void drawLine(uint64_t x1, uint64_t y1, uint64_t x2, uint64_t y2, uint16_t thickness, uint32_t color);
void drawRectangle(uint64_t x1, uint64_t y1, uint64_t x2, uint64_t y2, uint16_t thickness, uint32_t color);
void fillScreen(uint32_t color);
void drawFilledRectangle(uint64_t x1, uint64_t y1, uint64_t x2, uint64_t y2, uint32_t color);
void drawCircle(uint64_t x, uint64_t y, uint16_t radius, uint16_t thickness, uint32_t color);
void drawFilledCircle(uint64_t x, uint64_t y, uint16_t radius, uint32_t color);
void drawText(uint64_t x, uint64_t y, const char* text, uint16_t height, uint32_t color);
void swapBuffers();

void textMode();
void clearTextBuffer();
void selectStyle(char style);
void setTextSize(uint16_t height);
void print(const char* text);
void printChar(char c);
void deleteChar();
void scrollDown();
void scrollUp();

void printHex16(uint16_t value);
void printHex32(uint32_t value);
void printHex64(uint64_t value);

#endif