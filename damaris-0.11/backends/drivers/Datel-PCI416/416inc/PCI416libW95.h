///////////////////////////////////////////////////////////////////////////////
//	  pci416libw95.h: Function definitions for Datel PCI-416 Control library 
//					  for both the S593X PCI controller and the pass thru 
//					  data acq.logic
//	  Copyright (c) Datel, Inc. 1997
//	  Platform: Win95
//	  Compiler: Microsoft Visual C++ V4.0
//	  Version: 1.0
//	  Author: GS
//	  created: 3/3/97
//	  modified: 4/16/97
//	  Comment: Ports DOS p416_lib to WIN32  
//			   TYPE_BOARD_CONFIG and TYPE_DAQ_CONFIG structures are obsolete
//			   and not supported.
//			   Boards are accessed by a simple board index number.
//			   A number of functions from the DOS library are not ported 
//			   to Windows. These functions are marked as obsolete. See below.
/////////////////////////////////////////////////////////////////////////////


#ifndef P416LIBW95_H
#define P416LIBW95_H

#ifndef LIB_TYPE
#define LIB_TYPE WINAPI
#endif

#ifndef REALTYPE
#define REALTYPE double
#endif


/******************************************************************************
 Function get_adm_stats
 Returns the ADM characteristics.
 
 Parameters: 
 Input: 
  DWORD brdindex:	index of board
 
 Output:
   ADM_STATS *admstats : Pointer to ADM_STATS structure that gets the data.
 

 Return value: 0 -> no Error
			  -1 -> Error retreive error with pci416_getError
*******************************************************************************/
INT LIB_TYPE get_adm_stats(DWORD brdindex, ADM_STATS *admstats);
/*****************************************************************************
							PASS THROUGH (A/D control)
*****************************************************************************/
///////////////////////////////////////////////////////////////////////////////
// reads 8255 port control register
///////////////////////////////////////////////////////////////////////////////
DWORD LIB_TYPE read_pt_port_control_reg(DWORD brdindex);

///////////////////////////////////////////////////////////////////////////////
// set 8255 port control register
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE write_pt_port_control_reg(DWORD brdindex, UINT val);

///////////////////////////////////////////////////////////////////////////////
// Reads from one of the 8255 I/O registers ->  0=a, 1=b, 2=c
///////////////////////////////////////////////////////////////////////////////
DWORD LIB_TYPE read_pt_port_data(DWORD brdindex, UINT prt);

///////////////////////////////////////////////////////////////////////////////
// write to one of the 8255 I/O registers ->  0=a, 1=b, 2=c
///////////////////////////////////////////////////////////////////////////////
DWORD LIB_TYPE write_pt_port_data(DWORD brdindex, UINT prt, UINT val);

///////////////////////////////////////////////////////////////////////////////
//	Enables / Disables auto PT(A/D Data) FIFO to Bridge fifo 
//
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE	enable_pt_dma_logic(DWORD brdindex, UINT md);

///////////////////////////////////////////////////////////////////////////////
//	Suspends / Resumes auto PT(A/D Data) FIFO to Bridge fifo 
//  Returns 1 in case of time out otherwise 0
//  time out count is set to 100000
///////////////////////////////////////////////////////////////////////////////
DWORD LIB_TYPE start_stop_pt_dma_logic(DWORD brdindex, UINT md);

///////////////////////////////////////////////////////////////////////////////
//	Sets the trigger mode field in the P.thru command register
//	0 - internal , 1 - ext. digital, 2 - ext. analog rising edge, 
//									 3 - ext. analog falling edge
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE set_pt_trigger_mode(DWORD brdindex, UINT md);

///////////////////////////////////////////////////////////////////////////////
//	Sets the Pass thru interrupt condition
//	0 - FIFO HF,	1 - End of Scan
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE	set_pt_interrupt_mode(DWORD brdindex, UINT md);

