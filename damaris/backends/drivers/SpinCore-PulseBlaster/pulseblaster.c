/*
 * pulseblaster.c - Communication with pulseblaster
 * author: Achim Gaedke <achim.gaedke@physik.tu-darmstadt.de>
 * created: February 2005
 * the amcc_outb routine comes from SpinCore
 * initially the module is built from example 4 of "The Linux Kernel Module Programming Guide"
 * the version of June 2008 is written with the help of "Linux device drivers"
 * Jonathan Corbet, Alessandro Rubini, and Greg Kroah-Hartman, O' Reilly, 3rd Edition
 */

#include <linux/kernel.h>	/* We're doing kernel work */
#include <linux/module.h>	/* Specifically, a module */
#include <linux/fs.h>
#include <linux/pci.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/version.h>
#include <asm/uaccess.h>	/* for get_user and put_user */
#include <asm/io.h>

#include "pulseblaster.h"
#define SUCCESS 0
#define DEVICE_NAME "pulseblaster"

enum pulseblaster_board {
	PBB_DEBUG = 0,
	PBB_GENERIC_AMCC,
	PBB_GENERIC_PCI,
};

/* number of found and allocated devices */
static int pb_dev_no=0;
/*
 * dynamic allocated device number
 */
/* start of allocated minor number(s)*/
static dev_t pb_dev_no_start;

struct pulseblaster_device {
   /* 
    * Is the device open right now? Used to prevent
    * concurent access into the same device 
    */
   int device_open;

   /**
    * The type of this board
    */
   enum pulseblaster_board boardtype;

   /**
    * The pci device for this pulseblaster
    */
   struct pci_dev *pciboard;

   /*
    * the base io address
    * 0 means debug mode without hardware accesss, writes everything to kernel log
    */
   unsigned long base_addr;

   /*
    * protect access to io memory
    */
   spinlock_t io_lock;

   /*
    * char dev associated with it
    */
   struct cdev cdev;
};

/* array of char_devices */
/* todo: make this a pointer array with dynamic allocation... */
#define pulseblaster_max_devno 4
static struct pulseblaster_device pb_devs[pulseblaster_max_devno];
// a lock for that structure
static spinlock_t pb_devs_lock;

// debug version
static struct pulseblaster_device pb_dev_debug;

static int base_address=-1;

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Achim Gaedke");
MODULE_DESCRIPTION("Driver for SpinCore's PulseBlaster");
module_param(base_address, int, S_IRUGO);
MODULE_PARM_DESC(base_address, "not supported anymore, IO base addresses of device are detected automatically");

/*************************************************************************************/

/*  code taken form spincore example:
 *  This function writes a byte to a board that uses the amcc chip.
 *  SP3 and previous revision boards use this interface. Later designs
 *  can be programmed directly.
 */

#if 1
// version without delay (faster)
#define pb_out(bwl) out##bwl
#define pb_in(bwl) in##bwl
#else
// version with extra delay (works on odd hardware, too)
#define pb_out(bwl) out##bwl##_p
#define pb_in(bwl) in##bwl##_p
#endif


static int pb_outb_debug(char data, unsigned short address, unsigned long my_base_address)
{
	printk("%s: reg %02x=%02x\n",DEVICE_NAME, 0xff&address, 0xff&data);
	return 0;
}

