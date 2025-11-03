#ifndef TRON_CONFIG_H
#define TRON_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

#define CELL_SIZE 8
#define HUD_TOP 120
#define HUD_BOTTOM 72
#define PANEL_LEFT_WIDTH 128
#define PANEL_RIGHT_WIDTH 128

#define ARENA_LEFT PANEL_LEFT_WIDTH
#define ARENA_TOP HUD_TOP
#define ARENA_RIGHT (SCREEN_WIDTH - PANEL_RIGHT_WIDTH)
#define ARENA_BOTTOM (SCREEN_HEIGHT - HUD_BOTTOM)
#define ARENA_WIDTH (ARENA_RIGHT - ARENA_LEFT)
#define ARENA_HEIGHT (ARENA_BOTTOM - ARENA_TOP)
#define ARENA_COLS (ARENA_WIDTH / CELL_SIZE)
#define ARENA_ROWS (ARENA_HEIGHT / CELL_SIZE)

#define MAX_LIVES 3
#define BOOST_DURATION_MS 1300
#define BOOST_STEP_MULTIPLIER 2

#define MENU_ANIM_INTERVAL 45
#define STATUS_FLASH_INTERVAL 260
#define BOOST_FLASH_INTERVAL 320

#define COLOR_BACKGROUND 0x04020A
#define COLOR_PANEL_DARK 0x071425
#define COLOR_PANEL_ACCENT 0x0C2745
#define COLOR_PANEL_LIGHT 0x103258
#define COLOR_GRID_LINE 0x11263B
#define COLOR_GRID_GLOW 0x1C3F64
#define COLOR_STATUS 0x1A2E48
#define COLOR_TEXT_PRIMARY 0x7FE5FF
#define COLOR_TEXT_MUTED 0x9DB4CC
#define COLOR_STATUS_ALERT 0xFF5A89
#define COLOR_STATUS_SUCCESS 0x00F0AA

#define COLOR_P1_TRAIL 0x00D8FF
#define COLOR_P1_HEAD 0xFFFFFF
#define COLOR_P1_GLOW 0x0077A8
#define COLOR_P1_UI 0x20E3FF

#define COLOR_P2_TRAIL 0xFF4FC3
#define COLOR_P2_HEAD 0xFFFFFF
#define COLOR_P2_GLOW 0x8C0F5C
#define COLOR_P2_UI 0xFF71D1

#define OCC_EMPTY 0
#define OCC_P1 1
#define OCC_P2 2

#define MAX_TRAIL_UPDATES 32

typedef enum {
    DIR_UP = 0,
    DIR_RIGHT = 1,
    DIR_DOWN = 2,
    DIR_LEFT = 3
} Direction;

static const int TRON_DELTA_COL[4] = {0, 1, 0, -1};
static const int TRON_DELTA_ROW[4] = {-1, 0, 1, 0};

static Direction rotateLeft(Direction dir) {
    return (Direction)((dir + 3) % 4);
}

static Direction rotateRight(Direction dir) {
    return (Direction)((dir + 1) % 4);
}

static Direction oppositeDir(Direction dir) {
    return (Direction)((dir + 2) % 4);
}

typedef struct {
    int col;
    int row;
    Direction dir;
    Direction pendingDir;
    bool alive;
    bool boostUsed;
    uint64_t boostUntil;
    uint32_t trailColor;
    uint32_t headColor;
    uint32_t glowColor;
    uint32_t uiColor;
} Cycle;

// Struct para cachear cuales fueron las actualizaciones en la arena
typedef struct {
    int col;
    int row;
    uint8_t owner;
} TrailUpdate;

// Struct para guardar informacion sobre los choques y poder animarlos
typedef struct {
    bool active;
    int col;
    int row;
} CrashMarker;

// Struct que guarda informacion sobre el resultado de una partida
typedef struct {
    int lives1;
    int lives2;
    bool aborted;
} MatchResult;

typedef uint8_t (*TronCellFn)(int col, int row);

static void writeTwoDigitNumber(char *dest, int value) {
    if (value < 0) {
        value = 0;
    }
    if (value > 99) {
        value = 99;
    }
    dest[0] = (char)('0' + (value / 10));
    dest[1] = (char)('0' + (value % 10));
}

#endif
