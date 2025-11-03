#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <draw.h>
#include <inout.h>
#include "../../lib/time.h"
#include "tronConfig.h"
#include "tronAI.h"
#include "tronUI.h"
#include "tronInputQueue.h"
#include "tronArena.h"

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

    // Dibujo el MainMenu y actualizo sus animaciones

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
                             const CrashMarker *lastCrash1,
                             const CrashMarker *lastCrash2,
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
            tronUiEnsureStatic(mode, lives1, lives2, p1, p2, now);
            tronUiRedrawArena(arenaGet);
            tronUiDrawBoostMeter(p1, true, now, false);
            tronUiDrawBoostMeter(p2, false, now, false);
            if (p1->alive) {
                tronUiDrawCycleHead(p1, false);
            }
            if (p2->alive) {
                tronUiDrawCycleHead(p2, false);
            }
            if (lastCrash1 != NULL && lastCrash1->active) {
                tronUiDrawCrashMarker(lastCrash1, COLOR_P1_TRAIL, false);
            }
            if (lastCrash2 != NULL && lastCrash2->active) {
                tronUiDrawCrashMarker(lastCrash2, COLOR_P2_TRAIL, false);
            }
            tronUiUpdateStatus(message, color, flash);
            frameReady = true;
            swapBuffers();
            continue;
        }

        bool statusChanged = tronUiUpdateStatus(message, color, flash);
        if (statusChanged) {
            swapBuffers();
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

static int playRound(int mode, int *lives1, int *lives2) {
    inputQueueClear();
    arenaClear();

    // Configuro cada jugador e inicializo sus variables
    Cycle p1;
    Cycle p2;

    configureCycle(&p1,
                   ARENA_COLS / 4, // Empieza en un cuarto del ancho
                   ARENA_ROWS / 2, // A la mitad de la altura
                   DIR_RIGHT,
                   COLOR_P1_TRAIL,
                   COLOR_P1_HEAD,
                   COLOR_P1_GLOW,
                   COLOR_P1_UI);
    configureCycle(&p2,
                   (ARENA_COLS * 3) / 4, // En tres cuartos del ancho
                   ARENA_ROWS / 2, // A la mitad de la altura
                   DIR_LEFT,
                   COLOR_P2_TRAIL,
                   COLOR_P2_HEAD,
                   COLOR_P2_GLOW,
                   COLOR_P2_UI);

    markInitialPositions(&p1, &p2);

    // Dibujo la UI inicial.

    tronUiResetCache();
    uint64_t frameTime = getMilisFromBoot();
    tronUiEnsureStatic(mode, *lives1, *lives2, &p1, &p2, frameTime);
    tronUiRedrawArena(arenaGet);
    tronUiDrawBoostMeter(&p1, true, frameTime, false);
    tronUiDrawBoostMeter(&p2, false, frameTime, false);
    tronUiDrawCycleHead(&p1, false);
    tronUiDrawCycleHead(&p2, false);
    tronUiUpdateStatus("... game in progress ...", COLOR_TEXT_MUTED, false);
    swapBuffers();

    uint64_t tickInterval = computeTickInterval();
    uint64_t previousTime = frameTime;
    uint64_t roundStart = frameTime;
    uint64_t aiAccumulator = 0;
    const uint64_t aiDecisionInterval = (tickInterval > 45u) ? tickInterval : 45u;

    uint64_t nextAiDecision = previousTime;

    clear_audio_buffer();
    playRoundTheme();

    bool exitRequested = false;
    bool p1Crashed = false;
    bool p2Crashed = false;
    CrashMarker lastCrash1 = {0};
    CrashMarker lastCrash2 = {0};

    // En cada turno:
    while (!p1Crashed && !p2Crashed && !exitRequested) {
        uint64_t now = getMilisFromBoot();
        uint64_t delta = now - previousTime;
        previousTime = now;

        // Cambio direcciones y activo boosts si es necesario
        handleCycleInput(&p1, &p2, mode, &exitRequested);
        if (exitRequested) {
            break;
        }

        // Escucho decision de la IA de elegir direccion
        if (mode == 1 && now >= nextAiDecision) {
            Direction chosen = tronAiChooseDirection(&p2, &p1, arenaGet);
            p2.pendingDir = chosen;
            nextAiDecision = now + (tickInterval > 45 ? tickInterval : 45);
        }

        // Deshabilito boosts si ya vencieron
        maybeExpireBoost(&p1, now);
        maybeExpireBoost(&p2, now);

        // Escurcho la decision de la IA de activar boost
        if (mode == 1) {
            tronAiManageBoost(&p2, &p1, now, roundStart, arenaGet);
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
            tronUiDrawTrailCell(prevCol1, prevRow1, OCC_P1, true);
            frameUpdated = true;
        }
        if ((p2Moved || p2Crashed) && arenaGet(prevCol2, prevRow2) == OCC_P2) {
            tronUiDrawTrailCell(prevCol2, prevRow2, OCC_P2, true);
            frameUpdated = true;
        }

        for (int i = 0; i < updateCount; i++) {
            tronUiDrawTrailCell(updates[i].col, updates[i].row, updates[i].owner, true);
        }
        if (updateCount > 0) {
            frameUpdated = true;
        }

        if (p1Moved && p1.alive) {
            tronUiDrawCycleHead(&p1, true);
            frameUpdated = true;
        }
        if (p2Moved && p2.alive) {
            tronUiDrawCycleHead(&p2, true);
            frameUpdated = true;
        }

        if (p1Crashed && crash1.active) {
            tronUiDrawCrashMarker(&crash1, COLOR_P1_TRAIL, true);
            frameUpdated = true;
        }
        if (p2Crashed && crash2.active) {
            tronUiDrawCrashMarker(&crash2, COLOR_P2_TRAIL, true);
            frameUpdated = true;
        }

        if (p1Crashed || p2Crashed) {
            frameUpdated = true;
        }

        tronUiDrawBoostMeter(&p1, true, now, true);
        tronUiDrawBoostMeter(&p2, false, now, true);

        sleep(6);
    }

    // Actualizo vidas

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

    // Pauso el juego hasta que se reanuda

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

    return waitForSpaceOrEsc(&p1, &p2, &lastCrash1, &lastCrash2, mode, *lives1, *lives2, message, waitColor);
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

    // Muestro las vidas de cada jugador
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

    // Dibujo los textos con parpadeos
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

        inputQueueFetch(INPUT_FETCH_LIMIT);

        // Escucho nuevos inputs
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

    // Imprimo pantalla de resultado
    int summary = showMatchResult(mode, result.lives1, result.lives2);
    if (summary == -1) {
        result.aborted = true;
    }
    return result;
}

int tronGame(char **argv, int argc) {
    inputQueueClear();
    enableGraphicsMode();

    int running = 1;
    while (running) {
        int mode = showMainMenu();
        if (mode == 0) {
            break;
        }

        MatchResult result = runMatch(mode);
        if (result.aborted) {
            running = 0;
        }
    }

    clear_audio_buffer();
    disableGraphicsMode();
    return 0;
}