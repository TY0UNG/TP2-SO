#include <screen.h>

static char buffer[64] = { '0' };
static uint8_t * const video = (uint8_t*)0xB8000;
static uint8_t * currentVideo = (uint8_t*)0xB8000;
static const uint32_t width = 80;
static const uint32_t height = 25;

static char currentStyle = 0x0F;

void ncPrint(const char * string) {
	int i;

	for (i = 0; string[i] != 0; i++)
		if (string[i] == '\n') ncNewline();
		else ncPrintChar(string[i]);
}

void ncPrintChar(char character) {
	*(currentVideo++) = character;
	*(currentVideo++) = currentStyle;
}

void ncNewline() {
	do
	{
		ncPrintChar(0);
	}
	while((uint64_t)(currentVideo - video) % (width * 2) != 0);
}

void ncPrintDec(uint64_t value) {
	ncPrintBase(value, 10);
}

void ncPrintHex(uint64_t value) {
	ncPrintBase(value, 16);
}

void ncPrintBin(uint64_t value) {
	ncPrintBase(value, 2);
}

void ncPrintBase(uint64_t value, uint32_t base) {
    uintToBase(value, buffer, base);
    ncPrint(buffer);
}

void ncDelChar() {
	if (currentVideo == (uint8_t*)0xB8000) return;
	*(--currentVideo) = 0;
	*(--currentVideo) = 0;
	while(*(currentVideo-2) == 0) {
		*(--currentVideo) = 0;
		*(--currentVideo) = 0;
	};
}

void ncClear() {
	int i;

	currentStyle = 0x00;
	for (i = 0; i < height * width; i++) {
		video[i * 2] = ' ';
		video[i * 2 + 1] = currentStyle;
	}
	currentVideo = video;
}

void ncSetStyle(char style) {
	currentStyle = style;
}