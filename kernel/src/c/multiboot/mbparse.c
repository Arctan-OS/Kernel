#include <global.h>
#include <multiboot/mbparse.h>
#include <multiboot/multiboot2.h>
#include <interface/printf.h>

int strcmp(char *a, char *b) {
	int sum = 0;
	while (*a != 0) {
		sum += *a - *b;

		a++;
		b++;
	}

	return sum;
}

int parse_mbi() {
	struct multiboot_tag *tag = (struct multiboot_tag *)(Arc_BootMeta->mb2i + ARC_HHDM_VADDR);
	struct multiboot_tag *end = (struct multiboot_tag *)(tag + tag->type);

	tag += 8;

	while (tag < end && tag->type != 0) {
		switch (tag->type) {
		case MULTIBOOT_TAG_TYPE_MMAP: {
			printf("MMAP\n");
			break;
		}

		case MULTIBOOT_TAG_TYPE_MODULE: {
			struct multiboot_tag_module *info = (struct multiboot_tag_module *)tag;

			ARC_DEBUG(INFO, "----------------\n")
			ARC_DEBUG(INFO, "Found module: %s\n", info->cmdline);
			ARC_DEBUG(INFO, "\t0x%"PRIX32" -> 0x%"PRIX32" (%d B)\n", info->mod_start, info->mod_end, (info->mod_end - info->mod_start))

			if (strcmp(info->cmdline, "arctan-module.kernel.font.bin") == 0) {
				ARC_DEBUG(INFO, "\tFound kerne.font\n")
				global_kernel_font = (uint8_t *)(info->mod_start + ARC_HHDM_VADDR);
			}

			ARC_DEBUG(INFO, "----------------\n")

			break;
		}

		case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
			struct multiboot_tag_framebuffer *info = (struct multiboot_tag_framebuffer *)tag;
			struct multiboot_tag_framebuffer_common common = (struct multiboot_tag_framebuffer_common)info->common;

			ARC_DEBUG(INFO, "Framebuffer 0x%"PRIX64"(%d) %dx%dx%d\n", common.framebuffer_addr, common.framebuffer_type, common.framebuffer_width, common.framebuffer_height, common.framebuffer_bpp)

			for (int i = 0; i < common.framebuffer_height; i++) {
				for (int j = 0; j < common.framebuffer_width; j++) {
					*((uint32_t *)(common.framebuffer_addr + ARC_HHDM_VADDR) + (i * common.framebuffer_width) + j) = 0x0043210FF;
				}
			}

			global_framebuffer = info;

			break;
		}
		}

		tag = (struct multiboot_tag *)((uintptr_t)tag + ALIGN(tag->size, 8));
	}


	return 0;
}
