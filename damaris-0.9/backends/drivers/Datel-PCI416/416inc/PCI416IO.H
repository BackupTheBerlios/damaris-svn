///////////////////////////////////////////////////////////////////////////////
//
//	  pci416io.h: device driver IO control function definitions
//	  handles the funtion calls to the pci416.vxd device driver			  
//	  Copyright (c) Datel, Inc. 1997
//	  Platform: Win95, Win NT
//	  Compiler: MVC5.0 + DDK
//	  Version: 3.0
//	  Author: GS
//	  created: 3/3/97
//	  modified: 7/9/98
///////////////////////////////////////////////////////////////////////////////
#ifndef PCI416IO_H
#define PCI416IO_H

#define not_VxD

#include <windows.h>

#include "pcilib32.h"
#include "pci416df.h"


#ifndef LIB_TYPE
#define LIB_TYPE WINAPI
#endif

#define DMA_THREAD_TERMINATED STILL_ACTIVE + 0x1000
#define DMA_THREAD_FINISHED STILL_ACTIVE + 0x2000
typedef struct {
	HANDLE hndDevice;
	HANDLE hEvent;
  HANDLE hThread;	
	OVERLAPPED ovlp;
  BOOL bSuccess;
	BOOL bDone;
	DWORD dwStatus;
	DWORD dwSuccess;
} PCI416_DEVICE;

typedef PCI416_DEVICE *PCI416_DEVICEPtr;

/******************************************************************************
 Function pci416_init
 Board initialization -> opens device driver initializes all PCI416 boards 
 present. 
 
 !!!!!   This function must be called first    !!!!!!!!!!!!

 Parameters:
 WORD *brdcount: Holds the number of boards found.
 
 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST
*******************************************************************************/
DWORD LIB_TYPE pci416_init(DWORD *brdcount);
DWORD LIB_TYPE init_nt(DWORD nDevices);
DWORD LIB_TYPE init_95(void);

/******************************************************************************
 Function pci416_close
 Closes the device driver. 

  !!!!!   This function must be called before exit DLL    !!!!!!!!!!!!


 Parameters: none
 
 
 Return value: 
 Error codes as defined in winerror.h
 NOERROR
*******************************************************************************/
DWORD LIB_TYPE pci416_close(DWORD brdindex);


// general pci functions
/******************************************************************************
 Function find_pci_device
 Returns the device numbers for a given device on the PCI bus.
 
 Parameters: 
 Inputs:
   WORD busno: PCI bus number.
   WORD venid: Vendor ID 
   WORD deviceid: Device ID
   WORD bufsize : size of output buffer

 Outputs:
   BYTE *buf:  Array that holds the
			   device numbers found. The field after the last device found 
			   has the value 0x0ff.  
			   The size of the array must be the number of 
			   devices expected +1. 
			   -> minimum size: 2				   

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST
 ERROR_INSUFFICIENT_BUFFER
*******************************************************************************/
DWORD LIB_TYPE find_pci_device(WORD busno, WORD venid, WORD deviceid, WORD bufsize, 
							   BYTE *buf);

/******************************************************************************
 Function get_pci_device_info
 Returns the PCI configuration space for a given device on the PCI bus.
 
 Parameters: 
 Inputs:
   WORD busno: PCI bus number.   
   WORD devno: Device number

 Outputs:
   PCI_CONFIG_SPACE *conf: Pointer to a PCI_CONFIG_SPACE structure that
						  will take the return data of the PCI configuration 
						  space.
						  The size of the structure has to be at least 64 bytes, 
						  but no more than 260 bytes. 



 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST
 ERROR_INSUFFICIENT_BUFFER
*******************************************************************************/
DWORD LIB_TYPE get_pci_device_info(WORD busno, WORD devno, 
								   PCI_CONFIG_SPACE *conf);

