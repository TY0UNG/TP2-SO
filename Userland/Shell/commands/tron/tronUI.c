#include "tronUI.h"

#include <stddef.h>

static const int PANEL_LABEL_OFFSET = 18;
static const int BOOST_LABEL_Y = HUD_TOP + 110;
static const int BOOST_BAR_Y = BOOST_LABEL_Y + 22;
static const int BOOST_BAR_HEIGHT = 18;
static const int BOOST_BAR_PADDING = 4;

typedef struct {
    bool baseInitialized;
    int mode;
    int lives1;
    int lives2;
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
    drawFilledRectangle(x1, y1, x2, y2, fill, false);
    drawRectangle(x1, y1, x2, y2, 2, outline);
}

static void drawArenaBase(void) {
    drawFilledRectangle(ARENA_LEFT - 6, ARENA_TOP - 6, ARENA_RIGHT + 6, ARENA_BOTTOM + 6, COLOR_PANEL_LIGHT, false);
    drawFilledRectangle(ARENA_LEFT, ARENA_TOP, ARENA_RIGHT, ARENA_BOTTOM, COLOR_PANEL_DARK, false);

    for (int c = 0; c <= ARENA_COLS; c++) {
        uint64_t x = ARENA_LEFT + (uint64_t)c * CELL_SIZE;
        drawFilledRectangle(x, ARENA_TOP, x + 1, ARENA_BOTTOM, (c % 4 == 0) ? COLOR_GRID_GLOW : COLOR_GRID_LINE, false);
    }
    for (int r = 0; r <= ARENA_ROWS; r++) {
        uint64_t y = ARENA_TOP + (uint64_t)r * CELL_SIZE;
        drawFilledRectangle(ARENA_LEFT, y, ARENA_RIGHT, y + 1, (r % 4 == 0) ? COLOR_GRID_GLOW : COLOR_GRID_LINE, false);
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
        drawFilledRectangle(left, y, right, y + segmentHeight, color, false);
        drawRectangle(left, y, right, y + segmentHeight, 1, COLOR_PANEL_LIGHT);
    }
}

