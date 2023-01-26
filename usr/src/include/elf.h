/* `elf.h` -- Definitions for ELF object file format
 * Copyright (c) 2022 Alan Potteiger
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef _ELF_H_
#define _ELF_H_

/* 
 * Definitions for handling ELF files. Referencing "ELF-64 Object File Format"
 * as seen here: https://uclibc.org/docs/elf-64-gen.pdf
 * Definitions will be appropriately commented with descriptions as seen in
 * the document.
 */

/*
 * Basic integer types
 */
typedef uint64_t        Elf64_Addr;     /* unsigned program address */
typedef uint64_t        Elf64_Off;      /* unsigned file offset */
typedef uint16_t        Elf64_Half;     /* unsigned medium integer */
typedef uint32_t        Elf64_Word;     /* unsigned integer */
typedef int32_t         Elf64_Sword;    /* signed integer */
typedef uint64_t        Elf64_Xword;    /* unsigned long integer */
typedef int64_t         Elf64_Sxword;   /* signed long integer */

/*
 * Main file header
 */
typedef struct {

	unsigned char   e_ident[16];    /* ELF identification */
	Elf64_Half      e_type;         /* object file type */
	Elf64_Half      e_machine;      /* machine type */
	Elf64_Word      e_version;      /* object file version */
	Elf64_Addr      e_entry;        /* entry point address */
	Elf64_Off       e_phoff;        /* program header offset */
	Elf64_Off       e_shoff;        /* section header offset */
	Elf64_Word      e_flags;        /* processor-specific flags */
	Elf64_Half      e_ehsize;       /* ELF header size */
	Elf64_Half      e_phentsize;    /* size of program header entry */
	Elf64_Half      e_phnum;        /* number of program header entries */
	Elf64_Half      e_shentsize;    /* size of section header entry */
	Elf64_Half      e_shnum;        /* number of section header entries */
	Elf64_Half      e_shstrndx;     /* section name string table index */

} Elf64_Ehdr;

/* ELF identification, e_ident from above */
#define EI_MAG0		0  /* file identification */
#define EI_MAG1		1
#define EI_MAG2		2
#define EI_MAG3		3
#define EI_CLASS	4  /* file class */
#define EI_DATA 	5  /* data encoding */
#define EI_VERSION 	6  /* file version */
#define EI_OSABI 	7  /* OS/ABI identification */
#define EI_ABIVERSION 	8  /* ABI version */
#define EI_PAD 		9  /* start of padding bytes */
#define EI_NIDENT 	16 /* size of e_ident */

/* file identification, e_ident[EI_MAG0] - e_ident[EI_MAG3] */
#define ELFMAG0 0x7f
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'

/* object file classes, e_ident[EI_CLASS] */
#define ELFCLASS32 1 /* 32 bit objects */
#define ELFCLASS64 2 /* 64 bit objects */

/* data encodings, e_ident[EI_DATA] */
#define ELFDATA2LSB 1 /* data structures are little endian */
#define ELFDATA2MSB 2 /* data structures are big endian */

/* operating system and ABI identifiers, e_ident[EI_OSABI] */
#define ELFOSABI_NONE		0	/* Unspecified or */
#define ELFOSABI_SYSV 		0	/* System V */
#define ELFOSABI_HPUX 		1	/* HP-UX operating system */
#define ELFOSABI_NETBSD		2	/* NetBSD */
#define ELFOSABI_LINUX		3	/* Linux */
#define ELFOSABI_SOLARIS	6	/* Solaris */
#define ELFOSABI_AIX		7	/* AIX */
#define ELFOSABI_IRIX		8	/* IRIX */
#define ELFOSABI_FREEBSD	9	/* FreeBSD */
#define ELFOSABI_TRU64		10	/* Compaq TRU64 UNIX */
#define ELFOSABI_OPENBSD	12	/* OpenBSD */
#define ELFOSABI_STANDALONE 	255	/* standalone */

/* object file types, e_type */
#define ET_NONE 	0      /* no file type */
#define ET_REL 		1      /* relocatable object file */
#define ET_EXEC 	2      /* executable file */
#define ET_DYN 		3      /* shared object file */
#define ET_CORE 	4      /* core file */
#define ET_LOOS 	0xFE00 /* environment-specific use */
#define ET_HIOS 	0xFEFF
#define ET_LOPROC 	0xFF00 /* processor-specific use */
#define ET_HIPROC	0xFFFF

/* machine types, e_machine */
#define EM_NONE		0	/* No machine */
#define EM_386		3	/* Intel 80386 */
#define EM_ARM		40	/* 32-bit ARM */
#define EM_X86_64	62	/* AMD/x86_64 */
#define EM_RISCV	243	/* RISC-V */

/*
 * Section header entries
 */
typedef struct {
	Elf64_Word      sh_name;        /* section name */
	Elf64_Word      sh_type;        /* section type */
	Elf64_Xword     sh_flags;       /* section attributes */
	Elf64_Addr      sh_addr;        /* virtual address in memory */
	Elf64_Off       sh_offset;      /* offset in file */
	Elf64_Xword     sh_size;        /* size of section */
	Elf64_Word      sh_link;        /* link to other section */
	Elf64_Word      sh_info;        /* miscellaneous information */
	Elf64_Xword     sh_addralign;   /* address alignment boundary */
	Elf64_Xword     sh_entsize;     /* size of entries */
} Elf64_Shdr;                                   /* (if section has table) */