///////////////////////////////////////////////////////////////////////////////
//  enables pass thru to PCI interrupt on end of scan 
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE enable_pt_eos_interrupt(DWORD brdindex);

///////////////////////////////////////////////////////////////////////////////
//  enables pass thru to PCI interrupt on half full	
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE enable_pt_half_full_interrupt(DWORD brdindex);

///////////////////////////////////////////////////////////////////////////////
//	Returns 1 if a HF or EOS PT interrupt occured 
///////////////////////////////////////////////////////////////////////////////
DWORD LIB_TYPE read_pt_interrupt_status(DWORD brdindex);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//	Sets/Resets the Pass Thru A/D scan select bit (used for ADM model A)
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE	set_pt_scan_select(DWORD brdindex, UINT md);

///////////////////////////////////////////////////////////////////////////////
//	Sets/resets the Pass thru  A/D channel auto-increment bit
//	1 - Autoincrement, 0 - nope
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE	set_pt_auto_increment(DWORD brdindex, UINT md);

///////////////////////////////////////////////////////////////////////////////
//	Sets/resets the Pass thru  A/D Marker mode select bit
//	1 - mark, 0 - no marker
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE	set_pt_marker_select(DWORD brdindex, UINT md);

///////////////////////////////////////////////////////////////////////////////
//	Enables/Disables  the Pass thru  A/D Pre-trigger mode
//	1 - Pre-trigger, 0 - no pre_trigger
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE	set_pt_pretrigger(DWORD brdindex, UINT md);

///////////////////////////////////////////////////////////////////////////////
//	Disables  the Pass thru  to Host(PCI) interrupt
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE	disable_pt_interrupt(DWORD brdindex);

///////////////////////////////////////////////////////////////////////////////
// Clears the PCI interrupt on the S5933 Bridge	
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE	clear_pt_interrupt(DWORD brdindex);

///////////////////////////////////////////////////////////////////////////////
//	Enables  the Pass thru  to Host(PCI) interrupt
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE	enable_pt_interrupt(DWORD brdindex);

///////////////////////////////////////////////////////////////////////////////
//	Set A/D clock source MUX
//	  FREQ SYNTH DIRECT --> A/D
// 		0	20	-	40 MHz 	2.5Khz res
//		1	10	-	20	Mhz 	1.25
//		2	5	-	10			625Hz res
//	  	3 	Freq SYNTH -> 82C54 -> A/D
//			useful for exact freq < 5 Mhz
//   OTHERS
//		4	External Clock
//		5	20MHz x-tal  -->a/d
//		6	10Mhz x-tal  -->a/d
//		7	10 Mhz -> 8254-> a/d
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE  set_pt_ad_clock_source(DWORD brdindex, UINT src);

///////////////////////////////////////////////////////////////////////////////
//	Set A/D control bits
//
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE  set_pt_adm_control(DWORD brdindex, UINT val);

///////////////////////////////////////////////////////////////////////////////
//	Sets pass thru command register to desired value
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE set_pt_command_reg(DWORD brdindex,DWORD val);

/* obsolete use pci416_set_cmdreg(brdindex, mode, val, &retval) instead
///////////////////////////////////////////////////////////////////////////////
//	Write current variable value to pass thru command register hardware
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE update_pt_command_reg(DWORD brdindex);

///////////////////////////////////////////////////////////////////////////////
// Write to the Pass-Thru Address Register Latch (ARL)
//	sets hardware and software value of pt address (register select) register
///////////////////////////////////////////////////////////////////////////////
// obsolete, use write_pci_port(DWORD portadr, DWORD data) instead
// get portadr with pci416_get_badr(DWORD brdindex, WORD *badrbuf, WORD bufsize);
void LIB_TYPE set_pt_address_reg(DWORD brdindex,DWORD addr);
*/

