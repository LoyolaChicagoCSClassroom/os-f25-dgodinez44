#include "mmu.h"
#include <stddef.h>

// Global page directory and page table
struct page_directory_entry pd[1024] __attribute__((aligned(4096)));
struct page pt[1024] __attribute__((aligned(4096)));

// Create the page directory and page table to all zeros
void init_page_structures(void) {
    for (int i = 0; i < 1024; i++) {
        pd[i].present = 0;
        pd[i].rw = 1;
        pd[i].user = 0;
        pd[i].writethru = 0;
        pd[i].cachedisabled = 0;
        pd[i].accessed = 0;
        pd[i].pagesize = 0;
        pd[i].ignored = 0;
        pd[i].os_specific = 0;
        pd[i].frame = 0;
        
        pt[i].present = 0;
        pt[i].rw = 1;
        pt[i].user = 0;
        pt[i].accessed = 0;
        pt[i].dirty = 0;
        pt[i].unused = 0;
        pt[i].frame = 0;
    }
}

/*
 * map_pages - Maps a list of physical pages to a virtual address
 * 
 * vaddr: Virtual address to start mapping at
 * pglist: Linked list of physical pages to map
 * pd: Page directory pointer
 * 
 * Returns: The virtual address that was mapped
 */

void *map_pages(void *vaddr, struct ppage *pglist, struct page_directory_entry *pd) {
    void *start_vaddr = vaddr;
    struct ppage *current = pglist;
    
    while (current != NULL) {
        // Calculate page directory index (bits 22-31 of virtual address)
        uint32_t pd_index = ((uint32_t)vaddr >> 22) & 0x3FF;
        
        // Calculate page table index (bits 12-21 of virtual address)
        uint32_t pt_index = ((uint32_t)vaddr >> 12) & 0x3FF;
        
        // If this page directory entry is not present, set it up
        if (!pd[pd_index].present) {
            // Point to our global page table
            pd[pd_index].frame = ((uint32_t)&pt[0]) >> 12;
            pd[pd_index].present = 1;
            pd[pd_index].rw = 1;       // Read/Write
            pd[pd_index].user = 0;     // Supervisor only
        }
        
        // Set up the page table entry
        pt[pt_index].frame = ((uint32_t)current->physical_addr) >> 12;
        pt[pt_index].present = 1;
        pt[pt_index].rw = 1;           // Read/Write
        pt[pt_index].user = 0;         // Supervisor only
        
        // Move to next page
        vaddr = (void *)((uint32_t)vaddr + 0x1000); // 4KB page
        current = current->next;
    }
    
    return start_vaddr;
}

// Loads the page directory into CR3
void loadPageDirectory(struct page_directory_entry *pd) {
    asm volatile("mov %0, %%cr3"
        :
        : "r"(pd)
        :);
}

// Enable paging by setting CR0 bits
void enable_paging(void) {
    asm volatile(
        "mov %%cr0, %%eax\n"
        "or $0x80000001, %%eax\n"
        "mov %%eax, %%cr0"
        :
        :
        : "eax"
    );
}
