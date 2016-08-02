ENTRY(_start)
SECTIONS {
    . = 0xC0100000;
    
    .text : AT(ADDR(.text) - 0xC0000000) {
        *(.text)
        *(.rodata*)
        _start -= 0xC0000000; /* needs to exist before paging */
    }

    .data ALIGN(0x1000): AT(ADDR(.data) - 0xC0000000) {
        *(.data)
    }

    .bss ALIGN(0x1000): AT(ADDR(.bss) - 0xC0000000) {
		_bss_start = .;
        *(COMMON)
        *(.bss)
        _bss_end = .;
    }
    
	_kimg_end = .;
}

