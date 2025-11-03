#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <draw.h>
#include <sound.h>
#include <inout.h>
#include "../../lib/time.h"
#include "tronConfig.h"
#include "tronAI.h"
#include "tronUI.h"

#define INPUT_QUEUE_CAPACITY 32
#define INPUT_FETCH_LIMIT 12
#define INPUT_PROCESS_LIMIT 16

typedef struct {
    KeyEvent items[INPUT_QUEUE_CAPACITY];
    int head;
    int tail;
    int size;
} InputQueue;

static InputQueue pendingEvents;

static void inputQueueClear(void) {
    pendingEvents.head = 0;
    pendingEvents.tail = 0;
    pendingEvents.size = 0;
}

static void inputQueuePush(const KeyEvent *event) {
    pendingEvents.items[pendingEvents.head] = *event;
    pendingEvents.head = (pendingEvents.head + 1) % INPUT_QUEUE_CAPACITY;
    if (pendingEvents.size == INPUT_QUEUE_CAPACITY) {
        pendingEvents.tail = (pendingEvents.tail + 1) % INPUT_QUEUE_CAPACITY;
    } else {
        pendingEvents.size++;
    }
}

static bool inputQueuePop(KeyEvent *event) {
    if (pendingEvents.size == 0) {
        return false;
    }
    *event = pendingEvents.items[pendingEvents.tail];
    pendingEvents.tail = (pendingEvents.tail + 1) % INPUT_QUEUE_CAPACITY;
    pendingEvents.size--;
    return true;
}

static void inputQueueFetch(int limit) {
    for (int count = 0; count < limit; count++) {
        KeyEvent *raw = getKey();
        if (raw == NULL) {
            break;
        }
        inputQueuePush(raw);
    }
}

static bool insideArena(int col, int row);

static uint8_t arenaBits[((ARENA_ROWS * ARENA_COLS) * 2 + 7) / 8];

static size_t arenaCellIndex(int col, int row) {
    return (size_t)row * ARENA_COLS + (size_t)col;
}