static int amcc_outb(char data, unsigned short address, unsigned long my_base_address) {

	unsigned int MAX_RECV_TIMEOUT = 10;
	unsigned int RECV_START, RECV_TOGGLE, RECV_TIMEOUT = 0;
	int XFER_ERROR = 0;
	int RECV_POLLING = 0;

	unsigned int OGMB = 0x0C;
	unsigned int CHK_RECV = 0x1F;
	unsigned int SIG_TRNS = 0x0F;

	unsigned int CLEAR31 = 0x00000001;
	unsigned int CLEAR24 = 0x000000FF;
	unsigned int CLEAR28 = 0x0000000F;
	unsigned int SET_XFER = 0x01000000;

	unsigned int Temp_Address = address;
	unsigned int Temp_Data = data;

	if (my_base_address==0) {
		return pb_outb_debug(data, address, my_base_address);
	}

		// Prepare Address Transfer
		Temp_Address &= CLEAR28;
		Temp_Address |= SET_XFER;

		// Prepare Data Transfer
		Temp_Data &= CLEAR24;
		Temp_Data |= SET_XFER;

		// Clear the XFER bit (Should already be cleared)
		pb_out(b) (0,my_base_address+SIG_TRNS);

		// Transfer Address
		
		// Read RECV bit from the Board
		RECV_START = pb_in(b) (my_base_address+CHK_RECV);

		RECV_START &= CLEAR31;	// Only Save LSB


		// Send Address to OGMB
		pb_out(l) (Temp_Address, my_base_address+OGMB);

		RECV_POLLING = 1;	// Set Polling Flag
		RECV_TIMEOUT = 0;
		while ((RECV_POLLING == 1) && (RECV_TIMEOUT < MAX_RECV_TIMEOUT)) 
		{
			RECV_TOGGLE = pb_in(b)(my_base_address+CHK_RECV);

			RECV_TOGGLE &= CLEAR31;	// Only Save LSB
			if (RECV_TOGGLE != RECV_START) RECV_POLLING = 0;	// Finished if Different
			else RECV_TIMEOUT++;
			if (RECV_TIMEOUT == MAX_RECV_TIMEOUT) XFER_ERROR = -2;
		}

		// Transfer Complete (Clear) Signal
		pb_out(b)(0, my_base_address+SIG_TRNS);

		// Transfer Data

		// Read RECV bit from the Board
		RECV_START = pb_in(b)(my_base_address+CHK_RECV);

		RECV_START &= CLEAR31;	// Only Save LSB

		// Send Data to OGMB
		pb_out(l)(Temp_Data, my_base_address+OGMB);

		RECV_POLLING = 1;	// Set Polling Flag
		RECV_TIMEOUT = 0;
		while ((RECV_POLLING == 1) && (RECV_TIMEOUT < MAX_RECV_TIMEOUT)) 
		{
			// Check for Toggled RECV
                        RECV_TOGGLE = pb_in(b)(my_base_address+CHK_RECV);

			RECV_TOGGLE &= CLEAR31;	// Only Save LSB
			if (RECV_TOGGLE != RECV_START) RECV_POLLING = 0;	// Finished if Different
			else RECV_TIMEOUT++;
			if (RECV_TIMEOUT == MAX_RECV_TIMEOUT) XFER_ERROR = -2;
		}

		// Transfer Complete (Clear) Signal
		pb_out(b)(0, my_base_address+SIG_TRNS);

	return XFER_ERROR;	
}

static int pb_outb_pci(char data, unsigned short address, unsigned long my_base_address)
{
	if (my_base_address == 0)
		return pb_outb_debug(data, address, my_base_address);
	pb_out(b)(data, my_base_address + address);
	return 0;
}

/**
 * write to byte register on board
 */
static int pb_dev_outb(struct pulseblaster_device *my_dev, char data, unsigned short address)
{
	unsigned long my_base_address;

	if (my_dev == NULL)
		return -1;

	my_base_address = my_dev->base_addr;

	switch (my_base_address != 0
		? my_dev->boardtype
		: PBB_DEBUG)
	{
		case PBB_DEBUG:
			return pb_outb_debug(data, address, my_base_address);
		case PBB_GENERIC_AMCC:
			return amcc_outb(data, address, my_base_address);
		case PBB_GENERIC_PCI:
			return pb_outb_pci(data, address, my_base_address);
		default:
			return -1;
	}

	return -1;
}


static int pb_inb_debug(unsigned int address, unsigned long my_base_address)
{
	printk("%s: reg(%02x)=0 (guessed)\n",DEVICE_NAME, 0xff&address);
	return 0;
}

