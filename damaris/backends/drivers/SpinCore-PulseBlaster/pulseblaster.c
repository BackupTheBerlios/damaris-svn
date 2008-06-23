/*
 * pulseblaster.c - Communication with pulseblaster via amcc chip registers
 * author: Achim Gaedke <achim.gaedke@physik.tu-darmstadt.de>
 * created: February 2005
 * the amcc_outb routine comes from SpinCore
 * the module is built from example 4 of "The Linux Kernel Module Programming Guide"
 */

#include <linux/kernel.h>	/* We're doing kernel work */
#include <linux/module.h>	/* Specifically, a module */
#include <linux/fs.h>
#include <linux/pci.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <asm/uaccess.h>	/* for get_user and put_user */
#include <asm/io.h>

#include "pulseblaster.h"
#define SUCCESS 0
#define DEVICE_NAME "pulseblaster"

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
   /*
    * the base io address
    * 0 means debug mode without hardware accesss, writes everything to kernel log
    */
   struct pci_dev *pciboard;
   /*
    * protect access to io memory
    */
   spinlock_t io_lock;

   /*
    * char dev associated with it
    */
   struct cdev cdev;
   
   /*
    * and the device instance in pulseblaster class
    */
    struct class_device *classdev;
};

/* array of char_devices */
/* todo: make this a pointer array with dynamic allocation... */
#define pulseblaster_max_devno 4
static struct pulseblaster_device pb_devs[pulseblaster_max_devno];

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Achim Gaedke");

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
#define pb_out(bwl) out_p##bwl
#define pb_out(bwl) in_p##bwl
#endif

static int amcc_outb(char data, unsigned short address, unsigned long base_address) {

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

	if (base_address==0) {
	  printk("%s: reg %02x=%02x\n",DEVICE_NAME, 0xff&address, 0xff&data);
	  return 0;
	}

		// Prepare Address Transfer
		Temp_Address &= CLEAR28;
		Temp_Address |= SET_XFER;

		// Prepare Data Transfer
		Temp_Data &= CLEAR24;
		Temp_Data |= SET_XFER;

		// Clear the XFER bit (Should already be cleared)
		pb_out(b) (0,base_address+SIG_TRNS);

		// Transfer Address
		
		// Read RECV bit from the Board
		RECV_START = pb_in(b) (base_address+CHK_RECV);

		RECV_START &= CLEAR31;	// Only Save LSB


		// Send Address to OGMB
		pb_out(l) (Temp_Address, base_address+OGMB);

		RECV_POLLING = 1;	// Set Polling Flag
		RECV_TIMEOUT = 0;
		while ((RECV_POLLING == 1) && (RECV_TIMEOUT < MAX_RECV_TIMEOUT)) 
		{
			RECV_TOGGLE = pb_in(b)(base_address+CHK_RECV);

			RECV_TOGGLE &= CLEAR31;	// Only Save LSB
			if (RECV_TOGGLE != RECV_START) RECV_POLLING = 0;	// Finished if Different
			else RECV_TIMEOUT++;
			if (RECV_TIMEOUT == MAX_RECV_TIMEOUT) XFER_ERROR = -2;
		}

		// Transfer Complete (Clear) Signal
		pb_out(b)(0, base_address+SIG_TRNS);

		// Transfer Data

		// Read RECV bit from the Board
		RECV_START = pb_in(b)(base_address+CHK_RECV);

		RECV_START &= CLEAR31;	// Only Save LSB

		// Send Data to OGMB
		pb_out(l)(Temp_Data, base_address+OGMB);

		RECV_POLLING = 1;	// Set Polling Flag
		RECV_TIMEOUT = 0;
		while ((RECV_POLLING == 1) && (RECV_TIMEOUT < MAX_RECV_TIMEOUT)) 
		{
			// Check for Toggled RECV
                        RECV_TOGGLE = pb_in(b)(base_address+CHK_RECV);

			RECV_TOGGLE &= CLEAR31;	// Only Save LSB
			if (RECV_TOGGLE != RECV_START) RECV_POLLING = 0;	// Finished if Different
			else RECV_TIMEOUT++;
			if (RECV_TIMEOUT == MAX_RECV_TIMEOUT) XFER_ERROR = -2;
		}

		// Transfer Complete (Clear) Signal
		pb_out(b)(0, base_address+SIG_TRNS);

	return XFER_ERROR;	
}


/**
 * Read a byte from a board using the AMCC chip
 *
 *
 */
