///////////////////////////////////////////////////////////////////////////////
//
//	  41632dll.h: Function definition of pci41632.dll exported functions
//				  See pci416io.h, pci416libw95.h and pci416_32dll.h
//				  for the description of each function.
//	  Copyright (c) Datel, Inc. 1997
//	  Platform: Win32, Win95
//	  Compiler: MVC4.0 
//	  Version: 3.0
//	  Author: GS
//	  created: 4/16/97
//	  modified: 7/15/97
///////////////////////////////////////////////////////////////////////////////
#ifndef _41632DLL_H
#define _41632DLL_H

#include <windows.h>

#include "pcilib32.h"
#include "pci416df.h"

#ifndef LIB_TYPE
#define LIB_TYPE WINAPI
#endif

// function description -> see pci416io.h
typedef DWORD (LIB_TYPE *TFP_pci416_init)(DWORD *brdcount);
typedef DWORD (LIB_TYPE *TFP_pci416_close)(DWORD brdindex);
typedef DWORD (LIB_TYPE *TFP_find_pci_device)(WORD busno, WORD venid, 
											  WORD deviceid, WORD bufsize, 
											  BYTE *buf);
typedef DWORD (LIB_TYPE *TFP_get_pci_device_info)(WORD busno, WORD devno, 
								   PCI_CONFIG_SPACE *conf);
typedef DWORD (LIB_TYPE *TFP_write_pci_port)(DWORD portadr, DWORD data);
typedef DWORD (LIB_TYPE *TFP_read_pci_port)(DWORD portadr, DWORD *data);

typedef DWORD (LIB_TYPE *TFP_pci416_count)(DWORD *brdcount);
typedef DWORD (LIB_TYPE *TFP_pci416_get_cm_devnodes)(INT brdindex, 
													 WORD bufsize, 
													 DWORD *devnodebuf);
typedef DWORD (LIB_TYPE *TFP_pci416_get_cmdevinf)(DWORD brdindex, 
										  PCI_CONFIG_SPACE *conf);
typedef DWORD (LIB_TYPE *TFP_pci416_get_badr)(DWORD brdindex, WORD bufsize, 
									  WORD *badrbuf);
typedef DWORD (LIB_TYPE *TFP_pci416_getcaps)(DWORD brdindex, WORD bufsize, DWORD *buf);
typedef DWORD (LIB_TYPE *TFP_pci416_set_cmdreg)(DWORD brdindex, WORD mode, 
										DWORD regval, DWORD *shregval);
typedef DWORD (LIB_TYPE *TFP_pci416_read_statusreg)(DWORD brdindex, DWORD *regval);
typedef DWORD (LIB_TYPE *TFP_pci416_set_smplcntr)(DWORD brdindex, DWORD samples);
typedef DWORD (LIB_TYPE *TFP_pci416_set_chanadr)(DWORD brdindex, WORD mode, 
										 DWORD regval, DWORD *shregval);
typedef DWORD (LIB_TYPE *TFP_pci416_clear_fifo)(DWORD brdindex);
typedef DWORD (LIB_TYPE *TFP_pci416_enablead)(DWORD brdindex, BOOL enable);
typedef DWORD (LIB_TYPE *TFP_pci416_set_pllreg)(DWORD brdindex, DWORD valA, DWORD valN);
typedef DWORD (LIB_TYPE *TFP_pci416_read_fifo)(DWORD brdindex, WORD count, DWORD *buf);
typedef DWORD (LIB_TYPE *TFP_pci416_set_timer)(DWORD brdindex, WORD mode, 
									   WORD counter02, WORD counter1);
typedef DWORD (LIB_TYPE *TFP_pci416_set_portctrreg)(DWORD brdindex, DWORD regval);
typedef DWORD (LIB_TYPE *TFP_pci416_get_portctrreg)(DWORD brdindex, DWORD *regval);
typedef DWORD (LIB_TYPE *TFP_pci416_read_port)(DWORD brdindex, WORD port, DWORD *data);
typedef DWORD (LIB_TYPE *TFP_pci416_write_port)(DWORD brdindex, WORD port, DWORD data);
typedef DWORD (LIB_TYPE *TFP_pci416_write_dac)(DWORD brdindex, WORD data);
typedef DWORD (LIB_TYPE *TFP_pci416_fifo_status)(DWORD brdindex, DWORD *data);
typedef DWORD (LIB_TYPE *TFP_pci416_check_fifohf)(DWORD brdindex, DWORD *data);
typedef DWORD (LIB_TYPE *TFP_pci416_setup_dma)(DWORD brdindex, DWORD mode, 
									   DWORD *bufsize, DWORD *hndDMAbuf);
