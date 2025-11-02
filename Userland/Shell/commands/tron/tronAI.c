#include "tronAI.h"
#include <stddef.h>

static int manhattan_distance(int ax, int ay, int bx, int by) {
    int dx = ax - bx;
    if (dx < 0) {
        dx = -dx;
    }
    int dy = ay - by;
    if (dy < 0) {
        dy = -dy;
    }
    return dx + dy;
}

static bool inside_arena(int col, int row) {
    return col >= 0 && col < ARENA_COLS && row >= 0 && row < ARENA_ROWS;
}

static int score_direction(int col,
                           int row,
                           Direction dir,
                           int depth,
                           TronCellFn cellAt) {
    int score = 0;
    int currentCol = col;
    int currentRow = row;

    for (int step = 0; step < depth; step++) {
        currentCol += TRON_DELTA_COL[dir];
        currentRow += TRON_DELTA_ROW[dir];

        if (!inside_arena(currentCol, currentRow)) {
            break;
        }
        if (cellAt(currentCol, currentRow) != OCC_EMPTY) {
            break;
        }

        score += 2;

        Direction left = rotateLeft(dir);
        Direction right = rotateRight(dir);
        int leftCol = currentCol + TRON_DELTA_COL[left];
        int leftRow = currentRow + TRON_DELTA_ROW[left];
        int rightCol = currentCol + TRON_DELTA_COL[right];
        int rightRow = currentRow + TRON_DELTA_ROW[right];
        if (inside_arena(leftCol, leftRow) && cellAt(leftCol, leftRow) == OCC_EMPTY) {
            score++;
        }
        if (inside_arena(rightCol, rightRow) && cellAt(rightCol, rightRow) == OCC_EMPTY) {
            score++;
        }
    }

    return score;
}

static bool path_clear_to_target(const Cycle *from,
                                 const Cycle *target,
                                 Direction dir,
                                 int maxSteps,
                                 TronCellFn cellAt) {
    if (target == NULL || !target->alive) {
        return false;
    }
    int col = from->col;
    int row = from->row;
    for (int step = 0; step < maxSteps; step++) {
        col += TRON_DELTA_COL[dir];
        row += TRON_DELTA_ROW[dir];
        if (!inside_arena(col, row)) {
            return false;
        }
        if (col == target->col && row == target->row) {
            return true;
        }
        if (cellAt(col, row) != OCC_EMPTY) {
            return false;
        }
    }
    return false;
}

static const int AI_DECISION_DEPTH = 8;
static const int AI_CHASE_WEIGHT = 70;
static const int AI_BOOST_DISTANCE_THRESHOLD = 20;
static const int AI_BOOST_PATH_STEPS = 23;

Direction tronAiChooseDirection(const Cycle *ai,
                                const Cycle *target,
                                TronCellFn cellAt) {
    int depth = AI_DECISION_DEPTH;

    Direction options[3];
    options[0] = ai->dir;
    options[1] = rotateLeft(ai->dir);
    options[2] = rotateRight(ai->dir);

    int bestIndex = 0;
    int bestScore = -32000;

    for (int i = 0; i < 3; i++) {
        Direction dir = options[i];
        if (dir == oppositeDir(ai->dir)) {
            continue;
        }
        int nextCol = ai->col + TRON_DELTA_COL[dir];
        int nextRow = ai->row + TRON_DELTA_ROW[dir];
        if (!inside_arena(nextCol, nextRow)) {
            continue;
        }
        uint8_t occ = cellAt(nextCol, nextRow);
        bool targetCell = false;
        if (target != NULL && target->alive) {
            targetCell = (nextCol == target->col && nextRow == target->row);
        }
        if (occ != OCC_EMPTY && !targetCell) {
            continue;
        }

    int spaceScore = score_direction(ai->col, ai->row, dir, depth, cellAt) * 3;

        int chaseScore = 0;
        if (target != NULL && target->alive) {
            int distance = manhattan_distance(nextCol, nextRow, target->col, target->row);
            chaseScore = AI_CHASE_WEIGHT - distance * 6;
        }

        int totalScore = spaceScore + chaseScore;
        if (totalScore > bestScore || (totalScore == bestScore && dir == ai->dir)) {
            bestScore = totalScore;
            bestIndex = i;
        }
    }

    return options[bestIndex];
}

void tronAiManageBoost(Cycle *ai,
                       const Cycle *target,
                       uint64_t now,
                       uint64_t roundStart,
                       TronCellFn cellAt) {
    if (ai->boostUsed || (ai->boostUntil > now)) {
        return;
    }
    if (now - roundStart < 900) {
        return;
    }
    if (target == NULL || !target->alive) {
        return;
    }

    int distance = manhattan_distance(ai->col, ai->row, target->col, target->row);
    if (distance < 6) {
        return;
    }

    Direction chaseDir = ai->pendingDir;
    if (chaseDir == oppositeDir(ai->dir)) {
        chaseDir = ai->dir;
    }

    bool alignedHoriz = (ai->row == target->row);
    bool alignedVert = (ai->col == target->col);

    if ((alignedHoriz && ((target->col > ai->col && chaseDir == DIR_RIGHT) ||
                          (target->col < ai->col && chaseDir == DIR_LEFT))) ||
        (alignedVert && ((target->row > ai->row && chaseDir == DIR_DOWN) ||
                         (target->row < ai->row && chaseDir == DIR_UP)))) {
        if (path_clear_to_target(ai, target, chaseDir, distance + 2, cellAt)) {
            ai->boostUsed = true;
            ai->boostUntil = now + BOOST_DURATION_MS;
            return;
        }
    }

    if (distance <= AI_BOOST_DISTANCE_THRESHOLD &&
        path_clear_to_target(ai, target, chaseDir, AI_BOOST_PATH_STEPS, cellAt)) {
        ai->boostUsed = true;
        ai->boostUntil = now + BOOST_DURATION_MS;
    }
}
