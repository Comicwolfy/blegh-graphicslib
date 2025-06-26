#include <stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_BUFFER_ADDR 0xB8000

#define VGA_COLOR_BLACK         0
#define VGA_COLOR_BLUE          1
#define VGA_COLOR_GREEN         2
#define VGA_COLOR_CYAN          3
#define VGA_COLOR_RED           4
#define VGA_COLOR_MAGENTA       5
#define VGA_COLOR_BROWN         6
#define VGA_COLOR_LIGHT_GREY    7
#define VGA_COLOR_DARK_GREY     8
#define VGA_COLOR_LIGHT_BLUE    9
#define VGA_COLOR_LIGHT_GREEN   10
#define VGA_COLOR_LIGHT_CYAN    11
#define VGA_COLOR_LIGHT_RED     12
#define VGA_COLOR_LIGHT_MAGENTA 13
#define VGA_COLOR_LIGHT_BROWN   14
#define VGA_COLOR_WHITE         15

#define VGA_COLOR(fg, bg) (fg | bg << 4)

uint16_t* const VGA_BUFFER = (uint16_t*)VGA_BUFFER_ADDR;

static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;
static uint8_t default_color = VGA_COLOR(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

void vga_scroll(void);
void vga_update_cursor(void);
void vga_put_char(char c, uint8_t color);
void vga_clear_screen(void);
void vga_print(const char* str, uint8_t color);
void vga_print_line(const char* str, uint8_t color);
void vga_print_hex(uint32_t n, uint8_t color);
void vga_set_cursor(uint8_t x, uint8_t y);
void vga_get_cursor(uint8_t* x, uint8_t* y);
void vga_set_default_color(uint8_t color);
uint8_t vga_get_default_color(void);
void vga_put_char_at(char c, uint8_t color, uint8_t x, uint8_t y);
void vga_clear_line(uint8_t line);
void vga_print_int(int32_t n, uint8_t color);
void vga_print_uint(uint32_t n, uint8_t color);
void vga_printf(uint8_t color, const char* format, ...);
void vga_fill_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, char fill_char, uint8_t color);
void vga_draw_box(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color);

void vga_scroll(void) {
    if (cursor_y >= VGA_HEIGHT) {
        for (int y = 1; y < VGA_HEIGHT; y++) {
            for (int x = 0; x < VGA_WIDTH; x++) {
                VGA_BUFFER[(y - 1) * VGA_WIDTH + x] = VGA_BUFFER[y * VGA_WIDTH + x];
            }
        }
        for (int x = 0; x < VGA_WIDTH; x++) {
            VGA_BUFFER[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = (default_color << 8) | ' ';
        }
        cursor_y = VGA_HEIGHT - 1;
    }
}

void vga_update_cursor(void) {
    uint16_t pos = cursor_y * VGA_WIDTH + cursor_x;
    
    /*
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
    */
}

void vga_set_cursor(uint8_t x, uint8_t y) {
    if (x < VGA_WIDTH && y < VGA_HEIGHT) {
        cursor_x = x;
        cursor_y = y;
        vga_update_cursor();
    }
}

void vga_get_cursor(uint8_t* x, uint8_t* y) {
    if (x) *x = cursor_x;
    if (y) *y = cursor_y;
}

void vga_set_default_color(uint8_t color) {
    default_color = color;
}

uint8_t vga_get_default_color(void) {
    return default_color;
}

void vga_put_char_at(char c, uint8_t color, uint8_t x, uint8_t y) {
    if (x < VGA_WIDTH && y < VGA_HEIGHT) {
        VGA_BUFFER[y * VGA_WIDTH + x] = (color << 8) | c;
    }
}

void vga_clear_line(uint8_t line) {
    if (line < VGA_HEIGHT) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            VGA_BUFFER[line * VGA_WIDTH + x] = (default_color << 8) | ' ';
        }
    }
}