typedef DWORD (LIB_TYPE *TFP_pci416_stop_dma)(DWORD brdindex, DWORD *tcount);
typedef DWORD (LIB_TYPE *TFP_pci416_reload_dma)(DWORD brdindex, DWORD bufno,
								 DWORD *bufsize, DWORD *hndDMAbuf);
typedef DWORD (LIB_TYPE *TFP_pci416_dma_status)(DWORD brdindex, DWORD *data);
typedef DWORD (LIB_TYPE *TFP_pci416_read_intcsrreg)(DWORD brdindex, DWORD *data);
typedef DWORD (LIB_TYPE *TFP_pci416_set_intcsrreg)(DWORD brdindex, WORD mode, 
										   DWORD regval, DWORD *shregval);
typedef DWORD (LIB_TYPE *TFP_pci416_read_mcsrreg)(DWORD brdindex, DWORD *data);
typedef DWORD (LIB_TYPE *TFP_pci416_set_mcsrreg)(DWORD brdindex, WORD mode, 
										 DWORD regval, DWORD *shregval);
typedef DWORD (LIB_TYPE *TFP_pci416_read_mwarreg)(DWORD brdindex, DWORD *data);
typedef DWORD (LIB_TYPE *TFP_pci416_set_mwarreg)(DWORD brdindex, WORD mode, DWORD regval, 
									 DWORD *shregval);
typedef DWORD (LIB_TYPE *TFP_pci416_read_mwtcreg)(DWORD brdindex, DWORD *data);
typedef DWORD (LIB_TYPE *TFP_pci416_set_mwtcreg)(DWORD brdindex, WORD mode, DWORD regval, 
									 DWORD *shregval);
typedef DWORD (LIB_TYPE *TFP_pci416_copy_dmabuffer)(DWORD brdindex, DWORD bufno, 
									 DWORD start, DWORD *count, 
									 DWORD *pDest );
typedef DWORD (LIB_TYPE *TFP_pci416_get_dmabuf_hndl)(DWORD brdindex, DWORD bufno, 
									 DWORD offset, DWORD *pHndl, 
									 DWORD *bufsize);
typedef DWORD (LIB_TYPE *TFP_pci416_pause_resume_dma)(DWORD brdindex, DWORD flags);

typedef DWORD (LIB_TYPE *TFP_pci416_getError)(LPTSTR *str);

// function description -> see pci416libw95.h
typedef INT (LIB_TYPE *TFP_get_adm_stats)(DWORD brdindex, 
								  ADM_STATS *admstats);
