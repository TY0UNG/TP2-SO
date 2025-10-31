#include <tick.h>
#include <audio.h>

static unsigned long ticks = 0;

void timer_handler() {
	ticks++;
	 audio_timer_handler();

}

int ticks_elapsed() {
	return ticks;
}

int seconds_elapsed() {
	return ticks / 18;
}