/******************************************************************************
 Function write_pci_port
 Writes a DWORD (4 Bytes) to an output port
 
 Parameters:
 Inputs:
   DWORD portadr: port address
   DWORD data: data to be written
    
 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  W A R N I N G  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 This function does NOT check if the input values are valid!
 Not valid values may cause system crashes!
 If this function is used to perform operations on write only registers on PCI416
 boards than their software shadow register will become invalid. 
 Software shadow registers keep track of the status of the write only registers.
 
 Return value: 
 Error codes as defined in winerror.h
 NOERROR  
*******************************************************************************/
DWORD LIB_TYPE write_pci_port(DWORD portadr, DWORD data);

/******************************************************************************
 Function READ_PCI_PORT_proc
 Reads a DWORD (4 Bytes) from an input port
 
 Parameters:
 Inputs:
   DWORD portadr: port address
  Outputs:
   DWORD *data: Data read.
 
 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  W A R N I N G  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 This function does NOT check if the input values (the port address) is valid.
 Not valid values may cause system crashes!

 Return value: 
 Error codes as defined in winerror.h
 NOERROR  
*******************************************************************************/
DWORD LIB_TYPE read_pci_port(DWORD portadr, DWORD *data);

// specific pci416 functions
// misc. functions

/******************************************************************************
 Function pci416_count
 Returns the number of PCI416 Boards
 
 Parameters:
  Inputs: none
   
  Outputs:
   DWORD *brdcount: number of PCI416 boards found
 
 Return value: 
 Error codes as defined in winerror.h
 NOERROR 
*******************************************************************************/
DWORD LIB_TYPE pci416_count(DWORD *brdcount);

/******************************************************************************
 Function pci416_get_cm_devnodes
 Returns the DEVNODEs assigned by the Configuration Manager for PCI416 Boards
 
 Parameters:
  Inputs: 
	INT brdindex: Index of board. 
				  Values: 
						-1	  : The devnodes of all the PCI416 boards
								are returned in devnodeptr.
						n>=0  :	The devnode of the board with index n is
								returned in devnodeptr.
	WORD bufsize: Size of the output array.
 Outputs:
	DWORD *devnodebuf: Array that takes the devnodes returned.
					   The array must be large enough to hold the data
					   returned.
 
 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST
 ERROR_INSUFFICIENT_BUFFER
*******************************************************************************/
DWORD LIB_TYPE pci416_get_cm_devnodes(INT brdindex, WORD bufsize, 
									  DWORD *devnodebuf);

/******************************************************************************
 Function DWORD pci416_get_cmdevinf
 Returns the PCI configuration space information of the PCI416 Boards
 
 Parameters: 
 Inputs:
	DWORD brdindex: Index of board.  Max. index = number of PCI-416 boards - 1
   
 Outputs:
   PCI_CONFIG_SPACE *conf: Pointer to a PCI_CONFIG_SPACE structure that
						  will take the return data of the PCI configuration 
						  space.
						  The size of the structure has to be at least 64 bytes, 
						  but no more than 260 bytes. 

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: no PCI416 board found
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer 
 ERROR_INVALID_HANDLE		: CM devnode not found
 ERROR_GEN_FAILURE			: Device is not functioning.
 ERROR_INVALID_DATA			: invalid pointer to out buffer or other
							  API function return error 
*******************************************************************************/
DWORD LIB_TYPE pci416_get_cmdevinf(DWORD brdindex, PCI_CONFIG_SPACE *conf);

/******************************************************************************
 Function pci416_get_badr
 Returns base address array of the given PCI416 board.
 
 Parameters: 
 Inputs:
	DWORD brdindex: Index of board.  Max. index = number of PCI-416 boards - 1
	WORD bufsize : Size of output buffer.
 Outputs:
	WORD *badrbuf: Array that take base address values. The array must hold
				   at least 6 values!	
    
 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer  
*******************************************************************************/
DWORD LIB_TYPE pci416_get_badr(DWORD brdindex, WORD bufsize, WORD *badrbuf);

