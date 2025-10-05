#include <stdint.h>
#include <string.h>
#include <lib.h>
#include <moduleLoader.h>
#include <screen.h>
#include <idtLoader.h>
#include <interrupts.h>

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

#include <keyboard.h>									////

int main()
{	
    ncSetStyle(0x2F);

    ncPrint("[Kernel init]");
    ncNewline();

    load_idt();              // IDT
    ncPrint("IDT loaded");
    ncNewline();
	
    _sti();                   // habilita interrupciones (usa inline asm o función si la tienes)
    ncPrint("Interrupts ON");
    ncNewline();

    ncPrint("Jumping to shell at 0x");
    ncPrintHex((uint64_t)shell);
    ncNewline();

	// veo si funcona el keyboard 
/*
	while(1){											///     funcion bien el drive, es tema de la sys 
		if (!isKeyBufferEmpty()) {
            KeyEvent event = getNextKey();
            
            // Muestra scancode
            ncPrint("Scancode: 0x");
            ncPrintHex(event.scancode);
            
            // Muestra ASCII
            ncPrint(" | ASCII: ");
            if (event.ascii >= 32 && event.ascii <= 126) {
                ncPrint("'");
                ncPrintChar(event.ascii);
                ncPrint("' (");
                ncPrintDec(event.ascii);
                ncPrint(")");
            } else {
                ncPrintDec(event.ascii);
            }
            
            // Muestra si es press o release
            ncPrint(" | ");
            if (event.is_release) {
                ncPrint("RELEASE");
            } else {
                ncPrint("PRESS");
            }
            
            ncNewline();
            
            // Eco del carácter si es imprimible y PRESS
            if (!event.is_release && event.ascii != 0) {
                ncPrint("  -> Eco: ");
                ncPrintChar(event.ascii);
                ncNewline();
            }
        }

	}
	*/

    ((EntryPoint)shell)();

    ncPrint("Returned from shell (unexpected)");
    ncNewline();
    return 0;
}
