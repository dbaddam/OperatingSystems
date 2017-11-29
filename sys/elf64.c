#include <sys/kprintf.h>
#include <sys/defs.h>
#include <sys/elf64.h>
#include <sys/task.h>
#include <sys/util.h>

typedef int bool;
#define true  1
#define false 0





/* checking the elf header for elf magic number */
bool elf_check_file(Elf64_Ehdr *hdr)
{
   if(!hdr) return false;
   if(hdr->e_ident[EI_MAG0] != ELFMAG0)
   {
      return false;
   }
   if(hdr->e_ident[EI_MAG1] != ELFMAG1)
   {   
      return false;
   }
   if(hdr->e_ident[EI_MAG2] != ELFMAG2)
   {   
      return false;
   }
   if(hdr->e_ident[EI_MAG3] != ELFMAG3)
   {   
      return false;
   } 
   return true;
}

/* check if elf supports 64 bit x86, little endian byte order and etc */
bool elf_check_supported(Elf64_Ehdr *hdr)
{
   if(!elf_check_file(hdr)) return false;
   if(hdr->e_ident[EI_CLASS] != ELFCLASS64) {
      return false;
   }
   if(hdr->e_ident[EI_DATA] != ELFDATA2LSB) { //I doubt this
      return false;
   }
   if(hdr->e_machine != EM_X86) {
      return false;
   }
   if(hdr->e_ident[EI_VERSION] != EV_CURRENT) {
      return false;
   }
   if(hdr->e_type != ET_REL && hdr->e_type != ET_EXEC) {
      return false;
   }
   return true;
}

/* load a relocatable elf file */
static inline void *elf_load_rel(Elf64_Ehdr *hdr)
{
   int result;
   //result = elf_load_stage1(hdr);
   if(result == ELF_RELOC_ERROR) return NULL;
   //result = elf_load_stage2(hdr);
   if(result == ELF_RELOC_ERROR) return NULL;

   //TODO: parse the program header (if present)
   return (void *)hdr->e_entry;
}


void elf_copy_contents(uint64_t vaddr, uint64_t contents, uint64_t fsize, 
                       uint64_t msize, uint64_t cr3)
{
   uint64_t i,j;
   uint8_t* from = (uint8_t*)vaddr;
   uint8_t* to = (uint8_t*) contents;

   for (i = 0;i < msize; i+= PAGE_SIZE)
   {
      /* We are doomed if section addresses do not align with PAGE boundaries */
      uint64_t page = (uint64_t) _get_page();
      create_page_tables(vaddr + i, vaddr + i + PAGE_SIZE - 1,
                         PHYS_ADDR((uint64_t)page), (uint64_t*)VIRT_ADDR(cr3),
                         PG_P|PG_RW|PG_U);

      for (j = i;j < min(msize, i+PAGE_SIZE);j++)
      {
         if (j < fsize)
            from[j] = to[j];
         else
            from[j] = 0;
      }
   }
}

void get_pheaders(Elf64_Ehdr *hdr, uint64_t cr3)
{
   //no. of program header table entries
   // WARNING: Do we have to care for big-endian/little-endian conversion
   int phtEntries = hdr->e_phnum;
    
   //start program header pointer
   Elf64_Phdr *pheaders = (Elf64_Phdr *)((uint64_t)hdr + hdr->e_phoff); 
   //kprintf("phtEntries = %d\n",phtEntries);
   int i=0;
   cur_task->reg_rip = (uint64_t)hdr->e_entry;
   for(i=0;i<phtEntries;i++)
   {
      Elf64_Phdr *phdr = &pheaders[i];
      uint64_t seg_addr = (uint64_t)((uint64_t)hdr + phdr->p_offset);
      uint64_t seg_fsize  = (uint64_t)(phdr->p_filesz);
      uint64_t seg_msize = (uint64_t)(phdr->p_memsz);
      uint64_t seg_vaddr = (uint64_t)(phdr->p_vaddr);

      if (seg_msize > 0)
      {
         add_vma_file(seg_vaddr, seg_addr, seg_fsize, seg_msize);
         //elf_copy_contents(seg_vaddr, seg_addr, seg_fsize, seg_msize,cr3); 
      }
      //kprintf("i=%d",i);
      //kprintf("flags - %d\n", phdr->p_flags);
      //kprintf("segment start_addr=%p,fsz=%d,msz=%d,vaddr=%p\n",seg_addr,seg_fsize,seg_msize,seg_vaddr);  
   }
   return;
}




