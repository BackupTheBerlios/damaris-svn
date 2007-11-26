///////////////////////////////////////////////////////////////////////////////
//
//	  416vxdio.h: device driver pci416.vxd and pci416.sys
//								IO control function definitions
//	  Copyright (c) Datel, Inc. 1997, 98
//	  Platform: Win95, Win NT 4.0
//	  Compiler: MVC4.0 + DDK
//	  Version: 3.0
//	  Author: GS
//	  created: 2/28/97
//	  modified: 7/9/98
///////////////////////////////////////////////////////////////////////////////
//@doc
/* For other languages than C:
   Examine the function definitions of this header file. 
   Use the corresponding number as defined in this header file to access the
   proper function.    
*/
#if defined (__cplusplus)
extern "C"
{
#endif
#ifndef _416VXDIO_H
#define _416VXDIO_H

/*						Device driver IOCtl functions	for NT					 */

#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

#define METHOD_BUFFERED                 0
#define METHOD_IN_DIRECT                1
#define METHOD_OUT_DIRECT               2
#define METHOD_NEITHER                  3

#define FILE_ANY_ACCESS                 0
#define FILE_READ_ACCESS          ( 0x0001 )    // file & pipe
#define FILE_WRITE_ACCESS         ( 0x0002 )    // file & pipe

#define PCI416_TYPE 40010


// general pci functions
/******************************************************************************
 Function FIND_PCI_DEVICE_Proc
 Returns the device numbers for a given device on the PCI bus.
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer points to an array of two DWORDs
 The first array field contains the bus number.  The second field
 contains  the VendorID in the lower and deviceID in the higher WORD
 that the function searches for.

 lpDIOCtl->lpvOutBuffer points to a 32 byte large array that holds the
 device numbers found. The field after the last device number found has the 
 value 0x0ff.  

 lpDIOCtl->lpcbBytesReturned : If not NULL holds the bytes returned.
							   (== sizeof(lpDIOCtl->lpvOutBuffer))

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
*******************************************************************************/
#define FIND_PCI_DEVICE_PROC		  1  
#define FIND_PCI_DEVICE_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x901, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)
/******************************************************************************
 Function GET_PCI_DEVICE_INFO_Proc
 Returns the PCI configuration space for a given device on the PCI bus.
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (WORD*) lpDIOCtl->lpvInBuffer points to an array of two WORDs
 The first array field contains the bus number.  The second field
 contains  the device ID as returned by FIND_PCI_DEVICE_Proc.

 lpDIOCtl->lpvOutBuffer points to a PCI_TYPE_CONFIG structure that
 will take the return data of the PCI configuration space.
 The size of the structure has to be at least 64 bytes, but
 no more than 260 bytes.

 lpDIOCtl->lpcbBytesReturned : If not NULL holds the bytes returned.
							   (== sizeof(lpDIOCtl->lpvOutBuffer))

 Return value: 
 Error codes as defined in winerror.h
*******************************************************************************/
#define GET_PCI_DEVICE_INFO_PROC	  2
#define GET_PCI_DEVICE_INFO_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x902, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS) 
/******************************************************************************
 Function WRITE_PCI_PORT_PROC
 Writes a DWORD (4 Bytes) to an output port
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer points to an array of two DWORDs
 The first array field contains the port address.  The second field
 contains the value to be written.

 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  W A R N I N G  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 This function does NOT check if the input values as well as the 
 size of the input buffer (lpDIOCtl->lpvInBuffer) are valid!
 Not valid values may cause system crashes!
 If this function is used to perform operations on write only registers their
 software shadow register will become invalid. Software shadow registers keep track
 of the status of the write only registers.
 
 lpDIOCtl->lpvOutBuffer : should be zero

 lpDIOCtl->lpcbBytesReturned : not used, should be NULL

 Return value: 
 Error codes as defined in winerror.h
 NOERROR  
*******************************************************************************/
#define WRITE_PCI_PORT_PROC			  3
#define WRITE_PCI_PORT_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x903, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)
/******************************************************************************
 Function READ_PCI_PORT_PROC
 Reads a DWORD (4 Bytes) from an input port.
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer contains the port address.
 (DWORD*) lpDIOCtl->lpvOutBuffer : takes the value read from the port.

 lpDIOCtl->lpcbBytesReturned : not used, should be NULL

 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  W A R N I N G  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 This function does NOT check if the input values (the port address) is valid.
 Not valid values may cause system crashes!

 Return value: 
 Error codes as defined in winerror.h
 NOERROR  
*******************************************************************************/
#define READ_PCI_PORT_PROC			  4
#define READ_PCI_PORT_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x904, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)
    
// specific pci416 functions
// misc. functions

/******************************************************************************
 Function PCI416_COUNT_PROC
 Returns the number of PCI416 Boards
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 lpDIOCtl->lpvInBuffer: should be 0
 (DWORD*)lpDIOCtl->lpvOutBuffer : holds the number of boards

 lpDIOCtl->lpcbBytesReturned : not used, should be NULL

 Return value: 
 Error codes as defined in winerror.h
 NOERROR 
*******************************************************************************/
#define PCI416_COUNT_PROC			  5 
#define PCI416_COUNT_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x905, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)
/******************************************************************************
 Function PCI416_GET_CM_DEVNODES_PROC
 Returns the DEVNODEs assigned by the Configuration Manager for PCI416 Boards
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (INT*) lpDIOCtl->lpvInBuffer: Contains the index of the board. The order of the
							   boards is determind by the Windows 95 Configuration
							   Manager.
							   Values: 
								-1	  : The devnodes of all the PCI416 boards
										are returned in lpDIOCtl->lpvOutBuffer.
								n>=0  :	The devnode of the board with index n is
										returned in lpDIOCtl->lpvOutBuffer.
									   
 

 lpDIOCtl->lpvOutBuffer points to a DWORD array that
 will take the devnode values.

 If lpDIOCtl->lpvInBuffer contains -1 the size of the array pointed by 
 lpDIOCtl->lpvOutBuffer must be large enough to hold all the devnodes.

 lpDIOCtl->lpcbBytesReturned : If not NULL holds the bytes returned.
							   (== sizeof(lpDIOCtl->lpvOutBuffer))

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST
 ERROR_INSUFFICIENT_BUFFER
*******************************************************************************/
#define PCI416_GET_CM_DEVNODES_PROC	  6
#define PCI416_GET_CM_DEVNODES_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x906, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS) // not implemented in NT
/******************************************************************************
 Function PCI416_GET_CMDEVINF_PROC
 Returns the PCI configuration space information of the PCI416 Boards
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer: Contains the index of the board. The order of the
							   boards is determind by the Windows 95 Configuration
							   Manager.

 lpDIOCtl->lpvOutBuffer points to a PCI_TYPE_CONFIG structure that
 will take the return data of the PCI configuration space.
 The size of the structure has to be at least 64 bytes, but
 no more than 260 bytes.

 lpDIOCtl->lpcbBytesReturned : If not NULL holds the bytes returned.
							   (== sizeof(lpDIOCtl->lpvOutBuffer))

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
#define PCI416_GET_CMDEVINF_PROC	  7
#define PCI416_GET_CMDEVINF_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x907, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS) // not implemented in NT
/******************************************************************************
 Function PCI416_GET_BADR_PROC
 Returns base address array of the given PCI416 board.
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer: Contains the index of the board. The order of the
							   boards is determind by the Windows 95 Configuration
							   Manager.

 (WORD*)lpDIOCtl->lpvOutBuffer: array of 6 WORDs that take base address values

 lpDIOCtl->lpcbBytesReturned : If not NULL holds the bytes returned.
							   (== sizeof(lpDIOCtl->lpvOutBuffer))

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer  
*******************************************************************************/
#define PCI416_GET_BADR_PROC		  8
#define PCI416_GET_BADR_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x908, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)
/******************************************************************************
 Function PCI416_GETCAPS_Proc
 Returns the Device capabilities (FIFO size, DMA buffer size, 
 type of ADM module, acquisition mode.
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer: Contains the index of the board. The order of the
							   boards is determind by the Windows 95 Configuration
							   Manager.

 (DWORD*)lpDIOCtl->lpvOutBuffer: array of 4 DWORDs that take the device info
								 DWORD sizeFIFO;
								 DWORD bufsizeDMA;
								 DWORD indexADM;
								 DWORD acqmode;

 lpDIOCtl->lpcbBytesReturned : If not NULL holds the bytes returned.
							   (== sizeof(lpDIOCtl->lpvOutBuffer))

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer 
 ERROR_CANTREAD				: can't read registry
*******************************************************************************/
#define PCI416_GETCAPS_PROC			  9
#define PCI416_GETCAPS_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x909, METHOD_BUFFERED, FILE_READ_ACCESS)  
    

// pci416 register access functions

/******************************************************************************
 Function PCI416_SET_CMDREG_PROC
 Writes to the PCI416 command register
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer: Array of two DWORDs.
								 The first field contains the index of the board. 
								 The order of the boards is determind by the 
								 Windows 95 Configuration Manager. 
								 The second field holds the write mode.
								 (0 = overwrite, 1= OR, 2 = AND)
								 The third field holds the value to be written
								 to the command register.
								 DWORD index
								 DWORD mode
								 DWORD cmdval

 (DWORD*)lpDIOCtl->lpvOutBuffer: returns the current value of the shadow cmd. reg.

 lpDIOCtl->lpcbBytesReturned : not used, should be NULL

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not found
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer 
 ERROR_INVALID_PARAMETER	: wrong mode
*******************************************************************************/
#define PCI416_SET_CMDREG_PROC		 10
#define PCI416_SET_CMDREG_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x90a, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS) 

/******************************************************************************
 Function PCI416_READ_STATUSREG_PROC
 Reads the PCI416 Status register.
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer: Contains the index of the board. The order of the
							   boards is determind by the Windows 95 Configuration
							   Manager.

 (DWORD*)lpDIOCtl->lpvOutBuffer: holds the return lower word 
								 of the 32bit status register

 lpDIOCtl->lpcbBytesReturned : not used, should be NULL

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not found
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer 
*******************************************************************************/
#define PCI416_READ_STATUSREG_PROC	 11
#define PCI416_READ_STATUSREG_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x90b, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS) 


/******************************************************************************
 Function PCI416_SET_SMPLCNTR_PROC
 Writes to the PCI416 sample counter register
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer: Array of two DWORDs.
								 The first field contains the index of the board. 
								 The order of the boards is determind by the 
								 Windows 95 Configuration Manager. 
								 The second field holds the value to be written
								 to the sample counter register.
								 Note: write samples-1 to the register
								 e.g 1024 samples -> write 1023
								 DWORD index
								 DWORD regval

 lpDIOCtl->lpvOutBuffer: should be 0

 lpDIOCtl->lpcbBytesReturned : not used, should be NULL

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer 
*******************************************************************************/
#define PCI416_SET_SMPLCNTR_PROC	 12
#define PCI416_SET_SMPLCNTR_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x90C, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)
/******************************************************************************
 Function PCI416_SET_CHANADR_PROC
 Writes to the PCI416 channel address register
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer: Array of three DWORDs.
								 The first field contains the index of the board. 
								 The order of the boards is determind by the 
								 Windows 95 Configuration Manager. 
								 The second field holds the write mode.
								 (0 = overwrite, 1= OR, 2 = AND)
  								 The third field holds the value to be written
								 to the register.
								 DWORD index
								 DWORD mode
								 DWORD regval

 (DWORD*)lpDIOCtl->lpvOutBuffer: returns the current value of the shadow reg.

 lpDIOCtl->lpcbBytesReturned : not used, should be NULL

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer 
 ERROR_INVALID_PARAMETER	: wrong mode
*******************************************************************************/
#define PCI416_SET_CHANADR_PROC		 13
#define PCI416_SET_CHANADR_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x90D, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)
/******************************************************************************
 Function PCI416_CLEAR_FIFO_PROC
 Resets the A/D FIFO.
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer: Contains the index of the board. The order of the
							   boards is determind by the Windows 95 Configuration
							   Manager.

 (DWORD*)lpDIOCtl->lpvOutBuffer: should be zero

 lpDIOCtl->lpcbBytesReturned : not used, should be NULL

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not found 
*******************************************************************************/
#define PCI416_CLEAR_FIFO_PROC		 14
#define PCI416_CLEAR_FIFO_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x90e, METHOD_BUFFERED, FILE_WRITE_ACCESS) 
/******************************************************************************
 Function PCI416_ENABLEAD_PROC
 Writes to the PCI416 A/D convert enable register
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer: Array of two DWORDs.
								 The first field contains the index of the board. 
								 The order of the boards is determind by the 
								 Windows 95 Configuration Manager. 
								 If the value of second field 0 the A/D conversion
								 will be disabled otherwise enabled.								 
								 DWORD index
								 DWORD regval -> 0 disable, all other enable

 lpDIOCtl->lpvOutBuffer: should be 0

 lpDIOCtl->lpcbBytesReturned : not used, should be NULL

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found 
*******************************************************************************/
#define PCI416_ENABLEAD_PROC		 15
#define PCI416_ENABLEAD_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x90f, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)

/******************************************************************************
 Function PCI416_SET_PLLREG_PROC
 Writes to the PCI416 PLL data register to set the output frequency of the
 frequency synthesizer.
 This function will cycle through the 3 counters and
 set each counter value.
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer: Array of 3 DWORDs.
								 The first field contains the index of the board. 
								 The order of the boards is determind by the 
								 Windows 95 Configuration Manager. 
								 The next two field hold the data to be written
								 to the A and N counter.
								 DWORD index
								 DWORD valA 
								 DWORD valN

 lpDIOCtl->lpvOutBuffer: should be 0

 lpDIOCtl->lpcbBytesReturned : not used, should be NULL

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found 
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer 
*******************************************************************************/
#define PCI416_SET_PLLREG_PROC		 16
#define PCI416_SET_PLLREG_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x910, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)
/******************************************************************************
 Function PCI416_READ_FIFO_PROC
 Reads a block of data from the FIFO.
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer: Array of two DWORDs.
								 The first field contains the index of the board. 
								 The order of the boards is determind by the 
								 Windows 95 Configuration Manager. 
								 The second field contains the number of samples
								 to be read. If count is larger than the FIFO size
								 the whole FIFO will be read.
								 DWORD index
								 DWORD count

 (DWORD*)lpDIOCtl->lpvOutBuffer: Array of count/2 DWORDs that takes the FIFO data.

 lpDIOCtl->lpcbBytesReturned : If not NULL holds the bytes returned.
							   (== sizeof(lpDIOCtl->lpvOutBuffer))
								   
 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not found 
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer 
*******************************************************************************/
#define PCI416_READ_FIFO_PROC		 17
#define PCI416_READ_FIFO_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x911, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)
/******************************************************************************
 Function PCI416_SETTIMER_PROC
 Programs the 82C54 Timer
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer: Array of 4 DWORDs.
								 The first field contains the index of the board. 
								 The order of the boards is determind by the 
								 Windows 95 Configuration Manager. 
								 The next three fields hold the mode, the counter
								 selector and the counter values to be set.								 
								 DWORD index
								 DWORD mode 
								 DWORD counter02
								 DWORD counter1
  Note!
  Mode has to be one of the values as defined in timer.h
	TM_SINGLE_TRIGGER, generate a single internal trigger
	TM_CONT_TRIGGER,   generate continious internal trigger
	TM_RESET_TRIGGER,  reset (disable) trigger	
	TM_ADCLOCK		   set counter 2 for A/D clock
  counter02 holds the counter value for counter 0 or 2 in TM_ADCLOCK mode
  counter1 holds the counter value for counter 1 (not used in TM_ADCLOCK mode)
  Counter values smaller than 2 will be set to 2 and values larger than 0xFFFF will
  be set to 0xFFFF.
 
 (DWORD*)lpDIOCtl->lpvOutBuffer: should be 0

 lpDIOCtl->lpcbBytesReturned : not used, should be NULL

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found 
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer 
 ERROR_INVALID_PARAMETER	: wrong mode
*******************************************************************************/
#define PCI416_SETTIMER_PROC		 18
#define PCI416_SETTIMER_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x912, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)

/******************************************************************************
 Function PCI416_SET_PORTCTRREG_PROC
 Sets the 82C55 port control register
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer: Array of two DWORDs.
								 The first field contains the index of the board. 
								 The order of the boards is determind by the 
								 Windows 95 Configuration Manager. 
								 The second field holds the register value to
								 be written.
								 DWORD index
								 DWORD regval

 lpDIOCtl->lpvOutBuffer: should be 0

 lpDIOCtl->lpcbBytesReturned : not used, should be NULL

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found 
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer 
*******************************************************************************/
#define PCI416_SET_PORTCTRREG_PROC	 19
#define PCI416_SET_PORTCTRREG_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x913, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)
/******************************************************************************
 Function PCI416_GET_PORTCTRREG_PROC
 Reads the 82C55 port control register back
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer: Contains the index of the board. 
								 The order of the boards is determind by the 
								 Windows 95 Configuration Manager. 
								 The second field holds the register value to
								 be written.
								 DWORD index
								 

 (DWORD*)lpDIOCtl->lpvOutBuffer: holds the return value 

 lpDIOCtl->lpcbBytesReturned : not used, should be NULL

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found  
*******************************************************************************/
#define PCI416_GET_PORTCTRREG_PROC	 20
#define PCI416_GET_PORTCTRREG_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x914, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)
/******************************************************************************
 Function PCI416_READ_PORT_PROC
 Reads from one of the 82C55 ports
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer: Array of two DWORDs.
								 The first field contains the index of the board. 
								 The order of the boards is determind by the 
								 Windows 95 Configuration Manager. 
								 The second field holds port to be read.
								 DWORD index
								 DWORD port (0==A, 1==B, 2==C)

 (DWORD*)lpDIOCtl->lpvOutBuffer: holds the return data.

 lpDIOCtl->lpcbBytesReturned : not used, should be NULL

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found  
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer 
 ERROR_INVALID_PARAMETER	: wrong port
*******************************************************************************/
#define PCI416_READ_PORT_PROC		 21
#define PCI416_READ_PORT_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x915, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)
/******************************************************************************
 Function PCI416_WRITE_PORT_PROC
 Writes to one of the 82C55 ports
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer: Array of 3 DWORDs.
								 The first field contains the index of the board. 
								 The order of the boards is determind by the 
								 Windows 95 Configuration Manager. 
								 The second field holds port to be read and the 
								 third field contains the data.
								 DWORD index
								 DWORD port (0==A, 1==B, 2==C)
								 DWORD data

 (DWORD*)lpDIOCtl->lpvOutBuffer: should be 0.

 lpDIOCtl->lpcbBytesReturned : not used, should be NULL

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found  
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer 
 ERROR_INVALID_PARAMETER	: wrong port
*******************************************************************************/
#define PCI416_WRITE_PORT_PROC		 22
#define PCI416_WRITE_PORT_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x916, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)
/******************************************************************************
 Function PCI416_WRITE_DAC_PROC
 Sets the D/A converter register.
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer: Array of two DWORDs.
								 The first field contains the index of the board. 
								 The order of the boards is determind by the 
								 Windows 95 Configuration Manager. 
								 The second field holds port to be read.
								 DWORD index								 
								 DWORD data

 (DWORD*)lpDIOCtl->lpvOutBuffer: should be 0.

 lpDIOCtl->lpcbBytesReturned : not used, should be NULL

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found  
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer  
*******************************************************************************/
#define PCI416_WRITE_DAC_PROC		 23
#define PCI416_WRITE_DAC_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x917, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)
// FIFO polling functions

/******************************************************************************
 Function PCI416_FIFO_STATUS_PROC
 Reads the PCI416 FIFO status (bits 13-15 status reg).
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer: Contains the index of the board. The order of the
							   boards is determind by the Windows 95 Configuration
							   Manager.

 (DWORD*)lpDIOCtl->lpvOutBuffer: holds the FIFO status bits

 lpDIOCtl->lpcbBytesReturned : not used, should be NULL

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not found
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer 
*******************************************************************************/
#define PCI416_FIFO_STATUS_PROC		 24
#define PCI416_FIFO_STATUS_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x918, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)
/******************************************************************************
 Function PCI416_CHECK_FIFOHF_PROC
 Reads the PCI416 Half Full FIFO flag (bit 14 status reg).
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer: Contains the index of the board. The order of the
							   boards is determind by the Windows 95 Configuration
							   Manager.

 (DWORD*)lpDIOCtl->lpvOutBuffer: holds the flag (0= FIFO is half full or greater)
												(1= FIFO is less than half full)

 lpDIOCtl->lpcbBytesReturned : not used, should be NULL

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not found
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer 
*******************************************************************************/
#define PCI416_CHECK_FIFOHF_PROC	 25
#define PCI416_CHECK_FIFOHF_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x919, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)
// DMA functions
/******************************************************************************
 Function PCI416_SETUP_DMA_Proc
 Setup DMA.
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer: Array of (3)4 DWORDs that contain the index of 
								 the board, the requested DMA buffer size and 
								 the number of samples per trigger. The order 
								 of the boards is determind by the Windows 95 
								 Configuration Manager. 
								 If the requested size for the DMA buffers is 
								 larger than the size of the buffer allocated 
								 during bootup the allocated buffer size will 
								 be used. The first bufsize field is used as
								 MWTC count.
								 If mode is DMA_SINGLE one buffer is set up for
								 DMA operation. In DMA_DOUBLE mode two buffers
								 are set up that can be used as ping-pong 
								 buffers. If the sum of the requested buffer 
								 size is larger than the allocated buffer size
								 the second buffer will be truncated.								 
								 DWORD index
								 DWORD mode;
								 DWORD bufsize[n] 
								 n=1 if mode = DMA_SINGLE
								 n=2 if mode = DMA_DOUBLE
								 


 (DWORD*)lpDIOCtl->lpvOutBuffer: Array of (3)4 DWORDs that contain
								 the DMA buffer info.
								 DWORD hndDMAbuf : handle to first DMA buffer								 
  								 DWORD physAdrDMAbuf : physical address of DMA 
													   buffer
								 DWORD bufsizeDMA[n] : size of DMA buffer(s) 
													   used 
								 n=1 if mode = DMA_SINGLE
								 n=2 if mode = DMA_DOUBLE
		
 lpDIOCtl->lpcbBytesReturned : If not NULL holds the bytes returned.
							   (== sizeof(lpDIOCtl->lpvOutBuffer))

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer 
 ERROR_INVALID_PARAMETER	: wrong mode
 ERROR_INVALID_HANDLE		: memory handle is invalid
******************************************************************************/
#define PCI416_SETUP_DMA_PROC		 26
#define PCI416_SETUP_DMA_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x91A, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)
/******************************************************************************
 Function PCI416_STOP_DMA_PROC
 Stops DMA without disabeling A/D conversion!
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer: Contains the index of the board. The order of the
							   boards is determind by the Windows 95 Configuration
							   Manager.

 (DWORD*)lpDIOCtl->lpvOutBuffer: If the buffer is not zero it will hold the
								 value of the transfer count register (MWTC)
								 at the time the DMA transfer was stopped.

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not found 
*******************************************************************************/
#define PCI416_STOP_DMA_PROC		 27
#define PCI416_STOP_DMA_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x91B, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)
/******************************************************************************
 Function PCI416_RELOAD_DMA_PROC
 Reloads the DMA transfer count and start address!
 
 Parameters: 
 (DWORD*) lpDIOCtl->lpvInBuffer: Array of 2 DWORDS
								 DWORD index : Index of the board. The order of 
											   the boards is determind by the 
											   Windows 95 Configuration 
											   Manager.
								 DWORD bufno : Valid values are 0 and 1 
											   Selects which buffer to reload
											   for DMA op.
											   If mode is DMA_SINGLE bufno has
											   no effect.

(DWORD*)lpDIOCtl->lpvOutBuffer: Array of 3 DWORDs that contain
								the DMA buffer info.
								DWORD hndDMAbuf : handle to DMA buffer								 
  								DWORD physAdrDMAbuf : physical address of DMA 
													   buffer
								DWORD bufsizeDMA : size of DMA buffer
												   used 

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not found 
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer 
*******************************************************************************/
#define PCI416_RELOAD_DMA_PROC		 28
#define PCI416_RELOAD_DMA_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x91C, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)
/******************************************************************************
 Function PCI416_DMA_STATUS_PROC
 Reads the PCI416 controller interrupt control/status register ->
 DMA transfer status bit (bit18)
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer: Contains the index of the board. The order of the
							   boards is determind by the Windows 95 Configuration
							   Manager.

 (DWORD*)lpDIOCtl->lpvOutBuffer: holds the DMA transfer status bits
								 (1= interrupt generated-> transfer complete
								  0= Bus Master op. in progress)

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not found
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer 
*******************************************************************************/
#define PCI416_DMA_STATUS_PROC		 29
#define PCI416_DMA_STATUS_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x91D, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)
/******************************************************************************
 Function PCI416_READ_INTCSRREG_PROC
 Reads the PCI controller interrupt control/status register
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer: DWORD that contains the index of the board. 
								 The order of the boards is determind by the 
								 Windows 95 Configuration Manager. 

 (DWORD*)lpDIOCtl->lpvOutBuffer: Return value of the register.
 
 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer 
*******************************************************************************/
#define PCI416_READ_INTCSRREG_PROC	 30
#define PCI416_READ_INTCSRREG_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x91E, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)
/******************************************************************************
 Function PCI416_SET_INTCSRREG_PROC
 Writes to the PCI controller interrupt control/status register
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer: Array of three DWORDs.
								 The first field contains the index of the board. 
								 The order of the boards is determind by the 
								 Windows 95 Configuration Manager. 
								 The second field holds the write mode+ read back
								 option.
								 (0 = overwrite, 1= OR, 2 = AND
								  + 0x80 to enable read back actual register
								  e.g. mode = 0x81 -> write register using OR op. 
								  and then read register back)
								 The third field holds the value to be written  								 
								 to the register.
								 DWORD index
								 DWORD mode
								 DWORD regval

 (DWORD*)lpDIOCtl->lpvOutBuffer: returns the current value of the shadow reg.
 
 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer 
 ERROR_INVALID_PARAMETER	: wrong mode
*******************************************************************************/
#define PCI416_SET_INTCSRREG_PROC	 31
#define PCI416_SET_INTCSRREG_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x91F, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)
/******************************************************************************
 Function PCI416_READ_MCSRREG_PROC
 Reads the PCI controller bus master control/status register
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer: DWORD that contains the index of the board. 
								 The order of the boards is determind by the 
								 Windows 95 Configuration Manager. 
								 
 (DWORD*)lpDIOCtl->lpvOutBuffer: Return value of the register.
								 The value returned also contains the control 
								 bits (8-31) that are saved in a shadow register
								 by the device driver.
 
 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer  
*******************************************************************************/
#define PCI416_READ_MCSRREG_PROC	 32
#define PCI416_READ_MCSRREG_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x920, METHOD_BUFFERED, FILE_READ_ACCESS)
/******************************************************************************
 Function PCI416_SET_MCSRREG_PROC
 Writes to the PCI controller bus master control/status register
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer: Array of three DWORDs.
								 The first field contains the index of the board. 
								 The order of the boards is determind by the 
								 Windows 95 Configuration Manager. 
								 The second field holds the write mode+ read back
								 option.
								 (0 = overwrite, 1= OR, 2 = AND
								  + 0x80 to enable read back actual register
								  e.g. mode = 0x81 -> write register using OR op. 
								  and then read register back)
								 The third field holds the value to be written  								 
								 to the register.
								 DWORD index
								 DWORD mode
								 DWORD regval

 (DWORD*)lpDIOCtl->lpvOutBuffer: returns the current value of the shadow reg.
 
 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer 
 ERROR_INVALID_PARAMETER	: wrong mode
*******************************************************************************/
#define PCI416_SET_MCSRREG_PROC		 33
#define PCI416_SET_MCSRREG_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x921, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)
/******************************************************************************
 Function PCI416_COPY_DMABUFFER_PROC
 Copy DMA buffer to another buffer.
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer: Array of 4 DWORDs
								 DWORD index : Index of the board. The order of 
											   the boards is determind by the 
											   Windows 95 Configuration 
											   Manager.
								 DWORD bufno : Valid values are 0 and 1 
											   Selects which buffer to copy.
											   If mode is DMA_SINGLE bufno has
											   no effect.
								 DWORD start : start byte 
								 DWORD count : number of bytes to copy
								 
								 If start is larger than the size of the 
								 selected buffer no data are copied and the 
								 function returns ERROR_INVALID_HANDLE.
								 If start+count larger the size of the 
								 selected buffer only the remaining byte
								 will be copied.

 (DWORD*)lpDIOCtl->lpvOutBuffer: DWORD hnddestbuf : Handle to the 
													desitination buffer.
													The size of the 
													destination buffer
													must be at least count
													bytes!
  								 
 lpDIOCtl->lpcbBytesReturned : If not NULL holds the bytes copied.
							   

 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer  
 ERROR_INVALID_HANDLE		: memory handle is invalid
*******************************************************************************/
#define PCI416_COPY_DMABUFFER_PROC	 34
#define PCI416_COPY_DMABUFFER_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x922, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)
/******************************************************************************
 Function PCI416_GET_DMABUF_HNDL_PROC
 Returns the handle and size of the selected DMA buffer.
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer: Array of 3 DWORDs
								 DWORD index : Index of the board. The order of 
											   the boards is determind by the 
											   Windows 95 Configuration 
											   Manager.
								 DWORD bufno : Valid values are 0 and 1 
											   Selects which buffer to copy.
											   If mode is DMA_SINGLE bufno has
											   no effect.
								 DWORD offset: Offset of handle to be returned.
											   If offset is larger than the size of 
											   the allocated buffer ERROR_INVALID_HANDLE
											   will be returned and hnddestbuf is
											   set to NULL.
								 								 

 (DWORD*)lpDIOCtl->lpvOutBuffer: Array of 2 DWORDs
								 DWORD hndbuf  : Handle to the 
													DMA buffer positioned 
													at the offset byte.
													
								 DWORD bufsize  : Size of the DMA buffer - offset.
 							   
 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer  
 ERROR_INVALID_HANDLE		: memory handle is invalid
*******************************************************************************/
#define PCI416_GET_DMABUF_HNDL_PROC	 35
#define PCI416_GET_DMABUF_HNDL_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x923, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)
/******************************************************************************
 Function PCI416_READ_MWARREG_PROC
 Reads the PCI controller bus master write address register
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer: DWORD that contains the index of the board. 
								 The order of the boards is determind by the 
								 Windows 95 Configuration Manager. 
								 
 (DWORD*)lpDIOCtl->lpvOutBuffer: Return value of the register.
								 
 
 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer  
*******************************************************************************/
#define PCI416_READ_MWARREG_PROC	 36
#define PCI416_READ_MWARREG_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x924, METHOD_BUFFERED, FILE_READ_ACCESS)
/******************************************************************************
 Function PCI416_SET_MWARREG_PROC
 Writes to the PCI controller bus master control/status register
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer: Array of three DWORDs.
								 The first field contains the index of the board. 
								 The order of the boards is determind by the 
								 Windows 95 Configuration Manager. 
								 The second field holds the write mode+ read back
								 option.
								 (0 = overwrite, 1= OR, 2 = AND)
								 The third field holds the value to be written  								 
								 to the register.
								 DWORD index
								 DWORD mode
								 DWORD regval

 (DWORD*)lpDIOCtl->lpvOutBuffer: returns the current value of the shadow reg.
 
 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer 
 ERROR_INVALID_PARAMETER	: wrong mode
*******************************************************************************/
#define PCI416_SET_MWARREG_PROC		 37
#define PCI416_SET_MWARREG_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x925, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)

/******************************************************************************
 Function PCI416_READ_MWTCREG_PROC
 Reads the PCI controller bus master write transfer count register
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer: DWORD that contains the index of the board. 
								 The order of the boards is determind by the 
								 Windows 95 Configuration Manager. 
								 
 (DWORD*)lpDIOCtl->lpvOutBuffer: Return value of the register.
								 
 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer  
*******************************************************************************/
#define PCI416_READ_MWTCREG_PROC	 38
#define PCI416_READ_MWTCREG_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x926, METHOD_BUFFERED, FILE_READ_ACCESS)
/******************************************************************************
 Function PCI416_SET_MWTCREG_PROC
 Writes to the PCI controller bus master write transfer count register
 
 Parameters: 
 LPDIOC lpDIOCtl pointer to DIOCPARAMETERS  data structure.

 (DWORD*) lpDIOCtl->lpvInBuffer: Array of three DWORDs.
								 The first field contains the index of the board. 
								 The order of the boards is determind by the 
								 Windows 95 Configuration Manager. 
								 The second field holds the write mode+ read back
								 option.
								 (0 = overwrite, 1= OR, 2 = AND)
								 The third field holds the value to be written  								 
								 to the register.
								 DWORD index
								 DWORD mode
								 DWORD regval

 (DWORD*)lpDIOCtl->lpvOutBuffer: returns the current value of the shadow reg.
 
 Return value: 
 Error codes as defined in winerror.h
 NOERROR
 ERROR_DEV_NOT_EXIST		: PCI416 board not  found
 ERROR_INSUFFICIENT_BUFFER	: invalid size of in/out buffer 
 ERROR_INVALID_PARAMETER	: wrong mode
*******************************************************************************/
#define PCI416_SET_MWTCREG_PROC		 39
#define PCI416_SET_MWTCREG_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x927, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)


/******************************************************************************
@func pci416_dma_pause_resume

@parm Index of board.  Max. index = number of PCI-441 boards - 1
@parm  flags : 0 = resume 1 = pause
 
@rdesc NOERROR
	   ERROR_DEV_NOT_EXIST		: PCI416 board not found 
	   ERROR_SERVICE_REQUEST_TIMEOUT : time out accessing dma logic	
@comm Pauses/ Resumes DMA
*******************************************************************************/
#define PCI416_PAUSE_RESUME_DMA_PROC 40
#define PCI416_PAUSE_RESUME_DMA_PROC_NT \
    CTL_CODE(PCI416_TYPE, 0x928, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)

// NT only
/******************************************************************************
@func PCI416_START_DMA

@parm Array of two DWORDs:
		TRGMODE: 0== PreTrigger, external trigger, 
						 1== Single Internal Trigger, 
	  
		BRDINDEX:Index of board.  Max. index = number of PCI-416 boards - 1

 
@rdesc NOERROR,
	   ERROR_DEV_NOT_EXIST,		: PCI416 board not found 
	   ERROR_SERVICE_REQUEST_TIMEOUT : time out accessing dma logic	
@comm start DAQ asynchronously (Windows NT only)
*******************************************************************************/
#define PCI416_START_DMA_PROC_NT \
	CTL_CODE(PCI416_TYPE, 0x929, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)

/******************************************************************************
@func PCI416_CLOSE_DMAHNDL_PROC_NT

@rdesc NOERROR,
	   ERROR_DEV_NOT_EXIST,		: PCI416 board not found 
	   
@comm Close DMA handle opend with SETUP_DMA 
			Windows NT only
*******************************************************************************/
#define PCI416_CLOSE_DMAHNDL_PROC_NT \
	CTL_CODE(PCI416_TYPE, 0x92A, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)

/******************************************************************************
@func PCI416_MAP_MEMORY_PROC_NT

@parm : Array of two DWORDs:
				param[0] returns handle to beginning of DMA buffer
				param[1] size of mapped buffer

 
@rdesc NOERROR,
	   ERROR_DEV_NOT_EXIST,		: PCI416 board not found 
	   
@comm Maps portion of the DMA buffer into user space 
			Windows NT only
*******************************************************************************/
#define PCI416_MAP_MEMORY_PROC_NT \
	CTL_CODE(PCI416_TYPE, 0x92B, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)

/******************************************************************************
@func PCI416_UNMAP_MEMORY_PROC_NT

@parm : DWORD HANDLE : handle to be unmapped				
 
@rdesc NOERROR,
	   ERROR_DEV_NOT_EXIST,		: PCI416 board not found 
	   
@comm Unmaps mapped DMA buffer space
			Windows NT only
*******************************************************************************/
#define PCI416_UNMAP_MEMORY_PROC_NT \
	CTL_CODE(PCI416_TYPE, 0x92C, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)

#if defined (__cplusplus)
}
#endif
#endif