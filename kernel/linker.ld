ENTRY(kernel_man)

SECTIONS {
	. = 0xFFFFFFFF00000000; /* The kernel starts in the higher-half */

	.text ALIGN(0x1000) : BLOCK(0x1000) {
		*(.text .text.*)
	}

	.rodata ALIGN(0x1000) : BLOCK(0x1000) {
		*(.rodata .rodata.*)
	}

	.data ALIGN(0x1000) : BLOCK(0x1000) {
		*(.data .data.*)
	}

	.bss ALIGN(0x1000) : BLOCK(0x1000) {
		*(COMMON)
		*(.bss .bss.*)
	}
}