/******************************************************************************
 Function pci416_getcaps
 Returns the Device capabilities (FIFO size, DMA buffer size, 
 type of ADM module, acquisition mode.
 
 Parameters: 
 Inputs:
	DWORD brdindex: Index of board.  Max. index = number of PCI-416 boards - 1
	WORD bufsize : Size of output buffer.

 Outputs:
	DWORD *buf: array of 4 DWORDs that take the device info
								 DWORD sizeFIFO;
								 DWORD bufsizeDMA;
								 DWORD indexADM;
								 DWORD acqmode;

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer 
 ERROR_CANTREAD				: can't read registry
*******************************************************************************/
DWORD LIB_TYPE pci416_getcaps(DWORD brdindex, WORD bufsize, DWORD *buf);

// pci416 register access functions

/******************************************************************************
 Function pci416_set_cmdreg
 Writes to the PCI416 command register
 
 Parameters: 
 Inputs:
	DWORD brdindex: Index of board.  Max. index = number of PCI-416 boards - 1
	WORD mode : write mode (0 = overwrite, 1= OR, 2 = AND)
	DWORD regval: value to be written

 Outputs: 
	DWORD *shregval: Current value of the shadow cmd. reg.

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not found
 ERROR_INVALID_PARAMETER	: wrong mode
*******************************************************************************/
DWORD LIB_TYPE pci416_set_cmdreg(DWORD brdindex, WORD mode, DWORD regval, 
								 DWORD *shregval);

/******************************************************************************
 Function pci416_read_statusreg
 Reads the PCI416 Status register.
 
 Parameters: 
 Inputs:
	DWORD brdindex: Index of board.  Max. index = number of PCI-416 boards - 1

 Outputs: 
	DWORD *regval: Return value of the 32bit status register.
 
 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not found
*******************************************************************************/
DWORD LIB_TYPE pci416_read_statusreg(DWORD brdindex, DWORD *regval);

/******************************************************************************
 Function pci416_set_smplcntr
 Writes to the PCI416 sample counter register
 
 Parameters: 
 Inputs:
	DWORD brdindex: Index of board.  Max. index = number of PCI-416 boards - 1
	DWORD samples : Number of samples. 
					Note: Functions writes samples-1 to the register
						   e.g 1024 samples -> writes 1023
						   see PCI-416 documentation
 
 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found
*******************************************************************************/
DWORD LIB_TYPE pci416_set_smplcntr(DWORD brdindex, DWORD samples);

/******************************************************************************
 Function pci416_set_chanadr
 Writes to the PCI416 channel address register
 
 Parameters: 
 Inputs:
	DWORD brdindex: Index of board.  Max. index = number of PCI-416 boards - 1
	WORD mode : write mode (0 = overwrite, 1= OR, 2 = AND)
	DWORD regval: value to be written

 Outputs: 
	DWORD *shregval: Current value of the shadow register.

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found
 ERROR_INVALID_PARAMETER	: wrong mode
*******************************************************************************/
DWORD LIB_TYPE pci416_set_chanadr(DWORD brdindex, WORD mode, DWORD regval, 
								  DWORD *shregval);

/******************************************************************************
 Function pci416_clear_fifo
 Resets the A/D FIFO.
 
 Parameters: 
 Inputs:
	DWORD brdindex: Index of board.  Max. index = number of PCI-416 boards - 1

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not found 
*******************************************************************************/
DWORD LIB_TYPE pci416_clear_fifo(DWORD brdindex);

/******************************************************************************
 Function pci416_enablead
 Writes to the PCI416 A/D convert enable register
 
 Parameters: 
 Inputs:
	DWORD brdindex: Index of board.  Max. index = number of PCI-416 boards - 1
	BOOL enable : TRUE= enable, FALSE=disable

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found 
*******************************************************************************/
DWORD LIB_TYPE pci416_enablead(DWORD brdindex, BOOL enable);