///////////////////////////////////////////////////////////////////////////////
//	Reads the PT. hardware status register
//  returns 0xFFFFFFFF in case of error
//  use pci416_getError to retreive error code
///////////////////////////////////////////////////////////////////////////////
DWORD LIB_TYPE read_pt_status_reg(DWORD brdindex);

///////////////////////////////////////////////////////////////////////////////
//	Returns 1 if pass thru fifo is empty
//  returns 0xFFFFFFFF in case of error
//  use pci416_getError to retreive error code
///////////////////////////////////////////////////////////////////////////////
DWORD LIB_TYPE pt_fifo_empty(DWORD brdindex);

///////////////////////////////////////////////////////////////////////////////
//	Returns 1 if pass thru fifo is half full
//  returns 0xFFFFFFFF in case of error
//  use pci416_getError to retreive error code
///////////////////////////////////////////////////////////////////////////////
DWORD LIB_TYPE pt_fifo_half_full(DWORD brdindex);

///////////////////////////////////////////////////////////////////////////////
//	Returns 1 if pass thru fifo is full
//  returns 0xFFFFFFFF in case of error
//  use pci416_getError to retreive error code
///////////////////////////////////////////////////////////////////////////////
DWORD LIB_TYPE pt_fifo_full(DWORD brdindex);

///////////////////////////////////////////////////////////////////////////////
//	Returns 1 if data PT A/D ARM FF is SET 
///////////////////////////////////////////////////////////////////////////////
DWORD LIB_TYPE read_pt_arm_ff(DWORD brdindex);

///////////////////////////////////////////////////////////////////////////////
//	Returns 1 if ANATRG bit of PT -SR is high 
///////////////////////////////////////////////////////////////////////////////
DWORD LIB_TYPE read_pt_analog_trigger_status(DWORD brdindex);

///////////////////////////////////////////////////////////////////////////////
//	Returns 1 if data ACQ bit in PT -CR is high 
///////////////////////////////////////////////////////////////////////////////
DWORD LIB_TYPE read_pt_acquire_status(DWORD brdindex);

///////////////////////////////////////////////////////////////////////////////
//	Returns the AD Module given by DIP setting
///////////////////////////////////////////////////////////////////////////////
DWORD LIB_TYPE read_pt_adm(DWORD brdindex);

///////////////////////////////////////////////////////////////////////////////
//	Set A/D sample count register
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE  set_pt_sample_count(DWORD brdindex, DWORD count);

///////////////////////////////////////////////////////////////////////////////
//	Set A/D Channel field of pt channel register
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE  set_pt_ad_channel(DWORD brdindex, UINT chan);

///////////////////////////////////////////////////////////////////////////////
//	Set A/D Scan Count field of channel register
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE  set_pt_ad_scan_count(DWORD brdindex, UINT count);

///////////////////////////////////////////////////////////////////////////////
//	Sets Test LED bit of channel register
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE  set_pt_led(DWORD brdindex, UINT val);

///////////////////////////////////////////////////////////////////////////////
//	Reset the PT. A/D Fifo
///////////////////////////////////////////////////////////////////////////////

void LIB_TYPE reset_pt_fifos(DWORD brdindex);

///////////////////////////////////////////////////////////////////////////////
//	Set/Reset PT. A/D convert enable bit
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE  set_pt_convert_enable(DWORD brdindex, UINT val);

///////////////////////////////////////////////////////////////////////////////
// Sets output clock frequency of PLL hardware
// And sets A/D clock source to PLL output
///////////////////////////////////////////////////////////////////////////////
UINT LIB_TYPE set_pt_pll(DWORD brdindex, REALTYPE freq);

// obsolete, use pci416_set_pllreg(brdindex, valA,  valN) instead
///////////////////////////////////////////////////////////////////////////////
// writes current software value of the pll data register to hardware
//
///////////////////////////////////////////////////////////////////////////////
//void LIB_TYPE update_pt_pll_reg(DWORD brdindex);

