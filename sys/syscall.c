#include <sys/os.h>
#include <syscall.h>
#define _BITUL(x)       (1ULL << x) 
#define __AC(X,Y)       (X##Y)                                                                                
#define _AC(X,Y)        __AC(X,Y)   

#define X86_EFLAGS_CF_BIT       0 /* Carry Flag */                                                            
#define X86_EFLAGS_CF           _BITUL(X86_EFLAGS_CF_BIT)                                                     
#define X86_EFLAGS_FIXED_BIT    1 /* Bit 1 - always on */                                                     
#define X86_EFLAGS_FIXED        _BITUL(X86_EFLAGS_FIXED_BIT)                                                  
#define X86_EFLAGS_PF_BIT       2 /* Parity Flag */                                                           
#define X86_EFLAGS_PF           _BITUL(X86_EFLAGS_PF_BIT)                                                     
#define X86_EFLAGS_AF_BIT       4 /* Auxiliary carry Flag */                                                  
#define X86_EFLAGS_AF           _BITUL(X86_EFLAGS_AF_BIT)                                                     
#define X86_EFLAGS_ZF_BIT       6 /* Zero Flag */                                                             
#define X86_EFLAGS_ZF           _BITUL(X86_EFLAGS_ZF_BIT)                                                     
#define X86_EFLAGS_SF_BIT       7 /* Sign Flag */                                                             
#define X86_EFLAGS_SF           _BITUL(X86_EFLAGS_SF_BIT)                                                     
#define X86_EFLAGS_TF_BIT       8 /* Trap Flag */                                                             
#define X86_EFLAGS_TF           _BITUL(X86_EFLAGS_TF_BIT)                                                     
#define X86_EFLAGS_IF_BIT       9 /* Interrupt Flag */                                                        
#define X86_EFLAGS_IF           _BITUL(X86_EFLAGS_IF_BIT)                                                     
#define X86_EFLAGS_DF_BIT       10 /* Direction Flag */                                                       
#define X86_EFLAGS_DF           _BITUL(X86_EFLAGS_DF_BIT)                                                     
#define X86_EFLAGS_OF_BIT       11 /* Overflow Flag */                                                        
#define X86_EFLAGS_OF           _BITUL(X86_EFLAGS_OF_BIT)                                                     
#define X86_EFLAGS_IOPL_BIT     12 /* I/O Privilege Level (2 bits) */                                         
#define X86_EFLAGS_IOPL         (_AC(3,UL) << X86_EFLAGS_IOPL_BIT)                                            
#define X86_EFLAGS_NT_BIT       14 /* Nested Task */                                                          
#define X86_EFLAGS_NT           _BITUL(X86_EFLAGS_NT_BIT)                                                     
#define X86_EFLAGS_RF_BIT       16 /* Resume Flag */                                                          
#define X86_EFLAGS_RF           _BITUL(X86_EFLAGS_RF_BIT)                                                     
#define X86_EFLAGS_VM_BIT       17 /* Virtual Mode */                                                         
#define X86_EFLAGS_VM           _BITUL(X86_EFLAGS_VM_BIT)                                                     
#define X86_EFLAGS_AC_BIT       18 /* Alignment Check/Access Control */                                       
#define X86_EFLAGS_AC           _BITUL(X86_EFLAGS_AC_BIT)               
#define X86_EFLAGS_VIF_BIT      19 /* Virtual Interrupt Flag */                                               
#define X86_EFLAGS_VIF          _BITUL(X86_EFLAGS_VIF_BIT)                                                    
#define X86_EFLAGS_VIP_BIT      20 /* Virtual Interrupt Pending */                                            
#define X86_EFLAGS_VIP          _BITUL(X86_EFLAGS_VIP_BIT)                                                    
#define X86_EFLAGS_ID_BIT       21 /* CPUID detection */                                                      
#define X86_EFLAGS_ID           _BITUL(X86_EFLAGS_ID_BIT)           

#define EFER_SCE 0x000000001    /* System Call Extensions (R/W) */                                            
#define EFER_LME 0x000000100    /* Long mode enable (R/W) */                                                  
#define EFER_LMA 0x000000400    /* Long mode active (R) */                                                    
#define EFER_NXE 0x000000800    /* PTE No-Execute bit enable (R/W) */                                         
#define EFER_SVM 0x000001000    /* SVM enable bit for AMD, reserved for Intel */                              
#define EFER_LMSLE 0x000002000  /* Long Mode Segment Limit Enable */                                          
#define EFER_FFXSR 0x000004000  /* Fast FXSAVE/FSRSTOR */                                                     
#define EFER_TCE   0x000008000  /* Translation Cache Extension */            


