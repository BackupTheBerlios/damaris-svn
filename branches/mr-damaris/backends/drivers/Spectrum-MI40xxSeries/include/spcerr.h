// ***********************************************************************
// SpcErr.h: error codes of the Spectrum drivers. Until may 2004 this file
//           was errors.h. Name has been changed because errors.h has been
//           already in use by windows.
// ***********************************************************************

#define ERR_OK				0x0000		//   0 No Error
#define ERR_INIT			0x0001		//   1 Initialisation error
#define ERR_NR				0x0002		//   2 Board number out of range   
#define ERR_TYP				0x0003		//   3 Unknown board Typ
#define ERR_FNCNOTSUPPORTED	0x0004		//   4 This function is not supported by the hardware
#define ERR_BRDREMAP		0x0005		//   5 The Board Index Remap table is wrong 
#define ERR_KERNELVERSION   0x0006      //   6 The kernel version and the dll version are mismatching
#define ERR_HWDRVVERSION    0x0007      //   7 The driver version doesn't match the minimum requirements of the board
#define ERR_LASTERR			0x0010		//  16 Old Error waiting to be read
#define ERR_ABORT			0x0020		//  32 Abort of wait function
#define ERR_BOARDLOCKED     0x0030      //  48 Board acess already locked by another process. it's not possible to acess one board through multiple processes

#define ERR_REG				0x0100		// 256 unknown Register for this Board
#define ERR_VALUE			0x0101		// 257 Not a possible value in this state
#define ERR_FEATURE			0x0102		// 258 Feature of the board not installed
#define ERR_SEQUENCE		0x0103		// 259 Channel sequence not allowed
#define ERR_READABORT		0x0104		// 260 Read not allowed after abort
#define ERR_NOACCESS		0x0105		// 261 Access to this register denied
#define ERR_POWERDOWN		0x0106		// 262 not allowed in Powerdown mode
#define ERR_TIMEOUT			0x0107		// 263 timeout occured while waiting for interrupt
#define ERR_CHANNEL			0x0110		// 272 Wrong number of Channel to be read out
#define ERR_RUNNING			0x0120		// 288 Board is running, changes not allowed
#define ERR_ADJUST			0x0130		// 304 Auto Adjust has an error

#define ERR_NOPCI			0x0200		// 512 No PCI bus found
#define ERR_PCIVERSION		0x0201		// 513 Wrong PCI bus version
#define ERR_PCINOBOARDS		0x0202		// 514 No Spectrum PCI boards found
#define ERR_PCICHECKSUM		0x0203		// 515 Checksum error on PCI board
#define ERR_DMALOCKED		0x0204		// 516 DMA buffer in use, try later
#define ERR_MEMALLOC		0x0205		// 517 Memory Allocation error

#define ERR_FIFOBUFOVERRUN	0x0300		// 768 Buffer overrun in FIFO mode
#define ERR_FIFOHWOVERRUN	0x0301		// 769 Hardware buffer overrun in FIFO mode
#define ERR_FIFOFINISHED	0x0302		// 770 FIFO transfer hs been finished. Number of buffers has been transferred
#define ERR_FIFOSETUP       0x0309      // 777 FIFO setup not possible, transfer rate to high (max 250 MB/s)

#define ERR_TIMESTAMP_SYNC	0x0310		// 784 Synchronisation to ref clock failed
#define ERR_STARHUB			0x0320		// 800 Autorouting of Starhub failed

