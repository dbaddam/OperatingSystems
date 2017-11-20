#ifndef _ELF64_H
#define _ELF64_H

#define EI_NIDENT 16

#define ELF_RELOC_ERROR -1

typedef uint64_t Elf64_Addr;
typedef uint16_t Elf64_Half;
typedef uint64_t Elf64_Lword;
typedef uint64_t Elf64_Off;
typedef uint32_t Elf64_Sword;
typedef uint64_t Elf64_Sxword;
typedef uint32_t Elf64_Word;
typedef uint64_t Elf64_Xword;

typedef struct 
{
  unsigned char e_ident[EI_NIDENT];
  Elf64_Half    e_type;
  Elf64_Half    e_machine;
  Elf64_Word    e_version;
  Elf64_Addr    e_entry;
  Elf64_Off     e_phoff;
  Elf64_Off     e_shoff;
  Elf64_Word    e_flags;
  Elf64_Half    e_ehsize;
  Elf64_Half    e_phentsize;
  Elf64_Half    e_phnum;
  Elf64_Half    e_shentsize;
  Elf64_Half    e_shnum;
  Elf64_Half    e_shstrndx;
} Elf64_Ehdr;

typedef struct 
{
  Elf64_Word    p_type;
  Elf64_Word    p_flags;
  Elf64_Off     p_offset;
  Elf64_Addr    p_vaddr;
  Elf64_Addr    p_paddr;
  Elf64_Xword   p_filesz;
  Elf64_Xword   p_memsz;
  Elf64_Xword   p_align;
} Elf64_Phdr;

enum Elf_Ident 
{
	EI_MAG0		= 0, // 0x7F
	EI_MAG1		= 1, // 'E'
	EI_MAG2		= 2, // 'L'
	EI_MAG3		= 3, // 'F'
	EI_CLASS	= 4, // Architecture (32/64)
	EI_DATA		= 5, // Byte Order
	EI_VERSION	= 6, // ELF Version
	EI_OSABI	= 7, // OS Specific
	EI_ABIVERSION	= 8, // OS Specific
	EI_PAD		= 9  // Padding
};
 
# define ELFMAG0	0x7F // e_ident[EI_MAG0]
# define ELFMAG1	'E'  // e_ident[EI_MAG1]
# define ELFMAG2	'L'  // e_ident[EI_MAG2]
# define ELFMAG3	'F'  // e_ident[EI_MAG3]

# define ELFCLASS32	1    // 32-bit objects
# define ELFCLASS64	2    // 64-bit objects

# define ELFDATA2LSB	1    // obj file data structers are little-endian
# define ELFDATA2MSB	2    // obj file data structers are big-endian

enum Elf_Type 
{
   ET_NONE	= 0, // unknown type
   ET_REL	= 1, // Recolatable type
   ET_EXEC	= 2, // Executable type
};

# define EM_X86		0x3E  // x86 machine type, from elf wiki page
# define EV_CURRENT	1     // ELF current version


typedef struct
{
	Elf64_Word sh_name; 		/* Section name */
	Elf64_Word sh_type; 		/* Section type */
	Elf64_Xword sh_flags; 		/* Section attributes */
	Elf64_Addr sh_addr; 		/* Virtual address in memory */
	Elf64_Off sh_offset; 		/* Offset in file */
	Elf64_Xword sh_size; 		/* Size of section */
	Elf64_Word sh_link; 		/* Link to other section */
	Elf64_Word sh_info; 		/* Miscellaneous information */
	Elf64_Xword sh_addralign; 	/* Address alignment boundary */
	Elf64_Xword sh_entsize; 	/* Size of entries, if section has table */
} Elf64_Shdr;

# define SHN_UNDEF	(0x00) // Undefined/Not present
 
enum ShT_Types
{
	SHT_NULL	= 0,   // Null section
	SHT_PROGBITS	= 1,   // Program information
	SHT_SYMTAB	= 2,   // Symbol table
	SHT_STRTAB	= 3,   // String table
	SHT_RELA	= 4,   // Relocation (w/ addend)
	SHT_NOBITS	= 8,   // Not present in file
	SHT_REL		= 9    // Relocation (no addend)
};
 
enum ShT_Attributes
{
	SHF_WRITE	= 0x01, // Writable section
	SHF_ALLOC	= 0x02  // Exists in memory
};

typedef struct
{
   Elf64_Word st_name; 		/* Symbol name */
   unsigned char st_info; 	/* Type and Binding attributes */
   unsigned char st_other; 	/* Reserved */
   Elf64_Half st_shndx; 	/* Section table index */
   Elf64_Addr st_value; 	/* Symbol value */
   Elf64_Xword st_size; 	/* Size of object (e.g., common) */
} Elf64_Sym;


# define ELF32_ST_BIND(INFO)	((INFO) >> 4)
# define ELF32_ST_TYPE(INFO)	((INFO) & 0x0F)
 
enum StT_Bindings 
{
	STB_LOCAL		= 0, // Local scope
	STB_GLOBAL		= 1, // Global scope
	STB_WEAK		= 2  // Weak, (ie. __attribute__((weak)))
};
 
enum StT_Types 
{
	STT_NOTYPE		= 0, // No type
	STT_OBJECT		= 1, // Variables, arrays, etc.
	STT_FUNC		= 2  // Methods or functions
};


void *elf_load_file(void *file);




#endif
