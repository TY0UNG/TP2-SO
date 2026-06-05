#include <stdint.h>
#include <lib.h>
#include <moduleLoader.h>
#include <video.h>
#include <screen.h>
#include <stdlib.h>

// IMPORTANTE: el payload de modulos lo deja el bootloader inmediatamente
// despues del binario del kernel (en endOfKernelBinary), y el .bss del kernel
// (donde vive text_buffer de la terminal) se reserva en esa misma zona. Por eso
// loadModules NO debe escribir en la terminal (print -> text_buffer en .bss):
// eso pisaria el payload que todavia se esta copiando, corrompiendo los modulos.
// Se usa la consola naive (ncPrint -> 0xB8000), que no toca el .bss.

static void * loadModule(uint8_t ** module, void * targetModuleAddress, int idx);
static uint32_t readUint32(uint8_t ** address);

void loadModules(void * payloadStart, void ** moduleTargetAddress, void ** lastLoadedEnd) {
    ncPrint("[ML] start=0x");
    ncPrintHex((uint64_t)payloadStart);
    ncNewline();

	uint8_t * currentModule = (uint8_t*)payloadStart;
    void * maxLoadedEnd = 0;
    int moduleCount = readUint32(&currentModule);
    ncPrint("[ML] count=");
    ncPrintDec((uint64_t)moduleCount);
    ncNewline();
    ncPrint("[ML] payloadPtr=0x");
    ncPrintHex((uint64_t)currentModule);
    ncNewline();

    for(int i = 0; i < moduleCount; i++) {
        void * loadedEnd = loadModule(&currentModule, moduleTargetAddress[i], i);
        if (maxLoadedEnd == 0 || (uint64_t)loadedEnd > (uint64_t)maxLoadedEnd) {
            maxLoadedEnd = loadedEnd;
        }
    }

    ncPrint("[ML] maxLoadedEnd=0x");
    ncPrintHex((uint64_t)maxLoadedEnd);
    ncNewline();

    if (lastLoadedEnd != NULL) {
        *lastLoadedEnd = maxLoadedEnd;
    }
}

static void * loadModule(uint8_t ** module, void * targetModuleAddress, int idx)
{
    uint32_t moduleSize = readUint32(module);
    void * moduleEnd = (void *)((uint64_t)targetModuleAddress + moduleSize);

    ncPrint("  [Module ");
    ncPrintDec((uint64_t)idx);
    ncPrint("] src=0x");
    ncPrintHex((uint64_t)*module);
    ncPrint(" dst=0x");
    ncPrintHex((uint64_t)targetModuleAddress);
    ncPrint(" size=");
    ncPrintDec((uint64_t)moduleSize);
    ncPrint(" end=0x");
    ncPrintHex((uint64_t)moduleEnd);
    ncNewline();

    memcpy(targetModuleAddress, *module, moduleSize);
    *module += moduleSize;
    return moduleEnd;
}

static uint32_t readUint32(uint8_t ** address)
{
	uint32_t result = *(uint32_t*)(*address);
	*address += sizeof(uint32_t);

    ncPrint("[ML] readUint32= ");
    ncPrintDec((uint64_t)result);
    ncPrint(" at ptr=0x");
    ncPrintHex((uint64_t)(*address - sizeof(uint32_t)));
    ncNewline();
	return result;
}
