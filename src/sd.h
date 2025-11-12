#ifndef __SD_H__
#define __SD_H__

#define SECTOR_SIZE 512

// Forward declaration of the ATA read function from ide.s
int ata_lba_read(unsigned int lba, unsigned char *buffer, unsigned int numsectors);

// Wrapper function that matches the sd_readblock interface expected by the homework
// This just calls ata_lba_read with the same parameters
static inline int sd_readblock(unsigned int sector, char *buffer, unsigned int numsectors) {
    return ata_lba_read(sector, (unsigned char*)buffer, numsectors);
}

#endif // __SD_H__
