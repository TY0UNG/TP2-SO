#include <stdint.h>
#include <lib.h>
#include <moduleLoader.h>
#include <video.h>
#include <stdlib.h>

static void * loadModule(uint8_t ** module, void * targetModuleAddress, int idx);
static uint32_t readUint32(uint8_t ** address);

void loadModules(void * payloadStart, void ** moduleTargetAddress, void ** lastLoadedEnd) {
    print("[ML] start=0x");
    printHex64((uint64_t)payloadStart);
    print("\n");

	uint8_t * currentModule = (uint8_t*)payloadStart;
    void * maxLoadedEnd = 0;
    int moduleCount = readUint32(&currentModule);
    print("[ML] count=");
    printDec((uint64_t)moduleCount);
    print("\n");
    print("[ML] payloadPtr=0x");
    printHex64((uint64_t)currentModule);
    print("\n");

    for(int i = 0; i < moduleCount; i++) {
        void * loadedEnd = loadModule(&currentModule, moduleTargetAddress[i], i);
        if (maxLoadedEnd == 0 || (uint64_t)loadedEnd > (uint64_t)maxLoadedEnd) {
            maxLoadedEnd = loadedEnd;
        }
    }

    print("[ML] maxLoadedEnd=0x");
    printHex64((uint64_t)maxLoadedEnd);
    print("\n");

    if (lastLoadedEnd != NULL) {
        *lastLoadedEnd = maxLoadedEnd;
    }
}

static void * loadModule(uint8_t ** module, void * targetModuleAddress, int idx)
{
    uint32_t moduleSize = readUint32(module);
    void * moduleEnd = (void *)((uint64_t)targetModuleAddress + moduleSize);

    print("  [Module ");
    printDec((uint64_t)idx);
    print("] src=0x");
    printHex64((uint64_t)*module);
    print(" dst=0x");
    printHex64((uint64_t)targetModuleAddress);
    print(" size=");
    printDec((uint64_t)moduleSize);
    print(" end=0x");
    printHex64((uint64_t)moduleEnd);
    print("\n");

    memcpy(targetModuleAddress, *module, moduleSize);
    *module += moduleSize;
    return moduleEnd;
}

static uint32_t readUint32(uint8_t ** address)
{
	uint32_t result = *(uint32_t*)(*address);
	*address += sizeof(uint32_t);

    print("[ML] readUint32= ");
    printDec((uint64_t)result);
    print(" at ptr=0x");
    printHex64((uint64_t)(*address - sizeof(uint32_t)));
    print("\n");
	return result;
}
