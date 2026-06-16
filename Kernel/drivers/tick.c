#include <tick.h>
#include <audio.h>
#include <processes.h>
#include <keyboard.h>
#include <terminal.h>

static unsigned long ticks = 0;

void timer_handler() {
	ticks++;
	audio_timer_handler();
	// Ctrl+C: matar al foreground si se puede (aunque no este leyendo)
	if (consume_sigint()) {
		pid_t fg = get_foreground_pid();
		if (fg != 0 && process_is_killable(fg)) kill_process(fg);
	}
	timer_tick();

}

int ticks_elapsed() {
	return ticks;
}

int seconds_elapsed() {
	return ticks / 18;
}