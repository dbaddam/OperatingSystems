#include <stdlib.h>
#include <stdio.h>
#include <syscall.h>
#include <unistd.h>

/*
 * Implementation similar to 
 * http://www.inf.udec.cl/~leo/Malloc_tutorial.pdf 
 */

#define align(x) ( (((x-1) >> 3) << 3) + 8)

struct header{
   struct header* next;
   unsigned int size;
   int free;
}__attribute__((__packed__));

typedef struct header header;


static header* start = NULL;
static header* last = NULL;

void* sbrk(unsigned int incr)
{
   syscall1(sbrk, incr);

   return (void*)sysret;
}

header* bump_heap(unsigned int size)
{
   header* h;

   h = sbrk(0);

   if (sbrk(size + sizeof(header)) > 0)
   {
      h->size = size;
      h->next = NULL;
      h->free = 1;
      if (last)
      {
         last->next = h;
         last = last->next;
      }
      else
      {
         last = h;
      }
      return h;
   }
   else
   {
      return NULL;
   }
  
}

void split(header* p, unsigned int size)
{
   header* newblk = (header*)(((char*) p) + size + sizeof(header));
   newblk->size = p->size - size - sizeof(header);
   newblk->next = p->next;
   newblk->free = 1;
   p->next = newblk;
   p->size = size;

   if (last == p)
      last = newblk;
}

void* malloc(size_t sz)
{
   if (sz == 0 || sz >= (1ULL<<32))
      return NULL;

   int size = align(sz);
   char* ptr;

   if (start == NULL)
   {
      start = bump_heap(size);
      if (start == NULL)
      {
         return NULL;
      }
      else
      {
         start->free = 0;
         ptr = ((char*)start) + sizeof(header);
         return ptr;
      }
   }
   else
   {
      header* p;
      for (p = start;p != NULL;p = p->next)
      {
          if (p->free && 
              p->size >= size)
          {
             if (p->size >= size + sizeof(header) + 8)
             {
                split(p, size);
             }
             p->free = 0;
             ptr = ((char*)p) + sizeof(header);
             return ptr;
          }
      }

      p = bump_heap(size);
      if (p == NULL)
      {
         return NULL;
      }
      else
      {
         p->free = 0;
         ptr = ((char*)p) + sizeof(header);
         return ptr;
      }
   }

   return NULL;   
}

void free(void* ptr)
{
   header* p = (header*)(((char*)ptr) - sizeof(header));

   p->free = 1;
}

void print_mem()
{
   header* p = start;

   while (p != NULL)
   {
       printf("(%p,%d,%d),", p, p->size, p->free);
       p = p->next;
   }
}
