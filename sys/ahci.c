#include <sys/defs.h>
#include <sys/pci.h>
#include <sys/ahci.h>
#include <sys/kprintf.h>

#define	AHCI_BASE    0x400000 /* 4M */
 

hba_mem_t* abar;
int32_t    ahciportno = -1;

void memset(void* p, int32_t c, int32_t n)
{
   int32_t i;
   unsigned char* x = (unsigned char*)p;
   for (i = 0;i < n;i++,x++)
      *x = c; 
}

void ahci_probe()
{
   uint32_t pi = abar->pi;
   uint32_t i;
   uint32_t first_drive = 0;

   /* We need to probe the bits which are set in pi */
   for (i = 0 ;i < 32 && pi ;i++, pi >>= 1)
   {
      if (pi & 1)
      {
         hba_port_t *port = &abar->ports[i]; 
	 uint32_t    ssts = port->ssts;
	 uint8_t     ipm  = (ssts >> 8) & 0x0F;
	 uint8_t     det  = ssts & 0x0F;

	if (det != HBA_PORT_DET_PRESENT ||
            ipm != HBA_PORT_IPM_ACTIVE)
           continue;

	switch (port->sig)
	{
	   case AHCI_DEV_SATAPI:
	      kprintf("SATAPI drive found at port %d\n", i);
              break;
	   case AHCI_DEV_SEMB:
	      kprintf("SEMB drive found at port %d\n", i);
              break;
	   case AHCI_DEV_PM:
	      kprintf("PM drive found at port %d\n", i);
              break;
	   default:
	      kprintf("SATA drive found at port %d\n", i);
              if (!first_drive)
              {
                 first_drive = 1;
                 continue;
              }
	      kprintf("Will be using the SATA drive found at port %d for r/w\n", i);
              ahciportno = i;
              return;
	}

      }
   }
   kprintf("No SATA drive found\n");
}

// Find a free command list slot 
int find_cmdslot(hba_port_t *port)
{
   int i;

   // If not set in SACT and CI, the slot is free
   uint32_t slots = (port->sact | port->ci);
   for (i=0; i<32; i++)
   {
      if ((slots&1) == 0)
         return i;
      slots >>= 1;
   }

   kprintf("Cannot find free command list entry\n");
   return -1;
}

// readwrite() and find_cmdslot would match a lot with osdev code.
int readwrite(hba_port_t *port, uint64_t startl, uint64_t starth, 
              uint64_t count, uint64_t *buf, uint8_t read)
{
   int spin = 0; // Spin lock timeout counter
   int slot;
   hba_cmd_header_t *cmdheader;

   port->is_rwc = (uint32_t)-1; // Clear interrupts??

   slot = find_cmdslot(port);
   if (slot == -1)
      return 0;
 
   cmdheader = (hba_cmd_header_t*)port->clb;
   cmdheader += slot;
   cmdheader->cfl = sizeof(fis_reg_h2d_t)/sizeof(uint32_t);   // Command FIS size
   cmdheader->w = (read) ? 0 : 1;      // Read from device
   cmdheader->prdtl = 1;//(int32_t)((count-1)>>4) + 1;   // PRDT entries count
 
   hba_cmd_tbl_t *cmdtbl = (hba_cmd_tbl_t*)(cmdheader->ctba);
   memset(cmdtbl, 0, sizeof(hba_cmd_tbl_t) +
       (cmdheader->prdtl-1)*sizeof(hba_prdt_entry_t));

/* 
   // 8K bytes (16 sectors) per PRDT
   for (i=0; i<cmdheader->prdtl-1; i++)
   {
      cmdtbl->prdt_entry[i].dba = (uint64_t)buf;
      cmdtbl->prdt_entry[i].dbc = 8*1024;   // 8K bytes
      cmdtbl->prdt_entry[i].i = 1;
      buf += 4*1024;   // 4K words BEWARE if use this, it is 4K WORDS
      count -= 16;   // 16 sectors
   }
*/

   // Last entry
   cmdtbl->prdt_entry[0].dba = (uint64_t)buf;
   cmdtbl->prdt_entry[0].dbc = count<<9;   // 512 bytes per sector
   cmdtbl->prdt_entry[0].i = 1;
 
   // Setup command
   fis_reg_h2d_t *cmdfis = (fis_reg_h2d_t*)(&cmdtbl->cfis);
 
   cmdfis->fis_type = FIS_TYPE_REG_H2D;
   cmdfis->c = 1;   // Command
   cmdfis->command = (read) ? ATA_CMD_READ_DMA_EX : ATA_CMD_WRITE_DMA_EX;
 
   cmdfis->lba0 = (uint8_t)startl;
   cmdfis->lba1 = (uint8_t)(startl>>8);
   cmdfis->lba2 = (uint8_t)(startl>>16);
   cmdfis->device = 1<<6;   // LBA mode
 
   cmdfis->lba3 = (uint8_t)(startl>>24);
   cmdfis->lba4 = (uint8_t)starth;
   cmdfis->lba5 = (uint8_t)(starth>>8);
 
   cmdfis->count = count;
 
   // The below loop waits until the port is no longer busy before issuing a new command
   while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
      spin++;

   if (spin == 1000000)
   {
      kprintf("Port is hung\n");
      return 0;
   }
 
   port->ci = 1<<slot;   // Issue command
 
   // Wait for completion
   while (1)
   {
      // In some longer duration reads, it may be helpful to spin on the DPS bit 
      // in the PxIS port field as well (1 << 5)
      if ((port->ci & (1<<slot)) == 0) 
         break;
      if (port->is_rwc & HBA_PxIS_TFES)   // Task file error
      {
         kprintf("Read disk error\n");
         return 0;
      }
   }
 
   // Check again
   if (port->is_rwc & HBA_PxIS_TFES)
   {
      kprintf("Read disk error\n");
      return 0;
   }
 
   return 1;
}