/******************************************************************************
 Function pci416_set_pllreg
 Writes to the PCI416 PLL data register to set the output frequency of the
 frequency synthesizer.
 This function will cycle through the 3 counters and
 set each counter value.
 
 Parameters: 
 Inputs:
	DWORD brdindex: Index of board.  Max. index = number of PCI-416 boards - 1
	DWORD valA : Counter value Counter A
	DWORD valN : Counter value Counter N

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found 
*******************************************************************************/
DWORD LIB_TYPE pci416_set_pllreg(DWORD brdindex, DWORD valA, DWORD valN);

/******************************************************************************
 Function pci416_read_fifo
 Reads a block of data from the FIFO.
 
 Parameters: 
 Inputs:
	DWORD brdindex: Index of board.  Max. index = number of PCI-416 boards - 1
	WORD count   : Number of samples to be read.
				   If count is larger than the FIFO size
				   the whole FIFO will be read.
    
    DWORD *buf: Array of count/2 DWORDs that takes the FIFO data.
				The buffer size must be at least 2*count bytes!
								   
 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not found 
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer 
*******************************************************************************/
DWORD LIB_TYPE pci416_read_fifo(DWORD brdindex, WORD count, DWORD *buf);

/******************************************************************************
 Function pci416_set_timer
 Programs the 82C54 Timer
 
 Parameters: 
 Inputs:
	DWORD brdindex: Index of board.  Max. index = number of PCI-416 boards - 1
	WORD mode    : Mode has to be one of the values as defined in timer.h
					TM_SINGLE_TRIGGER, generate a single internal trigger
					TM_CONT_TRIGGER,   generate continious internal trigger
					TM_RESET_TRIGGER,  reset (disable) trigger	
					TM_ADCLOCK		   set counter 2 for A/D clock
	WORD counter02: Counter02 holds the counter value for counter 0 
					or counter 2 in TM_ADCLOCK mode respectively.
	WORD counter1 : Counter1 holds the counter value for counter 1 
					(not used in TM_ADCLOCK mode).
  
 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found 
 ERROR_INVALID_PARAMETER	: wrong mode
*******************************************************************************/
DWORD LIB_TYPE pci416_set_timer(DWORD brdindex, WORD mode, 
								WORD counter02, WORD counter1);


/******************************************************************************
 Function pci416_set_portctrreg
 Sets the 82C55 port control register
 
 Parameters: 
 Inputs:
	DWORD brdindex: Index of board.  Max. index = number of PCI-416 boards - 1
	DWORD regval : Register value to be written.

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found 
*******************************************************************************/
DWORD LIB_TYPE pci416_set_portctrreg(DWORD brdindex, DWORD regval);

/******************************************************************************
 Function pci416_get_portctrreg
 Reads the 82C55 port control register back
 
 Parameters: 
 Inputs:
	DWORD brdindex: Index of board.  Max. index = number of PCI-416 boards - 1

  Outputs:
	DWORD *regval : Holds the register value read.

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found  
*******************************************************************************/
DWORD LIB_TYPE pci416_get_portctrreg(DWORD brdindex, DWORD *regval);

/******************************************************************************
 Function pci416_read_port
 Reads from one of the 82C55 ports
 
 Parameters: 
 Inputs:
	DWORD brdindex: Index of board.  Max. index = number of PCI-416 boards - 1
	WORD port    : (0==A, 1==B, 2==C)

  Outputs:
	DWORD *data : Holds the data read.

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found  
 ERROR_INVALID_PARAMETER	: wrong port
*******************************************************************************/
DWORD LIB_TYPE pci416_read_port(DWORD brdindex, WORD port, DWORD *data);

