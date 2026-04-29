#include <video.h>
#include <font8x16.h>
#include <stdint.h>
#include "time.h"

struct vbe_mode_info_structure {
	uint16_t attributes;		// deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
	uint8_t window_a;			// deprecated
	uint8_t window_b;			// deprecated
	uint16_t granularity;		// deprecated; used while calculating bank numbers
	uint16_t window_size;
	uint16_t segment_a;
	uint16_t segment_b;
	uint32_t win_func_ptr;		// deprecated; used to switch banks from protected mode without returning to real mode
	uint16_t pitch;			// number of bytes per horizontal line
	uint16_t width;			// width in pixels
	uint16_t height;			// height in pixels
	uint8_t w_char;			// unused...
	uint8_t y_char;			// ...
	uint8_t planes;
	uint8_t bpp;			// bits per pixel in this mode
	uint8_t banks;			// deprecated; total number of banks in this mode
	uint8_t memory_model;
	uint8_t bank_size;		// deprecated; size of a bank, almost always 64 KB but may be 16 KB...
	uint8_t image_pages;
	uint8_t reserved0;
 
	uint8_t red_mask;
	uint8_t red_position;
	uint8_t green_mask;
	uint8_t green_position;
	uint8_t blue_mask;
	uint8_t blue_position;
	uint8_t reserved_mask;
	uint8_t reserved_position;
	uint8_t direct_color_attributes;
 
	uint32_t framebuffer;		// physical address of the linear frame buffer; write here to draw to the screen
	uint32_t off_screen_mem_off;
	uint16_t off_screen_mem_size;	// size of memory in the framebuffer but not being displayed on the screen
	uint8_t reserved1[206];
} __attribute__ ((packed));

typedef struct vbe_mode_info_structure * VBEInfoPtr;

VBEInfoPtr VBE_mode_info = (VBEInfoPtr) 0x0000000000005C00;

uint8_t * backbuffer = (void*)0x500000;

extern void vsync_wait();

static void printBuffer(int from, int to, bool fullRedraw);
static void checkBufferOverflow();
static int checkAndAutoScroll(bool redraw_all);
static int calculateTailOffset(void);
static void updateFpsOverlay(void);
static void drawFpsOverlay(void);

bool text_mode_enabled = true;

static uint16_t text_size = 16;  // tamaño de texto default

static bool fps_overlay_enabled = false;
static uint64_t fps_last_measure_ms = 0;
static uint32_t fps_accumulated_frames = 0;
static uint32_t fps_last_value = 0;
static char fps_text_buffer[32] = "FPS: 0";

static uint32_t getHexColor(char style);

static inline uint16_t scaled_char_width(uint16_t height) {
    uint32_t width = 8u * height;
    width /= 16u;
    if (width == 0) {
        width = 1;
    }
    return (uint16_t)width;
}

static inline uint16_t chars_per_line_for(uint16_t height) {
    uint16_t width = scaled_char_width(height);
    uint16_t per_line = width ? (uint16_t)(VBE_mode_info->width / width) : VBE_mode_info->width;
    if (per_line == 0) {
        per_line = 1;
    }
    return per_line;
}

static inline uint16_t max_visual_lines_for(uint16_t height) {
    if (height == 0) {
        return 1;
    }
    uint16_t lines = (uint16_t)(VBE_mode_info->height / height);
    if (lines == 0) {
        lines = 1;
    }
    return lines;
}

static void putPixel(uint32_t hexColor, uint64_t x, uint64_t y, bool directWrite) {
    uint8_t * framebuffer = directWrite ? (uint8_t *)(uintptr_t)VBE_mode_info->framebuffer : backbuffer;
    uint64_t offset = (x * ((VBE_mode_info->bpp)/8)) + (y * VBE_mode_info->pitch);
    framebuffer[offset]     =  (hexColor) & 0xFF;
    framebuffer[offset+1]   =  (hexColor >> 8) & 0xFF; 
    framebuffer[offset+2]   =  (hexColor >> 16) & 0xFF;
}

