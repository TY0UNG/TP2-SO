include Makefile.inc

MM ?= NAIVE

ifeq ($(MM), NAIVE)
    DEFINES = -DMM_NAIVE
else ifeq ($(MM), OWN)
    DEFINES = -DMM_OWN
else ifeq ($(MM), BUDDY)
    DEFINES = -DMM_BUDDY
else
    $(error Opción de MM no válida. Usa MM=NAIVE o MM=OWN o MM=BUDDY)
endif

DIST_DIR := dist
KERNEL   := kernel.bin

INCLUDES := -I./include -I./resources/fonts

LOADER_SRC := loader.asm
LOADER_OBJ := $(DIST_DIR)/loader.o

# Descubre fuentes recursivamente, ignora dist/, el loader y archivos vacíos.
SOURCES_C   := $(shell find . -path ./$(DIST_DIR) -prune -o -type f -name '*.c' -not -empty -print)
SOURCES_ASM := $(shell find asm -type f -name '*.asm')

OBJECTS_C   := $(patsubst ./%.c,$(DIST_DIR)/%.o,$(SOURCES_C))
OBJECTS_ASM := $(patsubst %.asm,$(DIST_DIR)/%.o,$(SOURCES_ASM))

all: $(KERNEL)

$(KERNEL): $(LOADER_OBJ) $(OBJECTS_C) $(OBJECTS_ASM)
	$(LD) $(LDFLAGS) -T kernel.ld -Map=kernel.map -o $(KERNEL) $(LOADER_OBJ) $(OBJECTS_C) $(OBJECTS_ASM)

$(DIST_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(GCC) $(GCCFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(DIST_DIR)/%.o: %.asm
	@mkdir -p $(dir $@)
	$(ASM) $(ASMFLAGS) $< -o $@

clean:
	rm -rf $(DIST_DIR) *.bin *.map

.PHONY: all clean
