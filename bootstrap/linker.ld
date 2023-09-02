ENTRY(_entry)

SECTIONS {
	. = 1M; /* The kernel starts at 1MB */

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