OUTPUT_FORMAT(elf64-x86-64)
ENTRY(main)

PHDRS
{
	text 	PT_LOAD	FLAGS(0x1); 	/* X */
	data 	PT_LOAD FLAGS(0x2|0x4);	/* R+W */
	rodata 	PT_LOAD FLAGS(0x4);	/* R */
}

SECTIONS
{
	. = 0xC0000000 ;
	.text . : {
		*(.text)
	} :text

	.data ALIGN(0x1000): {
		*(.data)
	} :data
}