typedef DWORD (LIB_TYPE *TFP_read_pt_port_control_reg)(DWORD brdindex);
typedef void (LIB_TYPE *TFP_write_pt_port_control_reg)(DWORD brdindex, UINT val);
typedef DWORD (LIB_TYPE *TFP_read_pt_port_data)(DWORD brdindex, UINT prt);
typedef DWORD (LIB_TYPE *TFP_write_pt_port_data)(DWORD brdindex, UINT prt, UINT val);
typedef void (LIB_TYPE	*TFP_enable_pt_dma_logic)(DWORD brdindex, UINT md);
typedef DWORD (LIB_TYPE	*TFP_start_stop_pt_dma_logic)(DWORD brdindex, UINT md);
typedef void (LIB_TYPE *TFP_set_pt_trigger_mode)(DWORD brdindex, UINT md);
typedef void (LIB_TYPE	*TFP_set_pt_interrupt_mode)(DWORD brdindex, UINT md);
typedef void (LIB_TYPE *TFP_enable_pt_eos_interrupt)(DWORD brdindex);
typedef void (LIB_TYPE *TFP_enable_pt_half_full_interrupt)(DWORD brdindex);
typedef DWORD (LIB_TYPE *TFP_read_pt_interrupt_status)(DWORD brdindex);
typedef void (LIB_TYPE	*TFP_set_pt_scan_select)(DWORD brdindex, UINT md);
typedef void (LIB_TYPE	*TFP_set_pt_auto_increment)(DWORD brdindex, UINT md);
typedef void (LIB_TYPE	*TFP_set_pt_marker_select)(DWORD brdindex, UINT md);
typedef void (LIB_TYPE	*TFP_set_pt_pretrigger)(DWORD brdindex, UINT md);
typedef void (LIB_TYPE	*TFP_disable_pt_interrupt)(DWORD brdindex);
typedef void (LIB_TYPE	*TFP_clear_pt_interrupt)(DWORD brdindex);
typedef void (LIB_TYPE	*TFP_enable_pt_interrupt)(DWORD brdindex);
typedef void (LIB_TYPE  *TFP_set_pt_ad_clock_source)(DWORD brdindex, UINT src);
typedef void (LIB_TYPE  *TFP_set_pt_adm_control)(DWORD brdindex, UINT val);
typedef void (LIB_TYPE *TFP_set_pt_command_reg)(DWORD brdindex,DWORD val);
typedef DWORD (LIB_TYPE *TFP_read_pt_status_reg)(DWORD brdindex);
typedef DWORD (LIB_TYPE *TFP_pt_fifo_empty)(DWORD brdindex);
typedef DWORD (LIB_TYPE *TFP_pt_fifo_half_full)(DWORD brdindex);
typedef DWORD (LIB_TYPE *TFP_pt_fifo_full)(DWORD brdindex);
typedef DWORD (LIB_TYPE *TFP_read_pt_arm_ff)(DWORD brdindex);
typedef DWORD (LIB_TYPE *TFP_read_pt_analog_trigger_status)(DWORD brdindex);
typedef DWORD (LIB_TYPE *TFP_read_pt_acquire_status)(DWORD brdindex);
typedef DWORD (LIB_TYPE *TFP_read_pt_adm)(DWORD brdindex);
typedef void (LIB_TYPE  *TFP_set_pt_sample_count)(DWORD brdindex, DWORD count);
typedef void (LIB_TYPE  *TFP_set_pt_ad_channel)(DWORD brdindex, UINT chan);
typedef void (LIB_TYPE  *TFP_set_pt_ad_scan_count)(DWORD brdindex, UINT count);
typedef void (LIB_TYPE  *TFP_set_pt_led)(DWORD brdindex, UINT val);
typedef void (LIB_TYPE *TFP_reset_pt_fifos)(DWORD brdindex);
typedef void (LIB_TYPE  *TFP_set_pt_convert_enable)(DWORD brdindex, UINT val);
typedef UINT (LIB_TYPE *TFP_set_pt_pll)(DWORD brdindex, REALTYPE freq);
typedef int (LIB_TYPE *TFP_set_pt_ad_clock_rate)(DWORD brdindex,REALTYPE fs);
typedef UINT (LIB_TYPE *TFP_set_pt_trigger_rate)(DWORD brdindex,REALTYPE ft,
			REALTYPE *actual);
typedef void (LIB_TYPE *TFP_generate_single_internal_trigger)(DWORD brdindex);
typedef void (LIB_TYPE *TFP_reset_pt_trigger_timer)(DWORD brdindex);
typedef void (LIB_TYPE *TFP_set_pt_dac_voltage)(DWORD brdindex, REALTYPE volts);
typedef void (LIB_TYPE *TFP_set_pt_dac_code)(DWORD brdindex, UINT code);
typedef DWORD (LIB_TYPE *TFP_read_pt_fifo)(DWORD brdindex);
typedef void (LIB_TYPE *TFP_read_pt_fifo_many)(DWORD brdindex, LPDWORD buffer, 
									   DWORD count);
typedef void (LIB_TYPE *TFP_reset_dma_fifo)( DWORD brdindex);
typedef void (LIB_TYPE *TFP_set_dma_request_half_full)( DWORD brdindex,UINT md);
typedef void (LIB_TYPE *TFP_enable_dma_transfers)( DWORD brdindex,UINT enable);
typedef void (LIB_TYPE *TFP_set_dma_transfer_count)(DWORD brdindex, DWORD bytes);
typedef DWORD (LIB_TYPE *TFP_read_dma_transfer_count)(DWORD brdindex);
// NT only
typedef DWORD (LIB_TYPE *TFP_start_daq_irq)(DWORD brdindex,DWORD TriggerMode, 
						 REALTYPE *TriggerRate);