/**
 * Read a byte from a board using the AMCC chip
 */
static int amcc_inb(unsigned int address, unsigned long my_base_address) {
  unsigned int MAX_RECV_TIMEOUT = 10000;
  unsigned int RECV_START, RECV_STOP, RECV_TOGGLE, RECV_TIMEOUT = 0;
  int XFER_ERROR = 0;
  int RECV_POLLING = 0;

  unsigned int CHK_RECV = 0x1F;
  unsigned int SIG_TRNS = 0x0F;
  unsigned int ICMB = 0x1C;

  unsigned int CLEAR24 = 0x000000FF;

  unsigned int BIT1 = 0x00000002;
  unsigned int INV1 = 0x000000FD;

  unsigned short READ_ADDR = 0x9;

  int Toggle = 0;
  int Toggle_Temp = 0;

  unsigned int Temp_Data = 0;

  if (my_base_address==0) {
      return pb_inb_debug(address, my_base_address);
  }

  // CHEK FOR 1 in MD1
  if ((XFER_ERROR=amcc_outb(address, 8, my_base_address))!=0 ||
  (XFER_ERROR=amcc_outb(0, READ_ADDR, my_base_address))!=0)  // Tell board to start a read cycle
      return XFER_ERROR;

  RECV_POLLING = 1;	// Set Polling Flag
  RECV_TIMEOUT = 0;
  RECV_START = 0x2; // Value expected when data is ready

  while ((RECV_POLLING == 1) && (RECV_TIMEOUT < MAX_RECV_TIMEOUT)) {
      // Check for Toggled RECV
      //RECV_TOGGLE = _inp(CHK_RECV);
      RECV_TOGGLE = pb_in(b) (my_base_address+CHK_RECV);
      RECV_TOGGLE &= BIT1;	// Only Save Bit 1
      
      if (RECV_TOGGLE == RECV_START) RECV_POLLING = 0;	// Finished if Different
      else RECV_TIMEOUT++;
      if (RECV_TIMEOUT == MAX_RECV_TIMEOUT) XFER_ERROR = -2;
  }
  if (XFER_ERROR != 0) {
      return XFER_ERROR;
  }

  //Temp_Data = _inp(ICMB);
  // Read RECV bit from the Board
  Temp_Data = pb_in(b)(my_base_address+ICMB);
  Temp_Data &= CLEAR24;

		
  //Toggle = _inp(SIG_TRNS);
  Toggle = pb_in(b) (my_base_address+SIG_TRNS);
  
  Toggle_Temp = Toggle & BIT1; // Only Save Bit 1
  if (Toggle_Temp == 0x0)
    {
      Toggle |= BIT1; // If Bit 1 is zero, set it to 1
    }
  else
    {
      Toggle &= INV1; // IF Bit 1 is 1, set it to 0
    }

  //_outp(SIG_TRNS, Toggle);
  pb_out(b) (Toggle,my_base_address+SIG_TRNS);
  
  RECV_POLLING = 1;	// Set Polling Flag
  RECV_TIMEOUT = 0;
  RECV_STOP = 0x0;
		
  while ((RECV_POLLING == 1) && (RECV_TIMEOUT < MAX_RECV_TIMEOUT)) {
    // Check for Toggled RECV
    //RECV_TOGGLE = _inp(CHK_RECV);
    RECV_TOGGLE = pb_in(b) (my_base_address + CHK_RECV);
    RECV_TOGGLE &= BIT1;	// Only Save Bit 1
      
    if (RECV_TOGGLE == RECV_STOP) RECV_POLLING = 0;	// Finished if Different
    else RECV_TIMEOUT++;
    if (RECV_TIMEOUT == MAX_RECV_TIMEOUT) XFER_ERROR = -3;
  }
  if (XFER_ERROR != 0) {
    return XFER_ERROR;
  }

  return Temp_Data;
}

