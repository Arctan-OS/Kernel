#include <arctan.h>
#include <global.h>
#include <arch/x86/io/port.h>
#include <stdint.h>
#include <interface/printf.h>
#include <arch/x86/ctrl_regs.h>
#include <multiboot/mbparse.h>

#include <arch/x86/idt.h>
#include <arch/x86/gdt.h>

#include <interface/terminal.h>

struct ARC_BootMeta *Arc_BootMeta = NULL;
struct ARC_TermMeta main_terminal = { 0 };
static char main_terminal_mem[120 * 120] = { 0 };

int kernel_main(struct ARC_BootMeta *boot_meta) {
	Arc_BootMeta = boot_meta;

	main_terminal.term_width = 120;
	main_terminal.term_height = 120;
	main_terminal.term_mem = main_terminal_mem;

	main_terminal.cx = 0;
	main_terminal.cy = 0;

	ARC_DEBUG(INFO, "Sucessfully entered long mode\n")

	install_gdt();
	install_idt();

	parse_mbi();

	printf("Welcome to 64-bit wonderland! Please enjoy your stay.\n");

	for (int i = 0; i < 600; i++) {
		for (int y = 0; y < main_terminal.fb_height; y++) {
			for (int x = 0; x < main_terminal.fb_width; x++) {
				*((uint32_t *)main_terminal.framebuffer + (y * main_terminal.fb_width) + x) = (x * y * i / 300) & 0x3FFF;
			}
		}
	}

	for (;;) {
		Arc_TermDraw(&main_terminal);
	}

	return 0;
}