typedef DWORD (LIB_TYPE *TFP_pci416_close_dmabuf_hndl)(DWORD brdindex);
typedef DWORD (LIB_TYPE *TFP_pci416_map_dmabuf)(DWORD brdindex, DWORD *pHndl, DWORD size);
typedef DWORD (LIB_TYPE *TFP_pci416_unmap_dmabuf)(DWORD brdindex, DWORD Hndl);

// function description -> see pci416_32dll.h
typedef DWORD (LIB_TYPE *TFP_get_adm_inf) (DWORD brdindex, WORD *model, 
								   WORD *bits, WORD *channels,
								   WORD *shortcycle, 
								   REALTYPE *fmax_single, REALTYPE *fmax_scan);
typedef DWORD (LIB_TYPE *TFP_set_modes) (DWORD brdindex,
						  WORD ClockSrc,
						  REALTYPE SampleRate,						 
						  DWORD SamplesPerTrigger,						  
						  WORD TriggerSrc,						  
						  WORD Channel,
						  WORD PreTrigger,
						  WORD Scan,
						  WORD Marker);
typedef DWORD (LIB_TYPE *TFP_start_daq)(DWORD brdindex, WORD TriggerMode, 
						 REALTYPE *TriggerRate);

typedef DWORD (LIB_TYPE *TFP_stop_daq)(DWORD brdindex);
typedef INT (LIB_TYPE *TFP_scan_status)(DWORD brdindex,
						  WORD ClockSrc,
						  REALTYPE SampleRate,
						  DWORD TotalSamples,
						  DWORD SamplesPerTrigger,
						  DWORD Frames,
						  WORD TriggerSrc,
						  REALTYPE TriggerRate,
						  WORD Channel,
						  BOOL PreTrigger,
						  BOOL Scan,
						  BOOL Marker,
						  PVOID buffer);

/* function instance definition 
   assign function by loading the address from DLL with
   func_name = (TFP_func_name)GetProcAddress(hModule, "func_name");
*/
TFP_pci416_init pci416_init;
TFP_pci416_close pci416_close;
TFP_find_pci_device find_pci_device;
TFP_get_pci_device_info get_pci_device_info;
TFP_write_pci_port write_pci_port; 
TFP_read_pci_port read_pci_port;
TFP_pci416_count pci416_count;
TFP_pci416_get_cm_devnodes pci416_get_cm_devnodes;
TFP_pci416_get_cmdevinf pci416_get_cmdevinf;
TFP_pci416_get_badr pci416_get_badr;
TFP_pci416_getcaps pci416_getcaps;
TFP_pci416_set_cmdreg pci416_set_cmdreg;
TFP_pci416_read_statusreg pci416_read_statusreg;
TFP_pci416_set_smplcntr pci416_set_smplcntr;
TFP_pci416_set_chanadr pci416_set_chanadr;
TFP_pci416_clear_fifo pci416_clear_fifo;
TFP_pci416_enablead pci416_enablead;
TFP_pci416_set_pllreg pci416_set_pllreg;
TFP_pci416_read_fifo pci416_read_fifo;
TFP_pci416_set_timer pci416_set_timer;
TFP_pci416_set_portctrreg pci416_set_portctrreg;
TFP_pci416_get_portctrreg pci416_get_portctrreg;
TFP_pci416_read_port pci416_read_port;
TFP_pci416_write_port pci416_write_port;
TFP_pci416_write_dac pci416_write_dac;
TFP_pci416_fifo_status pci416_fifo_status;
TFP_pci416_check_fifohf pci416_check_fifohf;
TFP_pci416_setup_dma pci416_setup_dma;
TFP_pci416_stop_dma pci416_stop_dma;
TFP_pci416_reload_dma pci416_reload_dma;
TFP_pci416_dma_status pci416_dma_status;
TFP_pci416_read_intcsrreg pci416_read_intcsrreg;
TFP_pci416_set_intcsrreg pci416_set_intcsrreg;
TFP_pci416_read_mcsrreg pci416_read_mcsrreg;
TFP_pci416_set_mcsrreg pci416_set_mcsrreg;
TFP_pci416_read_mwarreg pci416_read_mwarreg;
TFP_pci416_set_mwarreg pci416_set_mwarreg;
TFP_pci416_read_mwtcreg pci416_read_mwtcreg;
TFP_pci416_set_mwtcreg pci416_set_mwtcreg; 
TFP_pci416_copy_dmabuffer pci416_copy_dmabuffer;
TFP_pci416_get_dmabuf_hndl pci416_get_dmabuf_hndl;
TFP_pci416_pause_resume_dma pci416_pause_resume_dma;
TFP_pci416_close_dmabuf_hndl pci416_close_dmabuf_hndl;
TFP_pci416_map_dmabuf pci416_map_dmabuf;
TFP_pci416_unmap_dmabuf pci416_unmap_dmabuf;

