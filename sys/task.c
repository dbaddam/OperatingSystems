#include <sys/os.h>

#define TOP_PROCESS         1
#define INVALID_PID         2048
#define MAX_STACK_PAGES     (1 << 15)
#define INTERPRETER_MAX_LEN 127

#define NOEXEC_FILE         0
#define ELF_FILE            1
#define INTERPRETER_FILE    2
task tasks[MAX_PROCESSES];

char* state_str[10] = {"INVALID", 
                       "READY  ",
                       "WAIT   ",
                       "SUSPEND", 
                       "ZOMBIE ",
                       "AVAIL  ",
                       "COOK   "};

uint32_t run_head;

void kill_task();
void create_first_user_task(void (*main)());
void create_kernel_task(task* t, void (*main)());

uint64_t get_cur_cr3();
uint64_t get_new_pid();

void init_task_system();
void idle();
void wait_forever();
void start_sbush();

void copy_vmas(vma* dst, vma* src);
void add_vma(uint64_t start, uint64_t size, uint64_t fstart, 
             uint64_t fsize, uint64_t msize, uint64_t anon);
void free_vmas(task* t);
void add_heap_vma();

void schedule();
void mark_children_zombie(uint32_t pid);
void destroy_process(task* t, uint32_t free_pcb);

void create_kernel_task(task* t, void (*main)())
{
   t->reg_r15 = 0;
   t->reg_r14 = 0;
   t->reg_r13 = 0;
   t->reg_r12 = 0;
   t->reg_rbp = 0;
   t->reg_rsp = ((uint64_t)&(t->kstack[510]));
   t->reg_rbx = 0;
   t->reg_rip = (uint64_t) main;
   //t->reg_rflags = flags;
   t->reg_cr3 = (uint64_t) PHYS_ADDR((uint64_t)kernel_pml4);
   t->state = RUNNING_STATE;
   t->mm_struct.start  = t->mm_struct.end = 0;
 
   t->kstack[510] = 0; 
}

uint64_t get_new_pid()
{
   int i;
   for (i = 0;i < MAX_PROCESSES;i++)
   {
       if (tasks[i].state == AVAIL_STATE)
          break;
   }

   tasks[i].state = COOK_STATE;
   if (i == MAX_PROCESSES)
      ERROR("Out of processes");

   return i;
}

void create_first_user_task(void (*main)())
{
   task* t;
   uint64_t pid = get_new_pid();

   t = &tasks[pid];
   cur_task = t;
   t->ppid = TOP_PROCESS;

   create_kernel_task(t, main);
   t->reg_cr3 = (uint64_t)init_user_pt();
   __asm__ __volatile__("movq %0, %%cr3"
                        :
                        : "r"(t->reg_cr3));
   strcpy(t->pwd, "/");
   /* 
   for (i = 0;i < INITIAL_STACK_PAGES;i++)
   { 
      uint64_t page = (uint64_t )_get_page(); 
      //t->ustack = (uint8_t*) USER_STACK_TOP;
      create_page_tables(USER_STACK_TOP - (i+1)*PAGE_SIZE, 
                         USER_STACK_TOP + i*PAGE_SIZE - 1,
                         PHYS_ADDR((uint64_t)page), 
                         (uint64_t*)VIRT_ADDR(t->reg_cr3),
                         PG_P|PG_RW|PG_U);
   }

   t->reg_ursp = ((uint64_t)USER_STACK_TOP)- 40;
   add_vma(USER_STACK_TOP - INITIAL_STACK_PAGES*PAGE_SIZE, INITIAL_STACK_PAGES*PAGE_SIZE);
   */
}

// Idle thread
void idle()
{
   while(1)
   {
      __asm__ __volatile("sti");
      __asm__ __volatile("hlt");
      __asm__ __volatile("cli");
      schedule();
   }
}

// Wait forever
void wait_forever()
{
   int status;
   while(1)
   {
      wait(&status);
   }
}

