#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <draw.h>
#include <sound.h>
#include <inout.h>
#include "../../lib/time.h"
#include "tronConfig.h"
#include "tronAI.h"
#include "tronUI.h"

static uint8_t occupancy[ARENA_ROWS][ARENA_COLS];

static bool insideArena(int col, int row) {
    return col >= 0 && col < ARENA_COLS && row >= 0 && row < ARENA_ROWS;
}

static void clearArena(void) {
    for (int r = 0; r < ARENA_ROWS; r++) {
        for (int c = 0; c < ARENA_COLS; c++) {
            occupancy[r][c] = OCC_EMPTY;
        }
    }
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
    occupancy[p1->row][p1->col] = OCC_P1;
    occupancy[p2->row][p2->col] = OCC_P2;
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
            if (!insideArena(nextCol1, nextRow1) || occupancy[nextRow1][nextCol1] != OCC_EMPTY) {
                crash1 = true;
                if (p1Crash != NULL && insideArena(nextCol1, nextRow1)) {
                    p1Crash->active = true;
                    p1Crash->col = nextCol1;
                    p1Crash->row = nextRow1;
                }
            }
        }
        if (move2) {
            if (!insideArena(nextCol2, nextRow2) || occupancy[nextRow2][nextCol2] != OCC_EMPTY) {
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
            occupancy[currentRow1][currentCol1] = OCC_P1;
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
            occupancy[currentRow2][currentCol2] = OCC_P2;
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
    KeyEvent *event;
    while ((event = getKey()) != NULL) {
        if (event->is_release) {
            continue;
        }

        int scancode = event->scancode;

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

        bool printable = event->printable;
        char ascii = (char)event->ascii;
        if (printable && ascii >= 'A' && ascii <= 'Z') {
            ascii = (char)(ascii - 'A' + 'a');
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
    KeyEvent *event;
    while ((event = getKey()) != NULL) {
        if (event->is_release) {
            continue;
        }
        if (event->scancode == 1) {
            *exitRequest = true;
            return;
        }
        if (event->ascii == '1') {
            *selection = 0;
            *confirm = true;
        } else if (event->ascii == '2') {
            *selection = 1;
            *confirm = true;
        } else if (event->scancode == 75 || event->scancode == 72) {
            *selection = 0;
        } else if (event->scancode == 77 || event->scancode == 80) {
            *selection = 1;
        } else if (event->scancode == 28) {
            *confirm = true;
        }
    }
}

static int showMainMenu(void) {
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
            tronUiEnsureStatic(mode, lives1, lives2, p1, p2, false);
            tronUiRedrawArena(occupancy);
            if (p1->alive) {
                tronUiDrawCycleHead(p1);
            }
            if (p2->alive) {
                tronUiDrawCycleHead(p2);
            }
            frameReady = true;
        }

        tronUiUpdateStatus(message, color, flash);
        swapBuffers();

        KeyEvent *event;
        while ((event = getKey()) != NULL) {
            if (event->is_release) {
                continue;
            }
            if (event->scancode == 1) {
                return -1;
            }
            if (event->ascii == ' ' || event->scancode == 57) {
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
    clearArena();

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
    tronUiEnsureStatic(mode, *lives1, *lives2, &p1, &p2, false);
    tronUiRedrawArena(occupancy);
    tronUiDrawCycleHead(&p1);
    tronUiDrawCycleHead(&p2);
    tronUiUpdateStatus("... game in progress ...", COLOR_TEXT_MUTED, false);
    swapBuffers();

    uint64_t tickInterval = computeTickInterval();
    uint64_t previousTime = getMilisFromBoot();
    uint64_t roundStart = previousTime;
    uint64_t accumulator = 0;

    uint64_t lastStatusToggle = previousTime;
    uint64_t lastBoostToggle = previousTime;

    bool statusFlash = false;
    bool boostFlash = false;

    uint64_t nextAiDecision = previousTime;

    clear_audio_buffer();
    playRoundTheme();

    bool exitRequested = false;
    bool p1Crashed = false;
    bool p2Crashed = false;

    while (!p1Crashed && !p2Crashed && !exitRequested) {
        uint64_t now = getMilisFromBoot();
        uint64_t delta = now - previousTime;
        previousTime = now;
        accumulator += delta;

        if (now - lastBoostToggle >= BOOST_FLASH_INTERVAL) {
            boostFlash = !boostFlash;
            lastBoostToggle = now;
        }

        if (now - lastStatusToggle >= STATUS_FLASH_INTERVAL) {
            statusFlash = !statusFlash;
            lastStatusToggle = now;
        }

        tronUiEnsureStatic(mode, *lives1, *lives2, &p1, &p2, boostFlash);

        handleCycleInput(&p1, &p2, mode, &exitRequested);
        if (exitRequested) {
            break;
        }

        if (mode == 1 && now >= nextAiDecision) {
            Direction chosen = tronAiChooseDirection(&p2, &p1, occupancy);
            p2.pendingDir = chosen;
            nextAiDecision = now + (tickInterval > 45 ? tickInterval : 45);
        }

        maybeExpireBoost(&p1, now);
        maybeExpireBoost(&p2, now);

        if (mode == 1) {
            tronAiManageBoost(&p2, &p1, now, roundStart, occupancy);
        }

        bool frameUpdated = false;

        while (accumulator >= tickInterval && !p1Crashed && !p2Crashed) {
            accumulator -= tickInterval;

            bool p1Moved = false;
            bool p2Moved = false;

            int prevCol1 = p1.col;
            int prevRow1 = p1.row;
            int prevCol2 = p2.col;
            int prevRow2 = p2.row;

            TrailUpdate updates[MAX_TRAIL_UPDATES];
            CrashMarker crash1;
            CrashMarker crash2;
            int updateCount = stepCycles(&p1,
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

            if ((p1Moved || p1Crashed) && occupancy[prevRow1][prevCol1] == OCC_P1) {
                tronUiDrawTrailCell(prevCol1, prevRow1, OCC_P1);
            }
            if ((p2Moved || p2Crashed) && occupancy[prevRow2][prevCol2] == OCC_P2) {
                tronUiDrawTrailCell(prevCol2, prevRow2, OCC_P2);
            }

            for (int i = 0; i < updateCount; i++) {
                tronUiDrawTrailCell(updates[i].col, updates[i].row, updates[i].owner);
            }

            if (p1.alive) {
                tronUiDrawCycleHead(&p1);
            }
            if (p2.alive) {
                tronUiDrawCycleHead(&p2);
            }

            if (p1Crashed && crash1.active) {
                tronUiDrawCrashMarker(&crash1, COLOR_P1_TRAIL);
            }
            if (p2Crashed && crash2.active) {
                tronUiDrawCrashMarker(&crash2, COLOR_P2_TRAIL);
            }

            frameUpdated = true;
        }

        tronUiUpdateStatus("... game in progress ...", COLOR_TEXT_MUTED, statusFlash || frameUpdated);
        swapBuffers();

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
        swapBuffers();

        KeyEvent *event;
        while ((event = getKey()) != NULL) {
            if (event->is_release) {
                continue;
            }
            if (event->scancode == 1) {
                return -1;
            }
            if (event->ascii == ' ' || event->scancode == 57) {
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