/******************************************************************************
 Function pci416_write_port
 Writes to one of the 82C55 ports
 
 Parameters: 
 Inputs:
	DWORD brdindex: Index of board.  Max. index = number of PCI-416 boards - 1
	WORD port    : (0==A, 1==B, 2==C)
	DWORD data	 : Data to be written.

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found  
 ERROR_INVALID_PARAMETER	: wrong port
*******************************************************************************/
DWORD LIB_TYPE pci416_write_port(DWORD brdindex, WORD port, DWORD data);

/******************************************************************************
 Function pci416_write_dac
 Sets the D/A converter register.
  
 Parameters: 
 Inputs:
	DWORD brdindex: Index of board.  Max. index = number of PCI-416 boards - 1
	WORD data	 : Data to be written.

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found  
*******************************************************************************/
DWORD LIB_TYPE pci416_write_dac(DWORD brdindex, WORD data);

// FIFO polling functions

/******************************************************************************
 Function pci416_fifo_status
 Reads the PCI416 FIFO status (bits 13-15 status reg).

 Parameters: 
 Inputs:
	DWORD brdindex: Index of board.  Max. index = number of PCI-416 boards - 1

 Outputs:
	DWORD *data	 : Holds the FIFO status bits.

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not found
*******************************************************************************/
DWORD LIB_TYPE pci416_fifo_status(DWORD brdindex, DWORD *data);

/******************************************************************************
 Function pci416_check_fifohf
 Reads the PCI416 Half Full FIFO flag (bit 14 status reg).
 
 Parameters: 
 Inputs:
	DWORD brdindex: Index of board.  Max. index = number of PCI-416 boards - 1

 Outputs:
	DWORD *data	 : Holds the status of the FIFO HF flag.
				   (0= FIFO is half full or greater)
				   (1= FIFO is less than half full)	

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not found
*******************************************************************************/
DWORD LIB_TYPE pci416_check_fifohf(DWORD brdindex, DWORD *data);

// DMA functions
/******************************************************************************
 Function pci416_setup_dma
 Setup DMA.
 
 Parameters: 
 Inputs:
	DWORD brdindex: Index of board.  Max. index = number of PCI-416 boards - 1
	DWORD mode: DMA_SINGLE or DMA_DOUBLE
	DWORD *bufsize: requested size of DMA buffer(s)


 Outputs:
	DWORD *bufsize: size of allocated DMA buffer(s)
	DWORD *hndDMAbuf : handle to first DMA buffer

 Note: if mode = DMA_DOUBLE bufsize must point to an array of 2 DWORDs!
 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer 
 ERROR_INVALID_PARAMETER	: wrong mode
 ERROR_INVALID_HANDLE		: memory handle is invalid
*******************************************************************************/
DWORD LIB_TYPE pci416_setup_dma(DWORD brdindex, DWORD mode, DWORD *bufsize, 
								DWORD *hndDMAbuf);


/******************************************************************************
 Function pci416_stop_dma
 Stops the A/D conversion and DMA!
 
 Parameters: 
 Inputs:
	DWORD brdindex: Index of board.  Max. index = number of PCI-416 boards - 1

 Outputs:
	DWORD *tcount: Remainder value of transfer count register after DMA stopped.
	Parameter can be NULL.				   
	
 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not found 
*******************************************************************************/
DWORD LIB_TYPE pci416_stop_dma(DWORD brdindex, DWORD *tcount);

/******************************************************************************
 Function pci416_reload_dma
 reloads the DMA transfer count and start address!
 
 Parameters: 
 Inputs:
	DWORD brdindex: Index of board.  Max. index = number of PCI-416 boards - 1
	DWORD bufno : Valid values are 0 and 1 
				  Selects which buffer to reload for DMA operation.
				  If setup mode is DMA_SINGLE bufno has no effect.

  Outputs:
	DWORD *bufsize: size of allocated DMA buffer
	DWORD *hndDMAbuf : handle to DMA buffer


 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not found 
*******************************************************************************/
DWORD LIB_TYPE pci416_reload_dma(DWORD brdindex, DWORD bufno,
								 DWORD *bufsize, DWORD *hndDMAbuf);

