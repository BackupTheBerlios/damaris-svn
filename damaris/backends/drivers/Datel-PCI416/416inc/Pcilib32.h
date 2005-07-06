///////////////////////////////////////////////////////////////////////////////
//
//	  pcilib32.h: defines data structures to access Datel PCI devices
//	  Copyright (c) Datel, Inc. 1997
//	  Platform: Win32, Win95
//	  Compiler: MVC4.0 + DDK
//	  Version: 3.0
//	  Author: GS
//	  created: 2/4/97
//	  modified: 7/10/98
//		History: V1.1 MCSR and INTCSR moved from "pci416df.h"
//						V3.0 new defines for MWAR and MTC Reg
///////////////////////////////////////////////////////////////////////////////

#ifndef PCILIB32_H
#define PCILIB32_H
#if defined (__cplusplus)
extern "C"
{
#endif
#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

#ifndef DWORD
#define DWORD unsigned long
#define WORD unsigned short
#define BYTE unsigned char
#endif

#define HIGH_BYTE(ax) (ax>>8)
#define LOW_BYTE(ax)  (ax & 0xff)

#define HIGH_WORD(ax) (ax>>16)
#define LOW_WORD(ax)  (ax & 0xffff)


#ifndef LPDWORD
#define LPDWORD unsigned long far *
#endif

#define PCI_CONFIG_DATA_OPORT 0xCF8
#define PCI_CONFIG_DATA_IPORT 0xCFC

// Address offsets for PCI bus opertion registers
#define PCI_OPREG_FIFO	  0x20
#define PCI_OPREG_MWAR	  0x24
#define PCI_OPREG_MWTC	  0x28
#define PCI_OPREG_MRAR	  0x2C
#define PCI_OPREG_MRTC	  0x30
#define PCI_OPREG_INTCSR  0x38
#define PCI_OPREG_MCSR	  0x3C
#define PCI_OPREG_IMB4	  0x1C

#define BITS unsigned int

struct  STRUCT_ADDRESS{
	unsigned long	loc;
	unsigned int	type;			//TRUE = IO, FALSE = MEM
};
typedef struct STRUCT_ADDRESS  TYPE_ADDRESS;

// PCI configuration address register fields
typedef struct
{
	BITS	Zero		:	2; // must be set to zero
	BITS	RegisterNo	:	7;
	BITS	FunctionNo	:	3;
	BITS	DeviceNo	:	5;
	BITS	BusNo		:	7;
	BITS	Reserved	:	7;
	BITS	MSB			:	1; // must be set to 1
}PCI_CONF_ADDR_REGISTER_FIELDS;

typedef union
{
  PCI_CONF_ADDR_REGISTER_FIELDS fields;
  DWORD all;
} PCI_CONF_ADDR_REGISTER;


// PCI Configuration Space
typedef struct {
	DWORD   dev_ven_id;	  // = WORD vendor_id + WORD device_id
	DWORD	stat_cmd_rgs; //= WORD command +WORD    status;
	DWORD   class_code_revision_id; // = BYTE revision_id + BYTE class_code[3]
	DWORD	bist_head_latt_cach; //	=BYTE cache_line_size+ BYTE latency_timer+
								 // BYTE  header_type+ BYTE bist;
	DWORD   base_address[6];
	DWORD	reserved1[2];
	DWORD   expansion_rom_base;
	DWORD	reserved2[2];
	DWORD	mlat_mgrant_irqp_irql; // BYTE interrupt_line+ BYTE interrupt_pin+
								   // BYTE min_grant + BYTE max_latency;
//	BYTE	device[196];  not used on AMCC S5933 PCI controller
}PCI_TYPE_CONFIG;

//Readout structure for PCI configuration Space Header
typedef union {
	PCI_TYPE_CONFIG regs;
	DWORD buffer[16];
} PCI_CONFIG_SPACE;

typedef PCI_CONFIG_SPACE *PCI_CONFIGS_PTR;

struct BRIDGE_INTCSR{
	BITS 	out_mail_box_interrupt_select	:	4;	//	0:3
	BITS	enable_out_mail_box_interrupt	:	1;	//	4
	BITS	reserved1							:	3;	//	5:7
	BITS 	in_mail_box_interrupt_select	:	4;	// 8:11
	BITS	enable_in_mail_box_interrupt	:	1;	//	12
	BITS	reserved2							:	1;	//	13
	BITS	dma_complete_interrupt_enable	:	1;	//	14
	BITS	enable_read_complete_interrupt:	1;	//	15
	BITS	out_mail_box_interrupt_status	:	1;	//	16
	BITS	in_mail_box_interrupt_status	:	1;	//	17
	BITS	dma_transfer_status				:	1;	//	18
	BITS	read_transfer_status				:	1;	//	19
	BITS	master_abort_status				:	1;	//	20
	BITS	target_abort_status				:	1;	//	21
	BITS	reserved3							:	1;	//	22
	BITS	interrupt_status					:	1;	//	23
	BITS	fifo_endian_control				:	8;	// 24:31
};

typedef union{
  struct BRIDGE_INTCSR fields;
  DWORD buffer;
} TBRIDGE_INTCSR_REGISTER;

struct BRIDGE_MCSR{
	BITS	out_fifo_status					:	3;	//	0:2
	BITS	fifo_full							:	1;	//	3		(inbound fifo)
	BITS	fifo_half_full						:	1;	//	4          .
	BITS	fifo_empty							:	1;	//	5          .
	BITS	out_count_zero						:	1;	// 6
	BITS	dma_count_zero						:	1;	//	7		(incomming count)
	BITS	read_write_priority				:	1;	//	8
	BITS	dma_request_condition			:	1;	//	9     (write fifo)
	BITS 	dma_enable							:	1;	//	10		(write transfer enable)
	BITS	reserved1							:	1;	//	11
	BITS	not_needed_by_416					:  4;	//	12:15
	BITS	eprom_data							:	8;	//	16:23
	BITS	add_on_reset						:	1; //	24
	BITS	out_fifo_reset						:	1;	//	25   //used for DMA_ENABLE 22V10
	BITS	fifo_reset 							:	1;	//	26    (inbound fifo)
	BITS	mail_box_reset						:	1;	//	27
	BITS	reserved2							:	1;	//	28
	BITS	eprom_control  					:	3;	//	29:31
};

typedef union{
  struct BRIDGE_MCSR fields;
  DWORD buffer;
} TBRIDGE_MCSR_REGISTER;



typedef struct {
  DWORD buffer;
} TBRIDGE_MWAR_REGISTER;

typedef struct {
	DWORD buffer;
} TBRIDGE_MWTC_REGISTER;

#if defined (__cplusplus)
}
#endif
#endif
