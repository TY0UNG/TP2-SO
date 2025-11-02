#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <lib.h>
#include <moduleLoader.h>
#include <screen.h>
#include <idtLoader.h>
#include <interrupts.h>
#include <video.h>
#include <keyboard.h>
#include "./drivers/time.h"

#include <audio.h>
extern void diagnostic_test(); 					////

extern uint8_t text;
extern uint8_t rodata;
extern uint8_t data;
extern uint8_t bss;
extern uint8_t endOfKernelBinary;
extern uint8_t endOfKernel;

static const uint64_t PageSize = 0x1000;

static void * const sampleCodeModuleAddress = (void*)0x600000;
static void * const sampleDataModuleAddress = (void*)0x500000;

static void * const shell = (void*)0x400000;
typedef int (*EntryPoint)();

static uint64_t strLength(const char * str) {
	if (str == 0) {
		return 0;
	}
	uint64_t len = 0;
	while (str[len] != 0) {
		len++;
	}
	return len;
}

void * getShellAddress() {
    return (void*)shell;
}

void clearBSS(void * bssAddress, uint64_t bssSize)
{
	memset(bssAddress, 0, bssSize);
}

void * getStackBase()
{
	return (void*)(
		(uint64_t)&endOfKernel
		+ PageSize * 8				//The size of the stack itself, 32KiB
		- sizeof(uint64_t)			//Begin at the top of the stack
	);
}

void * initializeKernelBinary() {
	char buffer[10];

	ncPrint("[x64BareBones]");
	ncNewline();

	ncPrint("CPU Vendor:");
	ncPrint(cpuVendor(buffer));
	ncNewline();

	ncPrint("[Loading modules]");
	ncNewline();
	void * moduleAddresses[] = {
		sampleCodeModuleAddress,
		sampleDataModuleAddress,
		shell
	};

	loadModules(&endOfKernelBinary, moduleAddresses);
	ncPrint("[Done]");
	ncNewline();
	ncNewline();

	ncPrint("[Initializing kernel's binary]");
	ncNewline();

	clearBSS(&bss, &endOfKernel - &bss);

	ncPrint("  text: 0x");
	ncPrintHex((uint64_t)&text);
	ncNewline();
	ncPrint("  rodata: 0x");
	ncPrintHex((uint64_t)&rodata);
	ncNewline();
	ncPrint("  data: 0x");
	ncPrintHex((uint64_t)&data);
	ncNewline();
	ncPrint("  bss: 0x");
	ncPrintHex((uint64_t)&bss);
	ncNewline();

	ncPrint("[Done]");
	ncNewline();
	ncNewline();
	ncClear();
	return getStackBase();
}



 



/////ESTO ES APARTE
void MusicSO(){
	
	play_sound(440, 500);
    play_sound(440, 500);
    play_sound(440, 500);
    play_sound(349, 350);
    play_sound(523, 150);
    play_sound(440, 500);
    play_sound(349, 350);
    play_sound(523, 150);
    play_sound(440, 1000);

	

	/*
  play_sound(700, 200);
    play_sound(900, 200);

    // Cierre descendente rápido (hum futurista)
    play_sound(600, 150);
    play_sound(400, 150);
    play_sound(300, 200);
	*/
	
}
void sleep(uint64_t ms) {
    uint64_t start_time = getMilisFromBoot();
    uint64_t end_time = start_time + ms;
    while (getMilisFromBoot() < end_time);
}

static void bootAnimation(void) {
	const uint32_t bg_color = 0x050518;
	const uint32_t ring_color = 0x00A8FF;
	const uint32_t fill_outer_color = 0x35B4FF;
	const uint32_t fill_inner_color = 0x0C2E68;

	const uint16_t width = getScreenWidth();
	const uint16_t height = getScreenHeight();
	const int center_x = width / 2;
	const int center_y = height / 2;
	const int min_dimension = width < height ? width : height;

	int base_radius = min_dimension / 6;
	if (base_radius < 56) {
		base_radius = 56;
	}

	canvasMode();
	fillScreen(bg_color);
	swapBuffers();

	const int fill_steps = 10;
	for (int step = 0; step <= fill_steps; step++) {
		uint16_t outer_radius = (uint16_t)((base_radius * step) / fill_steps);
		uint16_t inner_radius = (uint16_t)((outer_radius * 2) / 3);

		fillScreen(bg_color);
		if (outer_radius > 0) {
			drawFilledCircle((uint64_t)center_x, (uint64_t)center_y, outer_radius, fill_outer_color);
		}
		if (inner_radius > 0) {
			drawFilledCircle((uint64_t)center_x, (uint64_t)center_y, inner_radius, fill_inner_color);
		}
		drawCircle((uint64_t)center_x, (uint64_t)center_y, outer_radius, 3, ring_color);
		drawCircle((uint64_t)center_x, (uint64_t)center_y, inner_radius / 2, 2, ring_color);
		swapBuffers();
		sleep(28);
	}

	const char * label = "TobaOS";
	const int text_frames = 10;
	uint16_t text_height = (uint16_t)base_radius;
	if (text_height < 48) {
		text_height = 48;
	}
	uint16_t char_width = (uint16_t)((8 * text_height) / 16);
	uint64_t text_width = strLength(label) * char_width;
	uint64_t text_x = (uint64_t)center_x - text_width / 2;
	uint64_t text_y = (uint64_t)center_y - text_height / 2;

	for (int step = 0; step < text_frames; step++) {
		uint8_t intensity = (uint8_t)(96 + ((159 * step) / (text_frames - 1)));
		uint32_t text_color = ((uint32_t)intensity << 16) | ((uint32_t)intensity << 8) | intensity;

		fillScreen(bg_color);
		drawFilledCircle((uint64_t)center_x, (uint64_t)center_y, (uint16_t)base_radius, fill_outer_color);
		drawFilledCircle((uint64_t)center_x, (uint64_t)center_y, (uint16_t)(base_radius * 2 / 3), fill_inner_color);
		drawCircle((uint64_t)center_x, (uint64_t)center_y, (uint16_t)base_radius, 3, ring_color);
		drawCircle((uint64_t)center_x, (uint64_t)center_y, (uint16_t)(base_radius / 3), 2, ring_color);
		drawText(text_x, text_y, label, text_height, text_color);
		swapBuffers();
		sleep(32);
	}

	sleep(220);
	textMode();
}

int main() {	
	load_idt();
    ncSetStyle(0x0F);
	start_T();
	MusicSO();
	keyboard_set_enabled(false);
	
	canvasMode();
	fillScreen(0x050518);
	swapBuffers();
	calibrateMilis();
	bootAnimation();

	keyboard_set_enabled(true);
	clearTextBuffer();

    ((EntryPoint)shell)();

    print("Returned from shell (unexpected)\n");
    
    return 0;
}