// obsolete, use pci416_set_timer(DWORD brdindex, WORD mode, 
//								WORD counter02, WORD counter1);
///////////////////////////////////////////////////////////////////////////////
// Updates 8254 counter register hardware with current software values
//
///////////////////////////////////////////////////////////////////////////////
//UINT LIB_TYPE set_pt_timer(DWORD brdindex, UINT n,UINT mode,UINT count);

///////////////////////////////////////////////////////////////////////////////
// Set A/D Clock  Rate
// Automatically sets best possible clock source mux and counter values
///////////////////////////////////////////////////////////////////////////////
int LIB_TYPE set_pt_ad_clock_rate(DWORD brdindex,REALTYPE fs);

///////////////////////////////////////////////////////////////////////////////
// Set A/D Trigger Rate (timers 0,1)
// Returns (1) if error
// *actual = real trigger rate produced by hardware
///////////////////////////////////////////////////////////////////////////////
UINT LIB_TYPE set_pt_trigger_rate(DWORD brdindex,REALTYPE ft,
			REALTYPE *actual);

///////////////////////////////////////////////////////////////////////////////
//	set trigger mode to internal
//	generate single trigger
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE generate_single_internal_trigger(DWORD brdindex);

///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE reset_pt_trigger_timer(DWORD brdindex);

///////////////////////////////////////////////////////////////////////////////
//	Sets DAC hardware voltage 
//	assumes +-10V range
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE set_pt_dac_voltage(DWORD brdindex, REALTYPE volts);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//	Sets DAC hardware code
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE set_pt_dac_code(DWORD brdindex, UINT code);

///////////////////////////////////////////////////////////////////////////////
//	Reads 1 DWORDSfrom pt a/d sample storage fifo
// returns 0xffffffff in case of error
// get error code with pci416_getError
///////////////////////////////////////////////////////////////////////////////
DWORD LIB_TYPE read_pt_fifo(DWORD brdindex);

///////////////////////////////////////////////////////////////////////////////
//	Read Multiple DWORDS from pt a/d sample storage fifo
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE read_pt_fifo_many(DWORD brdindex, LPDWORD buffer, DWORD count);


/*****************************************************************************
							BRIDGE FUNCTIONS (S593X)
*****************************************************************************/
///////////////////////////////////////////////////////////////////////////////
//	Resets the (Inbound) DMA FIFO on the S593x controller
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE reset_dma_fifo( DWORD brdindex);


///////////////////////////////////////////////////////////////////////////////
//	Sets the (Inbound) DMA fif0 bus master mode 1-> request on HF
//											    0-> reqest on not(EF)
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE set_dma_request_half_full( DWORD brdindex,UINT md);

///////////////////////////////////////////////////////////////////////////////
//	Enable/Disable (T/F) Bus master write transfers from FIFO
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE enable_dma_transfers( DWORD brdindex,UINT enable);

///////////////////////////////////////////////////////////////////////////////
//	Updates the value of the S593x Bus Master Control/Status Reg
///////////////////////////////////////////////////////////////////////////////
/* obsolete
// use pci416_set_mcsrreg(brdindex, mode, data, &retdata);
void LIB_TYPE update_mcr(DWORD brdindex);
*/

///////////////////////////////////////////////////////////////////////////////
//	Updates the DMA(Write) transfer count register (count is in bytes)
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE set_dma_transfer_count(DWORD brdindex, DWORD bytes);

///////////////////////////////////////////////////////////////////////////////
//	Reads the DMA(Write) transfer count register (count is in bytes)
//  returns 0xffffffff in case of error
///////////////////////////////////////////////////////////////////////////////
DWORD LIB_TYPE read_dma_transfer_count(DWORD brdindex);

