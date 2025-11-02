#include "tronUI.h"

#include <stddef.h>
#include "../../lib/time.h"

typedef struct {
    bool baseInitialized;
    int mode;
    int lives1;
    int lives2;
    bool boostReady1;
    bool boostReady2;
    bool boostFlash;
    const char *statusText;
    uint32_t statusColor;
    bool statusFlash;
} UiCache;

static UiCache uiCache;

static void drawPanelFrame(uint64_t x1,
                           uint64_t y1,
                           uint64_t x2,
                           uint64_t y2,
                           uint32_t fill,
                           uint32_t outline) {
    drawFilledRectangle(x1, y1, x2, y2, fill);
    drawRectangle(x1, y1, x2, y2, 2, outline);
}

static void drawArenaBase(void) {
    drawFilledRectangle(ARENA_LEFT - 6, ARENA_TOP - 6, ARENA_RIGHT + 6, ARENA_BOTTOM + 6, COLOR_PANEL_LIGHT);
    drawFilledRectangle(ARENA_LEFT, ARENA_TOP, ARENA_RIGHT, ARENA_BOTTOM, COLOR_PANEL_DARK);

    for (int c = 0; c <= ARENA_COLS; c++) {
        uint64_t x = ARENA_LEFT + (uint64_t)c * CELL_SIZE;
        drawFilledRectangle(x, ARENA_TOP, x + 1, ARENA_BOTTOM, (c % 4 == 0) ? COLOR_GRID_GLOW : COLOR_GRID_LINE);
    }
    for (int r = 0; r <= ARENA_ROWS; r++) {
        uint64_t y = ARENA_TOP + (uint64_t)r * CELL_SIZE;
        drawFilledRectangle(ARENA_LEFT, y, ARENA_RIGHT, y + 1, (r % 4 == 0) ? COLOR_GRID_GLOW : COLOR_GRID_LINE);
    }
}

static void drawLivesBar(int panelLeft,
                         int panelWidth,
                         int y,
                         int lives,
                         uint32_t activeColor) {
    const int segmentHeight = 18;
    const int spacing = 6;
    int segmentWidth = (panelWidth / MAX_LIVES) - spacing;
    if (segmentWidth < 18) {
        segmentWidth = 18;
    }
    int totalWidth = MAX_LIVES * segmentWidth + (MAX_LIVES - 1) * spacing;
    if (totalWidth > panelWidth) {
        segmentWidth = (panelWidth - spacing * (MAX_LIVES - 1)) / MAX_LIVES;
        if (segmentWidth < 12) {
            segmentWidth = 12;
        }
        totalWidth = MAX_LIVES * segmentWidth + (MAX_LIVES - 1) * spacing;
    }
    int startX = panelLeft + (panelWidth - totalWidth) / 2;
    for (int i = 0; i < MAX_LIVES; i++) {
        int left = startX + i * (segmentWidth + spacing);
        int right = left + segmentWidth;
        uint32_t color = (i < lives) ? activeColor : COLOR_GRID_LINE;
        drawFilledRectangle(left, y, right, y + segmentHeight, color);
        drawRectangle(left, y, right, y + segmentHeight, 1, COLOR_PANEL_LIGHT);
    }
}