static int pb_inb_pci(unsigned int address, unsigned long my_base_address)
{
	if (my_base_address == 0)
		return pb_inb_debug(address, my_base_address);
	return pb_in(b)(my_base_address + address);
}


/**
 * Read byte register from board
 */
static int pb_dev_inb(struct pulseblaster_device *my_dev, unsigned short address)
{
	unsigned long my_base_address;

	if (my_dev == NULL)
		return -1;

	my_base_address = my_dev->base_addr;

	switch (my_base_address != 0
		? my_dev->boardtype
		: PBB_DEBUG)
	{
		case PBB_DEBUG:
			return pb_inb_debug(address, my_base_address);
		case PBB_GENERIC_AMCC:
			return amcc_inb(address, my_base_address);
		case PBB_GENERIC_PCI:
			return pb_inb_pci(address, my_base_address);
		default:
			return -1;
	}

	return -1;
}

/**
 * Setup and get the base_addr
 */
static unsigned long pb_dev_setup_baseaddr(struct pulseblaster_device *my_dev)
{
	unsigned long addr = 0x0;

	if (my_dev == NULL)
		return 0x0;

	if ((my_dev->boardtype != PBB_DEBUG)
	    && (my_dev->pciboard != NULL))
	{
		addr = pci_resource_start(my_dev->pciboard, 0);
	}

	my_dev->base_addr = addr;

	return addr;
}

static inline bool pb_is_debug(struct pulseblaster_device *my_dev)
{
	return ((my_dev->boardtype == PBB_DEBUG)
		|| (my_dev->pciboard == NULL));
}


/* 
 * This is called whenever a process attempts to open the device file 
 */
static int device_open(struct inode *inode, struct file *file)
{
  struct pulseblaster_device* my_dev=container_of(inode->i_cdev, struct pulseblaster_device, cdev);
  file->private_data=my_dev;

  if (pb_is_debug(my_dev))
    printk("%s: device_open(%p)\n", DEVICE_NAME,file);

  /* 
   * We don't want to talk to two processes at the same time 
   */
  if (my_dev->device_open)
    return -EBUSY;  
  my_dev->device_open++;
  try_module_get(THIS_MODULE);

  return SUCCESS;
}

/*
 * Device released  (a.k.a. closed)
 */
static int device_release(struct inode *inode, struct file *file)
{
  struct pulseblaster_device* my_dev=container_of(inode->i_cdev, struct pulseblaster_device, cdev);

  pb_dev_setup_baseaddr(my_dev);

  if (pb_is_debug(my_dev))
    printk("%s: device_release(%p,%p)\n",DEVICE_NAME, inode, file);

  /*
   * reset device
   */
  if (0) {
    // this procedure works only on DDS cards
    // DDS Manual
    spin_lock(&(my_dev->io_lock));
    pb_dev_outb(my_dev,    0, 0); //dev reset
    pb_dev_outb(my_dev,    4, 2); //bytes per word
    pb_dev_outb(my_dev, 0xFF, 3); //dev to program
    pb_dev_outb(my_dev,    0, 4); //reset address counter
    pb_dev_outb(my_dev,    0, 6); //data out
    pb_dev_outb(my_dev,    0, 6); //data out
    pb_dev_outb(my_dev,    0, 6); //data out
    pb_dev_outb(my_dev,    0, 6); //data out
    pb_dev_outb(my_dev,    0, 5); //strobe clock
    pb_dev_outb(my_dev,    0, 5); //strobe clock
    spin_unlock(&(my_dev->io_lock));

  }
  if (0) {
    spin_lock(&(my_dev->io_lock));
    // this procedure is successful for 24Bit cards
    // SpinCore by mail Oct 10th 2007
    pb_dev_outb(my_dev, 0,6);//store 0's to memory
    pb_dev_outb(my_dev, 0,6);
    pb_dev_outb(my_dev, 0,6);
    pb_dev_outb(my_dev, 0,6);
    pb_dev_outb(my_dev, 0,6);
    pb_dev_outb(my_dev, 0,6);
    pb_dev_outb(my_dev, 0,6);
    pb_dev_outb(my_dev, 0,6);

    pb_dev_outb(my_dev, 0,0);//dev reset
    pb_dev_outb(my_dev, 4,2);//bytes per word
    pb_dev_outb(my_dev, 0,3);//write to internal memory
    pb_dev_outb(my_dev, 0,4);//clear address counter
    pb_dev_outb(my_dev, 0,6);//data out
    pb_dev_outb(my_dev, 0,6);//data out
    pb_dev_outb(my_dev, 0,6);//data out
    pb_dev_outb(my_dev, 0,6);//data out
    pb_dev_outb(my_dev, 7,7);//programming finished
    pb_dev_outb(my_dev, 7,1);//trigger
    spin_unlock(&(my_dev->io_lock));
  }

  /* 
   * We're now ready for our next caller 
   */
  my_dev->device_open--;

  module_put(THIS_MODULE);
  return SUCCESS;
}

