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
#include <asm/uaccess.h>	/* for get_user and put_user */
#include <asm/io.h>

#include "pulseblaster.h"
#define SUCCESS 0
#define DEVICE_NAME "pulseblaster"

/* 
 * Is the device open right now? Used to prevent
 * concurent access into the same device 
 */
static int Device_Open = 0;

/*
 * dynamic allocated device number
 */
static int major_dev_num = 0;

/*
 * the base io address
 * 0 means debug mode without hardware accesss, writes everything to kernel log
 */
static int base_address = 0;

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Achim Gaedke");
module_param(base_address,int, 0);
MODULE_PARM_DESC(base_address, " IO base address of device, use 'lspci -v' to find out, 0 is debug mode");

/* 
 * This is called whenever a process attempts to open the device file 
 */
static int device_open(struct inode *inode, struct file *file)
{

  if (base_address==0) printk("%s: device_open(%p)\n", DEVICE_NAME,file);

  /* 
   * We don't want to talk to two processes at the same time 
   */
  if (Device_Open)
    return -EBUSY;  
  Device_Open++;
  try_module_get(THIS_MODULE);
  return SUCCESS;
}

/*************************************************************************************/

/*  code taken form spincore example:
 *  This function writes a byte to a board that uses the amcc chip.
 *  SP3 and previous revision boards use this interface. Later designs
 *  can be programmed directly.
 */

static int amcc_outb(char data, unsigned short address) {

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
		outb (0,base_address+SIG_TRNS);

		// Transfer Address
		
		// Read RECV bit from the Board
		RECV_START = inb_p(base_address+CHK_RECV);

		RECV_START &= CLEAR31;	// Only Save LSB


		// Send Address to OGMB
		outl(Temp_Address, base_address+OGMB);

		RECV_POLLING = 1;	// Set Polling Flag
		RECV_TIMEOUT = 0;
		while ((RECV_POLLING == 1) && (RECV_TIMEOUT < MAX_RECV_TIMEOUT)) 
		{
			RECV_TOGGLE = inb_p(base_address+CHK_RECV);

			RECV_TOGGLE &= CLEAR31;	// Only Save LSB
			if (RECV_TOGGLE != RECV_START) RECV_POLLING = 0;	// Finished if Different
			else RECV_TIMEOUT++;
			if (RECV_TIMEOUT == MAX_RECV_TIMEOUT) XFER_ERROR = -2;
		}

		// Transfer Complete (Clear) Signal
		outb_p(0, base_address+SIG_TRNS);

		// Transfer Data

		// Read RECV bit from the Board
		RECV_START = inb_p(base_address+CHK_RECV);

		RECV_START &= CLEAR31;	// Only Save LSB

		// Send Data to OGMB
		outl_p(Temp_Data, base_address+OGMB);

		RECV_POLLING = 1;	// Set Polling Flag
		RECV_TIMEOUT = 0;
		while ((RECV_POLLING == 1) && (RECV_TIMEOUT < MAX_RECV_TIMEOUT)) 
		{
			// Check for Toggled RECV
                        RECV_TOGGLE = inb_p(base_address+CHK_RECV);

			RECV_TOGGLE &= CLEAR31;	// Only Save LSB
			if (RECV_TOGGLE != RECV_START) RECV_POLLING = 0;	// Finished if Different
			else RECV_TIMEOUT++;
			if (RECV_TIMEOUT == MAX_RECV_TIMEOUT) XFER_ERROR = -2;
		}

		// Transfer Complete (Clear) Signal
		outb_p(0, base_address+SIG_TRNS);

	return XFER_ERROR;	
}


/**
 * Read a byte from a board using the AMCC chip
 *
 *
 */
static char amcc_inb(unsigned int address) {
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

  amcc_outb(address, 8);
  amcc_outb(0,READ_ADDR); // Tell board to start a read cycle

  RECV_POLLING = 1;	// Set Polling Flag
  RECV_TIMEOUT = 0;
  RECV_START = 0x2; // Value expected when data is ready

  while ((RECV_POLLING == 1) && (RECV_TIMEOUT < MAX_RECV_TIMEOUT)) {
      // Check for Toggled RECV
      //RECV_TOGGLE = _inp(CHK_RECV);
      RECV_TOGGLE = inb_p (base_address+CHK_RECV);
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
  Temp_Data = inb_p (base_address+ICMB);
  Temp_Data &= CLEAR24;

		
  //Toggle = _inp(SIG_TRNS);
  Toggle = inb_p (base_address+SIG_TRNS);
  
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
  outb_p (Toggle,base_address+SIG_TRNS);
  
  RECV_POLLING = 1;	// Set Polling Flag
  RECV_TIMEOUT = 0;
  RECV_STOP = 0x0;
		
  while ((RECV_POLLING == 1) && (RECV_TIMEOUT < MAX_RECV_TIMEOUT)) {
    // Check for Toggled RECV
    //RECV_TOGGLE = _inp(CHK_RECV);
    RECV_TOGGLE = inb_p (base_address + CHK_RECV);
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


static int device_release(struct inode *inode, struct file *file)
{

  if (base_address==0) printk("%s: device_release(%p,%p)\n",DEVICE_NAME, inode, file);

  /* 
   * We're now ready for our next caller 
   */
  Device_Open--;

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
	int i;
	for (i = 0; i < length; i++) {
	  unsigned char temp_byte;
	  get_user(temp_byte, buffer + i);
	  /*
	    ToDo: error checking!
	  */
	  amcc_outb(temp_byte,6);
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
  if (ioctl_num==IOCTL_OUTB){
    unsigned char reg=(ioctl_param>>8)&0xFF;
    unsigned char val=ioctl_param&0xFF;
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
    return amcc_outb(val, reg);
  }
  if (ioctl_num==IOCTL_INB) {
    unsigned char reg=ioctl_param&0xFF;
    unsigned char val;
    //printk("%s: reading register number %02x\n",DEVICE_NAME,0x0ff&reg);
    val=amcc_inb(reg);
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
struct file_operations Fops = {
	.read = device_read,
	.write = device_write,
	.ioctl = device_ioctl,
	.open = device_open,
	.release = device_release,	/* a.k.a. close */
};

/*
 * Initialize the module - Register the character device 
 */
int init_module()
{
	int ret_val;
	/* 
	 * Register the character device (atleast try) 
	 */
	ret_val = register_chrdev(0, DEVICE_NAME, &Fops);

	/* 
	 * Negative values signify an error 
	 */
	if (ret_val < 0) {
		printk("%s failed with %d\n",
		       "Sorry, registering the character device failed", ret_val);
		return ret_val;
	}
	major_dev_num=ret_val;

	/*
	  todo: insert some code for pci bus scanning
	 */
	printk("%s: The major device number is %d, IO base address is 0x%x.\n", DEVICE_NAME, major_dev_num,base_address);

	return 0;
}

/* 
 * Cleanup - unregister the appropriate file from /proc 
 */
void cleanup_module()
{
	int ret;

	/* 
	 * Unregister the device 
	 */
	ret = unregister_chrdev(major_dev_num, DEVICE_NAME);

	/* 
	 * If there's an error, report it 
	 */
	if (ret < 0)
		printk("Error in module_unregister_chrdev: %d\n", ret);
}
