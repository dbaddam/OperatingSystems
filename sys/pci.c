#include <sys/pci.h>
#include <sys/io.h>
#include <sys/kprintf.h>

#define CONFIG_ADDRESS  0xCF8
#define CONFIG_DATA     0xCFC

#define CONFIG_ENABLE_BIT 0x80000000
#define REG_UPPER_MASK    0xFFFF0000
#define REG_INNER_MASK    0x0000FFFF


uint32_t pci_config_read_regsiter(uint32_t bus, uint32_t slot,
                               uint32_t func, uint32_t regnum)
{
   uint32_t address = (bus << 16) | (slot << 11) | (func << 8) |
                      (regnum & 0xfc) | CONFIG_ENABLE_BIT;

   outl(CONFIG_ADDRESS, address);
   return inl(CONFIG_DATA);
}

uint16_t pci_vendor(uint8_t bus, uint8_t slot)
{
   uint16_t x = (pci_config_read_regsiter(bus, slot, 0, 0) >> 16 ) && 0xFFFF;

   kprintf("vendor - %d\n",(int32_t)x);

   return x;
}

uint16_t pci_enum()
{
   uint8_t bus;
   uint8_t device;

   for (bus = 0; bus < 256;bus++)
      for (device = 0; device < 32; device++)
         if (pci_vendor(bus, device) != 0xFFFF)
            kprintf("bus - %d, device - %d\n", (int32_t)bus, (int32_t)device);
} 
