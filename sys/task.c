#include <sys/os.h>

task tasks[MAX_PROCESSES];

typedef struct 
{
   uint32_t head;
   uint32_t tail;
   uint64_t ele[MAX_PROCESSES+1];
}queue;

queue run_queue;
queue spd_queue;
queue exit_queue;

//static task task1;
//static task task2;
//static task dummytask;
//static task maintask;
//static task usertask;


//static task task[MAX_PROCESSES];

//static task* queue;

void kill_task();

void user_main1()
{
   while (1){
   //char* buf = "Sending message from userland1";
   kprintf("Hello user\n");
   //write(1, buf, sizeof(buf));
   //__asm__ __volatile__("syscall\n\t"::);
   while(1);
   //uyield();
   }
}

void user_main2()
{
   while (1){
   //char* buf = "Sending message from userland2";
   kprintf("Hello user\n");
   //write(1, buf, sizeof(buf));
   //__asm__ __volatile__("syscall\n\t"::);
   while(1);
   //uyield();
   }
}

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
   t->state = RUNNABLE_STATE;
   t->mm_struct.start  = t->mm_struct.end = 0;
   bis(t->flags_task, KERNEL_TASK);
 
   t->kstack[510] = 0; 
   //t->next = cur_task->next;
   //cur_task->next =t;
}

uint64_t get_new_pid()
{
   int i;
   for (i = 0;i < MAX_PROCESSES;i++)
   {
       if (!bit(tasks[i].flags_task, ALLOCATED_TASK))
          break;
   }

   if (i == MAX_PROCESSES)
      ERROR("Out of processes");

   bis(tasks[i].flags_task, ALLOCATED_TASK);
   return i;
}

void create_user_task(void (*main)())
{
   task* t;
   uint64_t pid = get_new_pid();

   t = &tasks[pid];
   cur_task = t;

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
   bic(t->flags_task, KERNEL_TASK);
}
void idle()
{
   while(1)
   {
      schedule();
   }
}

void start_sbush()
{
   char* file_content;
   task* t;
   uint64_t fsize;

   create_user_task(NULL);
   t = cur_task;
   fsize = getFileFromTarfs("bin/simsh", &file_content);
   if (fsize != 0)
   {
      kprintf("Starting sbush....\n");
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

// add_vma inserts in a sorted order
void add_vma(uint64_t start, uint64_t size)
{
   task *t = cur_task;
   vma  *p = &t->mm_struct;
   vma  *c;

   while (p->next != NULL && p->next->start < start)
      p = p->next;
  
   c = (vma*)kmalloc(sizeof(vma));
   c->start = start;
   c->end = start+size;
   c->next = p->next;
   p->next = c; 
}

uint64_t get_cur_cr3()
{
   return cur_task->reg_cr3;
}

void kill_task()
{
   kprintf("Inside kill task\n");
   while(1);
   bis(cur_task->flags_task, KILL_TASK); 
}
/*
static void main1()
{
   int x1,x2,x3,x4,x5,x6;
   kprintf("Inside main1\n");

   x1 = 1;
   x2 = 2;
   x3 = 3;
   x4 = 4;
   x5 = 5;
   x6 = 6;
   while(1){
   x1++;
   x2++;
   x3++;
   x4++;
   x5++;
   x6++;
   yield();
   kprintf("Inside main1 - %d,%d,%d,%d,%d,%d\n",x1,x2,x3,x4,x5,x6);
   sleep(5);
   }
   //while(1);
}

static void main2()
{
   while(1){
   kprintf("Inside main2\n");
   yield();
   kprintf("Inside main2 - After yield()\n");
   sleep(5);
   }
}
*/
/*task* next_task()
{
    cur_task 
}*/


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

void init_task_system()
{
   int i;
   for (i = 0;i < MAX_PROCESSES;i++)
       tasks[i].pid = i;


   uint64_t pid = get_new_pid();
   task    *t   = &tasks[pid];

   create_kernel_task(t, idle);
   memcpy(t->name, "idle", 5);
   add_run_queue(t);
   //void* rsp = _get_page();
   //cur_task = &maintask; 
   //cur_task->next = &maintask; 
   //create_kernel_task(&maintask, 0, 0);

   //create_kernel_task(&task1, main1, 0);
   //create_kernel_task(&task2, main2, 0);

   //cur_task = &tasks[0]; 
   //cur_task->next = cur_task;
   //create_user_task(user_main1, 0);
   //create_user_task(user_main2, 0);
//   task1.next = &task2;
//   task2.next = &task1;
//   dummytask.next = &task1;
//   cur_task = &dummytask;
}


int first_yield = 1;
void schedule()
{
   task* me = cur_task;
   task* to;
   //task dummy_task;

   //kprintf("%d\n",first_yield++);
   //sleep(1);
   if (bit(cur_task->flags_task, KILL_TASK))
   {
      add_exit_queue(cur_task); 
   }
   else
   {
      add_run_queue(cur_task);
   }

   to = remove_run_queue();
   cur_task = to;

   if (cur_task->kstack[511] == 1234567) // HACK - To see if this is child process
   {
      cur_task->reg_rsp = (uint64_t) &cur_task->kstack[494];
   }
   set_tss_rsp((void*)&cur_task->kstack[505]);
   switch_task(me, to);
/*
   if (first_yield)
   {
      me = &dummy_task;
      cur_task = &tasks[0];
      first_yield = 0;
   }
   else
   {
      me = cur_task;
      cur_task = cur_task->next;
   }
   //task* last;
   //switch_task(me, cur_task, &last);

   set_tss_rsp((void*)cur_task->reg_rsp);
   if (cur_task->kstack[510] != 0)
      switch_task(me, cur_task);
   else
      uswitch_task(me, cur_task);
   //if (bit(last->flags_task, KILL_TASK)){
   //   kprintf("Killing a task\n");
      // Free stack page
   //}
*/
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
