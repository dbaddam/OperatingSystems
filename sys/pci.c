#include <sys/pci.h>
#include <sys/io.h>
#include <sys/kprintf.h>

#define CONFIG_ADDRESS  0xCF8
#define CONFIG_DATA     0xCFC

#define CONFIG_ENABLE_BIT 0x80000000
#define REG_UPPER_MASK    0xFFFF0000
#define REG_INNER_MASK    0x0000FFFF

uint32_t pci_config_read_register(uint32_t bus, uint32_t slot,
                               uint32_t func, uint32_t offset)
{
   uint32_t address = (bus << 16) | (slot << 11) | (func << 8) |
                      (offset & 0xfc) | CONFIG_ENABLE_BIT;

   outl(CONFIG_ADDRESS, address);
   return inl(CONFIG_DATA);
}

uint16_t pci_vendor(uint8_t bus, uint8_t slot)
{
   uint16_t x = (pci_config_read_register(bus, slot, 0, 0)) & 0xFFFF;

   //kprintf("vendor - %d",(int32_t)x);

   return x;
}

uint8_t pci_class(uint8_t bus, uint8_t slot)
{
   uint8_t x = (uint8_t)(((pci_config_read_register(bus, slot, 0, 8)) 
                           & 0xFF000000) >> 24);
   return x;
}

uint8_t pci_subclass(uint8_t bus, uint8_t slot)
{
   uint8_t x = (uint8_t)(((pci_config_read_register(bus, slot, 0, 8)) 
                           & 0x00FF0000) >> 16);
   return x;
}

uint8_t pci_progif(uint8_t bus, uint8_t slot)
{
   uint8_t x = (uint8_t)(((pci_config_read_register(bus, slot, 0, 8)) 
                           & 0x0000FF00) >> 8);
   return x;
}

uint16_t pci_enum()
{
   uint8_t bus;
   uint8_t device;
//   uint8_t class;
//   uint8_t subclass;
//   uint8_t progif;

   for (bus = 0; bus < 255;bus++)
      for (device = 0; device < 32; device++)
         if (pci_vendor(bus, device) != 0xFFFF &&
             pci_class(bus, device) == 0x01 &&
             pci_subclass(bus, device) == 0x06 &&
             pci_progif(bus, device) == 0x01)
         {
            kprintf("bus - %d, device - %d\n", (int32_t)bus, (int32_t)device);
            kprintf("AHCI controller found!!!! :)\n");
            
         }

   kprintf("Enumeration done\n");

   return 0;
} 
/*
uint16_t pciConfigReadWord (uint8_t bus, uint8_t slot,
                             uint8_t func, uint8_t offset)
{
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint16_t tmp = 0;
 
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
 
    outl(0xCF8, address);
    tmp = (uint16_t)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xffff);
    return (tmp);
}
 uint16_t pciCheckVendor(uint8_t bus, uint8_t slot)
 {
    uint16_t vendor;//, device;
    uint16_t class;
    uint8_t progif;

    if ((vendor = pciConfigReadWord(bus,slot,0,0)) != 0xFFFF) {
       kprintf("bus - %d, slot - %d\n", bus, slot);
       class = pciConfigReadWord(bus,slot,0,10);
       kprintf("class - %x\n", class);
       class = pciConfigReadWord(bus,slot,0,8);
       kprintf("full  - %x\n", class);
       progif = (uint8_t)((pciConfigReadWord(bus,slot,0,8) & 0xFF00) >> 8);
       kprintf("progif - %x\n", progif);
    } 
    return (vendor);
 }
*/

