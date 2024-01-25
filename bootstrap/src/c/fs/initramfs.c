#include <fs/initramfs.h>
#include <global.h>

/**
 * CPIO binary file header.
 *
 * As described by https://www.systutorials.com/docs/linux/man/5-cpio/.
 * */
struct ARC_CPIOHeader {
	uint16_t magic;
	uint16_t dev;
	uint16_t ino;
	uint16_t mode;
	uint16_t uid;
	uint16_t gid;
	uint16_t nlink;
	uint16_t rdev;
	uint16_t mtime[2];
	uint16_t namesize;
	uint16_t filesize[2];
}__attribute__((packed));

int load_file(void *image, uint32_t size, char *path, uint64_t vaddr) {
	uint32_t offset = 0;

	while (offset < size) {
		struct ARC_CPIOHeader *header = (struct ARC_CPIOHeader *)(image + offset);

		if (header->magic != 0070707) {
			// Header magic mismatch
			return 1;
		}

		uint16_t name_size = header->namesize + (header->namesize % 2);
		uint32_t file_size = (header->filesize[0] << 16) | header->filesize[1];

		char *name_base = (char *)(image + offset + sizeof(struct ARC_CPIOHeader));
		uint8_t *file_data = (uint8_t *)(name_base + name_size);

		if (header->namesize % 2 != 0) {
			// Namesize is odd, add one byte
			// to align it
			file_data += 1;
		}

		if (strcmp(name_base, path) != 0) {
			// Not the file we are looking for, go to next file
			offset += sizeof(struct ARC_CPIOHeader) + name_size + file_size;
			continue;
		}

		// Found file
		ARC_DEBUG(INFO, "Found file %s\n", name_base);

		return 0;
	}

	// File not found
	return -1;
}
