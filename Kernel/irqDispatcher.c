#include <tick.h>
#include <stdint.h>
#include <keyboard.h>

static void int_20();
static void int_21();
extern void backupregs();

void irqDispatcher(uint64_t irq) {
	backupregs();
	switch (irq) {
		case 0:
			int_20();
			break;
		case 1:
			int_21();
			break;
	}
	return;
}

void int_21() {
	keyboard_handler();
}

void int_20() {
	timer_handler();
}
