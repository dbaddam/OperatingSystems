#include <sys/os.h>

static task task1;
static task task2;
//static task dummytask;
static task maintask;
static task usertask;


//static task task[MAX_PROCESSES];

//static task* queue;

void kill_task();

void user_main()
{
   kprintf("Hello user\n");
   __asm__ __volatile__("syscall\n\t"::);
   kprintf("Hello user 2\n");
   
   while(1);
}

void create_kernel_task(task* t, void (*main)(), uint64_t flags)
{
   uint64_t* st;
   t->stack   = (uint8_t*)_get_page(); 
   t->reg_r15 = 0;
   t->reg_r14 = 0;
   t->reg_r13 = 0;
   t->reg_r12 = 0;
   t->reg_rbp = 0;
   t->reg_rsp = ((uint64_t)t->stack)+ 0x1000 - 32;
   t->reg_rbx = 0;
   t->reg_rip = (uint64_t) main;
   //t->reg_rflags = flags;
   t->reg_cr3 = (uint64_t) kernel_pml4;
   bis(t->flags_task, KERNEL_TASK);
  
   st = (uint64_t*) t->stack;
   st[511] = (uint64_t)0x0123456789abcdefULL;
   st[510] = (uint64_t)0x0011223344556677ULL;
   st[509] = (uint64_t)0x0001112223334445ULL;
   st[508] = (uint64_t)0xaaaaaaaaaaaaaaaaULL;
   st[507] = (uint64_t)0xbbbbbbbbbbbbbbbbULL;
   st[506] = (uint64_t)0xccccccccccccccccULL;
   st[505] = (uint64_t)0xddddddddddddddddULL;
   st[504] = (uint64_t)0xeeeeeeeeeeeeeeeeULL;
   //st[510] = (uint64_t)kill_task;
   //st[509] = (uint64_t)kill_task;
   //st[510] = (uint64_t)1234;
   t->next = cur_task->next;
   cur_task->next =t;
}

void create_user_task(task* t, void (*main)(), uint64_t flags)
{
   //uint64_t* pml4 = create_user_page_tables();
   create_kernel_task(t, main, flags);
   //t->reg_cr3 = (uint64_t)pml4;
   //t->reg_rip = t->reg_rip;//%(1<<30);
   bic(t->flags_task, KERNEL_TASK);
}

void kill_task()
{
   kprintf("Inside kill task\n");
   while(1);
   bis(cur_task->flags_task, KILL_TASK); 
}

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

void init_task_system()
{
   void* rsp = _get_page();
   set_tss_rsp(rsp);
   cur_task = &maintask; 
   cur_task->next = &maintask; 
   create_kernel_task(&maintask, 0, 0);

   create_kernel_task(&task1, main1, 0);
   create_kernel_task(&task2, main2, 0);
   kprintf("user_main - %p\n", user_main);
   create_user_task(&usertask, user_main, 0);
//   task1.next = &task2;
//   task2.next = &task1;
//   dummytask.next = &task1;
//   cur_task = &dummytask;
}

void yield()
{
   task* me = cur_task;
   //task* last;
   cur_task = cur_task->next;
   //switch_task(me, cur_task, &last);
   if (bit(cur_task->flags_task, KERNEL_TASK))
      switch_task(me, cur_task);
   else
      uswitch_task(me, cur_task);
   //if (bit(last->flags_task, KILL_TASK)){
   //   kprintf("Killing a task\n");
      // Free stack page
   //}
}
