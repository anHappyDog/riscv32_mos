OUTPUT_ARCH(riscv)

ENTRY(_start) 

PHDRS {
	code PT_LOAD FLAGS (5);
	data PT_LOAD FLAGS (6);
}

SECTIONS {
	. = 0x004000000;
	
	.text : {
		*(.text)
		*(.rodata)
		*(.rodata.*)
	} : code
	
	.data ALIGN(4096) : {
		*(.data)
	} : data 
	
	.bss : {
		*(.bss)
	} : data
	end = .;
}

