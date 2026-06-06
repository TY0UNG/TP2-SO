#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <lib.h>
#include <moduleLoader.h>
#include <screen.h>
#include <idtLoader.h>
#include <interrupts.h>
#include <video.h>
#include <terminal.h>
#include <keyboard.h>
#include <memory.h>
#include <time.h>
#include <audio.h>
#include <processes.h>

extern void diagnostic_test();
extern uint64_t get_rsp();

extern uint8_t text;
extern uint8_t rodata;
extern uint8_t data;
extern uint8_t bss;
extern uint8_t endOfKernelBinary;
extern uint8_t endOfKernel;
static void * endOfModules = 0;

uint32_t * ramAmount = (uint32_t *) 0x5020;

static const uint64_t PageSize = 0x1000;

// El heap arranca por encima de la memoria de video (backbuffer en 0x500000,
// hasta ~0x800000 para 1024x768x32) y del modulo shell + su .bss (0x400000+).
// Fijo para que el memory manager nunca se solape con esas regiones.
static const uint64_t HeapStart = 0x800000;

static void * const shell = (void*)0x400000;
typedef void (*EntryPoint)();

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

void clearBSS(void * bssAddress, uint64_t bssSize) {
	memset(bssAddress, 0, bssSize);
}

void * getStackBase() {
	return (void*)((uint64_t)&endOfKernel + PageSize * 8 - sizeof(uint64_t));
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
		shell
	};

	void * loadedEnd = 0;
	loadModules(&endOfKernelBinary, moduleAddresses, &loadedEnd);

	ncPrint("[Done]");
	ncNewline();
	ncNewline();

	ncPrint("[Initializing kernel's binary]");
	ncNewline();

	clearBSS(&bss, &endOfKernel - &bss);

	endOfModules = (loadedEnd != 0) ? loadedEnd : &endOfKernel;

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

void sleep(uint64_t ms) {
    uint64_t start_time = getMilisFromBoot();
    uint64_t end_time = start_time + ms;
    while (getMilisFromBoot() < end_time);
}

void MusicSO(){
	clearAudioBuffer();
	play_sound(784, 180);
	play_sound(988, 150);
	play_sound(1175, 240);
}

static void bootAnimation(void) {
	const uint32_t bg_color = 0x050518;
	const uint32_t circle_outer = 0x103054;
	const uint32_t circle_body = 0x1D58A1;
	const uint32_t circle_core = 0x091E36;
	const uint32_t circle_glow = 0x2F84E0;
	const uint32_t text_color = 0xF4F8FF;

	const uint16_t width = getScreenWidth();
	const uint16_t height = getScreenHeight();
	const int center_x = width / 2;
	const int center_y = height / 2;
	const int min_dimension = width < height ? width : height;

	int base_radius = min_dimension / 6;
	if (base_radius < 52) {
		base_radius = 52;
	}

	const int circle_center_y = center_y - base_radius / 3;

	fillScreen(bg_color);

	drawFilledCircle((uint64_t)center_x, (uint64_t)circle_center_y,
		(uint16_t)(base_radius + base_radius / 4), circle_outer);
	drawFilledCircle((uint64_t)center_x, (uint64_t)circle_center_y,
		(uint16_t)(base_radius + base_radius / 10), circle_body);

	drawFilledCircle((uint64_t)center_x, (uint64_t)circle_center_y,
		(uint16_t)(base_radius * 3 / 4), circle_glow);
	drawFilledCircle((uint64_t)center_x, (uint64_t)circle_center_y,
		(uint16_t)(base_radius / 2), circle_body);
	drawFilledCircle((uint64_t)center_x, (uint64_t)circle_center_y,
		(uint16_t)(base_radius / 3), circle_core);

	const char * label = "TobaOS";
	uint16_t text_height = (uint16_t)((base_radius * 4) / 5);
	if (text_height < 36) {
		text_height = 36;
	}
	uint16_t char_width = (uint16_t)((8u * text_height) / 16u);
	uint64_t text_width = strLength(label) * char_width;
	int text_x = center_x - (int)(text_width / 2);
	int text_y = circle_center_y - (int)(text_height / 2);
	drawText((uint64_t)text_x, (uint64_t)text_y, label, text_height, text_color);

	swapBuffers();
	sleep(120);
	MusicSO();
	sleep(280);
	textMode();
}

int main() {	
	load_idt();
    ncSetStyle(0x0F);
	start_T();
	keyboard_set_enabled(false);
	
	canvasMode();
	calibrateMilis();
	bootAnimation();

	keyboard_set_enabled(true);
	clearTextBuffer();

	if ((uint64_t) endOfKernel >= (uint64_t) shell) {
		print("Kernel demasiado grande");
		return 1;
	}

	if (endOfModules != 0) {
		if ((uint64_t) endOfModules > HeapStart) {
			print("Modulos demasiado grandes: se solapan con el heap");
			return 1;
		}
		initializeMemoryManager((void *) HeapStart, (*ramAmount) * 1024 * 1024 - HeapStart);
	} else {
		print("Error al inicializar memory manager");
		return 1;
	}

	initialize_terminal();

	const char ** args = { NULL };
	pid_t shell_pid = create_process("Shell", ((EntryPoint) shell), args);
	set_foreground_pid(shell_pid);

	// Capturamos el tope del kernel stack para idle ANTES de habilitar
	// interrupciones: apenas hacemos _sti() el primer timer tick (ya pendiente)
	// switchea a la shell y abandona este stack sin volver aca, asi que la
	// captura tiene que pasar si o si con las interrupciones deshabilitadas.
	kernel_rsp = get_rsp();

	_sti();

	// Cede al idle sobre el kernel stack. El primer timer tick switchea a la
	// shell; el scheduler vuelve aca (idle) cada vez que no haya proceso listo.
	idle();

    return 0;
}