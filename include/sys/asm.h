#ifndef _ASM_H
#define _ASM_H

#include <sys/defs.h>

#define LOW32(x) (x & 0xFFFFFFFF)
#define HI32(x)  (x>>32)

static __inline void wrmsr(uint32_t id, uint64_t val)
{
   __asm__ __volatile__ ("wrmsr" 
                   : 
                   : "a"((uint32_t)LOW32(val)), "d"((uint32_t)HI32(val)), "c"(id));
}

static __inline uint64_t rdmsr(uint32_t id)
{
   uint64_t low, high;
   __asm__ __volatile__ ("rdmsr" 
                   : "=a"(low), "=d"(high)
                   : "c"(id));
   return (high << 32) | low;
}
#endif
