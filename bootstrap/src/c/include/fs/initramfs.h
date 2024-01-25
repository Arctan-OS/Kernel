#ifndef ARC_FS_INITRAMFS_H
#define ARC_FS_INITRAMFS_H

#include <stdint.h>
#include <stddef.h>

/**
 * Load a file from the given CPIO image.
 *
 * The virtual addres must be page aligned.
 * If the given virtual address is NULL, then
 * physical contiguity is guaranteed.
 *
 * @param void *image - Physical pointer to the CPIO image.
 * @param char *path - The path of the file to load.
 * @param uint64_t - The virtual address at which to load the file.
 * @return 0 for success. */
int load_file(void *image, size_t size, char *path, uint64_t vaddr);

#endif