/******************************************************************************
 Function pci416_dma_status
 Reads the PCI416 controller interrupt control/status register ->
 DMA transfer status bit (bit18)
 
 Parameters: 
 Inputs:
	DWORD brdindex: Index of board.  Max. index = number of PCI-416 boards - 1

 Outputs:
	DWORD *data	 : Holds the DMA transfer status bits
				   (1= interrupt generated-> transfer complete
				    0= Bus Master op. in progress)

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not found 
*******************************************************************************/
DWORD LIB_TYPE pci416_dma_status(DWORD brdindex, DWORD *data);

/******************************************************************************
 Function pci416_read_intcsrreg
 Reads the PCI controller interrupt control/status register
 
 Parameters: 
 Inputs:
	DWORD brdindex: Index of board.  Max. index = number of PCI-416 boards - 1

 Outputs:
	DWORD *data	 : Holds register data read.

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found
*******************************************************************************/
DWORD LIB_TYPE pci416_read_intcsrreg(DWORD brdindex, DWORD *data);

/******************************************************************************
 Function pci416_set_intcsrreg
 Writes to the PCI controller interrupt control/status register
 
  Parameters: 
 Inputs:
	DWORD brdindex: Index of board.  Max. index = number of PCI-416 boards - 1
	WORD mode : write mode (0 = overwrite, 1= OR, 2 = AND
				+ 0x80 to enable read back actual register
				e.g. mode = 0x81 -> write register using OR op. 
				and then read register back)
	DWORD regval: value to be written

 Outputs: 
	DWORD *shregval: Current value of the shadow INTCSR reg.

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found
 ERROR_INVALID_PARAMETER	: wrong mode
*******************************************************************************/
DWORD LIB_TYPE pci416_set_intcsrreg(DWORD brdindex, WORD mode, DWORD regval, 
									 DWORD *shregval);

/******************************************************************************
 Function pci416_read_mcsrreg
 Reads the PCI controller bus master control/status register
 
 Parameters: 
 Inputs:
	DWORD brdindex: Index of board.  Max. index = number of PCI-416 boards - 1

 Outputs:
	DWORD *data	 : Holds register data read. 
				   The value returned also contains the control 
				   bits (8-31) that are saved in a shadow register
				   by the device driver.

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found
*******************************************************************************/
DWORD LIB_TYPE pci416_read_mcsrreg(DWORD brdindex, DWORD *data);

/******************************************************************************
 Function pci416_set_mcsrreg
 Writes to the PCI controller bus master control/status register
 
  Parameters: 
 Inputs:
	DWORD brdindex: Index of board.  Max. index = number of PCI-416 boards - 1
	WORD mode : write mode (0 = overwrite, 1= OR, 2 = AND
				+ 0x80 to enable read back actual register
				e.g. mode = 0x81 -> write register using OR op. 
				and then read register back)
	DWORD regval: value to be written

 Outputs: 
	DWORD *shregval: Current value of the shadow reg.

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found
 ERROR_INVALID_PARAMETER	: wrong mode
*******************************************************************************/
DWORD LIB_TYPE pci416_set_mcsrreg(DWORD brdindex, WORD mode, DWORD regval, 
									 DWORD *shregval);

/******************************************************************************
func pci416_read_mwarreg

parm Index of board.  Max. index = number of PCI-416 boards - 1

parm Holds register data read. 
     
rdesc
 Return value: 
 Error codes as defined in winerror.h -
 NOERROR,
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found

comm Reads the PCI controller bus master write address register (MWAR)
*******************************************************************************/
DWORD LIB_TYPE pci416_read_mwarreg(DWORD brdindex, DWORD *data);

