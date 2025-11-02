#include "time.h"

#define PIT_INPUT_FREQUENCY_HZ 1193182.0
#define PIT_CHANNEL0_DIVISOR   65536.0
#define PIT_TICK_FREQUENCY_HZ  (PIT_INPUT_FREQUENCY_HZ / PIT_CHANNEL0_DIVISOR)
#define CALIBRATION_WINDOW_MS  320.0

extern uint64_t getMilis();
extern uint8_t getSeconds();
extern uint8_t getMinutes();
extern uint8_t getHour();
extern uint8_t getDayOfMonth();
extern uint8_t getMonth();
extern uint8_t getYear();

double ticksPerMs;

void calibrateMilis(void) {
    if (ticksPerMs > 0.0) return;

    const int target_ticks =
        (int)((CALIBRATION_WINDOW_MS / 1000.0) * PIT_TICK_FREQUENCY_HZ + 0.5);

    const int start_ticks = ticks_elapsed();
    const uint64_t start_cycles = getMilis();

    int ticks_delta;
    do {
        ticks_delta = ticks_elapsed() - start_ticks;
    } while (ticks_delta < target_ticks);

    if (ticks_delta <= 0) {
        ticks_delta = target_ticks;
    }

    const double elapsed_ms =
        ticks_delta * (1000.0 / PIT_TICK_FREQUENCY_HZ);

    const uint64_t cycles_delta = getMilis() - start_cycles;
    ticksPerMs = cycles_delta / elapsed_ms;
}

uint64_t getMilisFromBoot() {
    return getMilis() / ticksPerMs;
}

void getTime(DateTime *output) {
    if (!output) return;
    output->year = getYear();
    output->month = getMonth();
    output->daymonth = getDayOfMonth();
    output->hour = getHour();
    output->minutes = getMinutes();
    output->seconds = getSeconds();
}