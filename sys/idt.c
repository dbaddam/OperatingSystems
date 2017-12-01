#include <sys/defs.h>
#include <sys/idt.h>
#include <sys/pic.h>
#include <sys/kprintf.h>

#define MAX_IDT 256


#define P        0x80    /* Present */
#define NP       0x00    /* Not Present */
#define DPL0     0x00
#define S0       0x00
#define S1       0x10
#define GTASK32  0x05
#define GINT16   0x06
#define GTRAP16  0x07
#define GINT32   0x0E
#define GTRAP32  0x0F

/*
   selector    -  [ 0 - 1] - Requested privilege level
                  [ 2 - 2] - Table Index 0-GDT, 1- IDT 
                  [ 3 -15] - Index into the table

   type_attr   -  [ 0 - 3] - GateType
                  [ 4 - 4] - Storage Segment
                  [ 5 - 6] - Descriptor Privelege Level
                  [ 7 - 7] - Present
    
*/
struct IDTEntry{
   uint16_t offset_1; // offset bits 0..15
   uint16_t selector; // a code segment selector in GDT or LDT
   uint8_t zero1;     // unused, set to 0
   uint8_t type_attr; // type and attributes
   uint16_t offset_2; // offset bits 16..31
   uint32_t offset_3; // offset bits 32..63
   uint32_t zero2;     // unused, set to 0
}__attribute__((packed));
typedef struct IDTEntry IDTEntry;

struct idtr_t {
  uint16_t size;
  uint64_t addr;
}__attribute__((packed));

 
static IDTEntry idt[MAX_IDT];

static struct idtr_t idtr = {sizeof(idt)-1, (uint64_t)idt};


void _x86_64_asm_lidt(struct idtr_t *idtr);

void dummyitr(uint64_t* inum)
{
    ERROR("dummyitr - %p\n", *inum);
}

void init_idt()
{

   int i;
   for (i = 0;i < MAX_IDT;i++)
   {
       register_irq(i, (uint64_t) &dummyitr);
   }
   register_all_irqs();
  //_x86_64_asm_lidt(&idtr);
   __asm__ __volatile__("cli\n\t"
                        "lidt (%0)\n\t"
                        "sti" 
                         : 
                         : "r"((uint64_t)(&idtr)));
}

void register_irq(unsigned char num, uint64_t fp)
{
    idt[num].offset_1 = (((uint64_t)fp) & (0x000000000000FFFF));
    idt[num].selector = 8;
    idt[num].zero1 = 0;
    idt[num].type_attr = P|S0|DPL0|GINT32;
    idt[num].offset_2 = (((uint64_t)fp>>16) & (0x000000000000FFFF));
    idt[num].offset_3 = (((uint64_t)fp>>32) & (0x00000000FFFFFFFF));
    idt[num].zero2 = 0;
}

void register_all_irqs()
{
   register_irq(MASTER_PIC_OFFSET + TIMER_IRQ, (uint64_t)_isr_timer);
   //isr_timer_init();
   register_irq(MASTER_PIC_OFFSET + KEYBOARD_IRQ, (uint64_t)_isr_keyboard);
   register_irq(PAGE_FAULT_IRQ, (uint64_t) _isr_page_fault);
}

