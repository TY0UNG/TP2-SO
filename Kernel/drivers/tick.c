#include <tick.h>
#include <audio.h>
#include <processes.h>

static unsigned long ticks = 0;

void timer_handler() {
	ticks++;
	audio_timer_handler();
	scheduler();
}

int ticks_elapsed() {
	return ticks;
}

int seconds_elapsed() {
	return ticks / 18;
}