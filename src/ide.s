# ATA read sectors (LBA mode) 
# C Prototype: ata_lba_read(unsigned int lba, unsigned char *buffer, unsigned int numsectors)

    .code32
    .global ata_lba_read
ata_lba_read:
    pushl %ebp
    movl %esp, %ebp
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %edi
    pushl %esi

    # Get parameters from stack
    movl 8(%ebp), %ebx      # Get LBA and save in EBX
    movl 12(%ebp), %edi     # Get buffer pointer
    movl 16(%ebp), %esi     # Get sector count and save in ESI

    # Send drive and bits 24-27 of LBA
    movl $0x01F6, %edx
    movl %ebx, %eax         # Copy LBA to EAX
    shrl $24, %eax          # Get bits 24-27
    orb $0xE0, %al          # Set LBA mode bits
    outb %al, %dx

    # Send number of sectors
    movl $0x01F2, %edx
    movl %esi, %eax         # Get sector count
    outb %al, %dx

    # Send bits 0-7 of LBA
    movl $0x1F3, %edx
    movl %ebx, %eax
    outb %al, %dx

    # Send bits 8-15 of LBA
    movl $0x1F4, %edx
    movl %ebx, %eax
    shrl $8, %eax
    outb %al, %dx

    # Send bits 16-23 of LBA
    movl $0x1F5, %edx
    movl %ebx, %eax
    shrl $16, %eax
    outb %al, %dx

    # Send read command
    movl $0x1F7, %edx
    movb $0x20, %al
    outb %al, %dx

.wait_ready:
    # Wait for drive to be ready
    movl $0x1F7, %edx
    inb %dx, %al
    testb $0x80, %al        # BSY bit
    jnz .wait_ready
    testb $0x08, %al        # DRQ bit
    jz .wait_ready

    # Read 256 words (512 bytes) from data port
    movl $0x1F0, %edx
    movl $256, %ecx
    rep insw

    # Small delay
    movl $0x1F7, %edx
    inb %dx, %al
    inb %dx, %al
    inb %dx, %al
    inb %dx, %al

    # Check if more sectors to read
    decl %esi
    jnz .wait_ready

    # Return 0 for success
    xorl %eax, %eax

    popl %esi
    popl %edi
    popl %edx
    popl %ecx
    popl %ebx
    popl %ebp
    ret
