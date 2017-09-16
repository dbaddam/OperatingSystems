#include <sys/defs.h>
#include <sys/idt.h>

#define MAX_IDT 256


#define P        0xF0    /* Present */
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
   uint8_t zero2;     // unused, set to 0
}__attribute__((packed));
typedef struct IDTEntry IDTEntry;

struct idtr_t {
  uint16_t size;
  uint64_t addr;
}__attribute__((packed));

 
static IDTEntry idt[MAX_IDT] = {
 {0, 8, 0, 0, 0, 0, 0}
};

static struct idtr_t idtr = {sizeof(idt), (uint64_t)idt};


void init_idt()
{
   __asm__ __volatile__("lidt %0" 
                         : 
                         : "m"(idtr)); 
}