/* special section indices */
#define SHN_UNDEF  0      /* mark undefined or meaningless section reference */
#define SHN_LOPROC 0xFF00 /* processor-specific use */
#define SHN_HIPROC 0xFF1F
#define SHN_LOOS   0xFF20 /* environment-specific use */
#define SHN_HIOS   0xFF3F
#define SHN_ABS    0xFFF1 /* corresponding reference is an absolute value */
#define SHN_COMMON 0xFFF2 /* symbol that has been declared as a common block */

/* section types, sh_type */
#define SHT_NULL     0          /* marks an unused section header */
#define SHT_PROGBITS 1          /* contains information defined by the program */
#define SHT_SYMTAB   2          /* contains a linker symbol table */
#define SHT_STRTAB   3          /* contains a string table */
#define SHT_RELA     4          /* contains “Rela” type relocation entries */
#define SHT_HASH     5          /* contains a symbol hash table */
#define SHT_DYNAMIC  6          /* contains dynamic linking tables */
#define SHT_NOTE     7          /* contains note information */
#define SHT_NOBITS   8          /* contains uninitialized space */
#define SHT_REL      9          /* contains “Rel” type relocation entries */
#define SHT_SHLIB    10         /* reserved */
#define SHT_DYNSYM   11         /* contains a dynamic loader symbol table */
#define SHT_LOOS     0x60000000 /* environment-specific use */
#define SHT_HIOS     0x6FFFFFFF
#define SHT_LOPROC   0x70000000 /* processor-specific use */
#define SHT_HIPROC   0x7FFFFFFF

/* section attributes, sh_flags */
#define SHF_WRITE     0x1        /* contains writable data */
#define SHF_ALLOC     0x2        /* allocated in memory image of program */
#define SHF_EXECINSTR 0x4        /* contains executable instructions */
#define SHF_MASKOS    0x0F000000 /* environment-specific use */
#define SHF_MASKPROC  0xF0000000 /* processor-specific use */

/*
 * Symbol table entry
 */
typedef struct {
	Elf64_Word      st_name;        /* symbol name */
	unsigned char   st_info;        /* type and binding attributes */
	unsigned char   st_other;       /* reserved */
	Elf64_Half      st_shndx;       /* section table index */
	Elf64_Addr      st_value;       /* symbol value */
	Elf64_Xword     st_size;        /* size of object (e.g., common) */
} Elf64_Sym;

/* symbol bindings */
#define STB_LOCAL  0  /* not visible outside the object file */
#define STB_GLOBAL 1  /* global symbol visible to all object files */
#define STB_WEAK   2  /* global but with lower precedence */
#define STB_LOOS   10 /* environment-specific use */
#define STB_HIOS   12
#define STB_LOPROC 13 /* processor-specific use */
#define STB_HIPROC 15

/* symbol types */
#define STT_NOTYPE  0 /* no type specified */
#define STT_OBJECT  1 /* data object */
#define STT_FUNC    2 /* function entry point */
#define STT_SECTION 3 /* symbol is associated with a section */
#define STT_FILE    4 /* source file associated with the object */
#define STT_LOOS    10 /* environment-specific use */
#define STT_HIOS    12
#define STT_LOPROC  13 /* processor-specific use */
#define STT_HIPROC  15

/*
 * Relocation entries
 */
typedef struct {
	Elf64_Addr      r_offset;       /* address of reference */
	Elf64_Xword     r_info;         /* symbol index and type of reloc */
} Elf64_Rel;

typedef struct {
	Elf64_Addr      r_offset;       /* address of reference */
	Elf64_Xword     r_info;         /* symbol index and type of reloc */
	Elf64_Sxword    r_addend;       /* constant part of expression */
} Elf64_Rela;

/* Returns symbol index from r_info */
#define ELF64_R_SYM(i) ((i) >> 32)
/* Returns relocation type from r_info */
#define ELF64_R_TYPE(i) ((i) & 0xffffffffL)
/* accepts symbol index and reloc type to store in r_info */
#define ELF64_R_INFO(s, t) (((s) << 32) + ((t) & 0xffffffffL))

/*
 * Program header table entry (segment)
 */
typedef struct {
	Elf64_Word      p_type;         /* type of segment */
	Elf64_Word      p_flags;        /* segment attributes */
	Elf64_Off       p_offset;       /* offset in file */
	Elf64_Addr      p_vaddr;        /* virtual address in memory */
	Elf64_Addr      p_paddr;        /* reserved for physical addressing*/
	Elf64_Xword     p_filesz;       /* size of segment in file */
	Elf64_Xword     p_memsz;        /* size of segment in memory */
	Elf64_Xword     p_align;        /* alignment of segment */
} Elf64_Phdr;

/* segment types, p_type */
#define PT_NULL	   0          /* unused entry */
#define PT_LOAD    1          /* loadable segment */
#define PT_DYNAMIC 2          /* dynamic linking tables */
#define PT_INTERP  3          /* program interpreter path name */
#define PT_NOTE    4          /* note sections */
#define PT_SHLIB   5          /* reserved */
#define PT_PHDR    6          /* program header table */
#define PT_LOOS    0x60000000 /* environment-specific use */
#define PT_HIOS    0x6FFFFFFF
#define PT_LOPROC  0x70000000 /* processor-specific use */
#define PT_HIPROC  0x7FFFFFFF

/* segment attributes, p_flags */
#define PF_X        0x1        /* execute permission */
#define PF_W        0x2        /* write permission */
#define PF_R        0x4        /* read permission */
#define PF_MASKOS   0x00FF0000 /* reserved for environment-specific use */
#define PF_MASKPROC 0xFF000000 /* reserved for processor-specific use */

/* TODO: Dynamic table entries? */

#endif /* _ELF_H_ */