extern void *fast_memcpy(void *dest, const void *src, uint64_t n);
extern void *fast_memset(void *s, int c, uint64_t n);

void setFpsOverlayEnabled(bool enabled) {
    fps_overlay_enabled = enabled;
    fps_accumulated_frames = 0;
    fps_last_measure_ms = 0;
    fps_last_value = 0;
    uintToBase(0, fps_text_buffer + 5, 10);
}

// Actualiza pantalla mostrando lo que se dibujo previamente en el backbuffer
void swapBuffers() {
    if (fps_overlay_enabled && !text_mode_enabled) {
        updateFpsOverlay();
        drawFpsOverlay();
    }
    fast_memcpy((void*)(uintptr_t)VBE_mode_info->framebuffer, backbuffer, (uint64_t)VBE_mode_info->pitch * VBE_mode_info->height);
}

void canvasMode() {
	text_mode_enabled = false;
    clearCanvas();
}

void clearCanvas() {
    fast_memset(backbuffer, 0, (uint64_t)VBE_mode_info->pitch * VBE_mode_info->height);
}

void drawPixel(uint64_t x, uint64_t y, uint32_t color) {
	putPixel(color, x, y, false);
}

static int abs(int number) {
	return number < 0 ? -number : number;
}

void drawLine(uint64_t x1, uint64_t y1, uint64_t x2, uint64_t y2, uint16_t thickness, uint32_t color) {
	int dx = abs((int)x2 - (int)x1);
    int dy = abs((int)y2 - (int)y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        for (int i = 0; i < thickness; i++) {
            for (int j = 0; j < thickness; j++) {
                putPixel(color, x1 + i, y1 + j, false);
            }
        }
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void fillScreen(uint32_t hexColor) {
    uint8_t* fb = backbuffer; // O el framebuffer directo
    uint64_t width = VBE_mode_info->width;
    uint64_t height = VBE_mode_info->height;
    uint64_t pitch = VBE_mode_info->pitch; // Ancho en bytes de una línea
    uint64_t bpp_bytes = VBE_mode_info->bpp / 8; // Debería ser 3
    
    uint8_t* first_row = fb;
    uint8_t b = (hexColor) & 0xFF;
    uint8_t g = (hexColor >> 8) & 0xFF;
    uint8_t r = (hexColor >> 16) & 0xFF;

    for (uint64_t x = 0; x < width; x++) {
        uint64_t offset = x * bpp_bytes; // offset = x * 3
        first_row[offset]   = b;
        first_row[offset+1] = g;
        first_row[offset+2] = r;
    }

    // --- Paso 2: Copiar esa primera fila al resto de la pantalla ---
    
    // La longitud de datos a copiar por fila
    uint64_t row_data_size_bytes = width * bpp_bytes; 
    
    // Apuntador a la fuente (siempre será la primera fila)
    const void* src_row = (const void*)first_row;

    for (uint64_t y = 1; y < height; y++) {
        // Calcula el apuntador de destino para la fila actual
        void* dest_row = (void*)(fb + (y * pitch));
        
        // ¡Usa tu función rápida!
        fast_memcpy(dest_row, src_row, row_data_size_bytes);
    }
}

void drawRectangle(uint64_t x1, uint64_t y1, uint64_t x2, uint64_t y2, uint16_t thickness, uint32_t color) {
	drawLine(x1, y1, x2, y1, thickness, color);
    drawLine(x2, y1, x2, y2, thickness, color);
    drawLine(x1, y2, x2, y2, thickness, color);
    drawLine(x1, y1, x1, y2, thickness, color);
}

void drawFilledRectangle(uint64_t x1, uint64_t y1, uint64_t x2, uint64_t y2, uint32_t color, bool directWrite) {
	for (uint64_t y = y1; y <= y2; y++) {
        for (uint64_t x = x1; x <= x2; x++) {
            putPixel(color, x, y, directWrite);
        }
    }
}

static void drawCirclePoints(uint64_t x_center, uint64_t y_center, int x, int y, uint16_t thickness, uint32_t color) {
    for (int i = 0; i < thickness; i++) {
        for (int j = 0; j < thickness; j++) {
            putPixel(color, x_center + x + i, y_center + y + j, false);
            putPixel(color, x_center - x + i, y_center + y + j, false);
            putPixel(color, x_center + x + i, y_center - y + j, false);
            putPixel(color, x_center - x + i, y_center - y + j, false);
            putPixel(color, x_center + y + i, y_center + x + j, false);
            putPixel(color, x_center - y + i, y_center + x + j, false);
            putPixel(color, x_center + y + i, y_center - x + j, false);
            putPixel(color, x_center - y + i, y_center - x + j, false);
        }
    }
}

void drawCircle(uint64_t x_center, uint64_t y_center, uint16_t radius, uint16_t thickness, uint32_t color) {
    int x = radius;
    int y = 0;
    int p = 1 - radius;

    drawCirclePoints(x_center, y_center, x, y, thickness, color);

    while (x > y) {
        y++;
        if (p <= 0) {
            p = p + 2 * y + 1;
        } else {
            x--;
            p = p + 2 * y - 2 * x + 1;
        }
        drawCirclePoints(x_center, y_center, x, y, thickness, color);
    }
}

void drawFilledCircle(uint64_t x_center, uint64_t y_center, uint16_t radius, uint32_t color) {
	int r2 = radius * radius;
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= r2) {
                putPixel(color, x_center + x, y_center + y, false);
            }
        }
    }
}

