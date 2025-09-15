
#include <stdint.h>
#include "rprintf.h"

#define MULTIBOOT2_HEADER_MAGIC         0xe85250d6

const unsigned int multiboot_header[]  __attribute__((section(".multiboot"))) = {MULTIBOOT2_HEADER_MAGIC, 0, 16, -(16+MULTIBOOT2_HEADER_MAGIC), 0, 12};

uint8_t inb (uint16_t _port) {
    uint8_t rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

int putc(int data){
    static int offset = 0; // Keep track of the current position
    unsigned char *video_memory = (unsigned char *)0xB8000;

    if (data == '\n'){
        //Move to the next line (80 chars * 2 bytes = 160 bytes per line)
        offset = ((offset / 160)) * 160;
    }
    else if (data == '\r') {
        // Move to beginning of current line
        offset = (offset /160) * 160;
    } else {
        video_memory[offset] = data;    // Write ASCII character
        video_memory[offset +1] = 0x07;  // White on black attribute
        offset += 2;                   // Move to the next position
    }

    // See if the end of the screen is reached
    if (offset >= 160 * 25){
        offset =0;
    }

    return data;
}

void main() {
    unsigned short *vram = (unsigned short*)0xb8000; // Base address of video mem
    const unsigned char color = 7; // gray text on black background

    while(1) {
        uint8_t status = inb(0x64);

        if(status & 1) {
            uint8_t scancode = inb(0x60);
        }
    }

    esp_printf(putc, "Current execution level: %d\n", 0);
    esp_printf(putc, "Hello from terminal driver!\n");
}