/* 
 * This function is called whenever a process which has already opened the
 * device file attempts to read from it.
 */
static ssize_t device_read(struct file *file,	/* see include/linux/fs.h   */
			   char __user * buffer,	/* buffer to be
							 * filled with data */
			   size_t length,	/* length of the buffer     */
			   loff_t * offset)
{

	// not implemented
	return 0;
}

/* 
 * This function is called when somebody tries to
 * write into our device file. 
 * everything goes to register 6: data
 */
static ssize_t
device_write(struct file *file,
	     const char __user * buffer, size_t length, loff_t * offset)
{
        struct pulseblaster_device* my_dev=(struct pulseblaster_device*)file->private_data;
	register int i=0;

	pb_dev_setup_baseaddr(my_dev);

	while (1) {
	  // sometimes
	  unsigned char temp_byte;
	  int retval;
	  get_user(temp_byte, buffer + i);
	  // the hardware access
	  spin_lock(&(my_dev->io_lock));
	  retval = pb_dev_outb(my_dev, temp_byte,6);
	  spin_unlock(&(my_dev->io_lock));
          if (retval!=0) {
		 printk("%s: IO Error, pb_dev_outb returned %d\n",DEVICE_NAME, retval);
		 return -EIO; // make an ordinary io error
	  }
	  i++;
          /* break codition */
	  if (i>=length) break;
          /* be decent to all other processes --- at least sometimes */
	  if ((i & ((1<<14)-1)) == 0) schedule();
	}

	/* 
	 * Again, return the number of input characters used 
	 */
	return i;
}

/* 
 * This function is called whenever a process tries to do an ioctl on our
 * device file. We get two extra parameters (additional to the inode and file
 * structures, which all device functions get): the number of the ioctl called
 * and the parameter given to the ioctl function.
 *
 * If the ioctl is write or read/write (meaning output is returned to the
 * calling process), the ioctl call returns the output of this function.
 *
 */
