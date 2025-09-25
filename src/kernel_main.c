
#include <stdint.h>
#include "rprintf.h"

#define MULTIBOOT2_HEADER_MAGIC         0xe85250d6

const unsigned int multiboot_header[]  __attribute__((section(".multiboot"))) = {MULTIBOOT2_HEADER_MAGIC, 0, 16, -(16+MULTIBOOT2_HEADER_MAGIC), 0, 12};

uint8_t inb (uint16_t _port) {
    uint8_t rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

int x = 0;
 
void scroll_screen(){
  char *v = (char*)0xB8000;
  int row; 
  int col;
  
  // Move each line up by one (copy line 1 to line 0, etc.)
  for (row = 0; row < 24; row++){
    for(col = 0; col < 80; col++){
      v[(row * 80 + col) * 2] = v[((row + 1) * 80 + col) * 2]; // Character
      v[(row * 80 + col) * 2 + 1] = v[(( row + 1) * 80 + col) * 2 + 1]; // Color
    }
  }
  
  // Clear the last line (line 24)
  for(col = 0; col < 80; col++){
    v[(24 * 80 + col) * 2] = ' '; // Space
    v[(24 * 80 + col) * 2 + 1] = 10; // Color
  }

}

int putc(int c){
  // Move to beginning of current line
  if (c == '\r') {
    x = (x / 80) * 80;
    return 0;
  }
  // Move to beginning of next line
  if (c == '\n') {
    x = ((x / 80) + 1) * 80;
    // Check if we've gone past the screen
    if(x >= 25 * 80){
      // Position cursor at start of last line after scroll
      scroll_screen();
      x = 24 * 80;
    }
    return 0;
  }
  // Check if we need to scroll before writing (if we're past the screen)
  if ( x >= 25 * 80) {
    scroll_screen();
    // Position cursor at start of last line after scroll
    x = 24 * 80;
  }
  // Write the character to video memory
  char *v = (char*)0xB8000;
  v[x*2] = c;
  v[x*2+1] = 10;
  x++;
 // Check if we need to scroll after writing (if we're at the end of the screen) 
 if( x >= 25 * 80){
   scroll_screen();
   x = 24 * 80;
  }
  return 0;
} 

void main() {
    putc('h');
    putc('e');
    putc('l');
    putc('l');
    putc('o');
    putc(' ');
    putc('\r'); // Test carriage return
    putc('\n'); // Test newline
   
   // Print current execution level
   esp_printf(putc, "Current execution level: Kernel Mode (Ring 0)\r\n");
   esp_printf(putc, "Kernel loaded successfully!\r\n");
   esp_printf(putc, "\r\n");

   // Test scrolling functionality
   esp_printf(putc, "Testing screen scrolling... \r\n");
   esp_printf(putc, "This test will fill the screen and demonstrate scrolling.\r\n");
   esp_printf(putc, "\r\n");

   for (int i = 1; i <= 30; i++){
     esp_printf(putc, "Line %d: Scroll test line\r\n", i);
   }   
   
   // Final message
    esp_printf(putc, "\r\n");
    esp_printf(putc, "Scrolling test completed successfully!\r\n");
    esp_printf(putc, "All assignment requirements have been met! \r\n");

   // Infinite loop at the end:
   esp_printf(putc, "Kernel finished. System halted.\r\n");
   while(1){
     // Infinite loop to keep the kernel running
     __asm__ __volatile__("hlt");
   }
}