int64_t load_process_test(char* filename, char* interpreter)
{
   char* file_content;
   uint64_t fsize;

   fsize = getFileFromTarfs(filename, &file_content);
   if (fsize >= 0) 
   {
      if (is_elf(file_content))
      {
         return ELF_FILE;
      }
      else
      {
         int fd = open(filename, 0);
         int rcount;
         int i;

         if (fd < 0)
            return NOEXEC_FILE;
         rcount = read(fd, interpreter, INTERPRETER_MAX_LEN);

         if (rcount > 2 &&
             strcmp(interpreter, "#! "))
         {
            for (i = 0;i < rcount;i++)
            {
               if (interpreter[i] == '\n')
                  interpreter[i] = '\0';
            }

            if (getFileFromTarfs(interpreter+3, &file_content) > 0)
               return INTERPRETER_FILE;
         }
         else
         {
            return NOEXEC_FILE;
         }
         close(fd); 
      }
   }
   else
   {
      return NOEXEC_FILE;
   }

   return NOEXEC_FILE;    
}

int64_t load_process(char* filename)
{
   char* file_content;
   task* t;
   uint64_t fsize;

   t = cur_task;
   fsize = getFileFromTarfs(filename, &file_content);
   if (fsize >= 0)
   {
      elf_load_file(file_content, t->reg_cr3);
   }
   else
   {
      return -1;
   }

   add_heap_vma();
   set_tss_rsp((void*)&t->kstack[511]);
   __asm__ __volatile__(
                        "movq %%cr3, %%rax\n\t" // Flush TLB
                        "movq %%rax, %%cr3\n\t"
                        "pushq $0x23\n\t"
                        "pushq %0\n\t"
                        "pushq $0x200\n\t"
                        "pushq $0x2B\n\t"
                        "pushq %1\n\t"
                        "iretq"
                         : 
                         : "g"(t->reg_ursp), "g"(t->reg_rip)
                         : "rax", "memory");

   return 0;
   
}

// Spawns the initial shell
void start_sbush()
{
   char* argv[] = {"bin/sbush", 0};
   char* envp[] = {"PATH=/bin", "HOME=/", 0};

   //mem_info();
   create_first_user_task(NULL);
   kprintf("Starting sbush....\n");
   execve("bin/sbush", argv, envp);
}

uint32_t first_load = 1;
uint64_t execve(char* filename, char* argv[], char* envp[])
{
   char      fname[256];
   char      interpreter[INTERPRETER_MAX_LEN];
   char*     iptr = interpreter + 3;
   int       i, len;
   int       argc = 0;
   int       envc = 0;
   uint64_t *ptr;
   uint64_t  page;
   uint64_t  start;
   int64_t  ftype;

   //mem_info();
   strncpy(fname, filename, 256); 

   ftype = load_process_test(fname, interpreter);
   if (ftype == NOEXEC_FILE)
   {
      return -1;
   }

   /* Copy all the arguments since we'll be destroying everything */
   page = (uint64_t)_get_page();
   start = page + PAGE_SIZE - 40;

   // Push all the envp content
   if (envp)
   {

      for (envc = 0;envp[envc];envc++);

      for (i = envc-1;i >= 0;i--)
      {
          len = strlen(envp[i]);
          start -= len + 1;
          strcpy((char*)start, envp[i]);
      }
   }

   // Push all the argv content
   if (argv)
   {
      for (argc = 0;argv[argc];argc++);

      for (i = argc-1;i >= 0;i--)
      {
          len = strlen(argv[i]);
          start -= len + 1;
          strcpy((char*)start, argv[i]);
      }
   }

   if (ftype == INTERPRETER_FILE)
   {
      len = strlen(fname);
      start -= len + 1;
      strcpy((char*)start, fname);
   
      len = strlen(iptr);
      start -= len + 1;
      strcpy((char*)start, iptr);   
   }

   // Align the ptr with 8 byte addr
   ptr = (uint64_t*)((start - 16) - ((start-16)%8));

   start = USER_STACK_TOP - 40;

   // Push 0 to denoted the end of envp
   *ptr = 0;
   ptr--;

   // Push all the env pointers
   if (envp)
   {
      for (i = envc-1;i >= 0;i--)
      {
          len = strlen(envp[i]);
          start -= len + 1;
          *ptr = start;
          ptr--;
      }
   }

   // Push 0 to denoted the end of argv
   *ptr = 0;
   ptr--;
    
   // Push all the argv pointers
   if (argv)
   {
      for (i = argc-1;i >= 0;i--)
      {
          len = strlen(argv[i]);
          start -= len + 1;
          *ptr = start;
           ptr--;
      }
   }
  
   if (ftype == INTERPRETER_FILE)
   {
      len = strlen(fname);
      start -= len + 1;
      *ptr = start;
      ptr--;
   
      len = strlen(iptr);
      start -= len + 1;
      *ptr = start;
      ptr--;
   }

   if (ftype == INTERPRETER_FILE)
   {
      *ptr = argc + 2;
      strcpy(fname, iptr);
   }
   else
      *ptr = argc;

   // The file name to execute now is the interpreter

   if (!first_load)
   {
      destroy_process(cur_task, 0/* pcb,cr3 also */); 
      flush_tlb();
   }
   else
   {
      first_load = 0;
   }

   //mem_info();
   //cur_task->reg_ursp = ((uint64_t)USER_STACK_TOP)- 40;
   // (ptr-page) denotes the size occupied by the contents
   cur_task->reg_ursp = ((uint64_t)USER_STACK_TOP) - (page + PAGE_SIZE - ((uint64_t)ptr));
   //kprintf("usp - %p\n", cur_task->reg_ursp);
   create_page_tables(USER_STACK_TOP - PAGE_SIZE, USER_STACK_TOP - 1,
                         PHYS_ADDR((uint64_t)page), (uint64_t*)VIRT_ADDR(cur_task->reg_cr3),
                         PG_P|PG_RW|PG_U);
   strncpy(cur_task->name, fname, 256); 
   add_vma_anon(USER_STACK_TOP - MAX_STACK_PAGES*PAGE_SIZE, MAX_STACK_PAGES*PAGE_SIZE);

   return load_process(fname);
}