static long  device_ioctl(
		 struct file *file,	/* ditto */
		 unsigned int ioctl_num,	/* number and param for ioctl */
		 unsigned long ioctl_param)
{
  struct inode *inode = file->f_path.dentry->d_inode;
  struct pulseblaster_device* my_dev=container_of(inode->i_cdev, struct pulseblaster_device, cdev);
  int ret_val=SUCCESS;

  pb_dev_setup_baseaddr(my_dev);

  if (ioctl_num==IOCTL_OUTB){
    unsigned char reg=(ioctl_param>>8)&0xFF;
    unsigned char val=ioctl_param&0xFF;
    /*
      check for register boundaries!
    */
    if (reg>7) {
      printk("%s: got bad register number %02x\n",DEVICE_NAME,0x0ff&reg);
      ret_val=-EINVAL;
    }
    else {
       spin_lock(&(my_dev->io_lock));
       ret_val = pb_dev_outb(my_dev, val, reg);
       spin_unlock(&(my_dev->io_lock));
       if (ret_val!=0) {
	    printk("%s: IO Error, pb_dev_outb returned %d\n",DEVICE_NAME, ret_val);
	    ret_val=-EIO; // make an ordinary io error
       }
    }
  }
  else if (ioctl_num==IOCTL_INB) {
    unsigned char reg=ioctl_param&0xFF;
    //printk("%s: reading register number %02x\n",DEVICE_NAME,0x0ff&reg);
    spin_lock(&(my_dev->io_lock));
    ret_val = pb_dev_inb(my_dev, reg);
    spin_unlock(&(my_dev->io_lock));
    //printk("%s: found %02x=%02x\n", DEVICE_NAME, 0x0ff&reg, 0x0ff&val);
    if (ret_val<0) {
	printk("%s: IO Error, pb_dev_inb returned %d\n",DEVICE_NAME, ret_val);
	ret_val=-EIO; // make an ordinary io error
    }
  }
  else {
	printk("%s: unknown ioctl request number %d\n",DEVICE_NAME, ioctl_num);
        ret_val=-EINVAL;
  }
  
  return ret_val;
}

/* Module Declarations */

/* 
 * This structure will hold the functions to be called
 * when a process does something to the device we
 * created. Since a pointer to this structure is kept in
 * the devices table, it can't be local to
 * init_module. NULL is for unimplemented functions. 
 */
struct file_operations pulseblaster_fops = {
	.read = device_read,
	.write = device_write,
	.unlocked_ioctl = device_ioctl,
	.open = device_open,
	.release = device_release,	/* a.k.a. close */
};

/*
 here all the basic pci stuff follows
*/

/*
 * no hotpluging after initialisation...
 */
static int pulseblaster_pci_probe(struct pci_dev *dev, const struct pci_device_id *id) {
    int ret_val;

    spin_lock(&pb_devs_lock);
    // check wether device numbers were already allocated
    if (pb_dev_no_start!=0) {
        //we are too late
        spin_unlock(&pb_devs_lock);
        printk("%s: Sorry, hotpluging not supported!\n", DEVICE_NAME);
        return -1;
    }

    if (pb_dev_no>=pulseblaster_max_devno) {
        spin_unlock(&pb_devs_lock);
        printk("%s: Sorry, maximum number of allocatable devices reached !\n", DEVICE_NAME);
        return -1;
    }
    /* do i need this?, what about pci_enable_device_bars() */
    ret_val=pci_enable_device(dev);
    if (ret_val!=0) {
        spin_unlock(&pb_devs_lock);
	printk("%s: failed to enable pci device!\n",DEVICE_NAME);
        return -1;
    }

#ifndef CONFIG_XEN
    // why is pci_request_region not defined in a XEN kernel?
    /* exclusive use */
    ret_val=pci_request_region(dev, 0, DEVICE_NAME);
    if (ret_val!=0) {
        spin_unlock(&pb_devs_lock);
	printk("%s: failed to enable pci device!\n",DEVICE_NAME);
	pci_disable_device(dev);
	return -1;
    }
#else
# warning "in XEN version pci_request_region and pci_release_region are not defined, omitting function call"
#endif

    /* initialize the structure */
    pb_devs[pb_dev_no].device_open=0;
    spin_lock_init(&(pb_devs[pb_dev_no].io_lock));
    pb_devs[pb_dev_no].pciboard=dev;
    pb_devs[pb_dev_no].boardtype = id->driver_data;
    pb_devs[pb_dev_no].base_addr = 0x0;

    // todo: inform about slots as a hint for the physical location of the board!
    printk("%s: found a pci board with i/o base address 0x%lx, assigning no %d\n",
          DEVICE_NAME,
          (unsigned long)pci_resource_start(dev, 0),
          pb_dev_no);

    ++pb_dev_no;
    spin_unlock(&pb_devs_lock);

    return SUCCESS;
}

