#include <stdint.h>
#include <lib.h>
#include <moduleLoader.h>
#include <screen.h>

static void loadModule(uint8_t ** module, void * targetModuleAddress);
static uint32_t readUint32(uint8_t ** address);

void loadModules(void * payloadStart, void ** moduleTargetAddress) {
    ncPrint("[ML] start=0x");
    ncPrintHex((uint64_t)payloadStart);
    ncNewline();

	uint8_t * currentModule = (uint8_t*)payloadStart;
    int moduleCount = readUint32(&currentModule);
    ncPrint("[ML] count=");
    ncPrintDec(moduleCount);
    ncNewline();

    for(int i = 0; i < moduleCount; i++)
        loadModule(&currentModule, moduleTargetAddress[i]);
}

static void loadModule(uint8_t ** module, void * targetModuleAddress)
{
    static int idx = 0;
    uint32_t moduleSize = readUint32(module);

    ncPrint("  [Module ");
    ncPrintDec(idx);
    ncPrint("] src=0x");
    ncPrintHex((uint64_t)*module);
    ncPrint(" dst=0x");
    ncPrintHex((uint64_t)targetModuleAddress);
    ncPrint(" size=");
    ncPrintDec(moduleSize);
    ncNewline();

    memcpy(targetModuleAddress, *module, moduleSize);
    *module += moduleSize;
    idx++;
}

static uint32_t readUint32(uint8_t ** address)
{
	uint32_t result = *(uint32_t*)(*address);
	*address += sizeof(uint32_t);
	return result;
}