#define MSR_EFER                0xc0000080 /* extended feature register */
#define MSR_STAR                0xc0000081 /* legacy mode SYSCALL target */                                   
#define MSR_LSTAR               0xc0000082 /* long mode SYSCALL target */                                     
#define MSR_CSTAR               0xc0000083 /* compat mode SYSCALL target */                                   
#define MSR_SYSCALL_MASK        0xc0000084 /* EFLAGS mask for syscall */        

#define KERNEL_CS 0x08ULL
#define KERNEL_SS 0x10ULL
#define USER_CS   0x1BULL
#define USER_SS   0x23ULL


void init_syscall()
{
   uint64_t star_val = ((KERNEL_CS) <<32 | (USER_CS) << 48);
   uint64_t mask = X86_EFLAGS_TF|X86_EFLAGS_DF|X86_EFLAGS_IF|
	           X86_EFLAGS_IOPL|X86_EFLAGS_AC|X86_EFLAGS_NT;
   uint64_t efer_val; 
 
    efer_val = rdmsr(MSR_EFER);
    wrmsr(MSR_EFER, efer_val|EFER_SCE);
    wrmsr(MSR_STAR, star_val);
    wrmsr(MSR_LSTAR, (uint64_t)_syscall_entry);
    wrmsr(MSR_CSTAR, (uint64_t)_syscall_entry);
    wrmsr(MSR_SYSCALL_MASK, mask);
}

size_t sys_write(int fd, char* buf, int size)
{
   if (fd == STDOUT || fd == STDERR)
      for (int i = 0;i < size;i++)
          kprintf("%c", buf[i]);
   return size;
}


/* If we want to add a 6th parameter to this function, we have to change
 * syscall_entry.s */
uint64_t syscall_handler(uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4,
                         uint64_t p5, uint64_t sysnum)
{
   uint64_t val;
   switch(sysnum)
   {
      case __NR_write:
         return sys_write((int)p1, (char*)p2, (int)p3);
         break;
      case __NR_sched_yield:
         schedule(); 
         break;
      case __NR_fork:
         val = fork();
         if (cur_task->kstack[511] == 1234567)
         {
            cur_task->kstack[511] = 0;
            return 0;
         }
         return val; 
         break;
      case __NR_execve:
         execve((char*)p1, (char**)p2, (char**)p3); 
         break;
      case __NR_exit:
         exit(p1);
         break;
      case __NR_wait4:
         return (uint64_t) waitpid((int32_t)p1, (int32_t*)p2);
         break;
      case __NR_getcwd:
         return (uint64_t) getcwd((char*)p1, (uint32_t) p2);
         break;
      case __NR_chdir:
         return (uint64_t) chdir((char*)p1);
         break;
      case __NR_getpid:
         return (uint64_t) getpid();
         break;
      case __NR_getppid:
         return (uint64_t) getppid();
         break;
      case __NR_open:
         return (uint64_t) open((char*)p1, (int32_t)p2);
         break;
      case __NR_read:
         return (uint64_t) read((uint32_t)p1, (char*)p2, (uint64_t)p3);
         break;
      case __NR_lseek:
         return (uint64_t) lseek((uint32_t)p1, (uint64_t)p2);
         break;
      case __NR_close:
         return (uint64_t) close((int32_t)p1);
         break;
      case __NR_opendirt:
         return (uint64_t) opendir_tarfs((char*)p1, (int32_t)p2);
         break;
      case __NR_readdirt:
         return (uint64_t) readdir_tarfs((uint32_t)p1, (char*)p2);
         break;
      case __NR_closedirt:
         return (uint64_t) closedir_tarfs((int32_t)p1);
         break;
      case __NR_sleep:
         return (uint64_t) sleep((uint32_t)p1);
         break;
      case __NR_sbrk:
         return (uint64_t) sbrk((int32_t)p1);
         break;
      case __NR_access:
         return (uint64_t) access((char*)p1);
         break;
      default:
         ERROR("Unknown syscall - %d\n",sysnum);
   }
   return 0; 
}
