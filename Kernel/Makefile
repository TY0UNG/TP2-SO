include Makefile.inc

MM ?= NAIVE

ifeq ($(MM), NAIVE)
    # Define MM_NAIVE para el preprocesador
    DEFINES = -DMM_NAIVE
else ifeq ($(MM), OWN)
    # Define MM_OWN para el preprocesador
    DEFINES = -DMM_OWN
else ifeq ($(MM), BUDDY)
    # Define MM_BUDDY para el preprocesador
    DEFINES = -DMM_BUDDY
else
    $(error Opción de MM no válida. Usa MM=NAIVE o MM=OWN o MM=BUDDY)
endif

KERNEL=kernel.bin
SOURCES := $(wildcard *.c) $(wildcard **/*.c)
SOURCES_ASM=$(wildcard asm/*.asm)
OBJECTS=$(SOURCES:.c=.o)
OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)
LOADERSRC=loader.asm

LOADEROBJECT=$(LOADERSRC:.asm=.o)
STATICLIBS=

all: $(KERNEL)

$(KERNEL): $(LOADEROBJECT) $(OBJECTS) $(STATICLIBS) $(OBJECTS_ASM)
	$(LD) $(LDFLAGS) -T kernel.ld -Map=kernel.map -o $(KERNEL) $(LOADEROBJECT) $(OBJECTS) $(OBJECTS_ASM) $(STATICLIBS)

%.o: %.c
	$(GCC) $(GCCFLAGS) $(DEFINES) -I./include -I./drivers -I./resources/fonts -c $< -o $@
	
%.o : %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

$(LOADEROBJECT):
	$(ASM) $(ASMFLAGS) $(LOADERSRC) -o $(LOADEROBJECT)

clean:
	rm -rf asm/*.o **/*.o *.bin

.PHONY: all clean
