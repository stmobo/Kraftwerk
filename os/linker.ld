/* The bootloader will look at this image and start execution at the symbol
   designated as the entry point. */
OUTPUT_FORMAT(elf64-x86-64)
ENTRY(start) /* Entry symbol at start */

/* Tell where the various sections of the object files will be put in the final
   kernel image. */

KERNEL_VMA = 0xFFFFFF8000000000;
KERNEL_LMA = 0x100000;

SECTIONS
{
	/* put early init code here */
	. = KERNEL_LMA;
	.multiboot : AT( 0 ) {
		*(.multiboot);
	}

	.entry ALIGN(4K) : AT( ADDR(.entry) ) {
		*(.entry);
		*(.boot_tables);
		*(.bootstrap_stack);
	}

	/* All other symbols should be linked to the kernel base address, but *placed* from 0x100000. */
	. += KERNEL_VMA;

	.text ALIGN(4K) : AT(ADDR(.text) - KERNEL_VMA)
	{
		kernel_start_phys = (. - KERNEL_VMA);
		kernel_start = .;
		*(.text*);
		*(.rdata*);
		*(.rodata*);
	}

	/* Read-write data (initialized) */
	.data ALIGN(4K) : AT(ADDR(.data) - KERNEL_VMA)
	{
		*(.data);
	}

	/* Read-write data (uninitialized) and stack */
	.bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_VMA)
	{
		_sbss = .;
		*(COMMON);
		*(.bss);
		_ebss = .;
	}

	/* The compiler may produce other sections, by default it will put them in
	   a segment with the same name. Simply add stuff here as needed. */
   kernel_end = .;
   kernel_end_phys = (. - KERNEL_VMA);
}
/*__CTOR_LIST__ = ADDR(.ctors);*/