TFP_pci416_getError pci416_getError;

TFP_get_adm_stats get_adm_stats;
TFP_read_pt_port_control_reg read_pt_port_control_reg;
TFP_write_pt_port_control_reg write_pt_port_control_reg;
TFP_read_pt_port_data read_pt_port_data;
TFP_write_pt_port_data write_pt_port_data;
TFP_enable_pt_dma_logic enable_pt_dma_logic;
TFP_start_stop_pt_dma_logic start_stop_pt_dma_logic;
TFP_set_pt_trigger_mode set_pt_trigger_mode;
TFP_set_pt_interrupt_mode set_pt_interrupt_mode;
TFP_enable_pt_eos_interrupt enable_pt_eos_interrupt;
TFP_enable_pt_half_full_interrupt enable_pt_half_full_interrupt;
TFP_read_pt_interrupt_status read_pt_interrupt_status;
TFP_set_pt_scan_select set_pt_scan_select;
TFP_set_pt_auto_increment set_pt_auto_increment;
TFP_set_pt_marker_select set_pt_marker_select;
TFP_set_pt_pretrigger set_pt_pretrigger;
TFP_disable_pt_interrupt disable_pt_interrupt;
TFP_clear_pt_interrupt clear_pt_interrupt;
TFP_enable_pt_interrupt enable_pt_interrupt;
TFP_set_pt_ad_clock_source set_pt_ad_clock_source;
TFP_set_pt_adm_control set_pt_adm_control;
TFP_set_pt_command_reg set_pt_command_reg;
TFP_read_pt_status_reg read_pt_status_reg;
TFP_pt_fifo_empty pt_fifo_empty;
TFP_pt_fifo_half_full pt_fifo_half_full;
TFP_pt_fifo_full pt_fifo_full;
TFP_read_pt_arm_ff read_pt_arm_ff;
TFP_read_pt_analog_trigger_status read_pt_analog_trigger_status;
TFP_read_pt_acquire_status read_pt_acquire_status;
TFP_read_pt_adm read_pt_adm;
TFP_set_pt_sample_count set_pt_sample_count;
TFP_set_pt_ad_channel set_pt_ad_channel;
TFP_set_pt_ad_scan_count set_pt_ad_scan_count;
TFP_set_pt_led set_pt_led;
TFP_reset_pt_fifos reset_pt_fifos;
TFP_set_pt_convert_enable set_pt_convert_enable;
TFP_set_pt_pll set_pt_pll;
TFP_set_pt_ad_clock_rate set_pt_ad_clock_rate;
TFP_set_pt_trigger_rate set_pt_trigger_rate;
TFP_generate_single_internal_trigger generate_single_internal_trigger;
TFP_reset_pt_trigger_timer reset_pt_trigger_timer;
TFP_set_pt_dac_voltage set_pt_dac_voltage;
TFP_set_pt_dac_code set_pt_dac_code;
TFP_read_pt_fifo read_pt_fifo;
TFP_read_pt_fifo_many read_pt_fifo_many;
TFP_reset_dma_fifo reset_dma_fifo;
TFP_set_dma_request_half_full set_dma_request_half_full;
TFP_enable_dma_transfers enable_dma_transfers;
TFP_set_dma_transfer_count set_dma_transfer_count;
TFP_read_dma_transfer_count read_dma_transfer_count;

TFP_get_adm_inf get_adm_inf;
TFP_set_modes set_modes;
TFP_start_daq start_daq;
TFP_start_daq_irq start_daq_irq;
TFP_stop_daq stop_daq;
TFP_scan_status scan_status;

#endif
