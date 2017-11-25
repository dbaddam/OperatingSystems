#include <sys/os.h>

#define TOP_PROCESS 1
#define INVALID_PID 2048

task tasks[MAX_PROCESSES];

/*typedef struct 
{
   uint32_t head;
   uint32_t tail;
   uint64_t ele[MAX_PROCESSES+1];
}queue;
*/


uint32_t run_head;

/*
queue run_queue;
queue spd_queue;
queue exit_queue;
*/

void kill_task();
void create_first_user_task(void (*main)());
void create_kernel_task(task* t, void (*main)());

uint64_t get_cur_cr3();
uint64_t get_new_pid();

/*
void enqueue(queue* q, uint64_t num);
uint64_t dequeue(queue* q);
void add_run_queue(task* t);
task* remove_run_queue();
void add_spd_queue(task* t);
task* remove_spd_queue();
void add_exit_queue(task* t);
task* remove_exit_queue();
*/
void add_run_queue(task* t);
void add_wait_queue(task* t);
void add_avail_queue(task* t);

void init_task_system();
void idle();
void wait_forever();
void start_sbush();

void copy_vmas(vma* dst, vma* src);
void add_vma(uint64_t start, uint64_t size);
void free_vmas(task* t);

void schedule();
void mark_children_zombie(uint32_t pid);

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
   //t->next = cur_task->next;
   //cur_task->next =t;
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
  
   uint64_t page = (uint64_t )_get_page(); 
   //t->ustack = (uint8_t*) USER_STACK_TOP;
   t->reg_ursp = ((uint64_t)USER_STACK_TOP)- 40;
   create_page_tables(USER_STACK_TOP - PAGE_SIZE, USER_STACK_TOP - 1,
                         PHYS_ADDR((uint64_t)page), (uint64_t*)VIRT_ADDR(t->reg_cr3),
                         PG_P|PG_RW|PG_U);
   add_vma(USER_STACK_TOP - PAGE_SIZE, PAGE_SIZE);
}

// Idle thread
void idle()
{
   while(1)
   {
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

void load_process(char* filename)
{
   char* file_content;
   task* t;
   uint64_t fsize;

   t = cur_task;
   fsize = getFileFromTarfs(filename, &file_content);
   if (fsize > 0)
   {
      elf_load_file(file_content, t->reg_cr3);
   }

   set_tss_rsp((void*)&t->kstack[511]);
   __asm__ __volatile__(
                        "movq %%cr3, %%rax\n\t" // Flush TLB
                        "movq %%rax, %%cr3\n\t"
                        "pushq $0x23\n\t"
                        "pushq %0\n\t"
                        "pushf\n\t"
                        "pushq $0x2B\n\t"
                        "pushq %1\n\t"
                        "iretq"
                         : 
                         : "g"(t->reg_ursp), "g"(t->reg_rip)
                         : "rax", "memory");
   
}

// Spawns the initial shell
void start_sbush()
{
   mem_info();
   create_first_user_task(NULL);
   kprintf("Starting sbush....\n");
   load_process("bin/simsh");
}

uint64_t execve(char* filename, char* argv[], char* envp[])
{
   char fname[256];

   mem_info();
   strncpy(fname, filename, 256); 
   destroy_address_space(cur_task);
   free_vmas(cur_task);
   flush_tlb();
   mem_info();
   uint64_t page = (uint64_t )_get_page(); 
   cur_task->reg_ursp = ((uint64_t)USER_STACK_TOP)- 40;
   create_page_tables(USER_STACK_TOP - PAGE_SIZE, USER_STACK_TOP - 1,
                         PHYS_ADDR((uint64_t)page), (uint64_t*)VIRT_ADDR(cur_task->reg_cr3),
                         PG_P|PG_RW|PG_U);
   add_vma(USER_STACK_TOP - PAGE_SIZE, PAGE_SIZE);
   load_process(fname);

   return 0;
}

uint64_t get_cur_cr3()
{
   return cur_task->reg_cr3;
}

/*
void enqueue(queue* q, uint64_t num)
{
   q->ele[q->tail] = num;
   q->tail = (q->tail + 1)%(MAX_PROCESSES+1);

   if (q->head == q->tail)
   {
      ERROR("enqueue full - q - %p, head - %d, tail - %d\n",q, q->head, q->tail);
   }
}

uint64_t dequeue(queue* q)
{
   if (q->head == q->tail)
   {
      ERROR("dequeue empty - q - %p, head - %d, tail - %d\n",q, q->head, q->tail);
   }

   uint64_t num = q->ele[q->head];
   q->head = (q->head + 1)%(MAX_PROCESSES+1);

   return num;
}

void add_run_queue(task* t)
{
   t->state = RUNNING_STATE;
   enqueue(&run_queue, t->pid);
} 

task* remove_run_queue()
{
   uint64_t num = dequeue(&run_queue);
   return &tasks[num];
}
 
void add_spd_queue(task* t)
{
   enqueue(&spd_queue, t->pid);
} 

task* remove_spd_queue()
{
   uint64_t num = dequeue(&spd_queue);
   return &tasks[num];
}

void add_exit_queue(task* t)
{
   enqueue(&exit_queue, t->pid);
} 

task* remove_exit_queue()
{
   uint64_t num = dequeue(&exit_queue);
   return &tasks[num];
}

*/

void add_run_queue(task* t)
{
   t->state = RUNNING_STATE;
}
 
void add_wait_queue(task* t)
{
   t->state = WAITING_STATE;
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
}

void init_task_system()
{
   int i;
   for (i = 0;i < MAX_PROCESSES;i++)
   {
       tasks[i].pid = i;
       tasks[i].state = AVAIL_STATE;
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
}


void schedule()
{
   task* me = cur_task;
   task* to;

   //sleep(1);

   to = next_running_task();
   cur_task = to;

   kprintf("Scheduling task %d\n", (int)to->pid);
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
void add_vma(uint64_t start, uint64_t size)
{
   task *t = cur_task;
   vma  *p = &t->mm_struct;
   vma  *c;

   while (p->next != NULL && p->next->start < start)
      p = p->next;
  
   c = (vma*)_get_page();
   c->start = start;
   c->end = start+size;
   c->next = p->next;
   p->next = c; 
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

void destroy_process(task* t)
{
   destroy_address_space(t);
   free_vmas(t);
   _free_page((void*)VIRT_ADDR(t->reg_cr3));
   add_avail_queue(t);
}

void mark_children_zombie(uint32_t pid)
{
   int i;

   for (i = 0;i < MAX_PROCESSES;i++)
   {
       if (tasks[i].state != AVAIL_STATE &&
           tasks[i].ppid == pid)
       {
          tasks[i].state = ZOMBIE_STATE;
          tasks[i].pid = TOP_PROCESS;
          mark_children_zombie(i);
       }
   }
     
   // Does this make sense here?
   add_run_queue(&tasks[TOP_PROCESS]); 
}

void exit(uint32_t status)
{
   task* t = cur_task;

   t->state = ZOMBIE_STATE;
   t->exit_status = status;
   mark_children_zombie(t->pid);

   if (tasks[t->ppid].state == WAITING_STATE)
      add_run_queue(&tasks[t->ppid]);

   schedule();
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
         *status = tasks[i].exit_status;
         destroy_process(&tasks[i]);
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
         *status = tasks[i].exit_status;
         destroy_process(&tasks[i]);
         return i;
      }
   }

   return INVALID_PID;
}