static uint8_t arenaGet(int col, int row) {
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

static void arenaSet(int col, int row, uint8_t owner) {
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

static void arenaClear(void) {
    memset(arenaBits, 0, sizeof(arenaBits));
}

static uint8_t arenaCellAt(int col, int row) {
    return arenaGet(col, row);
}

static uint32_t arenaCountOccupied(void) {
    uint32_t total = 0;
    for (int r = 0; r < ARENA_ROWS; r++) {
        for (int c = 0; c < ARENA_COLS; c++) {
            if (arenaGet(c, r) != OCC_EMPTY) {
                total++;
            }
        }
    }
    return total;
}

static bool insideArena(int col, int row) {
    return col >= 0 && col < ARENA_COLS && row >= 0 && row < ARENA_ROWS;
}

typedef struct {
    uint32_t frameCount;
    uint32_t trailCells;
    uint32_t headDraws;
    uint32_t hudUpdates;
    uint32_t statusUpdates;
    uint32_t swaps;
} StatsCounters;

static bool statsEnabled = false;
static StatsCounters statsBucket;
static uint64_t statsWindowStart = 0;
static char statsBuffers[2][64];
static int statsBufferIndex = 0;
static const char *statsMessage = NULL;

static char *statsWriteLiteral(char *dst, const char *text) {
    while (*text != '\0') {
        *dst++ = *text++;
    }
    return dst;
}

static char *statsWriteUnsigned(char *dst, uint32_t value) {
    char tmp[10];
    int len = 0;
    if (value == 0) {
        tmp[len++] = '0';
    } else {
        while (value > 0 && len < (int)sizeof(tmp)) {
            tmp[len++] = (char)('0' + (value % 10u));
            value /= 10u;
        }
    }
    while (len > 0) {
        *dst++ = tmp[--len];
    }
    return dst;
}

static void statsReset(uint64_t now) {
    memset(&statsBucket, 0, sizeof(statsBucket));
    statsWindowStart = now;
    statsBufferIndex = 0;
    char *buf = statsBuffers[0];
    buf = statsWriteLiteral(buf, "stats ready");
    *buf = '\0';
    statsBuffers[1][0] = '\0';
    statsMessage = statsBuffers[0];
}

static void statsToggle(void) {
    statsEnabled = !statsEnabled;
    if (statsEnabled) {
        statsReset(getMilisFromBoot());
    }
}

static void statsRecordFrame(void) {
    if (statsEnabled) {
        statsBucket.frameCount++;
    }
}

static void statsRecordTrail(uint32_t amount) {
    if (statsEnabled) {
        statsBucket.trailCells += amount;
    }
}

static void statsRecordHead(uint32_t amount) {
    if (statsEnabled) {
        statsBucket.headDraws += amount;
    }
}

static void statsRecordHud(void) {
    if (statsEnabled) {
        statsBucket.hudUpdates++;
    }
}

static void statsRecordStatus(void) {
    if (statsEnabled) {
        statsBucket.statusUpdates++;
    }
}

static void statsRecordSwap(void) {
    if (statsEnabled) {
        statsBucket.swaps++;
    }
}

static void statsMaybeReport(uint64_t now) {
    if (!statsEnabled) {
        return;
    }
    if (statsWindowStart == 0) {
        statsWindowStart = now;
        return;
    }
    uint64_t elapsed = now - statsWindowStart;
    if (elapsed < 250) {
        return;
    }

    uint32_t fps = (elapsed > 0) ? (uint32_t)((statsBucket.frameCount * 1000u) / elapsed) : 0u;

    statsBufferIndex ^= 1;
    char *buffer = statsBuffers[statsBufferIndex];
    char *cursor = buffer;
    cursor = statsWriteLiteral(cursor, "fps:");
    cursor = statsWriteUnsigned(cursor, fps);
    cursor = statsWriteLiteral(cursor, " sw:");
    cursor = statsWriteUnsigned(cursor, statsBucket.swaps);
    cursor = statsWriteLiteral(cursor, " tr:");
    cursor = statsWriteUnsigned(cursor, statsBucket.trailCells);
    cursor = statsWriteLiteral(cursor, " hd:");
    cursor = statsWriteUnsigned(cursor, statsBucket.headDraws);
    cursor = statsWriteLiteral(cursor, " hu:");
    cursor = statsWriteUnsigned(cursor, statsBucket.hudUpdates);
    *cursor = '\0';
    statsMessage = buffer;

    statsWindowStart = now;
    memset(&statsBucket, 0, sizeof(statsBucket));
}

static void tronSwapBuffers(void) {
    swapBuffers();
    statsRecordSwap();
}

static void playMenuTheme(void) {
    clear_audio_buffer();
    play_sound(392, 160);
    play_sound(0, 40);
    play_sound(523, 160);
    play_sound(0, 40);
    play_sound(659, 220);
    play_sound(0, 40);
    play_sound(784, 260);
    play_sound(0, 120);
}

static void playRoundTheme(void) {
    play_sound(440, 110);
    play_sound(587, 110);
    play_sound(784, 110);
    play_sound(0, 55);
    play_sound(659, 110);
    play_sound(523, 110);
    play_sound(0, 55);
}

static void playCrashSound(void) {
    clear_audio_buffer();
    play_sound(120, 120);
    play_sound(80, 120);
    play_sound(40, 180);
}

static void configureCycle(Cycle *cycle,
                           int col,
                           int row,
                           Direction dir,
                           uint32_t trailColor,
                           uint32_t headColor,
                           uint32_t glowColor,
                           uint32_t uiColor) {
    cycle->col = col;
    cycle->row = row;
    cycle->dir = dir;
    cycle->pendingDir = dir;
    cycle->alive = true;
    cycle->boostUsed = false;
    cycle->boostUntil = 0;
    cycle->trailColor = trailColor;
    cycle->headColor = headColor;
    cycle->glowColor = glowColor;
    cycle->uiColor = uiColor;
}

static void markInitialPositions(const Cycle *p1, const Cycle *p2) {
    arenaSet(p1->col, p1->row, OCC_P1);
    arenaSet(p2->col, p2->row, OCC_P2);
}

static int stepsForCycle(const Cycle *cycle, uint64_t now) {
    return (cycle->boostUntil > now) ? BOOST_STEP_MULTIPLIER : 1;
}

static void maybeExpireBoost(Cycle *cycle, uint64_t now) {
    if (cycle->boostUntil != 0 && cycle->boostUntil <= now) {
        cycle->boostUntil = 0;
    }
}

static int stepCycles(Cycle *p1,
                      Cycle *p2,
                      uint64_t now,
                      bool *p1Crashed,
                      bool *p2Crashed,
                      TrailUpdate *updates,
                      int maxUpdates,
                      bool *p1Moved,
                      bool *p2Moved,
                      CrashMarker *p1Crash,
                      CrashMarker *p2Crash) {
    if (p1Crash != NULL) {
        p1Crash->active = false;
    }
    if (p2Crash != NULL) {
        p2Crash->active = false;
    }

    if (p1->alive) {
        p1->dir = p1->pendingDir;
    }
    if (p2->alive) {
        p2->dir = p2->pendingDir;
    }

    int steps1 = p1->alive ? stepsForCycle(p1, now) : 0;
    int steps2 = p2->alive ? stepsForCycle(p2, now) : 0;
    int maxSteps = (steps1 > steps2) ? steps1 : steps2;

    int currentCol1 = p1->col;
    int currentRow1 = p1->row;
    int currentCol2 = p2->col;
    int currentRow2 = p2->row;

    int updateCount = 0;
    *p1Moved = false;
    *p2Moved = false;

    for (int step = 0; step < maxSteps; step++) {
        bool move1 = p1->alive && step < steps1;
        bool move2 = p2->alive && step < steps2;

        int nextCol1 = currentCol1;
        int nextRow1 = currentRow1;
        int nextCol2 = currentCol2;
        int nextRow2 = currentRow2;

        if (move1) {
            nextCol1 += TRON_DELTA_COL[p1->dir];
            nextRow1 += TRON_DELTA_ROW[p1->dir];
        }
        if (move2) {
            nextCol2 += TRON_DELTA_COL[p2->dir];
            nextRow2 += TRON_DELTA_ROW[p2->dir];
        }

        bool crash1 = false;
        bool crash2 = false;

        if (move1) {
            if (!insideArena(nextCol1, nextRow1) || arenaGet(nextCol1, nextRow1) != OCC_EMPTY) {
                crash1 = true;
                if (p1Crash != NULL && insideArena(nextCol1, nextRow1)) {
                    p1Crash->active = true;
                    p1Crash->col = nextCol1;
                    p1Crash->row = nextRow1;
                }
            }
        }
        if (move2) {
            if (!insideArena(nextCol2, nextRow2) || arenaGet(nextCol2, nextRow2) != OCC_EMPTY) {
                crash2 = true;
                if (p2Crash != NULL && insideArena(nextCol2, nextRow2)) {
                    p2Crash->active = true;
                    p2Crash->col = nextCol2;
                    p2Crash->row = nextRow2;
                }
            }
        }

        if (move1 && move2 && !crash1 && !crash2) {
            if (nextCol1 == nextCol2 && nextRow1 == nextRow2) {
                crash1 = true;
                crash2 = true;
                if (p1Crash != NULL) {
                    p1Crash->active = true;
                    p1Crash->col = nextCol1;
                    p1Crash->row = nextRow1;
                }
                if (p2Crash != NULL) {
                    p2Crash->active = true;
                    p2Crash->col = nextCol2;
                    p2Crash->row = nextRow2;
                }
            } else if (nextCol1 == currentCol2 && nextRow1 == currentRow2 &&
                       nextCol2 == currentCol1 && nextRow2 == currentRow1) {
                crash1 = true;
                crash2 = true;
                if (p1Crash != NULL) {
                    p1Crash->active = true;
                    p1Crash->col = currentCol2;
                    p1Crash->row = currentRow2;
                }
                if (p2Crash != NULL) {
                    p2Crash->active = true;
                    p2Crash->col = currentCol1;
                    p2Crash->row = currentRow1;
                }
            }
        }

        if (crash1) {
            p1->alive = false;
            *p1Crashed = true;
        }
        if (crash2) {
            p2->alive = false;
            *p2Crashed = true;
        }

        if (crash1 || crash2) {
            break;
        }

        if (move1) {
            currentCol1 = nextCol1;
            currentRow1 = nextRow1;
            p1->col = currentCol1;
            p1->row = currentRow1;
            arenaSet(currentCol1, currentRow1, OCC_P1);
            if (updateCount < maxUpdates) {
                updates[updateCount++] = (TrailUpdate){currentCol1, currentRow1, OCC_P1};
            }
            *p1Moved = true;
        }
        if (move2) {
            currentCol2 = nextCol2;
            currentRow2 = nextRow2;
            p2->col = currentCol2;
            p2->row = currentRow2;
            arenaSet(currentCol2, currentRow2, OCC_P2);
            if (updateCount < maxUpdates) {
                updates[updateCount++] = (TrailUpdate){currentCol2, currentRow2, OCC_P2};
            }
            *p2Moved = true;
        }
    }

    return updateCount;
}

static uint64_t computeTickInterval(void) {
    return 55;
}

static void handleCycleInput(Cycle *p1, Cycle *p2, int mode, bool *exitRequested) {
    inputQueueFetch(INPUT_FETCH_LIMIT);

    KeyEvent event;
    int processed = 0;
    while (processed < INPUT_PROCESS_LIMIT && inputQueuePop(&event)) {
        processed++;
        if (event.is_release) {
            continue;
        }

        int scancode = event.scancode;

        switch (scancode) {
        case 1:
            *exitRequested = true;
            return;
        case 72:
            if (p1->dir != DIR_DOWN) {
                p1->pendingDir = DIR_UP;
            }
            break;
        case 80:
            if (p1->dir != DIR_UP) {
                p1->pendingDir = DIR_DOWN;
            }
            break;
        case 75:
            if (p1->dir != DIR_RIGHT) {
                p1->pendingDir = DIR_LEFT;
            }
            break;
        case 77:
            if (p1->dir != DIR_LEFT) {
                p1->pendingDir = DIR_RIGHT;
            }
            break;
        default:
            break;
        }

        bool printable = event.printable;
        char ascii = (char)event.ascii;
        if (printable && ascii >= 'A' && ascii <= 'Z') {
            ascii = (char)(ascii - 'A' + 'a');
        }

        if (printable && ascii == 'f') {
            statsToggle();
            continue;
        }

        bool activateBoostP1 = printable && ascii == 'p';
        bool activateBoostP2 = printable && ascii == 'q';

        if ((activateBoostP1 || scancode == 25) && !p1->boostUsed) {
            p1->boostUsed = true;
            p1->boostUntil = getMilisFromBoot() + BOOST_DURATION_MS;
        }

        if (mode == 2) {
            bool upKey = (ascii == 'w') || (scancode == 17);
            bool downKey = (ascii == 's') || (scancode == 31);
            bool leftKey = (ascii == 'a') || (scancode == 30);
            bool rightKey = (ascii == 'd') || (scancode == 32);

            if (upKey && p2->dir != DIR_DOWN) {
                p2->pendingDir = DIR_UP;
            } else if (downKey && p2->dir != DIR_UP) {
                p2->pendingDir = DIR_DOWN;
            } else if (leftKey && p2->dir != DIR_RIGHT) {
                p2->pendingDir = DIR_LEFT;
            } else if (rightKey && p2->dir != DIR_LEFT) {
                p2->pendingDir = DIR_RIGHT;
            }

            if ((activateBoostP2 || scancode == 16) && !p2->boostUsed) {
                p2->boostUsed = true;
                p2->boostUntil = getMilisFromBoot() + BOOST_DURATION_MS;
            }
        }
    }
}

static void handleMenuInput(int *selection, bool *confirm, bool *exitRequest) {
    inputQueueFetch(INPUT_FETCH_LIMIT);

    KeyEvent event;
    while (inputQueuePop(&event)) {
        if (event.is_release) {
            continue;
        }
        if (event.scancode == 1) {
            *exitRequest = true;
            return;
        }
        if (event.ascii == '1') {
            *selection = 0;
            *confirm = true;
        } else if (event.ascii == '2') {
            *selection = 1;
            *confirm = true;
        } else if (event.scancode == 75 || event.scancode == 72) {
            *selection = 0;
        } else if (event.scancode == 77 || event.scancode == 80) {
            *selection = 1;
        } else if (event.scancode == 28) {
            *confirm = true;
        }
    }
}

static int showMainMenu(void) {
    inputQueueClear();

    uint64_t lastAnim = getMilisFromBoot();
    int animPhase = 0;
    int selection = 0;
    bool confirm = false;
    bool exitRequest = false;

    playMenuTheme();

    while (!confirm && !exitRequest) {
        uint64_t now = getMilisFromBoot();
        if (now - lastAnim >= MENU_ANIM_INTERVAL) {
            animPhase = (animPhase + (int)(now - lastAnim)) % SCREEN_HEIGHT;
            lastAnim = now;
        }
        tronUiDrawMenuFrame(selection, animPhase);

        if (is_audio_buffer_empty()) {
            playMenuTheme();
        }

        handleMenuInput(&selection, &confirm, &exitRequest);
        sleep(10);
    }

    if (exitRequest) {
        return 0;
    }
    return selection == 0 ? 1 : 2;
}

static int waitForSpaceOrEsc(const Cycle *p1,
                             const Cycle *p2,
                             int mode,
                             int lives1,
                             int lives2,
                             const char *message,
                             uint32_t color) {
    inputQueueClear();
    tronUiResetCache();
    bool frameReady = false;

    uint64_t lastFlash = getMilisFromBoot();
    bool flash = false;

    while (true) {
        uint64_t now = getMilisFromBoot();
        if (now - lastFlash >= STATUS_FLASH_INTERVAL) {
            flash = !flash;
            lastFlash = now;
        }

        if (!frameReady) {
            bool hudChanged = tronUiEnsureStatic(mode, lives1, lives2, p1, p2, false, now);
            if (hudChanged) {
                statsRecordHud();
            }
            tronUiRedrawArena(arenaCellAt);
            if (statsEnabled) {
                statsRecordTrail(arenaCountOccupied());
            }
            if (p1->alive) {
                tronUiDrawCycleHead(p1);
                statsRecordHead(1);
            }
            if (p2->alive) {
                tronUiDrawCycleHead(p2);
                statsRecordHead(1);
            }
            frameReady = true;
            tronSwapBuffers();
        }

        bool statusChanged = tronUiUpdateStatus(message, color, flash);
        if (statusChanged) {
            statsRecordStatus();
            tronSwapBuffers();
        }

        inputQueueFetch(INPUT_FETCH_LIMIT);

        KeyEvent event;
        while (inputQueuePop(&event)) {
            if (event.is_release) {
                continue;
            }
            if (event.scancode == 1) {
                return -1;
            }
            if (event.ascii == ' ' || event.scancode == 57) {
                return 1;
            }
        }
        sleep(14);
    }
}

typedef struct {
    int lives1;
    int lives2;
    bool aborted;
} MatchResult;

static int playRound(int mode, int *lives1, int *lives2) {
    inputQueueClear();
    arenaClear();

    Cycle p1;
    Cycle p2;

    configureCycle(&p1,
                   ARENA_COLS / 4,
                   ARENA_ROWS / 2,
                   DIR_RIGHT,
                   COLOR_P1_TRAIL,
                   COLOR_P1_HEAD,
                   COLOR_P1_GLOW,
                   COLOR_P1_UI);
    configureCycle(&p2,
                   (ARENA_COLS * 3) / 4,
                   ARENA_ROWS / 2,
                   DIR_LEFT,
                   COLOR_P2_TRAIL,
                   COLOR_P2_HEAD,
                   COLOR_P2_GLOW,
                   COLOR_P2_UI);

    markInitialPositions(&p1, &p2);

    tronUiResetCache();
    uint64_t frameTime = getMilisFromBoot();
    bool initHud = tronUiEnsureStatic(mode, *lives1, *lives2, &p1, &p2, false, frameTime);
    if (initHud) {
        statsRecordHud();
    }
    tronUiRedrawArena(arenaCellAt);
    if (statsEnabled) {
        statsRecordTrail(arenaCountOccupied());
    }
    tronUiDrawCycleHead(&p1);
    tronUiDrawCycleHead(&p2);
    statsRecordHead(2);
    tronUiUpdateStatus("... game in progress ...", COLOR_TEXT_MUTED, false);
    statsRecordStatus();
    tronSwapBuffers();

    uint64_t tickInterval = computeTickInterval();
    uint64_t previousTime = frameTime;
    uint64_t roundStart = frameTime;
    uint64_t aiAccumulator = 0;
    const uint64_t aiDecisionInterval = (tickInterval > 45u) ? tickInterval : 45u;

    uint64_t lastStatusToggle = previousTime;
    uint64_t lastBoostToggle = previousTime;

    bool statusFlash = false;
    bool boostFlash = false;

    clear_audio_buffer();
    playRoundTheme();

    bool exitRequested = false;
    bool p1Crashed = false;
    bool p2Crashed = false;

    while (!p1Crashed && !p2Crashed && !exitRequested) {
        uint64_t now = getMilisFromBoot();
        uint64_t delta = now - previousTime;
        previousTime = now;

        statsRecordFrame();
        statsMaybeReport(now);

        if (now - lastBoostToggle >= BOOST_FLASH_INTERVAL) {
            boostFlash = !boostFlash;
            lastBoostToggle = now;
        }

        if (now - lastStatusToggle >= STATUS_FLASH_INTERVAL) {
            statusFlash = !statusFlash;
            lastStatusToggle = now;
        }

        bool uiChanged = tronUiEnsureStatic(mode, *lives1, *lives2, &p1, &p2, boostFlash, now);
        if (uiChanged) {
            statsRecordHud();
        }

        handleCycleInput(&p1, &p2, mode, &exitRequested);
        if (exitRequested) {
            break;
        }

        if (mode == 1) {
            aiAccumulator += delta;
            while (aiAccumulator >= aiDecisionInterval) {
                Direction chosen = tronAiChooseDirection(&p2, &p1, arenaCellAt);
                p2.pendingDir = chosen;
                aiAccumulator -= aiDecisionInterval;
            }
        } else {
            aiAccumulator = 0;
        }

        maybeExpireBoost(&p1, now);
        maybeExpireBoost(&p2, now);

        if (mode == 1) {
            tronAiManageBoost(&p2, &p1, now, roundStart, arenaCellAt);
        }

        bool frameUpdated = false;

        bool p1Moved = false;
        bool p2Moved = false;

        int prevCol1 = p1.col;
        int prevRow1 = p1.row;
        int prevCol2 = p2.col;
        int prevRow2 = p2.row;

        TrailUpdate updates[MAX_TRAIL_UPDATES];
        CrashMarker crash1 = {0};
        CrashMarker crash2 = {0};
        int updateCount = 0;

        if (!p1Crashed && !p2Crashed) {
            updateCount = stepCycles(&p1,
                                     &p2,
                                     now,
                                     &p1Crashed,
                                     &p2Crashed,
                                     updates,
                                     MAX_TRAIL_UPDATES,
                                     &p1Moved,
                                     &p2Moved,
                                     &crash1,
                                     &crash2);
        }

        if ((p1Moved || p1Crashed) && arenaGet(prevCol1, prevRow1) == OCC_P1) {
            tronUiDrawTrailCell(prevCol1, prevRow1, OCC_P1);
            statsRecordTrail(1);
            frameUpdated = true;
        }
        if ((p2Moved || p2Crashed) && arenaGet(prevCol2, prevRow2) == OCC_P2) {
            tronUiDrawTrailCell(prevCol2, prevRow2, OCC_P2);
            statsRecordTrail(1);
            frameUpdated = true;
        }

        for (int i = 0; i < updateCount; i++) {
            tronUiDrawTrailCell(updates[i].col, updates[i].row, updates[i].owner);
        }
        if (updateCount > 0) {
            statsRecordTrail((uint32_t)updateCount);
            frameUpdated = true;
        }

        if (p1Moved && p1.alive) {
            tronUiDrawCycleHead(&p1);
            statsRecordHead(1);
            frameUpdated = true;
        }
        if (p2Moved && p2.alive) {
            tronUiDrawCycleHead(&p2);
            statsRecordHead(1);
            frameUpdated = true;
        }

        if (p1Crashed && crash1.active) {
            tronUiDrawCrashMarker(&crash1, COLOR_P1_TRAIL);
            frameUpdated = true;
        }
        if (p2Crashed && crash2.active) {
            tronUiDrawCrashMarker(&crash2, COLOR_P2_TRAIL);
            frameUpdated = true;
        }

        if (p1Crashed || p2Crashed) {
            frameUpdated = true;
        }

    const char *statusText = (statsEnabled && statsMessage != NULL)
                      ? statsMessage
                      : "... game in progress ...";
        bool statusChanged = tronUiUpdateStatus(statusText, COLOR_TEXT_MUTED, statusFlash || frameUpdated);
        if (statusChanged) {
            statsRecordStatus();
        }
        if (uiChanged || frameUpdated || statusChanged) {
            tronSwapBuffers();
        }

        if (is_audio_buffer_empty()) {
            playRoundTheme();
        }

        sleep(6);
    }

    if (exitRequested) {
        return -1;
    }

    if (p1Crashed || p2Crashed) {
        playCrashSound();
    }
    if (p1Crashed && *lives1 > 0) {
        (*lives1)--;
    }
    if (p2Crashed && *lives2 > 0) {
        (*lives2)--;
    }

    const char *message = "Press Space to continue";
    uint32_t waitColor = COLOR_STATUS_SUCCESS;
    if (*lives1 == 0 || *lives2 == 0) {
        message = "Press Space for results";
        if (*lives1 == 0 && *lives2 == 0) {
            waitColor = COLOR_STATUS_ALERT;
        } else if (*lives1 == 0) {
            waitColor = COLOR_P2_UI;
        } else {
            waitColor = COLOR_P1_UI;
        }
    } else if (p1Crashed && p2Crashed) {
        waitColor = COLOR_STATUS_ALERT;
    } else if (p1Crashed) {
        waitColor = COLOR_P2_UI;
    } else if (p2Crashed) {
        waitColor = COLOR_P1_UI;
    }

    return waitForSpaceOrEsc(&p1, &p2, mode, *lives1, *lives2, message, waitColor);
}

static int showMatchResult(int mode, int lives1, int lives2) {
    inputQueueClear();

    const char *headline;
    uint32_t color;
    if (lives1 > lives2) {
        headline = (mode == 1) ? "PLAYER ONE DEFEATED THE CPU" : "PLAYER ONE WINS";
        color = COLOR_P1_UI;
    } else if (lives2 > lives1) {
        headline = (mode == 1) ? "CPU OVERRULES" : "PLAYER TWO WINS";
        color = COLOR_P2_UI;
    } else {
        headline = "NEURAL GRID TIE";
        color = COLOR_TEXT_PRIMARY;
    }

    const char *prompt = "Press Space to return";

    uint64_t lastFlash = getMilisFromBoot();
    bool flash = false;

    char livesLine[32];
    const char *opponentLabel = (mode == 1) ? "CPU" : "P2";
    int idx = 0;
    const char prefix[] = "Lives P1 ";
    for (int i = 0; prefix[i] != '\0'; i++) {
        livesLine[idx++] = prefix[i];
    }
    livesLine[idx++] = (char)('0' + (lives1 > 9 ? 9 : lives1));
    livesLine[idx++] = ' ';
    livesLine[idx++] = '/';
    livesLine[idx++] = ' ';
    for (int i = 0; opponentLabel[i] != '\0'; i++) {
        livesLine[idx++] = opponentLabel[i];
    }
    livesLine[idx++] = ' ';
    livesLine[idx++] = (char)('0' + (lives2 > 9 ? 9 : lives2));
    livesLine[idx] = '\0';

    while (true) {
        uint64_t now = getMilisFromBoot();
        if (now - lastFlash >= STATUS_FLASH_INTERVAL) {
            flash = !flash;
            lastFlash = now;
        }

        drawFillScreen(COLOR_BACKGROUND);
        drawTextCentered(headline, 260, 40, flash ? color : COLOR_TEXT_PRIMARY, SCREEN_WIDTH);

        drawTextCentered(livesLine, 366, 24, COLOR_TEXT_MUTED, SCREEN_WIDTH);

    drawTextCentered(prompt, 432, 24, flash ? color : COLOR_TEXT_MUTED, SCREEN_WIDTH);
    tronSwapBuffers();

        inputQueueFetch(INPUT_FETCH_LIMIT);

        KeyEvent event;
        while (inputQueuePop(&event)) {
            if (event.is_release) {
                continue;
            }
            if (event.scancode == 1) {
                return -1;
            }
            if (event.ascii == ' ' || event.scancode == 57) {
                return 1;
            }
        }
        sleep(14);
    }
}

static MatchResult runMatch(int mode) {
    MatchResult result;
    result.lives1 = MAX_LIVES;
    result.lives2 = MAX_LIVES;
    result.aborted = false;

    while (result.lives1 > 0 && result.lives2 > 0) {
        int roundResult = playRound(mode, &result.lives1, &result.lives2);
        if (roundResult == -1) {
            result.aborted = true;
            return result;
        }
    }

    int summary = showMatchResult(mode, result.lives1, result.lives2);
    if (summary == -1) {
        result.aborted = true;
    }
    return result;
}

int tronGame(char **argv, int argc) {
    (void)argv;
    (void)argc;

    inputQueueClear();
    enableGraphicsMode();

    int running = 1;
    while (running) {
        int mode = showMainMenu();
        if (mode == 0) {
            break;
        }
        if (mode == 1) {
            MatchResult result = runMatch(mode);
            if (result.aborted) {
                running = 0;
            }
        } else {
            MatchResult versusResult = runMatch(mode);
            if (versusResult.aborted) {
                running = 0;
            }
        }
    }

    clear_audio_buffer();
    disableGraphicsMode();
    return 0;
}