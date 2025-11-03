#ifndef DRAW_H
#define DRAW_H

#include <stdbool.h>
#include <stdint.h>

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

void enableGraphicsMode();

void disableGraphicsMode();

void drawLine(uint64_t x1, uint64_t y1, uint64_t x2, uint64_t y2, uint16_t thickness, uint32_t color);

void drawRectangle(uint64_t x1, uint64_t y1, uint64_t x2, uint64_t y2, uint16_t thickness, uint32_t color);

void drawFilledRectangle(uint64_t x1, uint64_t y1, uint64_t x2, uint64_t y2, uint32_t color, bool directWrite);

void drawFillScreen(uint32_t color);

void drawCircle(uint64_t x_center, uint64_t y_center, uint16_t radius, uint16_t thickness, uint32_t color);

void drawFilledCircle(uint64_t x_center, uint64_t y_center, uint16_t radius, uint32_t color);

void drawText(uint64_t x, uint64_t y, const char* text, uint16_t height, uint32_t color);

void clearCanvas();

void swapBuffers();

void drawTextCentered(const char* text, uint64_t y, uint16_t height, uint32_t color,const int SCREEN_WIDTH);

#endif