/******************************************************************************
func pci416_set_mwarreg
 
parm Index of board.  Max. index = number of PCI-416 boards - 1
parm write mode (0 = overwrite, 1= OR, 2 = AND
parm value to be written
parm Current value of the shadow reg.

rdesc NOERROR,
	   ERROR_DEV_NOT_EXIST		: PCI416 board not  found ,
	   ERROR_INVALID_PARAMETER	: wrong mode

comm Writes to the PCI controller bus master write address SHADOW register
   NOTE: Normally the device driver handles the DMA buffer Addressing.  This function
         was created to allow USERS with dedicated memory on other devices to redirect
         the pci416 data during DMA to such devices.  The value is written to a shadow register.
         If the shadow register is non-zero, the contents of the shadow register is loaded into
         the hardware register when the pci416_setup_dma function is called.  
         This value must be a Physical not a Virtual address.
         Setting the value to zero will cause pci416_setup_dma to load the address of the
         reserved DMA buffer.By default the shadow register is loaded with zeros so this
	function does not have to be called under "normal"	circumstances.
*******************************************************************************/
DWORD LIB_TYPE pci416_set_mwarreg(DWORD brdindex, WORD mode, DWORD regval, 
									 DWORD *shregval);

/******************************************************************************
func pci416_read_mwtcreg

parm Index of board.  Max. index = number of PCI-416 boards - 1
parm Holds register data read. 
	  

rdesc NOERROR,
	   ERROR_DEV_NOT_EXIST		: PCI416 board not  found

comm Reads the PCI controller bus master write transfer count register (MWTC)
*******************************************************************************/
DWORD LIB_TYPE pci416_read_mwtcreg(DWORD brdindex, DWORD *data);

/******************************************************************************
func pci416_set_mwtcreg

parm Index of board.  Max. index = number of PCI-416 boards - 1
parm write mode (0 = overwrite, 1= OR, 2 = AND)
parm value to be written
parm Current value of the shadow reg.

rdesc NOERROR,
       ERROR_DEV_NOT_EXIST		: PCI416 board not  found,
       ERROR_INVALID_PARAMETER	: wrong mode

comm  Writes to the PCI controller bus master write transfer count register (MWTC)
	NOTE: Normally the device driver handles the DMA count register.  This function
	was created to allow USERS with dedicated memory on other devices to redirect
	the pci416 data during DMA to such devices. 
	The value is written to a shadow register.
    If the shadow register is non-zero, the contents of the shadow register 
	is loaded into the hardware register when the pci416_setup_dma function is called.
	
    Setting the value to zero will cause pci416_setup_dma to load the count from the
    DMA bufsize .  By default the shadow register is loaded with zeros so this
	function does not have to be called under "normal"	circumstances.
*******************************************************************************/
DWORD LIB_TYPE pci416_set_mwtcreg(DWORD brdindex, WORD mode, DWORD regval, 
									 DWORD *shregval);

/******************************************************************************
 Function pci416_copy_dmabuffer
 reloads the DMA transfer count and start address!
 
 Parameters: 
 Inputs:
	DWORD brdindex: Index of board.  Max. index = number of PCI-416 boards - 1
	DWORD bufno : Valid values are 0 and 1 
				  Selects from which buffer to copy.
				  If setup mode is DMA_SINGLE bufno has no effect.
	DWORD start : start byte 
	DWORD *count : number of bytes to copy
								 
								 If start is larger than the size of the 
								 selected buffer no data are copied and the 
								 function returns ERROR_INVALID_HANDLE.
								 If start+count larger the size of the 
								 selected buffer only the remaining byte
								 will be copied.	
	DWORD *pDest : Destination buffer
				   The size of the destibation buffer must be at least 
				   count bytes.
  Outputs:
	DWORD *count: number of bytes copied
	
 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not found 
 ERROR_INVALID_HANDLE		: memory handle is invalid
*******************************************************************************/
DWORD LIB_TYPE pci416_copy_dmabuffer(DWORD brdindex, DWORD bufno, 
									 DWORD start, DWORD *count, 
									 DWORD *pDest );

