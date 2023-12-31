#include <global.h>
#include <multiboot/mbparse.h>
#include <multiboot/multiboot2.h>
#include <interface/printf.h>

int parse_mbi() {
	struct multiboot_tag *current_tag = (struct multiboot_tag *)(Arc_BootMeta->mb2i + ARC_HHDM_VADDR);
	struct multiboot_tag *end = (struct multiboot_tag *)(current_tag + current_tag->type);

	current_tag += 8;

	while (current_tag < end && current_tag->type != 0) {
		switch (current_tag->type) {
		case MULTIBOOT_TAG_TYPE_MMAP: {
			printf("MMAP\n");
			break;
		}
		case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
			printf("Framebuffer\n");
			break;
		}
		}

		//printf("%d\n", current_tag->type);

		current_tag = (struct multiboot_tag *)((uintptr_t)current_tag + ALIGN(current_tag->size, 8));
	}


	return 0;
}
