#ifndef MMU_H
#define MMU_H

#include <stdint.h>
#include "page.h"

// Page directory entry structure for i386
struct page_directory_entry {
   uint32_t present       : 1;   // Page present in memory
   uint32_t rw            : 1;   // Read-only if clear, R/W if set
   uint32_t user          : 1;   // Supervisor only if clear
   uint32_t writethru     : 1;   // Cache this directory as write-thru only
   uint32_t cachedisabled : 1;   // Disable cache on this page table?
   uint32_t accessed      : 1;   // Has the directory been accessed?
   uint32_t pagesize      : 1;   // 0 = 4KB page, 1 = 4MB page
   uint32_t ignored       : 2;   // Ignored bits
   uint32_t os_specific   : 3;   // OS-specific bits
   uint32_t frame         : 20;  // Frame address (shifted right 12 bits)
};

// Page table entry structure for i386
struct page {
   uint32_t present    : 1;   // Page present in memory
   uint32_t rw         : 1;   // Read-only if clear, readwrite if set
   uint32_t user       : 1;   // Supervisor level only if clear
   uint32_t accessed   : 1;   // Has the page been accessed since last refresh?
   uint32_t dirty      : 1;   // Has the page been written to since last refresh?
   uint32_t unused     : 7;   // Combination of unused and reserved bits
   uint32_t frame      : 20;  // Frame address (shifted right 12 bits)
};

void init_page_structures(void);
void *map_pages(void *vaddr, struct ppage *pglist, struct page_directory_entry *pd);
void loadPageDirectory(struct page_directory_entry *pd);
void enable_paging(void);

#endif
