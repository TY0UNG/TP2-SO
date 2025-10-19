#include <video.h>
#include <font8x16.h>

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

static void printBuffer();

bool text_mode_enabled = true;

static void putPixel(uint32_t hexColor, uint64_t x, uint64_t y, bool directWrite) {
    uint8_t * framebuffer = directWrite ? (uint8_t *) VBE_mode_info->framebuffer : backbuffer;
    uint64_t offset = (x * ((VBE_mode_info->bpp)/8)) + (y * VBE_mode_info->pitch);
    framebuffer[offset]     =  (hexColor) & 0xFF;
    framebuffer[offset+1]   =  (hexColor >> 8) & 0xFF; 
    framebuffer[offset+2]   =  (hexColor >> 16) & 0xFF;
}

void swapBuffers() {
    vsync_wait();
    memcpy(VBE_mode_info->framebuffer, backbuffer, (uint64_t)VBE_mode_info->pitch * VBE_mode_info->height);
}

void canvasMode() {
	text_mode_enabled = false;
    clearCanvas();
}

void clearCanvas() {
    memset(backbuffer, 0, (uint64_t)VBE_mode_info->pitch * VBE_mode_info->height);
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

void drawRectangle(uint64_t x1, uint64_t y1, uint64_t x2, uint64_t y2, uint16_t thickness, uint32_t color) {
	drawLine(x1, y1, x2, y1, thickness, color);
    drawLine(x2, y1, x2, y2, thickness, color);
    drawLine(x1, y2, x2, y2, thickness, color);
    drawLine(x1, y1, x1, y2, thickness, color);
}

void drawFilledRectangle(uint64_t x1, uint64_t y1, uint64_t x2, uint64_t y2, uint32_t color) {
	for (uint64_t y = y1; y <= y2; y++) {
        for (uint64_t x = x1; x <= x2; x++) {
            putPixel(color, x, y, false);
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
    uint16_t char_width = 8;
    uint16_t char_height = 16;
    
    float scale_factor = (float)height / (float)char_height;
    
    if (scale_factor < 1.0f) {
        scale_factor = 1.0f;
    }

    while (*text) {
        unsigned char character = *text;
        const uint8_t* char_bitmap = font8x16[character];
        for (uint16_t row_out = 0; row_out < height; row_out++) {
            for (uint16_t col_out = 0; col_out < (uint16_t)(char_width * scale_factor); col_out++) {
                float y_src = (float)row_out / scale_factor;
                float x_src = (float)col_out / scale_factor;
                
                uint16_t y_int = (uint16_t)y_src;
                uint16_t x_int = (uint16_t)x_src;

                if (y_int >= char_height || x_int >= char_width) {
                    continue;
                }

                unsigned char mask = 0x80 >> x_int;
                if (mask & char_bitmap[y_int]) {
                    putPixel(color, current_x + col_out, current_y + row_out, true);
                }
            }
        }
        
        current_x += (uint16_t)(char_width * scale_factor);
        text++;
    }
}

unsigned char text_buffer[4096];
int cursor = 0;
char selected_style = 0x0F;
int offset = 0;

void textMode() {
	text_mode_enabled = true;
	clearCanvas();
	printBuffer(offset, cursor);
}

void selectStyle(char style) {
	selected_style = style;
}

void clearTextBuffer() {
	for (int i = 0; i < cursor; i++) {
		text_buffer[i] = 0;
	}
	cursor = 0;
	offset = 0;
	clearCanvas();
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

void printBuffer(int from, int to) {
	int width = VBE_mode_info->width / 8;
	int height = VBE_mode_info->height / 16;
	int line = 0;
	int ch = 0;
	int i = offset;
	while(line < height && ch < width && i < to) {
		if (text_buffer[i] == '\n') {
			ch = 0;
			line++;
		} else {
			if (i >= from) {
				for (int x = 0; x < 8; x++) {
					for (int y = 0; y < 16; y++) {
						unsigned char mask = 0x80 >> x;
						if (mask & font8x16[text_buffer[i]][y])
							putPixel(getHexColor(text_buffer[i+1]), ch*8+x, line*16+y, true);
						else
							putPixel(getHexColor(text_buffer[i+1] >> 4), ch*8+x, line*16+y, true);
					}
				}
			}
			if (++ch >= width) {
				ch = 0;
				line++;
			}
		}
		i+=2;
	}
}

void print(const char* text) {
	int initial = cursor;
	while (*text != 0) {
		text_buffer[cursor++] = *text;
		text_buffer[cursor++] = selected_style;
		text++;
	}
	if (text_mode_enabled) printBuffer(initial, cursor);
}

void printChar(char c) {
	text_buffer[cursor++] = c;
	text_buffer[cursor++] = selected_style;
	if (text_mode_enabled) printBuffer(cursor - 2, cursor);
}

void deleteChar() {
	if (cursor > 1) {
		text_buffer[cursor--] = 0;
		text_buffer[cursor--] = 0;
	}
	if (text_mode_enabled) printBuffer(cursor, cursor + 2);
}

void scrollDown() {
	if (!text_mode_enabled) return;
	while (*(text_buffer+offset) != '\n') offset++;
	offset++;
	printBuffer(offset, cursor);
}

void scrollUp() {
	if (!text_mode_enabled) return;
	if (offset < 1) return;
	offset-=2;
	while (*(text_buffer+offset) != '\n') offset--;
	offset++;
	printBuffer(offset, cursor);
}