#include "fat.h"
#include "sd.h"
#include "rprintf.h"
#include <stdint.h>

#define PARTITION_START_SECTOR 2048

// External putc function for printing
extern int putc(int c);

// Global variables - keep these small
struct boot_sector boot_sec;  // Store boot sector as struct, not buffer
struct boot_sector *bs = &boot_sec;
unsigned int root_sector;

// Helper functions
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, int n);
void toupper_str(char *dest, const char *src);
void extract_filename(struct root_directory_entry *rde, char *fname);

int fatInit() {
    char temp_buffer[512];
    
    esp_printf(putc, "Initializing FAT filesystem...\r\n");
    
    // Read boot sector
    sd_readblock(PARTITION_START_SECTOR, temp_buffer, 1);
    
    // Copy to our boot sector struct
    struct boot_sector *temp_bs = (struct boot_sector*)temp_buffer;
    boot_sec = *temp_bs;
    
    esp_printf(putc, "Bytes per sector: %d\r\n", bs->bytes_per_sector);
    esp_printf(putc, "Sectors per cluster: %d\r\n", bs->num_sectors_per_cluster);
    esp_printf(putc, "Number of FATs: %d\r\n", bs->num_fat_tables);
    esp_printf(putc, "Root directory entries: %d\r\n", bs->num_root_dir_entries);
    
    // Validate boot signature
    if (bs->boot_signature != 0xAA55) {
        esp_printf(putc, "Error: Invalid boot signature\r\n");
        return -1;
    }
    esp_printf(putc, "Boot signature valid\r\n");
    
    // Calculate root directory sector
    root_sector = PARTITION_START_SECTOR + bs->num_reserved_sectors + 
                  (bs->num_fat_tables * bs->num_sectors_per_fat);
    
    esp_printf(putc, "Root directory at sector: %d\r\n", root_sector);
    esp_printf(putc, "FAT init complete!\r\n\r\n");
    
    return 0;
}

struct file* fatOpen(const char *path) {
    static struct file f;
    static char root_buffer[8192];  // Enough for typical root directory
    
    esp_printf(putc, "Opening file: %s\r\n", path);
    
    // Skip leading /
    if (path[0] == '/') path++;
    
    // Convert to uppercase
    char upper_name[13];
    toupper_str(upper_name, path);
    
    // Calculate root directory size
    unsigned int root_dir_sectors = (bs->num_root_dir_entries * 32 + 511) / 512;
    if (root_dir_sectors > 16) root_dir_sectors = 16;  // Limit to buffer size
    
    // Read root directory
    sd_readblock(root_sector, root_buffer, root_dir_sectors);
    
    struct root_directory_entry *rde = (struct root_directory_entry*)root_buffer;
    
    // Search for file
    for (unsigned int i = 0; i < bs->num_root_dir_entries; i++) {
        if (rde[i].file_name[0] == 0x00) break;
        if (rde[i].file_name[0] == 0xE5) continue;
        if (rde[i].attribute & 0x08) continue;  // Skip volume labels
        
        char fname[13];
        extract_filename(&rde[i], fname);
        
        if (strcmp(fname, upper_name) == 0) {
            esp_printf(putc, "Found: %s (cluster %d, size %d)\r\n", 
                      fname, rde[i].cluster, rde[i].file_size);
            f.rde = rde[i];
            f.start_cluster = rde[i].cluster;
            f.next = NULL;
            f.prev = NULL;
            return &f;
        }
    }
    
    esp_printf(putc, "File not found\r\n");
    return NULL;
}

int fatRead(struct file *file, char *buffer, unsigned int size) {
    if (file == NULL) return -1;
    
    esp_printf(putc, "Reading file (max %d bytes)...\r\n", size);
    
    // Calculate data region start
    unsigned int root_dir_sectors = (bs->num_root_dir_entries * 32 + 511) / 512;
    unsigned int data_start = root_sector + root_dir_sectors;
    
    // Calculate cluster sector
    unsigned int cluster = file->start_cluster;
    unsigned int sector = data_start + (cluster - 2) * bs->num_sectors_per_cluster;
    
    esp_printf(putc, "Reading cluster %d at sector %d\r\n", cluster, sector);
    
    // Read one cluster
    unsigned int cluster_size = bs->num_sectors_per_cluster * 512;
    char cluster_buf[cluster_size];
    
    sd_readblock(sector, cluster_buf, bs->num_sectors_per_cluster);
    
    // Copy to output
    unsigned int bytes_to_copy = (size < file->rde.file_size) ? size : file->rde.file_size;
    if (bytes_to_copy > cluster_size) bytes_to_copy = cluster_size;
    
    for (unsigned int i = 0; i < bytes_to_copy; i++) {
        buffer[i] = cluster_buf[i];
    }
    
    esp_printf(putc, "Read %d bytes\r\n", bytes_to_copy);
    return bytes_to_copy;
}

void extract_filename(struct root_directory_entry *rde, char *fname) {
    int k = 0;
    while (k < 8 && rde->file_name[k] != ' ') {
        fname[k] = rde->file_name[k];
        k++;
    }
    if (rde->file_extension[0] != ' ') {
        fname[k++] = '.';
        int n = 0;
        while (n < 3 && rde->file_extension[n] != ' ') {
            fname[k++] = rde->file_extension[n];
            n++;
        }
    }
    fname[k] = '\0';
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

int strncmp(const char *s1, const char *s2, int n) {
    while (n > 0 && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) return 0;
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

void toupper_str(char *dest, const char *src) {
    while (*src) {
        if (*src >= 'a' && *src <= 'z') {
            *dest = *src - ('a' - 'A');
        } else {
            *dest = *src;
        }
        dest++;
        src++;
    }
    *dest = '\0';
}
