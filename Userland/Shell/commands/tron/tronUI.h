#ifndef TRON_UI_H
#define TRON_UI_H

#include <stdint.h>
#include <stdbool.h>
#include <draw.h>
#include "tronConfig.h"

void tronUiResetCache(void);
void tronUiEnsureStatic(int mode,
                        int lives1,
                        int lives2,
                        const Cycle *p1,
                        const Cycle *p2,
                        bool boostFlash);
void tronUiUpdateStatus(const char *text, uint32_t color, bool flash);
void tronUiRedrawArena(const uint8_t occupancy[ARENA_ROWS][ARENA_COLS]);
void tronUiDrawTrailCell(int col, int row, uint8_t owner);
void tronUiDrawCycleHead(const Cycle *cycle);
void tronUiDrawCrashMarker(const CrashMarker *marker, uint32_t fillColor);
void tronUiDrawMenuFrame(int selection, int animPhase);

#endif