static void drawTopHud(int mode,
                       int lives1,
                       int lives2,
                       const Cycle *p1,
                       const Cycle *p2) {
    drawFilledRectangle(0, 0, SCREEN_WIDTH, HUD_TOP, COLOR_PANEL_DARK);
    drawFilledRectangle(0, HUD_TOP - 6, SCREEN_WIDTH, HUD_TOP, COLOR_PANEL_ACCENT);

    drawText(40, 32, "TRON NEON GRID", 32, COLOR_TEXT_PRIMARY);

    drawTextCentered(mode == 1 ? "SOLO PROTOCOL" : "DUO PROTOCOL", 68, 22, COLOR_TEXT_MUTED, SCREEN_WIDTH);

    const int leftPanelLeft = 0;
    const int rightPanelLeft = SCREEN_WIDTH - PANEL_RIGHT_WIDTH;
    const int panelLabelY = HUD_TOP - 54;
    const int barY = HUD_TOP - 30;

    const char *rightLabel = (mode == 1) ? "CPU LIVES" : "P2 LIVES";

    drawText(leftPanelLeft + 18, panelLabelY, "P1 LIVES", 20, p1->uiColor);
    drawLivesBar(leftPanelLeft, PANEL_LEFT_WIDTH, barY, lives1, p1->uiColor);

    drawText(rightPanelLeft + 18, panelLabelY, rightLabel, 20, p2->uiColor);
    drawLivesBar(rightPanelLeft, PANEL_RIGHT_WIDTH, barY, lives2, p2->uiColor);
}

static void drawSidePanels(int mode,
                           const Cycle *p1,
                           const Cycle *p2,
                           bool boostFlash) {
    const int leftPanelLeft = 0;
    const int rightPanelLeft = SCREEN_WIDTH - PANEL_RIGHT_WIDTH;
    const int labelOffset = 18;

    drawFilledRectangle(leftPanelLeft, HUD_TOP, PANEL_LEFT_WIDTH, ARENA_BOTTOM, COLOR_PANEL_ACCENT);
    drawFilledRectangle(PANEL_LEFT_WIDTH - 4, HUD_TOP, PANEL_LEFT_WIDTH, ARENA_BOTTOM, COLOR_PANEL_LIGHT);

    drawFilledRectangle(ARENA_RIGHT, HUD_TOP, SCREEN_WIDTH, ARENA_BOTTOM, COLOR_PANEL_ACCENT);
    drawFilledRectangle(ARENA_RIGHT, HUD_TOP, ARENA_RIGHT + 4, ARENA_BOTTOM, COLOR_PANEL_LIGHT);

    const char *rightName = (mode == 1) ? "CPU" : "P2";

    drawText(leftPanelLeft + labelOffset, HUD_TOP + 54, "P1", 26, p1->uiColor);
    drawText(rightPanelLeft + labelOffset, HUD_TOP + 54, rightName, 26, p2->uiColor);

    bool boostActive1 = (p1->boostUntil > getMilisFromBoot());
    bool boostActive2 = (p2->boostUntil > getMilisFromBoot());

        const char *boost1 = p1->boostUsed ? "Spent" : (boostActive1 ? "Active" : "Ready");
        const char *boost2 = p2->boostUsed ? "Spent" : (boostActive2 ? "Active" : "Ready");
        uint32_t color1 = boostActive1 ? COLOR_STATUS_SUCCESS : (p1->boostUsed ? COLOR_TEXT_MUTED : (boostFlash ? COLOR_STATUS_SUCCESS : COLOR_TEXT_MUTED));
        uint32_t color2 = boostActive2 ? COLOR_STATUS_SUCCESS : (p2->boostUsed ? COLOR_TEXT_MUTED : (boostFlash ? COLOR_STATUS_SUCCESS : COLOR_TEXT_MUTED));

        const int boostLabelY = HUD_TOP + 110;
        const int boostValueY = boostLabelY + 22;

        drawText(leftPanelLeft + labelOffset, boostLabelY, "Boost", 18, COLOR_TEXT_MUTED);
        drawText(leftPanelLeft + labelOffset, boostValueY, boost1, 18, color1);

        drawText(rightPanelLeft + labelOffset, boostLabelY, "Boost", 18, COLOR_TEXT_MUTED);
        drawText(rightPanelLeft + labelOffset, boostValueY, boost2, 18, color2);

    drawText(leftPanelLeft + labelOffset, ARENA_BOTTOM - 134, "Controls:", 18, COLOR_TEXT_MUTED);
    drawText(leftPanelLeft + labelOffset, ARENA_BOTTOM - 106, "Arrows + P", 18, COLOR_TEXT_MUTED);
    drawText(leftPanelLeft + labelOffset, ARENA_BOTTOM - 78, "ESC to exit", 18, COLOR_TEXT_MUTED);

    drawText(rightPanelLeft + labelOffset, ARENA_BOTTOM - 134, "Controls:", 18, COLOR_TEXT_MUTED);
    drawText(rightPanelLeft + labelOffset, ARENA_BOTTOM - 106, "WASD + Q", 18, COLOR_TEXT_MUTED);
}