void vga_put_char(char c, uint8_t color) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        vga_scroll();
        vga_update_cursor();
        return;
    } 
    
    if (c == '\r') {
        cursor_x = 0;
        vga_update_cursor();
        return;
    }
    
    if (c == '\t') {
        uint8_t spaces = 4 - (cursor_x % 4);
        for (int i = 0; i < spaces; i++) {
            vga_put_char(' ', color);
        }
        return;
    }
    
    if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            VGA_BUFFER[cursor_y * VGA_WIDTH + cursor_x] = (color << 8) | ' ';
            vga_update_cursor();
        } else if (cursor_y > 0) {
            cursor_y--;
            cursor_x = VGA_WIDTH - 1;
            VGA_BUFFER[cursor_y * VGA_WIDTH + cursor_x] = (color << 8) | ' ';
            vga_update_cursor();
        }
        return;
    }
    
    if (c >= 32 && c <= 126) {
        VGA_BUFFER[cursor_y * VGA_WIDTH + cursor_x] = (color << 8) | c;
        cursor_x++;
        
        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
            vga_scroll();
        }
        vga_update_cursor();
    }
}

void vga_clear_screen(void) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        VGA_BUFFER[i] = (default_color << 8) | ' ';
    }
    cursor_x = 0;
    cursor_y = 0;
    vga_update_cursor();
}

void vga_print(const char* str, uint8_t color) {
    if (!str) return;
    while (*str) {
        vga_put_char(*str++, color);
    }
}

void vga_print_line(const char* str, uint8_t color) {
    vga_print(str, color);
    vga_put_char('\n', color);
}

void vga_print_int(int32_t n, uint8_t color) {
    if (n == 0) {
        vga_put_char('0', color);
        return;
    }
    
    if (n < 0) {
        vga_put_char('-', color);
        n = -n;
    }
    
    char buffer[12];
    int i = 0;
    
    while (n > 0) {
        buffer[i++] = '0' + (n % 10);
        n /= 10;
    }
    
    for (int j = i - 1; j >= 0; j--) {
        vga_put_char(buffer[j], color);
    }
}

void vga_print_uint(uint32_t n, uint8_t color) {
    if (n == 0) {
        vga_put_char('0', color);
        return;
    }
    
    char buffer[11];
    int i = 0;
    
    while (n > 0) {
        buffer[i++] = '0' + (n % 10);
        n /= 10;
    }
    
    for (int j = i - 1; j >= 0; j--) {
        vga_put_char(buffer[j], color);
    }
}

void vga_print_hex(uint32_t n, uint8_t color) {
    const char* hex_chars = "0123456789ABCDEF";
    vga_print("0x", color);
    
    for (int i = 28; i >= 0; i -= 4) {
        vga_put_char(hex_chars[(n >> i) & 0xF], color);
    }
}

void vga_fill_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, char fill_char, uint8_t color) {
    for (uint8_t row = y; row < y + height && row < VGA_HEIGHT; row++) {
        for (uint8_t col = x; col < x + width && col < VGA_WIDTH; col++) {
            vga_put_char_at(fill_char, color, col, row);
        }
    }
}

void vga_draw_box(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color) {
    if (width < 2 || height < 2) return;
    
    const char corner = '+';
    const char horizontal = '-';
    const char vertical = '|';
    
    vga_put_char_at(corner, color, x, y);
    for (uint8_t i = 1; i < width - 1; i++) {
        vga_put_char_at(horizontal, color, x + i, y);
    }
    vga_put_char_at(corner, color, x + width - 1, y);
    
    for (uint8_t i = 1; i < height - 1; i++) {
        vga_put_char_at(vertical, color, x, y + i);
        vga_put_char_at(vertical, color, x + width - 1, y + i);
    }
    
    vga_put_char_at(corner, color, x, y + height - 1);
    for (uint8_t i = 1; i < width - 1; i++) {
        vga_put_char_at(horizontal, color, x + i, y + height - 1);
    }
    vga_put_char_at(corner, color, x + width - 1, y + height - 1);
}

void vga_printf(uint8_t color, const char* format, ...) {
    vga_print(format, color);
}

void vga_print_default(const char* str) {
    vga_print(str, default_color);
}

void vga_print_line_default(const char* str) {
    vga_print_line(str, default_color);
}

void vga_put_char_default(char c) {
    vga_put_char(c, default_color);
}
