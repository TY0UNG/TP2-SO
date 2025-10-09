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

static void printBuffer();

static void putPixel(uint32_t hexColor, uint64_t x, uint64_t y) {
    uint8_t * framebuffer = (uint8_t *) VBE_mode_info->framebuffer;
    uint64_t offset = (x * ((VBE_mode_info->bpp)/8)) + (y * VBE_mode_info->pitch);
    framebuffer[offset]     =  (hexColor) & 0xFF;
    framebuffer[offset+1]   =  (hexColor >> 8) & 0xFF; 
    framebuffer[offset+2]   =  (hexColor >> 16) & 0xFF;
}

void canvas_mode() {

}

void clear_canvas() {
	for (uint64_t i = 0; i < VBE_mode_info->width; i++) {
		for (uint64_t j = 0; j < VBE_mode_info->height; j++) {
			putPixel(0, i, j);
		}
	}
}

void draw_pixel(uint64_t x, uint64_t y, uint32_t color) {

}

void draw_line(uint64_t x1, uint64_t y1, uint64_t x2, uint64_t y2, uint32_t color) {

}

void draw_rectangle(uint64_t x, uint64_t y, uint16_t width, uint16_t height, uint32_t color) {

}

void draw_circle(uint64_t x, uint64_t y, uint16_t radius, uint32_t color) {

}

void draw_text(uint64_t x, uint64_t y, const char* text, uint32_t color) {

}

bool text_mode_enabled = true;
signed char text_buffer[4096];
int cursor = 0;
char selected_style = 0x0F;
int offset = 0;

void text_mode() {
	text_mode_enabled = true;
	printBuffer();
}

void select_style(char style) {
	selected_style = style;
}

void clear_text_buffer() {
	for (int i = 0; i < sizeof(text_buffer); i++) {
		text_buffer[i] = 0;
	}
	cursor = 0;
	offset = 0;
	clear_canvas();
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

static void printBuffer() {
	clear_canvas();
	int width = VBE_mode_info->width / 8;
	int height = VBE_mode_info->height / 16;
	int line = 0;
	int ch = 0;
	int i = 0;
	while(line < height && ch < width && i < cursor) {
		if (text_buffer[i] == '\n') {
			ch = 0;
			line++;
		} else {
			for (int x = 0; x < 8; x++) {
				for (int y = 0; y < 16; y++) {
					unsigned char mask = 0x80 >> x;
					if (mask & font8x16[text_buffer[offset+i]][y])
						putPixel(0x0, ch*8+x, line*16+y);
					else
						putPixel(0xFFFFFFFFF, ch*8+x, line*16+y);
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
	while (*text != 0) {
		text_buffer[cursor++] = *text;
		text_buffer[cursor++] = selected_style;
		text++;
	}
	if (text_mode_enabled) printBuffer();
}

void printChar(char c) {
	text_buffer[cursor++] = c;
	text_buffer[cursor++] = selected_style;
	if (text_mode_enabled) printBuffer();
}

void deleteChar() {
	if (cursor > 1) {
		text_buffer[cursor--] = 0;
		text_buffer[cursor--] = 0;
	}
	if (text_mode_enabled) printBuffer();
}