static char amcc_inb(unsigned int address, unsigned long base_address) {
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

  if (base_address==0) {
      printk("%s: reg(%02x)=0 (guessed)\n",DEVICE_NAME, 0xff&address);
      return 0;
  }

  // CHEK FOR 1 in MD1
  amcc_outb(address, 8, base_address);
  amcc_outb(0, READ_ADDR, base_address); // Tell board to start a read cycle

  RECV_POLLING = 1;	// Set Polling Flag
  RECV_TIMEOUT = 0;
  RECV_START = 0x2; // Value expected when data is ready

  while ((RECV_POLLING == 1) && (RECV_TIMEOUT < MAX_RECV_TIMEOUT)) {
      // Check for Toggled RECV
      //RECV_TOGGLE = _inp(CHK_RECV);
      RECV_TOGGLE = pb_in(b) (base_address+CHK_RECV);
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
  Temp_Data = pb_in(b)(base_address+ICMB);
  Temp_Data &= CLEAR24;

		
  //Toggle = _inp(SIG_TRNS);
  Toggle = pb_in(b) (base_address+SIG_TRNS);
  
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
  pb_out(b) (Toggle,base_address+SIG_TRNS);
  
  RECV_POLLING = 1;	// Set Polling Flag
  RECV_TIMEOUT = 0;
  RECV_STOP = 0x0;
		
  while ((RECV_POLLING == 1) && (RECV_TIMEOUT < MAX_RECV_TIMEOUT)) {
    // Check for Toggled RECV
    //RECV_TOGGLE = _inp(CHK_RECV);
    RECV_TOGGLE = pb_in(b) (base_address + CHK_RECV);
    RECV_TOGGLE &= BIT1;	// Only Save Bit 1
      
    if (RECV_TOGGLE == RECV_STOP) RECV_POLLING = 0;	// Finished if Different
    else RECV_TIMEOUT++;
    if (RECV_TIMEOUT == MAX_RECV_TIMEOUT) XFER_ERROR = -3;
  }
  if (XFER_ERROR != 0) {
    return XFER_ERROR;
  }
  
  if (XFER_ERROR == 0) {
    return Temp_Data;
  }
  else {
    //printf("Errored data is %x\n",Temp_Data);
    //printf("RECV_TOGGLE = %i\n",RECV_TOGGLE);
    return XFER_ERROR;
  }
}


/* 
 * This is called whenever a process attempts to open the device file 
 */
static int device_open(struct inode *inode, struct file *file)
{
  struct pulseblaster_device* my_dev=container_of(inode->i_cdev, struct pulseblaster_device, cdev);
  file->private_data=my_dev;

  if (my_dev->pciboard==NULL) printk("%s: device_open(%p)\n", DEVICE_NAME,file);

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

  unsigned long base_address=0;
  if (my_dev->pciboard!=NULL)
       base_address=pci_resource_start(my_dev->pciboard, 0);
  if (base_address==0) printk("%s: device_release(%p,%p)\n",DEVICE_NAME, inode, file);

  /*
   * reset device
   */
  if (0) {
    // this procedure works only on DDS cards
    // DDS Manual
    spin_lock(&(my_dev->io_lock));
    amcc_outb(   0, 0, base_address); //dev reset
    amcc_outb(   4, 2, base_address); //bytes per word
    amcc_outb(0xFF, 3, base_address); //dev to program
    amcc_outb(   0, 4, base_address); //reset address counter
    amcc_outb(   0, 6, base_address); //data out
    amcc_outb(   0, 6, base_address); //data out
    amcc_outb(   0, 6, base_address); //data out
    amcc_outb(   0, 6, base_address); //data out
    amcc_outb(   0, 5, base_address); //strobe clock
    amcc_outb(   0, 5, base_address); //strobe clock
    spin_unlock(&(my_dev->io_lock));

  }
  if (0) {
    spin_lock(&(my_dev->io_lock));
    // this procedure is successful for 24Bit cards
    // SpinCore by mail Oct 10th 2007
    amcc_outb(0,6, base_address);//store 0's to memory
    amcc_outb(0,6, base_address);
    amcc_outb(0,6, base_address);
    amcc_outb(0,6, base_address);
    amcc_outb(0,6, base_address);
    amcc_outb(0,6, base_address);
    amcc_outb(0,6, base_address);
    amcc_outb(0,6, base_address);

    amcc_outb(0,0, base_address);//dev reset
    amcc_outb(4,2, base_address);//bytes per word
    amcc_outb(0,3, base_address);//write to internal memory
    amcc_outb(0,4, base_address);//clear address counter
    amcc_outb(0,6, base_address);//data out
    amcc_outb(0,6, base_address);//data out
    amcc_outb(0,6, base_address);//data out
    amcc_outb(0,6, base_address);//data out
    amcc_outb(7,7, base_address);//programming finished
    amcc_outb(7,1, base_address);//trigger
    spin_lock(&(my_dev->io_lock));
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
	unsigned long base_address=0;
	register int i=0;

        if (my_dev->pciboard!=NULL)
               base_address=pci_resource_start(my_dev->pciboard, 0);

	while (1) {
	  // sometimes
	  unsigned char temp_byte;
	  get_user(temp_byte, buffer + i);
	  /*
	    ToDo: error checking!
	  */
	  spin_lock(&(my_dev->io_lock));
	  amcc_outb(temp_byte,6, base_address);
	  spin_unlock(&(my_dev->io_lock));

	  i++;
          /* break codition */
	  if (i>=length) break;
          /* be decent to all other processes --- at least sometimes */
	  if ((i & ((1<<12)-1)) == 0) schedule();
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
int device_ioctl(struct inode *inode,	/* see include/linux/fs.h */
		 struct file *file,	/* ditto */
		 unsigned int ioctl_num,	/* number and param for ioctl */
		 unsigned long ioctl_param)
{
	struct pulseblaster_device* my_dev=container_of(inode->i_cdev, struct pulseblaster_device, cdev);
	unsigned long base_address=0;
        if (my_dev->pciboard!=NULL)
               base_address=pci_resource_start(my_dev->pciboard, 0);

  if (ioctl_num==IOCTL_OUTB){
    unsigned char reg=(ioctl_param>>8)&0xFF;
    unsigned char val=ioctl_param&0xFF;
    int ret_val;
    /*
      check for register boundaries!
    */
    if (reg>7) {
      printk("%s: got bad register number %02x",DEVICE_NAME,0x0ff&reg);
      return -1;
    }
    /*
      ToDo: error checking!
     */
    spin_lock(&(my_dev->io_lock));
    ret_val=amcc_outb(val, reg, base_address);
    spin_unlock(&(my_dev->io_lock));
    return ret_val;
  }
  if (ioctl_num==IOCTL_INB) {
    unsigned char reg=ioctl_param&0xFF;
    unsigned char val;
    //printk("%s: reading register number %02x\n",DEVICE_NAME,0x0ff&reg);
    spin_lock(&(my_dev->io_lock));
    val=amcc_inb(reg, base_address);
    spin_unlock(&(my_dev->io_lock));
    //printk("%s: found %02x=%02x\n", DEVICE_NAME, 0x0ff&reg, 0x0ff&val);
    return val;
  }
  
  return SUCCESS;
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
	.ioctl = device_ioctl,
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
    unsigned long iobase;

    /*
     check wether we are already initialized
     */
    if (pb_dev_no>0 && pb_devs[pb_dev_no-1].pciboard==NULL) {
       /* the debug instance is already allocated, so this is a hotpulg event! */
       printk("%s: Sorry, hotpluging not supported!\n", DEVICE_NAME);
       return -1;
    }
    if (pb_dev_no>=pulseblaster_max_devno-1) {
       /* leave space for debug device */
       printk("%s: Sorry, maximum number of allocatable devices (%d) reached (please consider recompiling :-) )\n",
              DEVICE_NAME,
              pulseblaster_max_devno-1);
       return -1;
    }
    /* do i need this?, what about pci_enable_device_bars() */
    ret_val=pci_enable_device(dev);
    if (ret_val!=0) {
	printk("%s: failed to enable pci device!\n",DEVICE_NAME);
        return -1;
    }

    /* exclusive use */
    ret_val=pci_request_region(dev, 0, DEVICE_NAME);
    if (ret_val!=0) {
	printk("%s: failed to enable pci device!\n",DEVICE_NAME);
	pci_disable_device(dev);
	return -1;
    }

    /* get iobase and initialize the structure */
    iobase=pci_resource_start(dev, 0);
    pb_devs[pb_dev_no].device_open=0;
    spin_lock_init(&(pb_devs[pb_dev_no].io_lock));
    pb_devs[pb_dev_no].pciboard=dev;

    // todo: inform about slots as a hint for the physical location of the board!
    printk("%s: found a pulseblaster device with i/o base address 0x%lx, assigning no %d\n", DEVICE_NAME, iobase, pb_dev_no);

    ++pb_dev_no;

    return SUCCESS;
}

static void pulseblaster_pci_remove(struct pci_dev* dev) {
    /* do the pci stuff only */
    pci_disable_device(dev);
    pci_release_region(dev, 0);
    printk("%s: device release\n", DEVICE_NAME);
}

static struct pci_device_id pulseblaster_pci_ids[] = {
{PCI_DEVICE(0x10e8, 0x8852)},
{0,},
};

MODULE_DEVICE_TABLE(pci, pulseblaster_pci_ids);

static struct pci_driver pulseblaster_pci_driver = {
 .name = DEVICE_NAME,
 .id_table=pulseblaster_pci_ids,
 .probe=pulseblaster_pci_probe,
 .remove=pulseblaster_pci_remove,
};

/*
 * Initialize the module - Register the character device 
 */
 
static struct class *pulseblaster_class=NULL;

static int __init init_pulseblaster_module(void)
{
	int ret_val;
	int i;
	int major_dev_num, minor_dev_num;

	pb_dev_no=0;

	/*
	  register code for pci bus scanning
	 */
	ret_val=pci_register_driver(&pulseblaster_pci_driver);
        // todo error checking

	// assume pb_dev_no<pulseblaster_max_devno
        // always provide a debug device as highest id
        if (pb_dev_no<pulseblaster_max_devno) {
          pb_devs[pb_dev_no].device_open=0;
	  spin_lock_init(&(pb_devs[pb_dev_no].io_lock));
	  pb_devs[pb_dev_no].pciboard=0x0; // this is the debug flag for amcc functions
          printk("%s: creating debug device as no. %d\n", DEVICE_NAME, pb_dev_no);
	  pb_dev_no++;
        }
        else {
          printk("%s: reached overall maximum number of allocatable devices before create debug device (should not happen!)\n", DEVICE_NAME);
        }
	/*
	   get device ids
	*/
	ret_val= alloc_chrdev_region(&pb_dev_no_start, 0, pb_dev_no, DEVICE_NAME);
	major_dev_num=MAJOR(pb_dev_no_start);
	minor_dev_num=MINOR(pb_dev_no_start);
	if (ret_val < 0) {
		printk("%s failed with %d\n",
		       "Sorry, registering the character device failed\n", ret_val);
        	pci_unregister_driver(&pulseblaster_pci_driver);
		return ret_val;
	}	
	
	/*
	 * create class and register devices in /sys
	 */
	pulseblaster_class = class_create(THIS_MODULE, DEVICE_NAME);
       	if (pulseblaster_class==NULL) {
		printk("%s: failed to register class", DEVICE_NAME);
        }
	
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
            /* add devices to our class */
            if (pulseblaster_class!=NULL) {
                struct device *assoc_dev=NULL;
                if (pb_devs[i].pciboard!=NULL) assoc_dev=&(pb_devs[i].pciboard->dev);
            	pb_devs[i].classdev=class_device_create(pulseblaster_class,
							NULL,
							devno,
							assoc_dev,
							DEVICE_NAME"%d",
							i
							);
            	if (pb_devs[i].classdev==NULL) {
                    printk("%s: failed to register class device", DEVICE_NAME);
                }
	    }
	}
	return 0;
}

module_init(init_pulseblaster_module);

/* 
 * Cleanup - unregister the appropriate file from /proc 
 */
static void __exit cleanup_pulseblaster_module(void)
{
	int major_dev_num=MAJOR(pb_dev_no_start);
	int minor_dev_num=MINOR(pb_dev_no_start);
	int i;
	/* 
	 * Unregister the device 
	 * check use count?!
	 */

	if (pulseblaster_class!=NULL) {
		for (i=0; i<pb_dev_no; ++i) {
	            dev_t devno = MKDEV(major_dev_num, minor_dev_num+i);
		    class_device_destroy(pulseblaster_class, devno);
        	}
		class_destroy(pulseblaster_class);
	}

	for (i=0; i<pb_dev_no; ++i) {
	    cdev_del(&(pb_devs[i].cdev));
        }
	unregister_chrdev_region(pb_dev_no_start, pb_dev_no);

	pci_unregister_driver(&pulseblaster_pci_driver);

        pb_dev_no=0;
}

module_exit(cleanup_pulseblaster_module);