void drawText(uint64_t x, uint64_t y, const char* text, uint16_t height, uint32_t color) {
	if (text == 0) {
        return;
    }

    uint64_t current_x = x;
    uint64_t current_y = y;
    const uint16_t char_width = 8;
    const uint16_t char_height = 16;

    if (height == 0) {
        return;
    }

    if (height < char_height) {
        height = char_height;
    }

    uint16_t scaled_width = scaled_char_width(height);

    while (*text) {
        unsigned char character = *text;
        const uint8_t* char_bitmap = font8x16[character];
        for (uint16_t row_out = 0; row_out < height; row_out++) {
            uint16_t y_int = (row_out * char_height) / height;
            if (y_int >= char_height) {
                continue;
            }
            unsigned char glyph_row = char_bitmap[y_int];
            for (uint16_t col_out = 0; col_out < scaled_width; col_out++) {
                uint16_t x_int = (col_out * char_width) / scaled_width;
                if (x_int >= char_width) {
                    continue;
                }

                unsigned char mask = 0x80u >> x_int;
                if (mask & glyph_row) {
                    putPixel(color, current_x + col_out, current_y + row_out, false);
                }
            }
        }
        
        current_x += scaled_width;
        text++;
    }
}

static void updateFpsOverlay(void) {
    uint64_t now = getMilisFromBoot();

    if (fps_last_measure_ms == 0) {
        fps_last_measure_ms = now;
    }

    fps_accumulated_frames++;

    uint64_t elapsed = now - fps_last_measure_ms;
    if (elapsed >= 1000) {
        if (elapsed == 0) {
            elapsed = 1;
        }

        fps_last_value = (uint32_t)((fps_accumulated_frames * 1000ULL) / elapsed);
        fps_accumulated_frames = 0;
        fps_last_measure_ms = now;

        uintToBase(fps_last_value, fps_text_buffer + 5, 10);
    }
}

static void drawFpsOverlay(void) {
    const uint64_t overlay_x1 = 16;
    const uint64_t overlay_y1 = 16;
    const uint64_t overlay_x2 = overlay_x1 + 160;
    const uint64_t overlay_y2 = overlay_y1 + 40;
    const uint16_t overlay_text_height = 24;

    drawFilledRectangle(overlay_x1, overlay_y1, overlay_x2, overlay_y2, 0x00202020, false);
    drawText(overlay_x1 + 8, overlay_y1 + 6, fps_text_buffer, overlay_text_height, 0x00FFFFFF);
}

uint16_t getScreenWidth() {
    return VBE_mode_info->width;
}