int ahci_read(hba_port_t *port, uint64_t startl, uint64_t starth, 
              uint64_t count, uint64_t *buf)
{
   return readwrite(port, startl, starth, count, buf, 1);
}

int ahci_write(hba_port_t *port, uint64_t startl, uint64_t starth, 
              uint64_t count, uint64_t *buf)
{
   return readwrite(port, startl, starth, count, buf, 0);
}
 
void ahci()
{
    hba_port_t *port;
    char       *buf = (char*)AHCI_BASE;
    char        ch;
    uint32_t    i, j;
    uint32_t    fail = 0;

    abar = (hba_mem_t *)pci_ahci_abr();
    ahci_probe();
    //ahci_hba_reset();
    port = &abar->ports[ahciportno];
    //ahci_port_rebase(port, ahciportno);
    port->cmd |= HBA_PxCMD_FRE;
    port->cmd |= HBA_PxCMD_ST;  // Start

    kprintf("Writing blocks onto the drive...\n");
    for (i = 0;i < 100;i++)
    {
       memset((void*) buf, i, 4*1024);
       ahci_write(port, 8*i, 0, 8, (uint64_t *)buf);
    }
    kprintf("Done writing\n");
    
    kprintf("Verifying by reading back...\n");
    for (i = 0;i < 100;i++)
    { 
       ahci_read(port, 8*i, 0, 8, (uint64_t *)buf);
       ch = (char) i;

       for (j = 0;j < 4*1024;j++)
       {
           if (buf[j] != ch)
           {
              kprintf("ALERT::: Wrong sequence detected - "
                      "%dth 4K block, %d byte, 0x%x found\n",i, j, buf[j]);
              fail = 1;
              break;   
           }
       }

       if (fail)
          break;
    }

    if (i == 100)
       kprintf("Verified all the blocks successfully\n");
}

/*
void ahci_hba_reset()
{
   abar->ghc |= HBA_GHC_HR;

   while(abar->ghc & HBA_GHC_HR);
   
   abar->ghc |= (HBA_GHC_AE | HBA_GHC_IE);
   kprintf("HBA reset complete\n");
}

void ahci_start_cmd(hba_port_t* port)
{
   while (port->cmd & HBA_PxCMD_CR);
   port->cmd |= HBA_PxCMD_FRE;
   port->cmd |= HBA_PxCMD_ST;
}

// Wait for pending commands and stop the port
void ahci_stop_cmd(hba_port_t* port)
{
   
   port->cmd &= ~HBA_PxCMD_ST;

   while(1)
   {
     if (port->cmd & HBA_PxCMD_FR)
        continue;
     if (port->cmd & HBA_PxCMD_CR)
        continue;
     break;
   }

   port->cmd &= ~HBA_PxCMD_FRE;
}

void ahci_port_rebase(hba_port_t *port, int32_t portno)
{
   uint32_t i;

   kprintf("Trying to stop\n");
   ahci_stop_cmd(port);
   kprintf("Stopped\n");

    * 1K per port  
    * Each port has 32 command headers.
    * Each header is worth 32 bytes, hence 1K per port*
   port->clb  = AHCI_BASE + (portno << 10);
   memset((void*)(port->clb), 0, 1024);

   * FIS base adresses start after all the ports
    * Each FIS entry is worth 256 bytes. 8K for all the FISs*
   port->fb  = AHCI_BASE + (32<<10) + (portno<<8); 
   memset((void*)(port->fb), 0, 256);

   hba_cmd_header_t* cmdheader = (hba_cmd_header_t*)(port->clb);

   for (i = 0; i < 32; i++)
   {
      cmdheader[i].prdtl = MAX_PRDT_CNT;
      * Each command header length is sizeof(hba_cmd_tbl_t)(call it tsize),
       * make sure you choose MAX_PRDT_CNT which would make cmdheader 
       * 128 aligned to make life easier. Now, each port has 32 
       * command headers, so the 0th command header of a port start from
       * portno*32*tsize after the FIS base addresses. Now, the i-th header
       * of this port is at i*tsize away from 0th header*
      cmdheader[i].ctba  = AHCI_BASE + (40<<10) + 
                           (portno* (32 * sizeof(hba_cmd_tbl_t))) + 
                           (i * (sizeof(hba_cmd_tbl_t)));

      memset((void*)(cmdheader[i].ctba), 0, sizeof(hba_cmd_tbl_t));
   }  
   kprintf("Trying to start\n");
   ahci_start_cmd(port);
   //port->serr_rwc = 0xFFFF//HBA_PxSERR_ALL; 
   kprintf("Started\n");
}
*/
