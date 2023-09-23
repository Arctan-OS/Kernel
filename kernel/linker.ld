ENTRY(kernel_main)

SECTIONS {
	. = 0xFFFFFFFF80000000;

	.text BLOCK(0x1000) : ALIGN(0x1000) {
		*(.text .text.*)
	}

	.rodata BLOCK(0x1000) : ALIGN(0x1000) {
		*(.rodata .rodata.*)
	}

	.data BLOCK(0x1000) : ALIGN(0x1000) {
		*(.data .data.*)
	}

	.bss BLOCK(0x1000) : ALIGN(0x1000) {
		*(COMMON)
		*(.bss .bss.*)
	}
}