static void drawStatusBar(const char *text, uint32_t color, bool flash) {
    drawFilledRectangle(0, ARENA_BOTTOM, SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_STATUS);
    drawFilledRectangle(0, ARENA_BOTTOM, SCREEN_WIDTH, ARENA_BOTTOM + 6, COLOR_PANEL_ACCENT);
    if (text != NULL) {
        uint32_t tint = flash ? color : COLOR_TEXT_MUTED;
        drawTextCentered(text, ARENA_BOTTOM + 26, 24, tint, SCREEN_WIDTH);
    }
}

void tronUiResetCache(void) {
    uiCache.baseInitialized = false;
    uiCache.mode = -1;
    uiCache.lives1 = -1;
    uiCache.lives2 = -1;
    uiCache.boostReady1 = false;
    uiCache.boostReady2 = false;
    uiCache.boostFlash = false;
    uiCache.statusText = NULL;
    uiCache.statusColor = 0;
    uiCache.statusFlash = false;
}

void tronUiEnsureStatic(int mode,
                        int lives1,
                        int lives2,
                        const Cycle *p1,
                        const Cycle *p2,
                        bool boostFlash) {
    bool boostReady1 = !p1->boostUsed;
    bool boostReady2 = !p2->boostUsed;

    if (!uiCache.baseInitialized) {
        drawFillScreen(COLOR_BACKGROUND);
        drawArenaBase();
        uiCache.baseInitialized = true;
        uiCache.mode = -1;
        uiCache.lives1 = -1;
        uiCache.lives2 = -1;
    }

    bool modeChanged = (uiCache.mode != mode);

    if (modeChanged || uiCache.lives1 != lives1 || uiCache.lives2 != lives2) {
        drawTopHud(mode, lives1, lives2, p1, p2);
        uiCache.mode = mode;
        uiCache.lives1 = lives1;
        uiCache.lives2 = lives2;
    }

    if (modeChanged || uiCache.boostReady1 != boostReady1 || uiCache.boostReady2 != boostReady2 || uiCache.boostFlash != boostFlash) {
        drawSidePanels(mode, p1, p2, boostFlash);
        uiCache.boostReady1 = boostReady1;
        uiCache.boostReady2 = boostReady2;
        uiCache.boostFlash = boostFlash;
    }
}

void tronUiUpdateStatus(const char *text, uint32_t color, bool flash) {
    if (uiCache.statusText != text || uiCache.statusColor != color || uiCache.statusFlash != flash) {
        drawStatusBar(text, color, flash);
        uiCache.statusText = text;
        uiCache.statusColor = color;
        uiCache.statusFlash = flash;
    }
}

void tronUiRedrawArena(const uint8_t occupancy[ARENA_ROWS][ARENA_COLS]) {
    for (int r = 0; r < ARENA_ROWS; r++) {
        for (int c = 0; c < ARENA_COLS; c++) {
            uint8_t cell = occupancy[r][c];
            if (cell == OCC_EMPTY) {
                continue;
            }
            tronUiDrawTrailCell(c, r, cell);
        }
    }
}

void tronUiDrawTrailCell(int col, int row, uint8_t owner) {
    if (col < 0 || col >= ARENA_COLS || row < 0 || row >= ARENA_ROWS) {
        return;
    }
    uint32_t color = (owner == OCC_P1) ? COLOR_P1_TRAIL : COLOR_P2_TRAIL;
    uint64_t x1 = ARENA_LEFT + (uint64_t)col * CELL_SIZE;
    uint64_t y1 = ARENA_TOP + (uint64_t)row * CELL_SIZE;
    drawFilledRectangle(x1 + 1, y1 + 1, x1 + CELL_SIZE - 1, y1 + CELL_SIZE - 1, color);
}