/* load a given file, ie from tarfs we get the start address of file content, may be 
 * that should be passed here
 */
void *elf_load_file(void *file, uint64_t cr3)
{
   Elf64_Ehdr *hdr = (Elf64_Ehdr *)file;
   if(!elf_check_supported(hdr)) return NULL;
   
   switch(hdr->e_type)
   {
      case ET_EXEC:
                  //TODO: Implement this later
                  //kprintf("It is executable elf file !! \n");
                  get_pheaders(hdr, cr3);
                  return NULL;
      case ET_REL:
                 ;
                //  return elf_load_rel(hdr);
   }
   return NULL;
}
/*
// Accessing the section headers: The section headers are continuous, given a pointer
// to the first entry next entries can be obtained by additions.
 
static inline Elf64_Shdr *elf_sheader(Elf64_Ehdr)
{
   return (Elf64_Shdr *)((int)hdr + hdr->e_shoff);
}

static inline Elf64_Shdr *elf_section(Elf64_Ehdr *hdr, int idx)
{
   return &elf_sheader(hdr)[idx];   
}



// Getting the section names, the following does section names lookup 
static inline char *elf_str_table(Elf64_Ehdr *hdr)
{
   if(hdr->e_shstrndx == SHN_UNDEF) return NULL;
   return (char *)hdr + elf_section(hdr, hdr->e_shstrndx)->sh_offset;
}

static inline char *elf_lookup_string(Elf64_Ehdr *hdr, int offset)
{
   char *strtab = elf_str_table(hdr);
   if(strtab == NULL) return NULL;
   return strtab + offset;
}

static int elf_get_symval(Elf64_Ehdr *hdr, int table, uint idx)
{
   if(table == SHN_UNDEF || idx == SHN_UNDEF) return 0;
   Elf64_Shdr *symtab = elf_section(hdr, table);

   uint64_t symtab_entries = symtab->sh_size / symtab->sh_entsize;
   if(idx >= symtab_entries) 
   {
      //ERROR("Symbol Index out of Range (%d:%u).\n", table, idx);
      return ELF_RELOC_ERR;
   }

   int symaddr = (int)hdr + symtab->sh_offset;
   Elf64_Sym *symbol = &((Elf64_Sym *)symaddr)[idx];


   if(symbol->st_shndx == SHN_UNDEF) 
   {
      // External symbol, lookup value
      Elf64_Shdr *strtab = elf_section(hdr, symtab->sh_link);
      const char *name = (const char *)hdr + strtab->sh_offset + symbol->st_name;

      extern void *elf_lookup_symbol(const char *name);
      void *target = elf_lookup_symbol(name);

	  if(target == NULL) 
	  {
	     // Extern symbol not found
             if(ELF64_ST_BIND(symbol->st_info) & STB_WEAK) 
	     {
	        // Weak symbol initialized as 0
		return 0;
             }
             else 
             {
      	        //ERROR("Undefined External Symbol : %s.\n", name);
		return ELF_RELOC_ERR;
	     }
	  } 
	  else 
	  {
	     return (int)target;
	  }

   } 
   else if(symbol->st_shndx == SHN_ABS) 
   {
      // Absolute symbol
	  return symbol->st_value;
   } 
   else 
   {
	  // Internally defined symbol
	  Elf64_Shdr *target = elf_section(hdr, symbol->st_shndx);
	  return (int)hdr + symbol->st_value + target->sh_offset;
   }
}

*/

































