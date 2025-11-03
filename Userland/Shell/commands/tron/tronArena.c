#include "tronArena.h"

// Matriz que determina que jugador posee cada espacio.
static uint8_t arenaBits[((ARENA_ROWS * ARENA_COLS) * 2 + 7) / 8];

static size_t arenaCellIndex(int col, int row) {
    return (size_t)row * ARENA_COLS + (size_t)col;
}

// Obtiene de quien es un dado espacio
uint8_t arenaGet(int col, int row) {
    if (!insideArena(col, row)) {
        return OCC_EMPTY;
    }
    size_t index = arenaCellIndex(col, row);
    size_t bitOffset = index * 2;
    size_t byteIndex = bitOffset >> 3;
    uint8_t shift = (uint8_t)(bitOffset & 7u);
    uint8_t mask = (uint8_t)(0x3u << shift);
    return (uint8_t)((arenaBits[byteIndex] & mask) >> shift);
}

// Setea el poseedor de un espacio
void arenaSet(int col, int row, uint8_t owner) {
    if (!insideArena(col, row)) {
        return;
    }
    size_t index = arenaCellIndex(col, row);
    size_t bitOffset = index * 2;
    size_t byteIndex = bitOffset >> 3;
    uint8_t shift = (uint8_t)(bitOffset & 7u);
    uint8_t mask = (uint8_t)(0x3u << shift);
    arenaBits[byteIndex] = (uint8_t)((arenaBits[byteIndex] & ~mask) | ((owner & 0x3u) << shift));
}

// Clerea las posesiones
void arenaClear(void) {
    memset(arenaBits, 0, sizeof(arenaBits));
}

bool insideArena(int col, int row) {
    return col >= 0 && col < ARENA_COLS && row >= 0 && row < ARENA_ROWS;
}