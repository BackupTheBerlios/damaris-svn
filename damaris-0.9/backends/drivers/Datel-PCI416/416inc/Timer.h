///////////////////////////////////////////////////////////////////////////////
//
//	  timer.h: 82C54 command register bits  
//	  and mode definition used by the PCI 416-board       
//	  Copyright (c) Datel, Inc. 1997
//	  Version: 1.0
//	  Author: GS
//	  created: 2/4/97
//	  modified: 4/15/96
///////////////////////////////////////////////////////////////////////////////
#ifndef TIMER_H
#define TIMER_H

#define SEL0     (unsigned int)  0x00    // select counter # 0 
#define SEL1     (unsigned int)  0x40    // select counter # 1 
#define SEL2     (unsigned int)  0x80    // select counter # 2 
#define LSBMSB   (unsigned int)  0x30    // load lsb followed by msb 
#define MODE0    (unsigned int)  0x00    // modes  
#define MODE1    (unsigned int)  0x02
// 8254 Constants
#define MODE2    (unsigned int)  0x04    // software trigger 
#define MODE3    (unsigned int)  0x06
#define MODE4    (unsigned int)  0x08    // hardware trigger 
#define MODE5    (unsigned int)  0x0A
#define BINARY   (unsigned int)  0x00    // binary count 
#define BCD      (unsigned int)  0x01    // bcd count   
#define READBACK (unsigned int)  0xC0    // read back counter command 
#define CLOCK1  8000000L // clock frequency for Sample rate counter (1)  
#define CLOCK2  500000L // clock frequency for internal trigger rate(2)  



enum{TM_SINGLE_TRIGGER, TM_CONT_TRIGGER, TM_RESET_TRIGGER, TM_ADCLOCK};

#endif