uint16_t getScreenHeight() {
    return VBE_mode_info->height;
}

#define TEXT_BUFFER_SIZE 32768  // 32KB total

// Calcular cuántas líneas caben en pantalla con tamaño mínimo
#define MIN_TEXT_SIZE 12
#define MAX_TEXT_SIZE 64
#define MAX_SCREEN_LINES (768 / MIN_TEXT_SIZE)  // = 64 líneas

// Queremos recordar ~1.5 pantallas
#define LINES_TO_REMEMBER (MAX_SCREEN_LINES * 3 / 2)  // = 96 líneas

unsigned char text_buffer[TEXT_BUFFER_SIZE];
int cursor = 0;
char selected_style = 0x0F;
int offset = 0;

void textMode() {
	text_mode_enabled = true;
	clearCanvas();
    swapBuffers();
    checkAndAutoScroll(true);
	printBuffer(offset, cursor, true);
}

void selectStyle(char style) {
	selected_style = style;
}

void setTextSize(uint16_t height) {
    if (height < MIN_TEXT_SIZE || height > MAX_TEXT_SIZE) return;
    uint16_t old_size = text_size;
    text_size = height;
    if (old_size != text_size) checkAndAutoScroll(true);
    printBuffer(offset, cursor, true);
}

void clearTextBuffer() {
	for (int i = 0; i < cursor; i++) {
		text_buffer[i] = 0;
	}
	cursor = 0;
	offset = 0;
	clearCanvas();
    swapBuffers();  //lo actualiza para que se vea el cambio
}

static int calculateTailOffset(void) {
    if (cursor <= 0) {
        return 0;
    }

    uint16_t max_lines = max_visual_lines_for(text_size);
    uint16_t chars_per_line = chars_per_line_for(text_size);
    if (chars_per_line == 0) {
        chars_per_line = 1;
    }

    int capacity = max_lines + 2;
    if (capacity > (MAX_SCREEN_LINES + 2)) {
        capacity = MAX_SCREEN_LINES + 2;
    }

    int line_starts[MAX_SCREEN_LINES + 2];
    int count = 1;
    line_starts[0] = 0;
    int current_line_start = 0;
    int col = 0;

    for (int i = 0; i < cursor; i += 2) {
        char ch = text_buffer[i];
        bool start_new_line = false;

        if (ch == '\n') {
            current_line_start = i + 2;
            col = 0;
            start_new_line = true;
        } else if (ch >= 32 && ch <= 126) {
            col++;
            if (col >= chars_per_line) {
                current_line_start = i + 2;
                col = 0;
                start_new_line = true;
            }
        }

        if (start_new_line) {
            if (count == capacity) {
                for (int j = 0; j < capacity - 1; j++) {
                    line_starts[j] = line_starts[j + 1];
                }
                count = capacity - 1;
            }

            if (count < capacity) {
                line_starts[count++] = current_line_start;
            }
        }
    }

    if (count <= max_lines) {
        return 0;
    }

    int index = count - max_lines;
    if (index < 0) {
        index = 0;
    }
    if (index >= count) {
        index = count - 1;
    }

    return line_starts[index];
}


static uint32_t makeColor(uint8_t r, uint8_t g, uint8_t b) {
    return (r << VBE_mode_info->red_position) |
           (g << VBE_mode_info->green_position) |
           (b << VBE_mode_info->blue_position);
}

static uint32_t getHexColor(char style) {
	switch (style & 0x0F) {
        case 0x00: return makeColor(0x00, 0x00, 0x00);
        case 0x01: return makeColor(0x00, 0x00, 0xAA);
        case 0x02: return makeColor(0x00, 0xAA, 0x00);
        case 0x03: return makeColor(0x00, 0xAA, 0xAA);
        case 0x04: return makeColor(0xAA, 0x00, 0x00);
        case 0x05: return makeColor(0xAA, 0x00, 0xAA);
        case 0x06: return makeColor(0xAA, 0x55, 0x00);
        case 0x07: return makeColor(0xAA, 0xAA, 0xAA);
        case 0x08: return makeColor(0x55, 0x55, 0x55);
        case 0x09: return makeColor(0x55, 0x55, 0xFF);
        case 0x0A: return makeColor(0x55, 0xFF, 0x55);
        case 0x0B: return makeColor(0x55, 0xFF, 0xFF);
        case 0x0C: return makeColor(0xFF, 0x55, 0x55);
        case 0x0D: return makeColor(0xFF, 0x55, 0xFF);
        case 0x0E: return makeColor(0xFF, 0xFF, 0x55);
        case 0x0F: return makeColor(0xFF, 0xFF, 0xFF);
        default:   return makeColor(0x00, 0x00, 0x00);
    }
	return 0;
}