static void pulseblaster_pci_remove(struct pci_dev* dev) {
    int i=0;
    /* for hotplugging, maintain the list in a smart way
       here: remove only dangling device pointer!
     */
    spin_lock(&pb_devs_lock);
    for (i=0; i<pb_dev_no; ++i) {
       if (pb_devs[i].pciboard==dev) {
           spin_lock(&(pb_devs[i].io_lock));
           pb_devs[i].pciboard=NULL;
           pb_devs[i].boardtype = PBB_DEBUG;
           pb_devs[i].base_addr = 0x0;
           spin_unlock(&(pb_devs[i].io_lock));
           printk("%s: releasing device no %d\n", DEVICE_NAME, i);
           break;
       }
    }
    spin_unlock(&pb_devs_lock);
    /* do the pci stuff only */
    pci_disable_device(dev);
#ifndef CONFIG_XEN
    // why is pci_release_region not defined in a XEN kernel?
    pci_release_region(dev, 0);
#endif
}

static struct pci_device_id pulseblaster_pci_ids[] = {
{PCI_DEVICE(0x10e8, 0x8852), .driver_data = PBB_GENERIC_AMCC},
{PCI_DEVICE(0x10e8, 0x8878), .driver_data = PBB_GENERIC_PCI},
{0,},
};

MODULE_DEVICE_TABLE(pci, pulseblaster_pci_ids);

static struct pci_driver pulseblaster_pci_driver = {
 .name = DEVICE_NAME,
 .id_table=pulseblaster_pci_ids,
 .probe=pulseblaster_pci_probe,
 .remove=pulseblaster_pci_remove,
};


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 26)

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
#define WITH_DRVDATA
#endif

static struct class *pulseblaster_class = NULL;

static void pb_class_setup(void)
{
	/*
	 * create class and register devices in /sys
	 */
	pulseblaster_class = class_create(THIS_MODULE, DEVICE_NAME);
	if (IS_ERR(pulseblaster_class))
	{
		printk(KERN_ERR "%s: failed to register class", DEVICE_NAME);
		pulseblaster_class = NULL;
	}
}

static void pb_class_destroy(void)
{
	if (pulseblaster_class)
		class_destroy(pulseblaster_class);
	pulseblaster_class = NULL;
}

/**
 * Add device to our class
 */
static void pb_class_device_create(dev_t devno, int nr)
{
	struct device *dev;

	if (pulseblaster_class == NULL)
		return;

	if (nr == -1)
	{
		dev = device_create(pulseblaster_class, NULL, devno,
#ifdef WITH_DRVDATA
				    NULL,
#endif
				    DEVICE_NAME"_dbg");
	}
	else
	{
		dev = device_create(pulseblaster_class, NULL, devno,
#ifdef WITH_DRVDATA
				    NULL,
#endif
				    DEVICE_NAME"%d", nr);
	}

	if (IS_ERR(dev))
	{
		printk(KERN_WARNING "%s: device_create for %d failed\n",
		       DEVICE_NAME, nr);
	}
}

/**
 * Remove device from class
 */
static void pb_class_device_destroy(dev_t devno)
{
	if (pulseblaster_class == NULL)
		return;

	device_destroy(pulseblaster_class, devno);
}
#else
static void pb_class_setup(void)
{
	printk(KERN_WARNING "%s: No udev support\n", DEVICE_NAME);
}
static void pb_class_destroy(void)
{
}
static void pb_class_device_create(dev_t devno, int nr)
{
}
static void pb_class_device_destroy(dev_t devno)
{
}
# warning "No class support, hotplug/udev wont wrok"
#endif
 
/*
 * Initialize the module - Register the character device 
 */

