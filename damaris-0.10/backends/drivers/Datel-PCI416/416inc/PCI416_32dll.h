///////////////////////////////////////////////////////////////////////////////
//
//	  pci416_32dll.h: Header file for DLL functions that perform board setup and 
//					  data acquisition using FIFO polling.
//	  Copyright (c) Datel, Inc. 1997
//	  Platform: Win95, Win NT
//	  Compiler: MVC4.0 + DDK
//	  Version: 3.0
//	  Author: GS
//	  created: 2/28/97
//	  modified: 7/9/98
///////////////////////////////////////////////////////////////////////////////

#ifndef PCI416_32DLL_H
#define PCI416_32DLL_H

#include <windows.h>

// REALTYPE is defined as double (size: 8 bytes)

/******************************************************************************
Function: get_adm_inf

Description: Returns specific info of installed A/D module.	
			 Use this function instead the get_adm_stats function for DLL calls. 
			 The get_adm_stats function expects a pointer to an ADM_STATS 
			 structure which is only available in C programs. Other compilers and 
			 programs may generate structures with different size and boundaries
			 that could cause sytem crashes!
		  
Inputs: DWORD brdindex : Index of board.  
						 Max. index = number of PCI-416 boards - 1
		Char *model  : ADM model
		WORD *bits : Resolution in bits
		WORD *channels : no.of channels
		WORD *shortcycle : 1== can be short cycled
		REALTYPE  *fmax_single:  max. sample freq. single channel mode
		REALTYPE  *fmax_scan:  max. sample freq. multi channel mode

return: Error status as define in winerror.h
NOERROR
ERROR_INVALID_HANDLE
ERROR_DEV_NOT_EXIST
******************************************************************************/
DWORD LIB_TYPE get_adm_inf(DWORD brdindex, WORD *model, WORD *bits, WORD *channels,
				  WORD *shortcycle, REALTYPE *fmax_single, REALTYPE *fmax_scan);

/******************************************************************************
Function: set_modes

Description: Setup board paramters: sample rate, trigger rate, clock and trigger
			 source etc.			 
		  
Inputs: DWORD brdindex : Index of board.  
						 Max. index = number of PCI-416 boards - 1
		WORD ClockSrc  : 0= INTERNAL, 1= EXTERNAL, 
						 other : internal with sample rate set to
						 max. value in scan mode.
		REALTYPE SampleRate : A/D sample rate, if src set to INTERNAL
							  If ClockSrc is not set to INTERNAL or 
							  EXTERNAL the sample rate is set to
							  max. value in scan mode.		
		DWORD SamplesPerTrigger: Number of samples to collect 
		                         at each trigger pulse.
		WORD TriggerSrc : 0= INTERNAL,1 = EXTERNAL,
				  2= ANALOG Rising Edge, 3= ANALOG Falling Edge, 
						  other: INTERNAL		
		WORD Channel : channel of acquisition in single channel mode
					   or last scan channel in scan mode if applicable
					   0 <= Channel < number of channels
		WORD PreTrigger : 1== PreTrigger
		WORD Scan : 1== Scan mode
		WORD Marker : 1== set marker bit		

return: Error status as define in winerror.h
NOERROR
ERROR_INVALID_HANDLE
ERROR_DEV_NOT_EXIST
******************************************************************************/
DWORD LIB_TYPE set_modes (DWORD brdindex,
						  WORD ClockSrc,
						  REALTYPE SampleRate,						 
						  DWORD SamplesPerTrigger,						  
						  WORD TriggerSrc,						  
						  WORD Channel,
						  WORD PreTrigger,
						  WORD Scan,
						  WORD Marker);
/******************************************************************************
Function: start_daq

Description: Starts data acquisition.
			 All registers have to be set prior this function call with 
			 set_mode!
		  
Inputs: DWORD brdindex : Index of board.  
						 Max. index = number of PCI-416 boards - 1
		WORD TriggerMode : 0== PreTrigger, external trigger, 
						   1== Single Internal Trigger, 
						   2== Multi Internal Trigger
						   other default to single trigger
		REALTYPE *TriggerRate: trigger rate for multi trigger
Output: REALTYPE *TriggerRate: actual trigger rate set

Note: If PreTrigger was selected in set_modes only an external trigger 
	  is valid. Calls other than start_daq(brdindex, 0) will result in 
	  invalid data. 
	  On the other hand if PreTrigger was NOT set in set_modes 
	  a start_daq(brdindex, 0) call does not start the A/D conversion.

return: Error status as define in winerror.h
NOERROR
******************************************************************************/
DWORD LIB_TYPE start_daq(DWORD brdindex, WORD TriggerMode, 
						 REALTYPE *TriggerRate);


/******************************************************************************
Function: stop_daq

Description: Stops data acquisition.			 
		  
Inputs: DWORD brdindex : Index of board.  
						 Max. index = number of PCI-416 boards - 1
		
return: Error status as define in winerror.h
NOERROR
******************************************************************************/
DWORD LIB_TYPE stop_daq(DWORD brdindex);


/******************************************************************************
Function: scan_status

Description: Acquires data by polling on the FIFO half full flag.			 
		  
Inputs: DWORD brdindex : Index of board.  
						 Max. index = number of PCI-416 boards - 1
		WORD ClockSrc  : C_INTERNAL, C_EXTERNAL, 
						 default : internal with sample rate set to
						 max. value in scan mode.
		REALTYPE SampleRate : A/D sample rate, if src set to C_INTERNAL
							  If ClockSrc is not set to C_INTERNAL or 
							  C_EXTERNAL the sample rate is set to
							  max. value in scan mode.
		DWORD TotalSamples : Total number of samples to collect 
		DWORD SamplesPerTrigger: Number of samples to collect 
		                         at each trigger pulse.
		DWORD Frames : Number of triggers. 
		WORD TriggerSrc : T_INTERNAL,T_EXTERNAL,T_ANALOG, 
						  default: T_INTERNAL
		REALTYPE TriggerRate : Trigger rate
		WORD Channel : channel of acquisition in single channel mode
					   or last scan channel in scan mode if applicable
					   0 <= Channel < number of channels
		BOOL PreTrigger : TRUE== PreTrigger
		BOOL Scan : TRUE== Scan mode
		BOOL Marker : TRUE== set marker bit
		PVOID buffer : User buffer to store the A/D data

return: state of the FIFO Full flag ( 0 = No overflow, 1 = FIFO FULL (overflow)
		-1 error -> retreive error with pci416_getError
******************************************************************************/
INT LIB_TYPE scan_status (DWORD brdindex,
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

#endif