/*
 * printBuffer - Renderiza el buffer de texto en pantalla
 * @param from: índice inicial del rango a dibujar (ignorado si fullRedraw=true)
 * @param to: índice final del rango a dibujar
 * @param fullRedraw: true = limpia pantalla y redibuja todo desde offset /
 *              false = dibuja solo caracteres nuevos directamente al framebuffer (rápido)
 */
void printBuffer(int from, int to, bool fullRedraw) {
	if (fullRedraw) {
		clearCanvas();
	}
	
    const uint16_t char_width = 8;
    const uint16_t char_height = 16;
    uint16_t scaled_width = scaled_char_width(text_size);
    if (scaled_width == 0) {
        scaled_width = 1;
    }
    int chars_per_line = chars_per_line_for(text_size);
    if (chars_per_line <= 0) {
        chars_per_line = 1;
    }
	
	int start_index = fullRedraw ? offset : from;
	int current_x = 0;
	int current_y = 0;
	int col = 0;
	
	// Si es dibujado incremental, calcular posición (x,y) del carácter 'from'
	if (!fullRedraw) {
		for (int i = offset; i < from; i += 2) {
			char c = text_buffer[i];
			if (c == '\n') {
				current_x = 0;
				current_y += text_size;
				col = 0;
			} else if (c >= 32 && c <= 126) {
				current_x += scaled_width;
				col++;
				if (col >= chars_per_line) {
					current_x = 0;
					current_y += text_size;
					col = 0;
				}
			}
		}
	}
	
	bool directWrite = !fullRedraw;
	
	for (int i = start_index; i < to; i += 2) {
        if (current_y >= (int)VBE_mode_info->height) {
            break;
        }

        char c = text_buffer[i];
		char style = text_buffer[i + 1];
		
		if (c == '\n') {
			current_x = 0;
			current_y += text_size;
			col = 0;
            continue;
		} else if (c >= 32 && c <= 126) {
			uint32_t fg_color = getHexColor(style);
			uint32_t bg_color = getHexColor(style >> 4);
			
			const uint8_t* char_bitmap = font8x16[(unsigned char)c];
			
            for (uint16_t row_out = 0; row_out < text_size; row_out++) {
                uint16_t y_int = (row_out * char_height) / text_size;
                if (y_int >= char_height) {
                    continue;
                }
                unsigned char glyph_row = char_bitmap[y_int];
                for (uint16_t col_out = 0; col_out < scaled_width; col_out++) {
                    uint16_t x_int = (col_out * char_width) / scaled_width;
                    if (x_int >= char_width) {
                        continue;
                    }
                    unsigned char mask = 0x80u >> x_int;
                    if (mask & glyph_row) {
                        putPixel(fg_color, current_x + col_out, current_y + row_out, directWrite);
                    } else {
                        putPixel(bg_color, current_x + col_out, current_y + row_out, directWrite);
                    }
                }
			}
			
			current_x += scaled_width;
			col++;
			
			if (col >= chars_per_line) {
				current_x = 0;
				current_y += text_size;
				col = 0;
			}
		}
	}
	
	if (fullRedraw) {
		swapBuffers();
	}
}

static void addToBuffer(char c) {
    text_buffer[cursor++] = c;
    text_buffer[cursor++] = selected_style;
    checkBufferOverflow();
}

