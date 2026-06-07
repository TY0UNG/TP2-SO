include ../Makefile.inc

MODULE=0000-shell.bin

# build/ de Userland (igual que el build/ de Kernel): los objetos espejan el
# arbol de fuentes y el .map quedan aca, fuera del arbol de fuentes.
BUILD_DIR := ../build

SOURCES_C  := $(wildcard [^_]*.c) \
			  $(wildcard commands/*.c) \
			  $(wildcard commands/benchmark/*.c) \
			  $(wildcard commands/tron/*.c) \
			  $(wildcard lib/*.c)
SOURCES_ASM:= $(wildcard [^_]*.asm) \
			   $(wildcard commands/*.asm) \
			   $(wildcard commands/tron/*.asm) \
			   $(wildcard lib/*.asm)

# Todos los objetos, espejando el arbol de fuentes dentro de build/.
OBJECTS_C   := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SOURCES_C))
OBJECTS_ASM := $(patsubst %.asm,$(BUILD_DIR)/%.o,$(SOURCES_ASM))
OBJECTS     := $(OBJECTS_C) $(OBJECTS_ASM)

MAP := $(BUILD_DIR)/shell.map

all: $(MODULE)

$(MODULE): $(OBJECTS)
	@mkdir -p $(BUILD_DIR)
	$(GCC) $(GCCFLAGS) -T shell.ld -Wl,-Map=$(MAP) ../_loader.c $(OBJECTS) -I./lib -I./commands -o ../$(MODULE)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(GCC) $(GCCFLAGS) -I./lib -I./commands -c $< -o $@

$(BUILD_DIR)/%.o: %.asm
	@mkdir -p $(dir $@)
	nasm -f elf64 $< -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean print