static void drawTopHud(int mode,
                       int lives1,
                       int lives2,
                       const Cycle *p1,
                       const Cycle *p2) {
    drawFilledRectangle(0, 0, SCREEN_WIDTH, HUD_TOP, COLOR_PANEL_DARK, false);
    drawFilledRectangle(0, HUD_TOP - 6, SCREEN_WIDTH, HUD_TOP, COLOR_PANEL_ACCENT, false);

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

static void drawSidePanelBase(int mode, const Cycle *p1, const Cycle *p2) {
    const int leftPanelLeft = 0;
    const int rightPanelLeft = SCREEN_WIDTH - PANEL_RIGHT_WIDTH;

    drawFilledRectangle(leftPanelLeft, HUD_TOP, PANEL_LEFT_WIDTH, ARENA_BOTTOM, COLOR_PANEL_ACCENT, false);
    drawFilledRectangle(PANEL_LEFT_WIDTH - 4, HUD_TOP, PANEL_LEFT_WIDTH, ARENA_BOTTOM, COLOR_PANEL_LIGHT, false);

    drawFilledRectangle(ARENA_RIGHT, HUD_TOP, SCREEN_WIDTH, ARENA_BOTTOM, COLOR_PANEL_ACCENT, false);
    drawFilledRectangle(ARENA_RIGHT, HUD_TOP, ARENA_RIGHT + 4, ARENA_BOTTOM, COLOR_PANEL_LIGHT, false);

    const char *rightName = (mode == 1) ? "CPU" : "P2";

    drawText(leftPanelLeft + PANEL_LABEL_OFFSET, HUD_TOP + 54, "P1", 26, p1->uiColor);
    drawText(rightPanelLeft + PANEL_LABEL_OFFSET, HUD_TOP + 54, rightName, 26, p2->uiColor);

    drawText(leftPanelLeft + PANEL_LABEL_OFFSET, BOOST_LABEL_Y, "Boost", 18, COLOR_TEXT_MUTED);
    drawText(rightPanelLeft + PANEL_LABEL_OFFSET, BOOST_LABEL_Y, "Boost", 18, COLOR_TEXT_MUTED);

    int barLeft = leftPanelLeft + PANEL_LABEL_OFFSET;
    int barRight = leftPanelLeft + PANEL_LEFT_WIDTH - PANEL_LABEL_OFFSET;
    int barTop = BOOST_BAR_Y - BOOST_BAR_PADDING;
    int barBottom = BOOST_BAR_Y + BOOST_BAR_HEIGHT + BOOST_BAR_PADDING;
    drawFilledRectangle(barLeft, barTop, barRight, barBottom, COLOR_PANEL_ACCENT, false);
    drawRectangle(barLeft, barTop, barRight, barBottom, 1, COLOR_PANEL_LIGHT);

    barLeft = rightPanelLeft + PANEL_LABEL_OFFSET;
    barRight = rightPanelLeft + PANEL_RIGHT_WIDTH - PANEL_LABEL_OFFSET;
    drawFilledRectangle(barLeft, barTop, barRight, barBottom, COLOR_PANEL_ACCENT, false);
    drawRectangle(barLeft, barTop, barRight, barBottom, 1, COLOR_PANEL_LIGHT);

    drawText(leftPanelLeft + PANEL_LABEL_OFFSET, ARENA_BOTTOM - 134, "Controls:", 18, COLOR_TEXT_MUTED);
    drawText(leftPanelLeft + PANEL_LABEL_OFFSET, ARENA_BOTTOM - 106, "Arrows + P", 18, COLOR_TEXT_MUTED);
    drawText(leftPanelLeft + PANEL_LABEL_OFFSET, ARENA_BOTTOM - 78, "ESC to exit", 18, COLOR_TEXT_MUTED);

    drawText(rightPanelLeft + PANEL_LABEL_OFFSET, ARENA_BOTTOM - 134, "Controls:", 18, COLOR_TEXT_MUTED);
    drawText(rightPanelLeft + PANEL_LABEL_OFFSET, ARENA_BOTTOM - 106, "WASD + Q", 18, COLOR_TEXT_MUTED);
}

static void drawStatusBar(const char *text, uint32_t color, bool flash) {
    drawFilledRectangle(0, ARENA_BOTTOM, SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_STATUS, false);
    drawFilledRectangle(0, ARENA_BOTTOM, SCREEN_WIDTH, ARENA_BOTTOM + 6, COLOR_PANEL_ACCENT, false);
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
    uiCache.statusText = NULL;
    uiCache.statusColor = 0;
    uiCache.statusFlash = false;
}

bool tronUiEnsureStatic(int mode,
                        int lives1,
                        int lives2,
                        const Cycle *p1,
                        const Cycle *p2,
                        uint64_t now) {
    (void)now;
    bool dirty = false;

    bool baseInvalidated = false;
    if (!uiCache.baseInitialized) {
        drawFillScreen(COLOR_BACKGROUND);
        drawArenaBase();
        uiCache.baseInitialized = true;
        baseInvalidated = true;
        uiCache.mode = -1;
        uiCache.lives1 = -1;
        uiCache.lives2 = -1;
        dirty = true;
    }

    bool modeChanged = (uiCache.mode != mode);

    if (modeChanged || uiCache.lives1 != lives1 || uiCache.lives2 != lives2) {
        drawTopHud(mode, lives1, lives2, p1, p2);
        uiCache.mode = mode;
        uiCache.lives1 = lives1;
        uiCache.lives2 = lives2;
        dirty = true;
    }

    if (modeChanged || baseInvalidated) {
        drawSidePanelBase(mode, p1, p2);
        dirty = true;
    }
    return dirty;
}

bool tronUiUpdateStatus(const char *text, uint32_t color, bool flash) {
    if (uiCache.statusText != text || uiCache.statusColor != color || uiCache.statusFlash != flash) {
        drawStatusBar(text, color, flash);
        uiCache.statusText = text;
        uiCache.statusColor = color;
        uiCache.statusFlash = flash;
        return true;
    }
    return false;
}

void tronUiRedrawArena(TronCellFn cellAt) {
    if (cellAt == NULL) {
        return;
    }
    for (int r = 0; r < ARENA_ROWS; r++) {
        for (int c = 0; c < ARENA_COLS; c++) {
            uint8_t cell = cellAt(c, r);
            if (cell == OCC_EMPTY) {
                continue;
            }
            tronUiDrawTrailCell(c, r, cell, false);
        }
    }
}

void tronUiDrawTrailCell(int col, int row, uint8_t owner, bool directWrite) {
    if (col < 0 || col >= ARENA_COLS || row < 0 || row >= ARENA_ROWS) {
        return;
    }
    uint32_t color = (owner == OCC_P1) ? COLOR_P1_TRAIL : COLOR_P2_TRAIL;
    uint64_t x1 = ARENA_LEFT + (uint64_t)col * CELL_SIZE;
    uint64_t y1 = ARENA_TOP + (uint64_t)row * CELL_SIZE;
    drawFilledRectangle(x1 + 1, y1 + 1, x1 + CELL_SIZE - 1, y1 + CELL_SIZE - 1, color, directWrite);
}

void tronUiDrawCycleHead(const Cycle *cycle, bool directWrite) {
    if (cycle == NULL || !cycle->alive) {
        return;
    }
    uint64_t x1 = ARENA_LEFT + (uint64_t)cycle->col * CELL_SIZE;
    uint64_t y1 = ARENA_TOP + (uint64_t)cycle->row * CELL_SIZE;
    uint64_t innerLeft = x1 + 2;
    uint64_t innerTop = y1 + 2;
    uint64_t innerRight = x1 + CELL_SIZE - 2;
    uint64_t innerBottom = y1 + CELL_SIZE - 2;

    drawFilledRectangle(innerLeft, innerTop, innerRight, innerBottom, cycle->headColor, directWrite);

    uint64_t borderLeft = x1 + 1;
    uint64_t borderRight = x1 + CELL_SIZE - 1;
    uint64_t borderTop = y1 + 1;
    uint64_t borderBottom = y1 + CELL_SIZE - 1;

    if (directWrite) {
        drawFilledRectangle(borderLeft, borderTop, borderRight, borderTop, cycle->uiColor, true);
        drawFilledRectangle(borderLeft, borderBottom, borderRight, borderBottom, cycle->uiColor, true);
        drawFilledRectangle(borderLeft, borderTop, borderLeft, borderBottom, cycle->uiColor, true);
        drawFilledRectangle(borderRight, borderTop, borderRight, borderBottom, cycle->uiColor, true);
    } else {
        drawRectangle(borderLeft, borderTop, borderRight, borderBottom, 1, cycle->uiColor);
    }
}

void tronUiDrawCrashMarker(const CrashMarker *marker, uint32_t fillColor, bool directWrite) {
    if (marker == NULL || !marker->active) {
        return;
    }
    if (marker->col < 0 || marker->col >= ARENA_COLS || marker->row < 0 || marker->row >= ARENA_ROWS) {
        return;
    }
    uint64_t x1 = ARENA_LEFT + (uint64_t)marker->col * CELL_SIZE;
    uint64_t y1 = ARENA_TOP + (uint64_t)marker->row * CELL_SIZE;
    uint64_t innerLeft = x1 + 1;
    uint64_t innerTop = y1 + 1;
    uint64_t innerRight = x1 + CELL_SIZE - 1;
    uint64_t innerBottom = y1 + CELL_SIZE - 1;

    drawFilledRectangle(innerLeft, innerTop, innerRight, innerBottom, fillColor, directWrite);

    if (directWrite) {
        drawFilledRectangle(innerLeft, innerTop, innerRight, innerTop, COLOR_STATUS_ALERT, true);
        drawFilledRectangle(innerLeft, innerBottom, innerRight, innerBottom, COLOR_STATUS_ALERT, true);
        drawFilledRectangle(innerLeft, innerTop, innerLeft, innerBottom, COLOR_STATUS_ALERT, true);
        drawFilledRectangle(innerRight, innerTop, innerRight, innerBottom, COLOR_STATUS_ALERT, true);
    } else {
        drawRectangle(innerLeft, innerTop, innerRight, innerBottom, 1, COLOR_STATUS_ALERT);
    }
}

void tronUiDrawBoostMeter(const Cycle *cycle, bool leftPanel, uint64_t now, bool directWrite) {
    int panelLeft = leftPanel ? 0 : (SCREEN_WIDTH - PANEL_RIGHT_WIDTH);
    int panelWidth = leftPanel ? PANEL_LEFT_WIDTH : PANEL_RIGHT_WIDTH;

    int frameLeft = panelLeft + PANEL_LABEL_OFFSET;
    int frameRight = panelLeft + panelWidth - PANEL_LABEL_OFFSET;
    int frameTop = BOOST_BAR_Y - BOOST_BAR_PADDING;
    int frameBottom = BOOST_BAR_Y + BOOST_BAR_HEIGHT + BOOST_BAR_PADDING;

    if (frameRight < frameLeft) {
        frameRight = frameLeft;
    }
    if (frameBottom < frameTop) {
        frameBottom = frameTop;
    }

    drawFilledRectangle(frameLeft, frameTop, frameRight, frameBottom, COLOR_PANEL_ACCENT, directWrite);

    int fillLeft = frameLeft + 1;
    int fillRightLimit = frameRight - 1;
    int fillTop = BOOST_BAR_Y;
    int fillBottom = BOOST_BAR_Y + BOOST_BAR_HEIGHT;

    if (fillRightLimit < fillLeft) {
        fillRightLimit = frameRight;
        fillLeft = frameLeft;
    }
    if (fillBottom < fillTop) {
        fillBottom = frameBottom;
        fillTop = frameTop;
    }

    int fillWidth = 0;
    uint32_t fillColor = COLOR_STATUS_SUCCESS;

    if (cycle != NULL) {
        if (cycle->boostUntil > now) {
            uint64_t remaining = cycle->boostUntil - now;
            if (remaining > BOOST_DURATION_MS) {
                remaining = BOOST_DURATION_MS;
            }
            uint64_t span = (uint64_t)(fillRightLimit - fillLeft);
            fillWidth = (int)((remaining * span) / BOOST_DURATION_MS);
            if (fillWidth > (int)span) {
                fillWidth = (int)span;
            }
            if (fillWidth > 0 && fillWidth < 2) {
                fillWidth = 2;
            }
            fillColor = cycle->uiColor;
        } else if (!cycle->boostUsed) {
            fillWidth = fillRightLimit - fillLeft;
            fillColor = COLOR_STATUS_SUCCESS;
        } else {
            fillWidth = 0;
        }
    }

    if (fillWidth > 0) {
        int fillRight = fillLeft + fillWidth;
        if (fillRight > fillRightLimit) {
            fillRight = fillRightLimit;
        }
        drawFilledRectangle(fillLeft, fillTop, fillRight, fillBottom, fillColor, directWrite);
    }

    if (directWrite) {
        drawFilledRectangle(frameLeft, frameTop, frameRight, frameTop, COLOR_PANEL_LIGHT, true);
        drawFilledRectangle(frameLeft, frameBottom, frameRight, frameBottom, COLOR_PANEL_LIGHT, true);
        drawFilledRectangle(frameLeft, frameTop, frameLeft, frameBottom, COLOR_PANEL_LIGHT, true);
        drawFilledRectangle(frameRight, frameTop, frameRight, frameBottom, COLOR_PANEL_LIGHT, true);
    } else {
        drawRectangle(frameLeft, frameTop, frameRight, frameBottom, 1, COLOR_PANEL_LIGHT);
    }
}

void tronUiDrawMenuFrame(int selection, int animPhase) {
    drawFillScreen(COLOR_BACKGROUND);

    for (int i = 0; i < 8; i++) {
        int offset = (animPhase + i * 40) % SCREEN_HEIGHT;
        int bandTop = offset;
        int bandBottom = offset + 24;
        uint32_t tint = (i % 2 == 0) ? COLOR_GRID_GLOW : COLOR_PANEL_LIGHT;
        drawFilledRectangle(0, bandTop, SCREEN_WIDTH, bandBottom, tint, false);
    }

    drawFilledRectangle(0, 0, SCREEN_WIDTH, HUD_TOP, COLOR_PANEL_DARK, false);
    drawFilledRectangle(0, HUD_TOP - 6, SCREEN_WIDTH, HUD_TOP, COLOR_PANEL_ACCENT, false);

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