void print(const char* text) {
    int initial = cursor;
    while (*text != 0) {
        addToBuffer(*text);
        text++;
    }
    
    if (text_mode_enabled) {
        if (checkAndAutoScroll(true)) {
            printBuffer(offset, cursor, true);
        } else {
            printBuffer(initial, cursor, false);
        }
    }
}

void printChar(char c) {
    addToBuffer(c);
    if (text_mode_enabled) {
        printBuffer(cursor - 2, cursor, false);
        if (checkAndAutoScroll(false)) {
            printBuffer(offset, cursor, true);
        }
    }
}

void deleteChar() {
    if (cursor <= 1) return;
    // Reemplazar con espacio en blanco (no borrar del buffer todavía)
    text_buffer[cursor - 2] = ' ';
    printBuffer(cursor - 2, cursor, false);
    text_buffer[cursor--] = 0;
    text_buffer[cursor--] = 0;
}

void scrollDown() {
	if (!text_mode_enabled) return;
	while (*(text_buffer+offset) != '\n') offset++;
	offset++;
	printBuffer(offset, cursor, true);
}

void scrollUp() {
	if (!text_mode_enabled) return;
	if (offset < 1) return;
	offset-=2;
	while (*(text_buffer+offset) != '\n') offset--;
	offset++;
	printBuffer(offset, cursor, true);
}

void checkBufferOverflow() {
    if (cursor < TEXT_BUFFER_SIZE - 512) return;  // Hay espacio (~256 chars), no hacer nada
    
    // Buffer casi lleno, eliminar líneas viejas. Para calcular cuántas líneas lógicas tenemos
    int total_logical_lines = 0;
    for (int i = 0; i < cursor; i += 2) {
        if (text_buffer[i] == '\n') total_logical_lines++;
    }

    if (total_logical_lines > LINES_TO_REMEMBER) {
        int lines_to_delete = 20;
        int new_start = 0;
        int deleted = 0;
        
        for (int i = 0; i < cursor && deleted < lines_to_delete; i += 2) {
            if (text_buffer[i] == '\n') {
                deleted++;
                new_start = i + 2;
            }
        }
        // Mover contenido hacia adelante
        int bytes_to_move = cursor - new_start;
        for (int i = 0; i < bytes_to_move; i++) {
            text_buffer[i] = text_buffer[new_start + i];
        }
        cursor = bytes_to_move;
        if (offset >= new_start) offset -= new_start;
        else offset = 0;
    }
}

int checkAndAutoScroll(bool redraw_all) {
    int previous_offset = offset;
    int optimal_offset = calculateTailOffset();

    if (redraw_all) {
        offset = optimal_offset;
    } else if (optimal_offset > offset) {
        offset = optimal_offset;
    }

    return offset != previous_offset;
}

void printHex64(uint64_t value) {
    char hex[17];  // 16 dg + null 
    const char* digits = "0123456789ABCDEF";
    
    for (int i = 15; i >= 0; i--) {
        hex[15 - i] = digits[(value >> (i * 4)) & 0xF];
    }
    hex[16] = '\0';
    
    print("0x");
    print(hex);
}

void printHex32(uint32_t value) {
    char hex[9];
    const char* digits = "0123456789ABCDEF";
    
    for (int i = 7; i >= 0; i--) {
        hex[7 - i] = digits[(value >> (i * 4)) & 0xF];
    }
    hex[8] = '\0';
    
    print("0x");
    print(hex);
}

void printHex16(uint16_t value) {
    char hex[5];
    const char* digits = "0123456789ABCDEF";
    
    for (int i = 3; i >= 0; i--) {
        hex[3 - i] = digits[(value >> (i * 4)) & 0xF];
    }
    hex[4] = '\0';
    
    print("0x");
    print(hex);
}

void printDec(uint64_t number) {
    char buffer[21];
    int i = 0;
    if (number == 0) {
        printChar('0');
        return;
    }
    while (number > 0) {
        buffer[i++] = '0' + (number % 10);
        number /= 10;
    }
    while (i--) {
        printChar(buffer[i]);
    }
}
