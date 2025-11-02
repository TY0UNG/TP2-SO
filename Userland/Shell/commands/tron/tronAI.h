#ifndef TRON_AI_H
#define TRON_AI_H

#include <stdint.h>
#include <stdbool.h>
#include "tronConfig.h"

Direction tronAiChooseDirection(const Cycle *ai,
                                const Cycle *target,
                                const uint8_t occupancy[ARENA_ROWS][ARENA_COLS]);

void tronAiManageBoost(Cycle *ai,
                       const Cycle *target,
                       uint64_t now,
                       uint64_t roundStart,
                       const uint8_t occupancy[ARENA_ROWS][ARENA_COLS]);

#endif
