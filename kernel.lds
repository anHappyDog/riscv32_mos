OUTPUT_ARCH(riscv)

ENTRY(_start)

SECTIONS {
    kernel_start = .;
    . = 0x80200000;
    .text : {
        *(.boot)
        *(.text)
    }

    .data : {
        *(.data .data* .sdata .sdata*)
    }

    .bss  : {
        *(.bss .bss* .sbss .sbss*)
    }
    bss_end = .;
    . = 0x80600000;
    .exc_gen_entry : {
		*(.exc_gen_entry)
	} 
	end = . ;
}


