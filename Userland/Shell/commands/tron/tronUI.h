#ifndef TRON_UI_H
#define TRON_UI_H
#include <draw.h>
#include "tronConfig.h"

void tronUiResetCache(void);
bool tronUiEnsureStatic(int mode,
                        int lives1,
                        int lives2,
                        const Cycle *p1,
                        const Cycle *p2,
                        uint64_t now);
bool tronUiUpdateStatus(const char *text, uint32_t color, bool flash);
void tronUiRedrawArena(TronCellFn cellAt);
void tronUiDrawTrailCell(int col, int row, uint8_t owner, bool directWrite);
void tronUiDrawCycleHead(const Cycle *cycle, bool directWrite);
void tronUiDrawCrashMarker(const CrashMarker *marker, uint32_t fillColor, bool directWrite);
void tronUiDrawBoostMeter(const Cycle *cycle, bool leftPanel, uint64_t now, bool directWrite);
void tronUiDrawMenuFrame(int selection, int animPhase);
void MenuEfect(int selection);

#endif