/******************************************************************************
 Function pci416_get_dmabuf_hndl
 Returns the handle and size of the selected DMA buffer.
 
 Parameters: 
 Inputs:
	DWORD brdindex: Index of board.  Max. index = number of PCI-416 boards - 1
	DWORD bufno : Valid values are 0 and 1 
				  Selects from which buffer to copy.
				  If setup mode is DMA_SINGLE bufno has no effect.
 
	DWORD offset: Offset of handle to be returned.
				  If offset is larger than the size of 
				  the allocated buffer ERROR_INVALID_HANDLE
				  will be returned and hnddestbuf is
				  set to NULL.
								 								 
 Outputs:
 
	DWORD *pHndl   : Handle to the DMA buffer positioned at the offset byte.
													
	DWORD bufsize : Size of the DMA buffer - offset.
 							   
 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer  
 ERROR_INVALID_HANDLE		: memory handle is invalid
*******************************************************************************/
DWORD LIB_TYPE pci416_get_dmabuf_hndl(DWORD brdindex, DWORD bufno, 
									 DWORD offset, DWORD *pHndl, 
									 DWORD *bufsize);

/******************************************************************************
 Function pci416_close_dmabuf_hndl
 Closes the handle to the DMA buffer that was returnd by pci416_setup_dma.
 Windows NT only.
 
 Parameters: 
 Inputs:
	DWORD brdindex: Index of board.  Max. index = number of PCI-416 boards - 1
							   
 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found
*******************************************************************************/
DWORD LIB_TYPE pci416_close_dmabuf_hndl(DWORD brdindex);

/******************************************************************************
 Function pci416_map_dmabuf
 Map portions of the DMA buffer into user space.
 Windows NT only.
  
 Parameters: 
 Inputs:
	DWORD brdindex: Index of board.  Max. index = number of PCI-416 boards - 1
	DWORD size : size of DMA buffer to map into user space
 Output:
	PVOID pHndl : user space handle
	
 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found
 ERROR_INVALID_HANDLE		: memory handle is invalid
*******************************************************************************/
DWORD LIB_TYPE pci416_map_dmabuf(DWORD brdindex, DWORD *pHndl, DWORD size);

/******************************************************************************
 Function pci416_unmap_dmabuf
 Unmap DMA buffer space that was mapped into user space with pci416_map_dmabuf.
 Windows NT only.
  
 Parameters: 
 Inputs:
	DWORD brdindex: Index of board.  Max. index = number of PCI-416 boards - 1
	PVOID pHndl	: handle to unmap
		
 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found
 ERROR_INVALID_HANDLE		: memory handle is invalid
*******************************************************************************/
DWORD LIB_TYPE pci416_unmap_dmabuf(DWORD brdindex, DWORD Hndl);

/******************************************************************************
 Function pci416_getError
 Returns the last error and clears the global error variable.
 
  Parameters: 
 Inputs:
	none

 Outputs: 
	LPTSTR *str : pointer that gets the address of the error message
				  (not implemented for now)

 Return value: 
 Error codes as defined in winerror.h 
*******************************************************************************/
DWORD LIB_TYPE pci416_getError(LPTSTR *str);

/******************************************************************************
@func start_daq_irq

@parm Index of board.  Max. index = number of PCI-416 boards - 1
@parm 0== PreTrigger, external trigger, 
	  1== Single Internal Trigger, 
	  2== Multi Internal Trigger
	  other default to single trigger
@parm trigger rate for multi trigger

 
@rdesc NOERROR,
	   ERROR_DEV_NOT_EXIST,		: PCI416 board not found 
	   ERROR_SERVICE_REQUEST_TIMEOUT : time out accessing dma logic	
@comm start DAQ asynchronously (Windows NT only)
*******************************************************************************/
DWORD LIB_TYPE start_daq_irq(DWORD brdindex,DWORD TriggerMode, 
						 REALTYPE *TriggerRate);

#endif