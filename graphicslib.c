#include <stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_BUFFER_ADDR 0xB8000

uint16_t* const VGA_BUFFER = (uint16_t*)VGA_BUFFER_ADDR;

uint8_t cursor_x = 0;
uint8_t cursor_y = 0;

// move cursor to next line & scroll if needed
void vga_scroll() {
    if (cursor_y >= VGA_HEIGHT) {
        // shift all lines up by 1
        for (int y = 1; y < VGA_HEIGHT; y++) {
            for (int x = 0; x < VGA_WIDTH; x++) {
                VGA_BUFFER[(y - 1) * VGA_WIDTH + x] = VGA_BUFFER[y * VGA_WIDTH + x];
            }
        }
        // clear last line
        for (int x = 0; x < VGA_WIDTH; x++) {
            VGA_BUFFER[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = (0x07 << 8) | ' ';
        }
        cursor_y = VGA_HEIGHT - 1;
    }
}

void vga_update_cursor() {
    // hardware cursor update - optional, needs outb to VGA ports, skip for now
}

void vga_put_char(char c, uint8_t color) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        vga_scroll();
        return;
    } 
    if (c == '\b') { // backspace
        if (cursor_x > 0) {
            cursor_x--;
            VGA_BUFFER[cursor_y * VGA_WIDTH + cursor_x] = (color << 8) | ' ';
        }
        return;
    }
    VGA_BUFFER[cursor_y * VGA_WIDTH + cursor_x] = (color << 8) | c;
    cursor_x++;
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
        vga_scroll();
    }
}

void vga_clear_screen() {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        VGA_BUFFER[i] = (0x07 << 8) | ' ';
    }
    cursor_x = 0;
    cursor_y = 0;
}

void vga_print(const char* str, uint8_t color) {
    while (*str) {
        vga_put_char(*str++, color);
    }
}

void vga_print_line(const char* str, uint8_t color) {
    vga_print(str, color);
    vga_put_char('\n', color);
}

// simple hex printing (uppercase)
void vga_print_hex(uint32_t n, uint8_t color) {
    const char* hex_chars = "0123456789ABCDEF";
    char buffer[9];
    buffer[8] = 0;
    for (int i = 7; i >= 0; i--) {
        buffer[i] = hex_chars[n & 0xF];
        n >>= 4;
    }
    vga_print(buffer, color);
}