/*  obsolete use pci416_setup_dma and pci416_reload_dma
///////////////////////////////////////////////////////////////////////////////
//	Updates the DMA(Write) transfer Address register
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE set_dma_dest_address(DWORD brdindex, DWORD dest);

///////////////////////////////////////////////////////////////////////////////
//	Reads the DMA(Write) destination addr. register
///////////////////////////////////////////////////////////////////////////////
DWORD LIB_TYPE read_dma_dest_address(DWORD brdindex);

*/
// not implemented DMA does not work on interrupt
///////////////////////////////////////////////////////////////////////////////
//	Enables/Disables S593x interrupt to HOST  on dma completion (Write Transfer)
///////////////////////////////////////////////////////////////////////////////
//void LIB_TYPE enable_dma_interrupt(DWORD brdindex, UINT enable);

/*
obsolete use pci416_dma_status
///////////////////////////////////////////////////////////////////////////////
//	Returns the Write(dma) transfer complete status bit (1=complete)
///////////////////////////////////////////////////////////////////////////////
DWORD LIB_TYPE read_dma_transfer_status(DWORD brdindex);

*/
/* not implemented DMA does not work on interrupt
///////////////////////////////////////////////////////////////////////////////
//	Resets the MAILBOX INTERRUPT status bit (INTERRUPT) (write1 clRs)
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE reset_dma_transfer_status(DWORD brdindex);

*/
/* obsolete, use pci416_read_intcsr,
				 pci416_set_intcsr,
				 pci416_read_mcsr,
				 pci416_set_mcsr,
				 read_pci_port,
				 write_pci_port
///////////////////////////////////////////////////////////////////////////////
//	Returns 1 is the the DMA(Write X-fer) FIFO on the AMCC controller is FULL 
///////////////////////////////////////////////////////////////////////////////
DWORD LIB_TYPE dma_fifo_full(DWORD brdindex);

///////////////////////////////////////////////////////////////////////////////
//	Returns 1 is the the DMA(Write X-fer) FIFO on the AMCC controller is empty
///////////////////////////////////////////////////////////////////////////////
DWORD LIB_TYPE dma_fifo_empty(DWORD brdindex);


///////////////////////////////////////////////////////////////////////////////
//	Reads the DMA(Write X-fer) FIFO on the AMCC controller
///////////////////////////////////////////////////////////////////////////////
DWORD LIB_TYPE read_dma_fifo_reg(DWORD brdindex);


///////////////////////////////////////////////////////////////////////////////
//	Updates the value of the S593x Interrupt Control/Status Reg
///////////////////////////////////////////////////////////////////////////////
void LIB_TYPE update_intcsr(DWORD brdindex);

///////////////////////////////////////////////////////////////////////////////
//	Reads the value of the S593x Interrupt Control/Status Reg
///////////////////////////////////////////////////////////////////////////////
DWORD LIB_TYPE read_intcsr(DWORD brdindex);

///////////////////////////////////////////////////////////////////////////////
//	Reads the value of the S593x Master Control/Status Reg
///////////////////////////////////////////////////////////////////////////////
DWORD LIB_TYPE read_msr(DWORD brdindex);

// functions not needed for board operation in Windows
// used for internal test only -> see DOS code p416_set
//////////////////////////////////////////////////////////////////////////////
//	Checks if NVRAM access pins are busy
//		returns non-zero if busy
//////////////////////////////////////////////////////////////////////////////
UINT LIB_TYPE nvram_busy(DWORD brdindex);

/////////////////////////////////////////////////////////////////////////////
// Set EPROM address for Data access
//////////////////////////////////////////////////////////////////////////////
void LIB_TYPE set_nvram_address(DWORD brdindex, UINT addr);

//////////////////////////////////////////////////////////////////////////////
// Read byte from NVRAM
//////////////////////////////////////////////////////////////////////////////
unsigned char LIB_TYPE read_nvram( DWORD brdindex, UINT addr);

//////////////////////////////////////////////////////////////////////////////
// Write byte to NVRAM
//////////////////////////////////////////////////////////////////////////////
void LIB_TYPE write_nvram( DWORD brdindex, UINT addr,
					unsigned char data);

*/
#endif
