ENTRY(_entry)

SECTIONS {
	. = 1M;

	.text BLOCK(0x1000) : ALIGN(0x1000) {
		*(.mb2header)
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

	__BOOTSTRAP_END__ = .;
}