void tronUiDrawCycleHead(const Cycle *cycle) {
    if (cycle == NULL || !cycle->alive) {
        return;
    }
    uint64_t x1 = ARENA_LEFT + (uint64_t)cycle->col * CELL_SIZE;
    uint64_t y1 = ARENA_TOP + (uint64_t)cycle->row * CELL_SIZE;
    drawFilledRectangle(x1 + 2, y1 + 2, x1 + CELL_SIZE - 2, y1 + CELL_SIZE - 2, cycle->headColor);
    drawRectangle(x1 + 1, y1 + 1, x1 + CELL_SIZE - 1, y1 + CELL_SIZE - 1, 1, cycle->uiColor);
}

void tronUiDrawCrashMarker(const CrashMarker *marker, uint32_t fillColor) {
    if (marker == NULL || !marker->active) {
        return;
    }
    if (marker->col < 0 || marker->col >= ARENA_COLS || marker->row < 0 || marker->row >= ARENA_ROWS) {
        return;
    }
    uint64_t x1 = ARENA_LEFT + (uint64_t)marker->col * CELL_SIZE;
    uint64_t y1 = ARENA_TOP + (uint64_t)marker->row * CELL_SIZE;
    drawFilledRectangle(x1 + 1, y1 + 1, x1 + CELL_SIZE - 1, y1 + CELL_SIZE - 1, fillColor);
    drawRectangle(x1 + 1, y1 + 1, x1 + CELL_SIZE - 1, y1 + CELL_SIZE - 1, 1, COLOR_STATUS_ALERT);
}

void tronUiDrawMenuFrame(int selection, int animPhase) {
    drawFillScreen(COLOR_BACKGROUND);

    for (int i = 0; i < 8; i++) {
        int offset = (animPhase + i * 40) % SCREEN_HEIGHT;
        int bandTop = offset;
        int bandBottom = offset + 24;
        uint32_t tint = (i % 2 == 0) ? COLOR_GRID_GLOW : COLOR_PANEL_LIGHT;
        drawFilledRectangle(0, bandTop, SCREEN_WIDTH, bandBottom, tint);
    }

    drawFilledRectangle(0, 0, SCREEN_WIDTH, HUD_TOP, COLOR_PANEL_DARK);
    drawFilledRectangle(0, HUD_TOP - 6, SCREEN_WIDTH, HUD_TOP, COLOR_PANEL_ACCENT);

    drawTextCentered("TRON: NEON GRID", 180, 40, COLOR_TEXT_PRIMARY, SCREEN_WIDTH);
    drawTextCentered("select protocol", 232, 24, COLOR_TEXT_MUTED, SCREEN_WIDTH);

    uint32_t optionColor[2] = {COLOR_TEXT_MUTED, COLOR_TEXT_MUTED};
    if (selection >= 0 && selection < 2) {
        optionColor[selection] = COLOR_TEXT_PRIMARY;
    }

    drawPanelFrame(280, 280, 744, 360, COLOR_PANEL_DARK, COLOR_PANEL_LIGHT);
    drawTextCentered("1. SOLO RUN", 302, 24, optionColor[0], SCREEN_WIDTH);
    drawTextCentered("2. DUO RUN", 334, 24, optionColor[1], SCREEN_WIDTH);

    drawPanelFrame(280, 380, 744, 460, COLOR_PANEL_DARK, COLOR_PANEL_LIGHT);
    drawTextCentered("Enter to confirm", 406, 20, COLOR_TEXT_MUTED, SCREEN_WIDTH);
    drawTextCentered("ESC to exit", 438, 20, COLOR_TEXT_MUTED, SCREEN_WIDTH);

    drawStatusBar("-- stand by --", COLOR_TEXT_MUTED, (animPhase / 40) % 2 == 0);
    swapBuffers();
}
