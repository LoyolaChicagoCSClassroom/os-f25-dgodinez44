#include "page.h"
#include <stddef.h>  // for NULL

// Array of 128 pages (128 *2MB) = 256MB of memory)
struct ppage physical_page_array[128];

// Head of the free list
struct ppage *free_list = NULL;

// Remove a node from whatever list it's in
void list_remove(struct ppage *node){
  if (node->prev != NULL){
    node->prev->next = node->next;
  }

  if (node->next != NULL){
    node->next->prev = node->prev;
  }

  node->next = NULL;
  node->prev = NULL;
}

// Add a node to the front of a list
void list_add_front(struct ppage **head, struct ppage *node){
  node->next = *head;
  node->prev = NULL;

  if (*head != NULL){
    (*head)->prev = node;
  }

  *head = node;
}

// Append one list to another
void list_append(struct ppage **head, struct ppage *list_to_add){
  if (*head == NULL){
    *head = list_to_add;
    return;
  }

  // Find the end of the first list
  struct ppage *tail = *head;
  while (tail->next != NULL){
    tail = tail->next;
  }

  // Connect the lists
  tail->next = list_to_add;
  if (list_to_add != NULL){
    list_to_add->prev = tail;
  }
}


void init_pfa_list(void){
  // Start with an empty free list
  free_list = NULL;

  // Initialize each page in the array
  for (int i = 0; i < 128; i++){
    //  Calculate the physical address for this page
    // Each page is 2 MB or 0x200000 bytes
    physical_page_array[i].physical_addr = (void *)(i * 0x200000);

    // Add this page to the free list
    list_add_front(&free_list, &physical_page_array[i]);
  }
}

struct ppage *allocate_physical_pages(unsigned int npages){
  if (npages == 0){
    return NULL;
  }

  // Check if we have enough free pages
  struct ppage *current = free_list;
  unsigned int count = 0;
  while (current != NULL && count < npages){
    count++;
    current = current->next;
  }

  if (count < npages){
    // Not enough free pages available
    return NULL;
  }

  // Create the allocated list by removing npages from the free list
  struct ppage *allocated_list = NULL;

  for (unsigned int i = 0; i < npages; i++){
    // Remove the first page from the free list
    struct ppage *page = free_list;
    free_list = free_list->next;
    if (free_list != NULL){
      free_list->prev = NULL;
    }

    // Add it to the allocated list
    page->next = allocated_list;
    page->prev = NULL;
    if (allocated_list != NULL){
      allocated_list->prev = page;
    }
    allocated_list = page;
  }

  return allocated_list;
}

void free_physical_pages(struct ppage *ppage_list){
  if (ppage_list == NULL){
    return;
  }

   // Find the end of the list being free
  struct ppage *tail = ppage_list;
  while (tail->next != NULL){
    tail = tail->next;
  }

  // Append the free list to the end of the list being returned
  tail->next = free_list;
  if (free_list != NULL){
    free_list->prev = tail;
  }

  // The freed list becomes the new free list
  free_list = ppage_list;
}