uint64_t get_cur_cr3()
{
   return cur_task->reg_cr3;
}

void add_run_queue(task* t)
{
   t->state = RUNNING_STATE;
}
 
void add_wait_queue(task* t)
{
   t->state = WAITING_STATE;
}

void add_suspend_queue(task* t)
{
   t->state = SUSPEND_STATE;
}

void add_avail_queue(task* t)
{
   t->state = AVAIL_STATE;
}

task* next_running_task()
{
   uint32_t cur = run_head;

   for (run_head = cur;run_head < MAX_PROCESSES;run_head++)
   {
       if (tasks[run_head].state == RUNNING_STATE)
       {
          uint32_t temp = run_head;
          run_head = (run_head + 1)%MAX_PROCESSES;
          return &tasks[temp]; 
       }
   }

   for (run_head = 0;run_head < cur;run_head++)
   {
       if (tasks[run_head].state == RUNNING_STATE)
       {
          uint32_t temp = run_head;
          run_head = (run_head + 1)%MAX_PROCESSES;
          return &tasks[temp]; 
       }
   }

   ERROR("No running tasks..Should not be here");
   return NULL;
}

void init_task_system()
{
   int i;
   for (i = 0;i < MAX_PROCESSES;i++)
   {
       tasks[i].pid = i;
       tasks[i].state = AVAIL_STATE;
       bis(tasks[i].file[STDIN].flags, ALLOCATED_FD);
       bis(tasks[i].file[STDOUT].flags, ALLOCATED_FD);
       bis(tasks[i].file[STDERR].flags, ALLOCATED_FD);
   }


   uint64_t pid;
   task    *t;

   /* Add idle task */
   pid = get_new_pid();
   t = &tasks[pid];
   create_kernel_task(t, idle);
   strncpy(t->name, "idle", MAX_FILE_NAME_SIZE);
   add_run_queue(t);

   /* Add forever wait task */
   pid = get_new_pid();
   t = &tasks[pid];
   create_kernel_task(t, wait_forever);
   strncpy(t->name, "wait_forever", MAX_FILE_NAME_SIZE);
   add_run_queue(t);
   cur_task = t;
}


void schedule()
{
   task* me = cur_task;
   task* to;

   //sleep(1);

   to = next_running_task();
   cur_task = to;

   //kprintf("Scheduling task %d\n", (int)to->pid);
   if (cur_task->kstack[511] == 1234567) // HACK - To see if this is child process
   {
      cur_task->reg_rsp = (uint64_t) &cur_task->kstack[494];
   }
   set_tss_rsp((void*)&cur_task->kstack[505]);
   switch_task(me, to);
}

void copy_vmas(vma* dst, vma* src)
{
   vma *c;
   vma *d_c = dst;
   while (src != NULL)
   {
      c = (vma*)_get_page();
      c->start = src->start;
      c->end = src->end; 
      d_c->next = c;
      d_c = d_c->next;
      src = src->next;
   }

   d_c->next = NULL;
}

