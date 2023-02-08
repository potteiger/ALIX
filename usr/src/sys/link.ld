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
	/* Kernel base address */
	. = 0xFFFFFFFF80000000 ;
	kbase = . ; /* Make a symbol for it */

	.text . : {
		*(.text)
	} :text

	.data ALIGN(0x1000): {
		*(.data)
	} :data

	.rodata ALIGN(0x1000): {
		*(.rodata*)
	} :rodata

	kend = ALIGN(0x1000); /* Symbol marking next free page after kernel */
}