static int __init init_pulseblaster_module(void)
{
	int ret_val;
	int i;
	int major_dev_num, minor_dev_num;


	if (base_address!=-1) {
	   printk("%s: no longer supporting 'base_address' parameter\n", DEVICE_NAME);
        }
        pb_dev_no_start=0;
        pb_dev_no=0;
        // lock for manipulating the device list
        spin_lock_init(&pb_devs_lock);

        // always provide a debug device
        pb_dev_debug.device_open=0;
	spin_lock_init(&(pb_dev_debug.io_lock));
	pb_dev_debug.pciboard=0x0; // this is the debug flag for amcc functions
	pb_dev_debug.boardtype = PBB_DEBUG;
	pb_dev_debug.base_addr = 0x0;

	// register code for pci bus scanning
	ret_val=pci_register_driver(&pulseblaster_pci_driver);
        // todo error checking
        if (ret_val<0) {
            printk("%s: registering the pci driver failed", DEVICE_NAME);
            return ret_val;
        }

	// get device ids
	ret_val= alloc_chrdev_region(&pb_dev_no_start, 0, pb_dev_no+1, DEVICE_NAME);
	major_dev_num=MAJOR(pb_dev_no_start);
	minor_dev_num=MINOR(pb_dev_no_start);
	if (ret_val < 0) {
		printk("%s failed with %d\n",
		       "Sorry, registering the character device failed\n", ret_val);
        	pci_unregister_driver(&pulseblaster_pci_driver);
		return ret_val;
	}
	
	pb_class_setup();
	
        // register debug device
        {
            dev_t devno = MKDEV(major_dev_num, minor_dev_num+pb_dev_no);
            cdev_init(&(pb_dev_debug.cdev), &pulseblaster_fops);
            pb_dev_debug.cdev.owner=THIS_MODULE;
            pb_dev_debug.cdev.ops=&pulseblaster_fops;
            ret_val=cdev_add(&(pb_dev_debug.cdev), devno, 1);

            pb_class_device_create(devno, -1);
        }

        spin_lock(&pb_devs_lock);
	for (i=0; i<pb_dev_no; ++i) {
            dev_t devno = MKDEV(major_dev_num, minor_dev_num+i);
            /* register char dev */
            cdev_init(&(pb_devs[i].cdev), &pulseblaster_fops);
            pb_devs[i].cdev.owner=THIS_MODULE;
            pb_devs[i].cdev.ops=&pulseblaster_fops;
            ret_val=cdev_add(&(pb_devs[i].cdev), devno, 1);
            if (ret_val<0) {
               printk("%s: failed to register char device", DEVICE_NAME);
               // todo cleanup and return;
            }

            pb_class_device_create(devno, i);
	}
        spin_unlock(&pb_devs_lock);

	return 0;
}

module_init(init_pulseblaster_module);

/* 
 * Cleanup - unregister the appropriate file from /proc 
 */
static void __exit cleanup_pulseblaster_module(void)
{
	/* 
	 * Unregister the device 
	 * ToDo: check use count?!
	 */
	int i;

	int major_dev_num=MAJOR(pb_dev_no_start);
	int minor_dev_num=MINOR(pb_dev_no_start);

	spin_lock(&pb_devs_lock);
	for (i=0; i<pb_dev_no; ++i)
	{
                pb_class_device_destroy(MKDEV(major_dev_num, minor_dev_num+i));
	}
	spin_unlock(&pb_devs_lock);
	pb_class_device_destroy(MKDEV(major_dev_num, minor_dev_num+pb_dev_no));
	pb_class_destroy();

        spin_lock(&pb_devs_lock);
	for (i=0; i<pb_dev_no; ++i) {
	    cdev_del(&(pb_devs[i].cdev));
        }
        spin_unlock(&pb_devs_lock);
        cdev_del(&(pb_dev_debug.cdev));
	unregister_chrdev_region(pb_dev_no_start, pb_dev_no+1);

	pci_unregister_driver(&pulseblaster_pci_driver);

        pb_dev_no=0;
}

module_exit(cleanup_pulseblaster_module);
