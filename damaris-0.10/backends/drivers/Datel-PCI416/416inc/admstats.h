/*****************************************************************************
*																			 
*	  ADMSTATS.H -- Header File A/D Module Parameters	
*     Copyright 1997 Datel Corporation.  All Rights Reserved.     
*	  Version 3.1.
*	  Author: GS
*	  created : 1/24/97
*     modified: 10/5/98														
*****************************************************************************/
#ifndef ADMSTATS_H
#define ADMSTATS_H
#include "pci416df.h"

const ADM_STATS adm_stats[]={
	//mod,bits,	chs, single, scan,chrg,inc,scne,scnt,ssh,scyc,mnV,mxV,mnc,mxc)
	 {'A', 12,	4,	1.0e6F,	250e3F,	0,	1,	1,	3,	 1,	 1,		0.0F,0.0F,0,0},	// A
	 {'B', 14,	4,	500e3F,	333e3F,	0,	1,	0,	0,	 0,	 0,		0.0F,0.0F,0,0},	// B
	 {'C', 12,	4,	1.6e6F,	500e3F,	0,	1,	0,	0,	 0,	 0,		0.0F,0.0F,0,0},	// C
	 {'D', 12,	1,	5e6F,		5e6F,   0,	0,	0,	0,	 0,	 0,		0.0F,0.0F,0,0},	// D
	 {'E', 12,	16, 2.0e6F,	500e3F,	0,	1,	0,	0,	 0,	 0,		0.0F,0.0F,0,0},	// E
	 {'F', 12,	2,	2.0e6F,	2.0e6F,	2,	0,	0,	0,	 1,	 0,		0.0F,0.0F,0,0},	// F
	 {'G', 14,	2,	1.0e6F,	1.0e6F,	2,	0,	0,	0,	 1,	 0,		0.0F,0.0F,0,0},	// G
   {'H', 12,	1,	10e6F,	10e6F,  0,	0,	0,	0,	 0,	 0,		0.0F,0.0F,0,0},	// H
	 {'J', 12,	8,	400e3F,	250e3F,	15,	0,	0,  0,	 1,	 1,		0.0F,0.0F,0,0},	// J
	 {'K', 12,	2,	5.0e6F,	5.0e6F,	2,	0,	0,	0,   1,	 0,		0.0F,0.0F,0,0},	// K
	 {'L', 12,	16, 400e3F,	190e3F,	15,	0,	0,  0,	 1,	 1,		0.0F,0.0F,0,0},	// L
	 {'M', 16,	4, 	200e3F,	200e3F,	8,	0,	0,  0,	 1,	 0,		0.0F,0.0F,0,0},	// M
	 {'N', 14,	2, 	5e6F,		5e6F,		2,	0,	0,  0,	 1,	 1,		0.0F,0.0F,0,0},	// N
	 {'P', 14,	4, 	3e6F,		3e6F,		8,	0,	0, 	0,	 1,	 0,		0.0F,0.0F,0,0},	// P
};

#define MAX_ADMNO (sizeof(adm_stats)/sizeof(ADM_STATS))
#endif