// add_vma inserts in a sorted order
void add_vma_int(uint64_t start, uint64_t size, uint64_t fstart, 
                 uint64_t fsize, uint64_t msize, uint64_t anon)
{
   task *t = cur_task;
   vma  *p = &t->mm_struct;
   vma  *c;

   while (p->next != NULL && p->next->start < start)
      p = p->next;
  
   c = (vma*)_get_page();
   c->start  = start;
   c->end    = start+size;
   c->anon   = anon;
   c->node.fstart = fstart;
   c->node.fsize  = fsize;
   c->node.msize  = msize;

   c->next   = p->next;
   p->next   = c; 
}

void add_vma_anon(uint64_t start, uint64_t size)
{
   add_vma_int(start, size, 0, 0, 0, 1);
}

void add_vma_file(uint64_t vaddr, uint64_t fstart, 
                  uint64_t fsize, uint64_t msize)
{
   add_vma_int(vaddr, msize, fstart, fsize, msize, 0);
}

void free_vmas(task* t)
{
   vma  *p = &t->mm_struct;
   vma  *c;

   while (p->next != NULL)
   {
      c = p->next;
      p->next = p->next->next;
      _free_page(c);
   }
}

void print_vmas()
{
   task* t = cur_task;
   vma  *p = &t->mm_struct;

   p = p->next;
   while (p != NULL)
   {
      kprintf("(%p,%p) ",p->start, p->end);
      p = p->next;
   }
   kprintf("\n");
}

void add_heap_vma()
{
   task *t = cur_task;
   vma  *p = &t->mm_struct;
   uint64_t heap_start;

   while (p->next->next != NULL)
      p = p->next;

   heap_start = ((p->end/PAGE_SIZE) * PAGE_SIZE) + 100*PAGE_SIZE;
   add_vma_anon(heap_start, 0);
   bis(p->next->flags, HEAP_VMA);
}

uint64_t fork()
{
   task* volatile c;
   task* volatile p;
   uint64_t volatile cpid = get_new_pid();

   p = cur_task; 
   c = &tasks[cpid];

   memcpy((char*)c, (char*)p, sizeof(task));
   c->pid = cpid;
   c->ppid = p->pid;
   copy_vmas(&c->mm_struct, &p->mm_struct);

   //mem_info();
   c->reg_cr3 = copy_page_tables(c, p);
   c->kstack[511] = 1234567;
   flush_tlb(); 
   
   add_run_queue(c);

   save_child_state(p, c);

   if (cur_task->pid == p->pid)
      return c->pid;
      
   return 0; 
}

void free_fds(task* t)
{
   int i;

   for (i = 3;i < MAX_FILES;i++)
      t->file[i].flags = 0;
}

void destroy_process(task* t, uint32_t free_pcb)
{
   destroy_address_space(t);
   free_vmas(t);
   t->sleep_time = 0;
   free_fds(t);

   if (free_pcb)
   {
      _free_page((void*)VIRT_ADDR(t->reg_cr3));
      add_avail_queue(t);
   }
}

void update_children_pid(uint32_t pid)
{
   int i;

   for (i = 0;i < MAX_PROCESSES;i++)
   {
       if (tasks[i].state != AVAIL_STATE &&
           tasks[i].ppid == pid)
       {
          //tasks[i].state = ZOMBIE_STATE;
          tasks[i].ppid = TOP_PROCESS;
          //mark_children_zombie(i);
       }
   }
     
   // Does this make sense here?
   //add_run_queue(&tasks[TOP_PROCESS]); 
}

void exit_int(task*t, uint32_t status)
{
   t->state = ZOMBIE_STATE;
   t->exit_status = status;
   update_children_pid(t->pid);

   if (tasks[t->ppid].state == WAITING_STATE)
      add_run_queue(&tasks[t->ppid]);

   schedule();
}

void exit(uint32_t status)
{
   exit_int(cur_task, status);
}

uint32_t wait(int32_t *status)
{
   task* t = cur_task;
   uint32_t i;

   for (i = 0;i < MAX_PROCESSES;i++)
   {
      if (tasks[i].ppid == t->pid &&
          tasks[i].state == ZOMBIE_STATE)
      {
         if (status != NULL)
            *status = tasks[i].exit_status;
         destroy_process(&tasks[i], 1);
         return i;
      }
   }

   add_wait_queue(cur_task);
   schedule();

   for (i = 0;i < MAX_PROCESSES;i++)
   {
      if (tasks[i].ppid == t->pid &&
          tasks[i].state == ZOMBIE_STATE)
      {
         if (status != NULL)
            *status = tasks[i].exit_status;
         destroy_process(&tasks[i], 1);
         return i;
      }
   }

   return -1;
}

