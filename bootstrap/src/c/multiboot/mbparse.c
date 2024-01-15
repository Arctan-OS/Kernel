#include <multiboot/mbparse.h>
#include <multiboot/multiboot2.h>
#include <global.h>
#include <mm/freelist.h>
#include <mm/pmm.h>

int strcmp(char *a, char *b) {
	int sum = 0;
	while (*a != 0) {
		sum += *a - *b;

		a++;
		b++;
	}

	return sum;
}

int read_mb2i(void *mb2i) {
	ARC_DEBUG(INFO, "Reading multiboot information structure\n")

	struct multiboot_tag *tag = (struct multiboot_tag *)(mb2i);
	struct multiboot_tag *end = (struct multiboot_tag *)(tag + tag->type);
	struct multiboot_tag_mmap *mmap = NULL;

	// We the memory map follows something along the lines of:
	// FREE / UNUSABLE, BOOTSTRAP, MODULES, FREE
	// In which case, this variable points ^ here
	uint64_t bootstrap_end = 0;

	tag = (struct multiboot_tag *)((uintptr_t)tag + 8);

	while (tag->type != 0 && tag < end) {
		switch (tag->type) {
		case MULTIBOOT_TAG_TYPE_MMAP: {
			mmap = (struct multiboot_tag_mmap *)tag;
			ARC_DEBUG(INFO, "Found memory map (%d)\n", mmap->entry_version);

			int entries = (mmap->size - sizeof(struct multiboot_tag_mmap)) / mmap->entry_size;

			const char *names[] = {
				[MULTIBOOT_MEMORY_AVAILABLE] = "Available",
				[MULTIBOOT_MEMORY_ACPI_RECLAIMABLE] = "ACPI Reclaimable",
				[MULTIBOOT_MEMORY_BADRAM] = "Bad",
				[MULTIBOOT_MEMORY_NVS] = "NVS",
				[MULTIBOOT_MEMORY_RESERVED] = "Reserved"
			};

			uint64_t total_memory = 0;

			for (int i = 0; i < entries; i++) {
				struct multiboot_mmap_entry entry = mmap->entries[i];

				total_memory += entry.len;

				ARC_DEBUG(INFO, "\t%4d : 0x%16"PRIX64", 0x%16"PRIX64" B (%s)\n", i, entry.addr, entry.len, names[entry.type])
			}

			page_count = total_memory / 0x1000;

			break;
		}

		case MULTIBOOT_TAG_TYPE_MODULE: {
			struct multiboot_tag_module *info = (struct multiboot_tag_module *)tag;

			ARC_DEBUG(INFO, "----------------\n")
			ARC_DEBUG(INFO, "Found module: %s\n", info->cmdline);
			ARC_DEBUG(INFO, "\t0x%"PRIX32" -> 0x%"PRIX32" (%d B)\n", info->mod_start, info->mod_end, (info->mod_end - info->mod_start))

			if (strcmp(info->cmdline, "arctan-module.kernel.efi") == 0) {
				ARC_DEBUG(INFO, "\tFound kernel\n")
				kernel_elf = (void *)info->mod_start;
			} else if (strcmp(info->cmdline, "arctan-module.kernel.font.bin") == 0) {
				ARC_DEBUG(INFO, "\tFound kernel.font\n")
			}

			ARC_DEBUG(INFO, "----------------\n")

			if (info->mod_end > bootstrap_end) {
				bootstrap_end = info->mod_end;
			}

			break;
		}

		case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
			struct multiboot_tag_framebuffer *info = (struct multiboot_tag_framebuffer *)tag;
			struct multiboot_tag_framebuffer_common common = (struct multiboot_tag_framebuffer_common)info->common;

			ARC_DEBUG(INFO, "Framebuffer 0x%"PRIX64"(%d) %dx%dx%d\n", common.framebuffer_addr, common.framebuffer_type, common.framebuffer_width, common.framebuffer_height, common.framebuffer_bpp)

			break;
		}

		case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME: {
			struct multiboot_tag_string *info = (struct multiboot_tag_string *)tag;
			ARC_DEBUG(INFO, "Booted using %s\n", info->string);

			break;
		}

		case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO: {
			struct multiboot_tag_basic_meminfo *info = (struct multiboot_tag_basic_meminfo *)tag;

			ARC_DEBUG(INFO, "------------\n")
			ARC_DEBUG(INFO, "Basic Memory\n")
			ARC_DEBUG(INFO, "\tLow Mem: %d KB\n", info->mem_lower)
			ARC_DEBUG(INFO, "\tHigh Mem: %d KB\n", info->mem_upper)
			ARC_DEBUG(INFO, "------------\n")

			break;
		}

		case MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR: {
			struct multiboot_tag_load_base_addr *info = (struct multiboot_tag_load_base_addr *)tag;

			ARC_DEBUG(INFO, "Loaded at address: %"PRIX32"\n", info->load_base_addr)
		}
		}

		tag = (struct multiboot_tag *)((uintptr_t)tag + ALIGN(tag->size, 8));
	}


	ARC_DEBUG(INFO, "Finished reading multiboot information structure\n");
	ARC_DEBUG(INFO, "End of bootstrap 0x%"PRIX32"\n", bootstrap_end)

	init_pmm(mmap, (uintptr_t)bootstrap_end);

	return 0;
}
