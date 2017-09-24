#include <sys/defs.h>
#include <sys/pci.h>
#include <sys/ahci.h>
#include <sys/kprintf.h>

#define	AHCI_BASE    0x400000 /* 4M */
//#define	AHCI_BASE    0x8000000 /* 4M */

int32_t ahciport = -1;

void memset(void* p, int32_t c, int32_t n)
{
   int32_t i;
   unsigned char* x = (unsigned char*)p;
   for (i = 0;i < n;i++,x++)
      *x = c; 
}

void ahci_probe(hba_mem_t* abar)
{
   uint32_t pi = abar->pi;
   uint32_t i;

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
              break;
	}

        ahciport = i;
        return;
      }
   }
   kprintf("No SATA drive found\n");
}

void ahci_start_cmd(hba_port_t* port)
{
   while (port->cmd & HBA_PxCMD_CR);
   port->cmd |= HBA_PxCMD_FRE;
   port->cmd |= HBA_PxCMD_ST; 
}

/* Wait for pending commands and stop the port */
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
   //ahci_stop_cmd(port);
   kprintf("Stopped\n");
   port->clb  = AHCI_BASE + (portno << 10);
   memset((void*)(port->clb), 0, 1024);

   port->fb  = AHCI_BASE + (32<<10) + (portno<<8); 
   memset((void*)(port->fb), 0, 256);

   hba_cmd_header_t* cmdheader = (hba_cmd_header_t*)(port->clb);

   for (i = 0; i < 32; i++)
   {
      cmdheader[i].ctba  = AHCI_BASE + (40<<10) + (portno <<13) + (i<<8);
      memset((void*)(cmdheader[i].ctba), 0, 256);
   }  
   kprintf("Trying to start\n");
   //ahci_start_cmd(port); 
   kprintf("Started\n");
}

#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ 0x08
 
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

int read(hba_port_t *port, uint64_t startl, uint64_t starth, uint64_t count, uint64_t *buf)
{
	port->is_rwc = 0xffffffff;		// Clear pending interrupt bits
	int spin = 0; // Spin lock timeout counter
	int slot = find_cmdslot(port);
        int i;
	if (slot == -1)
		return 0;
 
	hba_cmd_header_t *cmdheader = (hba_cmd_header_t*)port->clb;
	cmdheader += slot;
	cmdheader->cfl = sizeof(fis_reg_h2d_t)/sizeof(uint64_t);	// Command FIS size
	cmdheader->w = 0;		// Read from device
	cmdheader->prdtl = (int32_t)((count-1)>>4) + 1;	// PRDT entries count
 
	hba_cmd_tbl_t *cmdtbl = (hba_cmd_tbl_t*)(cmdheader->ctba);
	memset(cmdtbl, 0, sizeof(hba_cmd_tbl_t) +
 		(cmdheader->prdtl-1)*sizeof(hba_prdt_entry_t));
 
	// 8K bytes (16 sectors) per PRDT
	for (i=0; i<cmdheader->prdtl-1; i++)
	{
		cmdtbl->prdt_entry[i].dba = (uint64_t)buf;
		cmdtbl->prdt_entry[i].dbc = 8*1024;	// 8K bytes
		cmdtbl->prdt_entry[i].i = 1;
		buf += 4*1024;	// 4K words
		count -= 16;	// 16 sectors
	}
	// Last entry
	cmdtbl->prdt_entry[i].dba = (uint64_t)buf;
	cmdtbl->prdt_entry[i].dbc = count<<9;	// 512 bytes per sector
	cmdtbl->prdt_entry[i].i = 1;
 
	// Setup command
	fis_reg_h2d_t *cmdfis = (fis_reg_h2d_t*)(&cmdtbl->cfis);
 
	cmdfis->fis_type = FIS_TYPE_REG_H2D;
	cmdfis->c = 1;	// Command
	cmdfis->command = ATA_CMD_READ_DMA_EX;
 
	cmdfis->lba0 = (uint8_t)startl;
	cmdfis->lba1 = (uint8_t)(startl>>8);
	cmdfis->lba2 = (uint8_t)(startl>>16);
	cmdfis->device = 1<<6;	// LBA mode
 
	cmdfis->lba3 = (uint8_t)(startl>>24);
	cmdfis->lba4 = (uint8_t)starth;
	cmdfis->lba5 = (uint8_t)(starth>>8);
 
	cmdfis->count = count;
 
	// The below loop waits until the port is no longer busy before issuing a new command
	while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
	{
		spin++;
	}
	if (spin == 1000000)
	{
		kprintf("Port is hung\n");
		return 0;
	}
 
	port->ci = 1<<slot;	// Issue command
 
	// Wait for completion
	while (1)
	{
		// In some longer duration reads, it may be helpful to spin on the DPS bit 
		// in the PxIS port field as well (1 << 5)
		if ((port->ci & (1<<slot)) == 0) 
			break;
		if (port->is_rwc & HBA_PxIS_TFES)	// Task file error
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
 
void ahci()
{
    hba_mem_t* abar = (hba_mem_t *)pci_ahci_abr();

    ahci_probe(abar);
    kprintf("Port rebasing\n");
    ahci_port_rebase(&abar->ports[ahciport], ahciport);
    ahci_port_rebase(&abar->ports[ahciport+1], ahciport+1);
    kprintf("Start reading\n");
    char buf[1024];
    memset((void*) buf,0,1024);
    read(&abar->ports[ahciport+1], 20, 20, 2, (uint64_t *)buf);
    int i;

    kprintf("Start printing\n");
    for (i = 0;i < 1024;i++)
       kprintf("%c",buf[i]);
    kprintf("End printing\n");
}