uint32_t waitpid(int32_t pid, int* status)
{
   task* t = cur_task;

   if (pid == -1)
   {
      return wait(status);
   }

   if (pid < 0)
      pid *= -1;

   if (pid >= MAX_PROCESSES           ||
       tasks[pid].state == AVAIL_STATE ||
       tasks[pid].ppid != t->pid)
      return -1;

   while (1)
   { 
      if (tasks[pid].state == ZOMBIE_STATE)
      {
         if (status != NULL)
            *status = tasks[pid].exit_status;
         destroy_process(&tasks[pid], 1);
         return pid;
      }
      add_wait_queue(cur_task);
      schedule();
   }

   return -1;
}

char* getcwd(char* buf, uint32_t size)
{
   task* t = cur_task;

   if (strlen(t->pwd) >= size)
      return NULL;

   strcpy(buf, t->pwd);
   return buf;
}

int32_t chdir(char* path)
{
   task* t = cur_task;
   char spath[256];
   if (path == NULL)
      return -1;


   if (strcmp(path, "/") == 0)
   {
      strcpy(t->pwd, "/");
      return 0;
   }

   /* TODOKISHAN - Add / at the beginning */
   sanitize_path(path,spath+1);
   if (is_directory(spath+1) > 0)
   {
      spath[0] = '/';
      strcpy(t->pwd, spath);
      return 0;
   }
   else
   {
      return -1;
   } 
}

uint32_t getpid()
{
   return cur_task->pid;
}

uint32_t getppid()
{
   return cur_task->ppid;
}

uint32_t sleep(uint32_t secs)
{
   task* t = cur_task;

   if (secs > 0)
   {
      t->sleep_time = secs;
      add_suspend_queue(t);
      schedule();
   }
   return 0; 
}

void decrement_sleep()
{
   int i;
   for (i = 0;i < MAX_PROCESSES;i++)
   {
       if (tasks[i].state == SUSPEND_STATE &&
           tasks[i].sleep_time  > 0)
       {
          tasks[i].sleep_time--;
          if (tasks[i].sleep_time == 0)
             add_run_queue(&tasks[i]);
       }
   }
}

void wakeup_read()
{
   int i;
   for (i = 0;i < MAX_PROCESSES;i++)
   {
       if (tasks[i].state == SUSPEND_STATE &&
           tasks[i].sleep_time == 0)
       {
          add_run_queue(&tasks[i]);
       }
   }
}

uint64_t sbrk(int32_t incr)
{
   task *t = cur_task;
   vma  *p = &t->mm_struct;

   p = p->next;              //Skip dummy vma
   while (p != NULL)
   {
      if (bit(p->flags,HEAP_VMA))
      {
         if (incr >= 0)
         {
            p->end += incr;
            return p->end;
         }
         else
         {
            ERROR("Negative sbrk");
         }
      }
      p = p->next;
   }
 
   return -1; 
}

int32_t access(char* fname)
{
  char spath[256];
  sanitize_path(fname,spath);
  if (is_file(spath) > 0)
  {
     return 0;
  }
  else
  {
     return -1;
  }
}

char* state_to_str(uint32_t type)
{
   if (type >= COOK_STATE)
      return state_str[0];
   return state_str[type];
}

void ps()
{
   int i;

   kprintf("PID  PPID  STATE  CMD\n");
   for (i = 0;i < MAX_PROCESSES;i++)
   {
      if (tasks[i].state != AVAIL_STATE)
      {
         kprintf("%d %d %s %s\n", tasks[i].pid, tasks[i].ppid, state_to_str(tasks[i].state), tasks[i].name);
      }
   }
}

void kill(int32_t pid)
{
   if (pid == 0 || pid == 1)
   {
      kprintf("forever running process\n");
      return;
   }

   if (pid < 0 || pid >= MAX_PROCESSES ||
       tasks[pid].state == AVAIL_STATE)
   {
      kprintf("No such process - %d\n", pid);
   }

   exit_int(&tasks[pid], 1234);
}
