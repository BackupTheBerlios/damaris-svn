///////////////////////////////////////////////////////////////////////////////
//
//	  pci416df.h: defines data structures to access PCI416 DAQ devices 
//	  Copyright (c) Datel, Inc. 1997
//	  Platform: Win95, Win NT
//	  Compiler: MVC4.0 + DDK
//	  Version: 3.0
//	  Author: GS
//	  created: 2/4/97
//	  modified: 7/9/98
//		History: V1.1 MCSR and INTCSR moved to "pcilib32.h"
//						 V3.0 now for Win 95 and NT
///////////////////////////////////////////////////////////////////////////////
#ifndef PCI416DF_H
#define PCI416DF_H

#include "pcilib32.h"

#ifndef REALTYPE
#define REALTYPE double
#endif

#define UROUND(X) (DWORD)(X + 0.5)

#define FFIFO 0x2000 // samples = WORDS
#define HFIFO 0x1000 // samples

#define CAPS_SIZE 5 // number of ULONGS for Caps buffer

// PASS THRU ADDRESS REGISTER LATCH
#define ARL_CMD_STS_REG				   0l
#define ARL_SMPLCNT_FIFORD			   4l
#define ARL_CHADDRREG_CLEARFIFO		   8l	
#define ARL_AD_ENABLE				  12l
#define ARL_PLLREG					  16l

#define ARL_CNTR0					   0l
#define ARL_CNTR1					   4l
#define ARL_CNTR2					   8l
#define ARL_CNT_CTRL_REG			  12l

#define ARL_PORTA					  16l
#define ARL_PORTB					  20l
#define ARL_PORTC					  24l
#define ARL_PORT_CTRL_REG			  28l

#define ARL_DAC_REG					  32l

/* Bit masks for register write operations*/
#define RWF_OVR 0x00 // reg = newval		ONLY
#define RWF_OR	0x01 // reg |= newval    ONE OPTION
#define RWF_AND	0x02 // reg &= newval  VALID AT THE TIME
#define RWF_RB	0x80 // read back register value if register is r/w
					 // can be set with one of the above
/* DMA mode flags */
#define DMA_SINGLE 0
#define DMA_DOUBLE 1

struct PCI416_COMMAND_REGISTER{
	BITS	trigger_select		:	2;
	BITS	interrupt_select	:	1;
	BITS	scan_select			:	1;
	BITS	auto_increment		:	1;
	BITS	marker_select		:	1;
	BITS	pretrigger			:	1;
	BITS	interrupt_enable	:	1;
	BITS	pll					:	3;
	BITS	mux_select			:	3;
	BITS	adm_control			:	2;
	BITS	dma_on				:	1;
	BITS	spare					:	15;
};

typedef union{
  struct PCI416_COMMAND_REGISTER fields;
  DWORD buffer;
} TPCI416_COMMAND_REGISTER;

struct	PCI416_STATUS_REGISTER{
	BITS	trigger_select		:	2;
	BITS	scan_select			:	1;
	BITS	auto_increment		:	1;
	BITS	acquire				:	1;
	BITS	marker_select		:	1;
	BITS	pretrigger			:	1;
	BITS	analog_trigger		:	1;
	BITS	model					:	4;
	BITS	arm					:	1;
	BITS	empty					:	1;
	BITS	half_full			:	1;
	BITS	full					:	1;
	BITS	spare					:	16;
};

typedef union{
  struct PCI416_STATUS_REGISTER fields;
  DWORD buffer;
} TPCI416_STATUS_REGISTER;

struct PCI416_SAMPLE_COUNT_REGISTER{
	unsigned long	count;

};

struct PCI416_CHANNEL_REGISTER{
	BITS	channel				: 	4;
	BITS	scan					:	4;
	BITS	led					: 	1;
	BITS	spare1				:	7;
	BITS	spare2				:	16;
};

typedef union{
  struct PCI416_CHANNEL_REGISTER fields;
  DWORD buffer;
} TPCI416_CHANNEL_REGISTER;

                                        
struct PCI416_CONVERT_ENABLE_REGISTER{
	BITS	sparel				:	15;
	BITS	convert				:	1;
	BITS  spareu				:	16;
};

typedef union{
  struct PCI416_CONVERT_ENABLE_REGISTER fields;
  DWORD buffer;
} TPCI416_CONVERT_ENABLE_REGISTER;

struct PCI416_PLL_REGISTER{
	BITS	data					:	4;
	BITS	spare1				:	12;
	BITS	spare2				:	16;
};

typedef union{
  struct PCI416_PLL_REGISTER fields;
  DWORD buffer;
} TPCI416_PLL_REGISTER;

struct PCI416_DAC_REGISTER	{
	BITS	data					:	12;
	BITS	spare1				:	4;
   BITS	spare2				:	16;
};

typedef union{
  struct PCI416_DAC_REGISTER fields;
  DWORD buffer;
} TPCI416_DAC_REGISTER;

typedef struct {
	union{
		DWORD both;
		struct {
			int low;
			int high;
		} word;
	}sample;
} TYPE_FIFO_DATA;

typedef struct{
	WORD  model;
	WORD  bits;
	WORD  channels;
	float fmax_single;
	float fmax_scan;
	WORD channel_scan;
	WORD autoincr_scan;
	WORD scan_select;
	WORD scan_count;
	WORD ssh;
	WORD scycle;
	float minv;
	float maxv;
    long int	mincode,maxcode;

}ADM_STATS;

typedef struct BoardInfo
{
	DWORD ADMindex, 
	      BusNo,
	      SlotNo,
	      VendorID,
	      DeviceID,
	      CommandR,
	      StatusR,
	      ClassCodeR,
	      IRLR,
	      BADR[6];
} TBoardInfo;

typedef TBoardInfo *TBoardInfoPtr; 

enum{single_trigger,continuous_trigger};
enum{T_INTERNAL,T_EXTERNAL,T_ANALOG,T_ANALOG_FE}; //trigger source
enum{M_DMA, M_HF_POLL, M_HF_POLL_DDISK,M_HF_INT} ; //acq_mode
enum{D_MEM, D_DIRECT};	// disk_mode: data to memory or direct to disk
enum{F_BINARY};
enum{C_INTERNAL,C_EXTERNAL}; //clock_source
enum{R_NORMAL,R_BATCH,R_START_EXIT}; //run_mode
enum{I_DMA, I_HF, I_EOS }; // interrupt mode
enum{B_NORMAL, B_MARKER}; //marker mode

#endif
