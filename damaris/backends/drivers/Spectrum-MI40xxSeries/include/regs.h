// *********************************
// Datei: REGS.H
// ******************************** 


#define TYP_PCIDEVICEID             0x00000000l

// ***** Board Types ***************
#define TYP_EVAL					0x00000010l
#define TYP_RSDLGA					0x00000014l
#define TYP_GMG						0x00000018l
#define TYP_VAN8					0x00000020l
#define TYP_VAC						0x00000028l

#define TYP_PCIAUTOINSTALL			0x000000FFl

#define TYP_DAP116					0x00000100l
#define TYP_PAD82					0x00000200l
#define TYP_PAD82a					0x00000210l
#define TYP_PAD82b					0x00000220l
#define TYP_PCI212					0x00000300l
#define TYP_PAD1232a				0x00000400l
#define TYP_PAD1232b				0x00000410l
#define TYP_PAD1232c				0x00000420l
#define TYP_PAD1616a				0x00000500l
#define TYP_PAD1616b				0x00000510l
#define TYP_PAD1616c				0x00000520l
#define TYP_PAD1616d				0x00000530l
#define TYP_PAD52					0x00000600l
#define TYP_PAD242					0x00000700l
#define TYP_PCK400					0x00000800l
#define TYP_PAD164_2M				0x00000900l
#define TYP_PAD164_5M				0x00000910l
#define TYP_PCI208					0x00001000l
#define TYP_CPCI208					0x00001001l
#define TYP_PCI412					0x00001100l
#define TYP_PCIDIO32				0x00001200l
#define TYP_PCI248					0x00001300l
#define TYP_PADCO					0x00001400l
#define TYP_TRS582					0x00001500l
#define TYP_PCI258					0x00001600l


// ------ series and familiy identifiers -----
#define TYP_SERIESMASK              0x00FF0000l
#define TYP_FAMILYMASK              0x0000FF00l
#define TYP_VERSIONMASK             0x0000FFFFl
#define TYP_MISERIES                0x00000000l
#define TYP_MCSERIES                0x00010000l
#define TYP_MXSERIES                0x00020000l



// ----- MI.20xx, MC.20xx, MX.20xx -----
#define TYP_MI2020					0x00002020l
#define TYP_MI2021					0x00002021l
#define TYP_MI2025					0x00002025l
#define TYP_MI2030					0x00002030l
#define TYP_MI2031					0x00002031l

#define TYP_MC2020					0x00012020l
#define TYP_MC2021					0x00012021l
#define TYP_MC2025					0x00012025l
#define TYP_MC2030					0x00012030l
#define TYP_MC2031					0x00012031l

#define TYP_MX2020					0x00022020l
#define TYP_MX2025					0x00022025l
#define TYP_MX2030					0x00022030l



// ----- MI.30xx, MC.30xx, MX.30xx -----
#define TYP_MI3010					0x00003010l
#define TYP_MI3011					0x00003011l
#define TYP_MI3012					0x00003012l
#define TYP_MI3013					0x00003013l
#define TYP_MI3014					0x00003014l
#define TYP_MI3015					0x00003015l
#define TYP_MI3016					0x00003016l
#define TYP_MI3020					0x00003020l
#define TYP_MI3021					0x00003021l
#define TYP_MI3022					0x00003022l
#define TYP_MI3023					0x00003023l
#define TYP_MI3024					0x00003024l
#define TYP_MI3025					0x00003025l
#define TYP_MI3026					0x00003026l
#define TYP_MI3027					0x00003027l
#define TYP_MI3031					0x00003031l
#define TYP_MI3033					0x00003033l

#define TYP_MC3010					0x00013010l
#define TYP_MC3011					0x00013011l
#define TYP_MC3012					0x00013012l
#define TYP_MC3013					0x00013013l
#define TYP_MC3014					0x00013014l
#define TYP_MC3015					0x00013015l
#define TYP_MC3016					0x00013016l
#define TYP_MC3020					0x00013020l
#define TYP_MC3021					0x00013021l
#define TYP_MC3022					0x00013022l
#define TYP_MC3023					0x00013023l
#define TYP_MC3024					0x00013024l
#define TYP_MC3025					0x00013025l
#define TYP_MC3026					0x00013026l
#define TYP_MC3027					0x00013027l
#define TYP_MC3031					0x00013031l
#define TYP_MC3033					0x00013033l

#define TYP_MX3010					0x00023010l
#define TYP_MX3011					0x00023011l
#define TYP_MX3012					0x00023012l
#define TYP_MX3020					0x00023020l
#define TYP_MX3021					0x00023021l
#define TYP_MX3022					0x00023022l
#define TYP_MX3031					0x00023031l



// ----- MI.31xx, MC.31xx, MX.31xx -----
#define TYP_MI3110					0x00003110l
#define TYP_MI3111					0x00003111l
#define TYP_MI3112					0x00003112l
#define TYP_MI3120					0x00003120l
#define TYP_MI3121					0x00003121l
#define TYP_MI3122					0x00003122l
#define TYP_MI3130					0x00003130l
#define TYP_MI3131					0x00003131l
#define TYP_MI3132					0x00003132l

#define TYP_MC3110					0x00013110l
#define TYP_MC3111					0x00013111l
#define TYP_MC3112					0x00013112l
#define TYP_MC3120					0x00013120l
#define TYP_MC3121					0x00013121l
#define TYP_MC3122					0x00013122l
#define TYP_MC3130					0x00013130l
#define TYP_MC3131					0x00013131l
#define TYP_MC3132					0x00013132l

#define TYP_MX3110					0x00023110l
#define TYP_MX3111					0x00023111l
#define TYP_MX3120					0x00023120l
#define TYP_MX3121					0x00023121l
#define TYP_MX3130					0x00023130l
#define TYP_MX3131					0x00023131l



// ----- MI.40xx, MC.40xx, MX.40xx -----
#define TYP_MI4020					0x00004020l
#define TYP_MI4021					0x00004021l
#define TYP_MI4022					0x00004022l
#define TYP_MI4030					0x00004030l
#define TYP_MI4031					0x00004031l
#define TYP_MI4032					0x00004032l

#define TYP_MC4020					0x00014020l
#define TYP_MC4021					0x00014021l
#define TYP_MC4022					0x00014022l
#define TYP_MC4030					0x00014030l
#define TYP_MC4031					0x00014031l
#define TYP_MC4032					0x00014032l

#define TYP_MX4020					0x00024020l
#define TYP_MX4021					0x00024021l
#define TYP_MX4030					0x00024030l
#define TYP_MX4031					0x00024031l



// ----- MI.45xx, MC.45xx, MX.45xx -----
#define TYP_MI4520					0x00004520l
#define TYP_MI4521					0x00004521l
#define TYP_MI4530					0x00004530l
#define TYP_MI4531					0x00004531l
#define TYP_MI4540					0x00004540l
#define TYP_MI4541					0x00004541l

#define TYP_MC4520					0x00014520l
#define TYP_MC4521					0x00014521l
#define TYP_MC4530					0x00014530l
#define TYP_MC4531					0x00014531l
#define TYP_MC4540					0x00014540l
#define TYP_MC4541					0x00014541l

#define TYP_MX4520					0x00024520l
#define TYP_MX4530					0x00024530l
#define TYP_MX4540					0x00024540l



// ----- MI.60xx, MC.60xx, MX.60xx -----
#define TYP_MI6011					0x00006011l
#define TYP_MI6012					0x00006012l
#define TYP_MI6021					0x00006021l
#define TYP_MI6022					0x00006022l
#define TYP_MI6030					0x00006030l
#define TYP_MI6031					0x00006031l
#define TYP_MI6033					0x00006033l
#define TYP_MI6034					0x00006034l

#define TYP_MC6011					0x00016011l
#define TYP_MC6012					0x00016012l
#define TYP_MC6021					0x00016021l
#define TYP_MC6022					0x00016022l
#define TYP_MC6030					0x00016030l
#define TYP_MC6031					0x00016031l
#define TYP_MC6033					0x00016033l
#define TYP_MC6034					0x00016034l

#define TYP_MX6011					0x00026011l
#define TYP_MX6021					0x00026021l
#define TYP_MX6030					0x00026030l
#define TYP_MX6033					0x00026033l



// ----- MI.61xx, MC.61xx, MX.61xx -----
#define TYP_MI6110					0x00006110l
#define TYP_MI6111					0x00006111l

#define TYP_MC6110					0x00016110l
#define TYP_MC6111					0x00016111l

#define TYP_MX6110					0x00026110l



// ----- MI.70xx, MC.70xx, MX.70xx -----
#define TYP_MI7005					0x00007005l
#define TYP_MI7010					0x00007010l
#define TYP_MI7011					0x00007011l
#define TYP_MI7020					0x00007020l
#define TYP_MI7021					0x00007021l

#define TYP_MC7005					0x00017005l
#define TYP_MC7010					0x00017010l
#define TYP_MC7011					0x00017011l
#define TYP_MC7020					0x00017020l
#define TYP_MC7021					0x00017021l

#define TYP_MX7005					0x00027005l
#define TYP_MX7010					0x00027010l
#define TYP_MX7011					0x00027011l



// ----- MI.72xx, MC.72xx, MX.72xx -----
#define TYP_MI7210                  0x00007210l
#define TYP_MI7211                  0x00007211l
#define TYP_MI7220                  0x00007220l
#define TYP_MI7221                  0x00007221l

#define TYP_MC7210                  0x00017210l
#define TYP_MC7211                  0x00017211l
#define TYP_MC7220                  0x00017220l
#define TYP_MC7221                  0x00017221l

#define TYP_MX7210                  0x00027210l
#define TYP_MX7220                  0x00027220l



// ***** PCI Features Bits *********
#define PCIBIT_MULTI				0x00000001
#define PCIBIT_DIGITAL				0x00000002
#define PCIBIT_CH0DIGI				0x00000004
#define PCIBIT_EXTSAM				0x00000008
#define PCIBIT_3CHANNEL				0x00000010
#define PCIBIT_GATE					0x00000020
#define PCIBIT_SLAVE				0x00000040
#define PCIBIT_MASTER				0x00000080
#define PCIBIT_DOUBLEMEM			0x00000100
#define PCIBIT_SYNC					0x00000200
#define PCIBIT_TIMESTAMP			0x00000400
#define PCIBIT_STARHUB				0x00000800
#define PCIBIT_CA					0x00001000
#define PCIBIT_XIO					0x00002000



// ***** Error Request *************
#define ERRORTEXTLEN				100
#define SPC_LASTERRORTEXT			999996l
#define SPC_LASTERRORVALUE			999997l
#define SPC_LASTERRORREG			999998l
#define SPC_LASTERRORCODE			999999l     // Reading this reset the internal error-memory.



// ***** Register and Command Structure
#define SPC_COMMAND					0l
#define		SPC_RESET				0l
#define		SPC_START				10l
#define		SPC_STARTANDWAIT		11l
#define		SPC_FIFOSTART			12l
#define		SPC_FIFOWAIT			13l
#define		SPC_FORCETRIGGER		16l
#define		SPC_STOP				20l
#define		SPC_POWERDOWN			30l
#define		SPC_SYNCMASTER			100l
#define		SPC_SYNCTRIGGERMASTER	101l
#define		SPC_SYNCMASTERFIFO		102l
#define		SPC_SYNCSLAVE			110l
#define		SPC_SYNCTRIGGERSLAVE	111l
#define		SPC_SYNCSLAVEFIFO		112l
#define		SPC_NOSYNC				120l
#define		SPC_SYNCSTART			130l
#define		SPC_SYNCCALCMASTER		140l
#define		SPC_SYNCCALCMASTERFIFO	141l
#define		SPC_RELAISON			200l
#define		SPC_RELAISOFF			210l
#define		SPC_ADJUSTSTART			300l
#define		SPC_FIFO_BUFREADY0		400l
#define		SPC_FIFO_BUFREADY1		401l
#define		SPC_FIFO_BUFREADY2		402l
#define		SPC_FIFO_BUFREADY3		403l
#define		SPC_FIFO_BUFREADY4		404l
#define		SPC_FIFO_BUFREADY5		405l
#define		SPC_FIFO_BUFREADY6		406l
#define		SPC_FIFO_BUFREADY7		407l
#define		SPC_FIFO_BUFREADY8		408l
#define		SPC_FIFO_BUFREADY9		409l
#define		SPC_FIFO_BUFREADY10		410l
#define		SPC_FIFO_BUFREADY11		411l
#define		SPC_FIFO_BUFREADY12		412l
#define		SPC_FIFO_BUFREADY13		413l
#define		SPC_FIFO_BUFREADY14		414l
#define		SPC_FIFO_BUFREADY15		415l
#define		SPC_FIFO_AUTOBUFSTART	500l
#define		SPC_FIFO_AUTOBUFEND		510l

#define SPC_STATUS					10l
#define		SPC_RUN					0l
#define		SPC_TRIGGER				10l
#define		SPC_READY				20l



// Installation
#define SPC_VERSION					1000l
#define SPC_ISAADR					1010l
#define SPC_INSTMEM					1020l
#define SPC_INSTSAMPLERATE			1030l
#define SPC_BRDTYP					1040l

// MI/MC/MX type information (internal use)
#define SPC_MIINST_MODULES			1100l
#define SPC_MIINST_CHPERMODULE		1110l
#define SPC_MIINST_BYTESPERSAMPLE	1120l
#define SPC_MIINST_BITSPERSAMPLE    1125l
#define SPC_MIINST_MINADCLOCK		1130l
#define SPC_MIINST_MAXADCLOCK		1140l
#define SPC_MIINST_QUARZ			1150l
#define SPC_MIINST_FLAGS			1160l
#define SPC_MIINST_FIFOSUPPORT		1170l


// Driver information
#define SPC_GETDRVVERSION           1200l
#define SPC_GETKERNELVERSION        1210l
#define SPC_GETDRVTYPE              1220l
#define     DRVTYP_DOS              0l
#define     DRVTYP_LINUX            1l
#define     DRVTYP_VXD              2l
#define     DRVTYP_NTLEGACY         3l
#define     DRVTYP_WDM              4l

// PCI, CompactPCI and PXI Installation Information
#define SPC_PCITYP					2000l
#define SPC_PCIVERSION				2010l
#define SPC_PCIEXTVERSION           2011l
#define SPC_PCIDATE					2020l
#define SPC_PCISERIALNR				2030l
#define SPC_PCISERIALNO				2030l
#define SPC_PCISAMPLERATE			2100l
#define SPC_PCIMEMSIZE				2110l
#define SPC_PCIFEATURES				2120l
#define SPC_PCIINFOADR				2200l
#define SPC_PCIINTERRUPT			2300l
#define SPC_PCIBASEADR0				2400l
#define SPC_PCIBASEADR1				2401l
#define SPC_PCIREGION0				2410l
#define SPC_PCIREGION1				2411l
#define SPC_READTRGLVLCOUNT			2500l
#define SPC_READIRCOUNT				3000l
#define SPC_READUNIPOLAR0			3010l
#define SPC_READUNIPOLAR1			3020l
#define SPC_READUNIPOLAR2			3030l
#define SPC_READUNIPOLAR3			3040l
#define SPC_READMAXOFFSET			3100l
#define SPC_READRANGECH0_0			3200l
#define SPC_READRANGECH0_1			3201l
#define SPC_READRANGECH0_2			3202l
#define SPC_READRANGECH0_3			3203l
#define SPC_READRANGECH0_4			3204l
#define SPC_READRANGECH0_5			3205l
#define SPC_READRANGECH0_6			3206l
#define SPC_READRANGECH0_7			3207l
#define SPC_READRANGECH0_8			3208l
#define SPC_READRANGECH0_9			3209l
#define SPC_READRANGECH1_0			3300l
#define SPC_READRANGECH1_1			3301l
#define SPC_READRANGECH1_2			3302l
#define SPC_READRANGECH1_3			3303l
#define SPC_READRANGECH1_4			3304l
#define SPC_READRANGECH1_5			3305l
#define SPC_READRANGECH1_6			3306l
#define SPC_READRANGECH1_7			3307l
#define SPC_READRANGECH1_8			3308l
#define SPC_READRANGECH1_9			3309l
#define SPC_READRANGECH2_0			3400l
#define SPC_READRANGECH2_1			3401l
#define SPC_READRANGECH2_2			3402l
#define SPC_READRANGECH2_3			3403l
#define SPC_READRANGECH3_0			3500l
#define SPC_READRANGECH3_1			3501l
#define SPC_READRANGECH3_2			3502l
#define SPC_READRANGECH3_3			3503l

#define SPC_READRANGEMIN0			4000l
#define SPC_READRANGEMIN99			4099l
#define SPC_READRANGEMAX0			4100l
#define SPC_READRANGEMAX99			4199l
#define SPC_PCICOUNTER				9000l



// Memory
#define SPC_MEMSIZE					10000l
#define SPC_POSTTRIGGER				10100l
#define SPC_STARTOFFSET				10200l



// Channels
#define SPC_CHENABLE				11000l



// ----- channel enable flags for A/D and D/A boards -----
#define		CHANNEL0				1l
#define		CHANNEL1				2l
#define		CHANNEL2				4l
#define		CHANNEL3				8l
#define		CHANNEL4				16l
#define		CHANNEL5				32l
#define		CHANNEL6				64l
#define		CHANNEL7				128l
#define		CHANNEL8				256l
#define		CHANNEL9				512l
#define		CHANNEL10				1024l
#define		CHANNEL11				2048l
#define		CHANNEL12				4096l
#define		CHANNEL13				8192l
#define		CHANNEL14				16384l
#define		CHANNEL15				32768l

// ----- old digital i/o settings for 16 bit implementation -----
#define     CH0_8BITMODE            65536l  // for MI.70xx only     
#define		CH0_16BIT				1l
#define		CH0_32BIT				3l
#define		CH1_16BIT				4l
#define		CH1_32BIT				12l

// ----- new digital i/o settings for 8 bit implementation -----
#define     MOD0_8BIT               1l          
#define     MOD0_16BIT              3l          
#define     MOD0_32BIT              15l      
#define     MOD1_8BIT               16l      
#define     MOD1_16BIT              48l      
#define     MOD1_32BIT              240l      

#define SPC_CHROUTE0				11010l
#define SPC_CHROUTE1				11020l

#define SPC_BITENABLE               11030l



// ----- Clock Settings -----
#define SPC_SAMPLERATE				20000l
#define SPC_SAMPLERATE2				20010l
#define SPC_SR2						20020l
#define SPC_PLL_ENABLE				20030l
#define SPC_CLOCKDIV				20040l
#define SPC_INTCLOCKDIV				20041l
#define SPC_PLL_R					20060l
#define SPC_PLL_F					20061l
#define SPC_PLL_S					20062l
#define SPC_PLL_DIV					20063l
#define SPC_EXTERNALCLOCK			20100l
#define SPC_EXTERNOUT				20110l
#define SPC_CLOCKOUT				20110l
#define SPC_CLOCK50OHM				20120l
#define SPC_CLOCK110OHM				20120l
#define SPC_EXTERNRANGE				20130l
#define		EXRANGE_NOPLL			1l
#define		EXRANGE_SINGLE			2l
#define		EXRANGE_BURST_S			4l
#define		EXRANGE_BURST_M			8l
#define		EXRANGE_BURST_L			16l
#define		EXRANGE_BURST_XL		32l
#define SPC_REFERENCECLOCK			20140l
#define     REFCLOCK_PXI			-1l



// ----- In/Out Range -----
#define SPC_OFFS0					30000l
#define SPC_AMP0					30010l
#define SPC_ACDC0					30020l
#define SPC_50OHM0					30030l
#define SPC_DIFF0					30040l
#define SPC_DOUBLEOUT0				30041l
#define SPC_DIGITAL0				30050l
#define SPC_110OHM0					30060l
#define SPC_110OHM0L				30060l
#define SPC_INOUT0					30070l
#define SPC_FILTER0					30080l
#define SPC_BANKSWITCH0				30081l

#define SPC_OFFS1					30100l
#define SPC_AMP1					30110l
#define SPC_ACDC1					30120l
#define SPC_50OHM1					30130l
#define SPC_DIFF1					30140l
#define SPC_DOUBLEOUT1				30141l
#define SPC_DIGITAL1				30150l
#define SPC_110OHM1					30160l
#define SPC_110OHM0H				30160l
#define SPC_INOUT1					30170l
#define SPC_FILTER1					30180l
#define SPC_BANKSWITCH1				30181l

#define SPC_OFFS2					30200l
#define SPC_AMP2					30210l
#define SPC_ACDC2					30220l
#define SPC_50OHM2					30230l
#define SPC_DIFF2					30240l
#define SPC_DOUBLEOUT2				30241l
#define SPC_110OHM2					30260l
#define SPC_110OHM1L				30260l
#define SPC_INOUT2					30270l
#define SPC_FILTER2					30280l
#define SPC_BANKSWITCH2				30281l

#define SPC_OFFS3					30300l
#define SPC_AMP3					30310l
#define SPC_ACDC3					30320l
#define SPC_50OHM3					30330l
#define SPC_DIFF3					30340l
#define SPC_DOUBLEOUT3				30341l
#define SPC_110OHM3					30360l
#define SPC_110OHM1H				30360l
#define SPC_INOUT3					30370l
#define SPC_FILTER3					30380l
#define SPC_BANKSWITCH3				30381l

#define SPC_OFFS4					30400l
#define SPC_AMP4					30410l
#define SPC_ACDC4					30420l
#define SPC_50OHM4					30430l
#define SPC_DIFF4					30440l

#define SPC_OFFS5					30500l
#define SPC_AMP5					30510l
#define SPC_ACDC5					30520l
#define SPC_50OHM5					30530l
#define SPC_DIFF5					30540l

#define SPC_OFFS6					30600l
#define SPC_AMP6					30610l
#define SPC_ACDC6					30620l
#define SPC_50OHM6					30630l
#define SPC_DIFF6					30640l

#define SPC_OFFS7					30700l
#define SPC_AMP7					30710l
#define SPC_ACDC7					30720l
#define SPC_50OHM7					30730l
#define SPC_DIFF7					30740l

#define SPC_110OHMTRIGGER			30400l
#define SPC_110OHMCLOCK				30410l


#define   AMP_BI200					200l
#define   AMP_BI500					500l
#define   AMP_BI1000				1000l
#define   AMP_BI2000				2000l
#define   AMP_BI2500				2500l
#define   AMP_BI4000				4000l
#define   AMP_BI5000				5000l
#define   AMP_BI10000				10000l
#define   AMP_UNI400				100400l
#define   AMP_UNI1000				101000l
#define   AMP_UNI2000				102000l


// ----- Trigger Settings -----
#define SPC_TRIGGERMODE				40000l
#define SPC_TRIGGEROUT				40100l
#define SPC_TRIGGER50OHM			40110l
#define SPC_TRIGGER110OHM0			40110l
#define SPC_TRIGGER110OHM1			40111l
#define SPC_TRIGGERMODE0			40200l
#define SPC_TRIGGERMODE1			40201l
#define SPC_TRIGGERMODE2			40202l
#define SPC_TRIGGERMODE3			40203l
#define SPC_TRIGGERMODE4			40204l
#define SPC_TRIGGERMODE5			40205l
#define SPC_TRIGGERMODE6			40206l
#define SPC_TRIGGERMODE7			40207l
#define		TM_SOFTWARE				0l
#define		TM_NOTRIGGER			10l
#define		TM_CHXPOS				10000l
#define		TM_CHXPOS_LP			10001l
#define		TM_CHXPOS_SP			10002l
#define		TM_CHXPOS_GS			10003l
#define		TM_CHXPOS_SS			10004l
#define		TM_CHXNEG				10010l
#define		TM_CHXNEG_LP			10011l
#define		TM_CHXNEG_SP			10012l
#define		TM_CHXNEG_GS			10013l
#define		TM_CHXNEG_SS			10014l
#define		TM_CHXOFF				10020l
#define		TM_CHXBOTH				10030l
#define		TM_CHXWINENTER			10040l
#define		TM_CHXWINENTER_LP		10041l
#define		TM_CHXWINENTER_SP		10042l
#define		TM_CHXWINLEAVE			10050l
#define		TM_CHXWINLEAVE_LP		10051l
#define		TM_CHXWINLEAVE_SP		10052l

#define		TM_CH0POS				10000l
#define		TM_CH0NEG				10010l
#define		TM_CH0OFF				10020l
#define		TM_CH0BOTH				10030l
#define		TM_CH1POS				10100l
#define		TM_CH1NEG				10110l
#define		TM_CH1OFF				10120l
#define		TM_CH1BOTH				10130l
#define		TM_CH2POS				10200l
#define		TM_CH2NEG				10210l
#define		TM_CH2OFF				10220l
#define		TM_CH2BOTH				10230l
#define		TM_CH3POS				10300l
#define		TM_CH3NEG				10310l
#define		TM_CH3OFF				10320l
#define		TM_CH3BOTH				10330l

#define		TM_TTLPOS				20000l
#define		TM_TTLHIGH_LP			20001l
#define		TM_TTLHIGH_SP			20002l
#define		TM_TTLNEG				20010l
#define		TM_TTLLOW_LP			20011l
#define		TM_TTLLOW_SP			20012l
#define		TM_TTL					20020l
#define		TM_TTLBOTH				20030l
#define		TM_TTLBOTH_LP			20031l
#define		TM_TTLBOTH_SP			20032l
#define		TM_CHANNEL				20040l
#define		TM_PATTERN				21000l
#define		TM_PATTERN_LP			21001l
#define		TM_PATTERN_SP			21002l
#define		TM_PATTERNANDEDGE		22000l
#define		TM_PATTERNANDEDGE_LP	22001l
#define		TM_PATTERNANDEDGE_SP	22002l
#define		TM_GATELOW				30000l
#define		TM_GATEHIGH				30010l
#define		TM_GATEPATTERN			30020l
#define		TM_CHOR					35000l
#define		TM_CHAND				35010l

#define SPC_PXITRGOUT               40300l
#define     PTO_OFF                  0l
#define     PTO_LINE0                1l
#define     PTO_LINE1                2l
#define     PTO_LINE2                3l
#define     PTO_LINE3                4l
#define     PTO_LINE4                5l
#define     PTO_LINE5                6l
#define     PTO_LINE6                7l
#define     PTO_LINE7                8l
#define     PTO_LINESTAR             9l
#define SPC_PXITRGOUT_AVAILABLE     40301l  // bitmap register


#define SPC_PXITRGIN                40310l  // bitmap register
#define     PTI_OFF                  0l
#define     PTI_LINE0                1l
#define     PTI_LINE1                2l
#define     PTI_LINE2                4l
#define     PTI_LINE3                8l
#define     PTI_LINE4                16l
#define     PTI_LINE5                32l
#define     PTI_LINE6                64l 
#define     PTI_LINE7                128l
#define     PTI_LINESTAR             256l
#define SPC_PXITRGIN_AVAILABLE      40311l  // bitmap register


#define SPC_SINGLESHOT				41000l
#define SPC_OUTONTRIGGER			41100l
#define SPC_RESTARTCONT				41200l
#define SPC_TRIGGERLEVEL			42000l
#define SPC_TRIGGERLEVEL0			42000l
#define SPC_TRIGGERLEVEL1			42001l
#define SPC_TRIGGERLEVEL2			42002l
#define SPC_TRIGGERLEVEL3			42003l
#define SPC_TRIGGERLEVEL4			42004l
#define SPC_TRIGGERLEVEL5			42005l
#define SPC_TRIGGERLEVEL6			42006l
#define SPC_TRIGGERLEVEL7			42007l
#define SPC_HIGHLEVEL0				42000l
#define SPC_HIGHLEVEL1				42001l
#define SPC_HIGHLEVEL2				42002l
#define SPC_HIGHLEVEL3				42003l
#define SPC_HIGHLEVEL4				42004l
#define SPC_HIGHLEVEL5				42005l
#define SPC_HIGHLEVEL6				42006l
#define SPC_HIGHLEVEL7				42007l
#define SPC_LOWLEVEL0				42100l
#define SPC_LOWLEVEL1				42101l
#define SPC_LOWLEVEL2				42102l
#define SPC_LOWLEVEL3				42103l
#define SPC_LOWLEVEL4				42104l
#define SPC_LOWLEVEL5				42105l
#define SPC_LOWLEVEL6				42106l
#define SPC_LOWLEVEL7				42107l
#define SPC_TRIGGERPATTERN			43000l
#define SPC_TRIGGERPATTERN0			43000l
#define SPC_TRIGGERPATTERN1			43001l
#define SPC_TRIGGERMASK				43100l
#define SPC_TRIGGERMASK0			43100l
#define SPC_TRIGGERMASK1			43101l
#define SPC_PULSEWIDTH				44000l
#define SPC_PULSEWIDTH0				44000l
#define SPC_PULSEWIDTH1				44001l
#define SPC_READTROFFSET			45000l
#define SPC_TRIGGEREDGE				46000l
#define SPC_TRIGGEREDGE0			46000l
#define SPC_TRIGGEREDGE1			46001l
#define		TE_POS					10000l
#define		TE_NEG					10010l
#define		TE_BOTH					10020l
#define		TE_NONE					10030l


// ----- Timestamp -----
#define CH_TIMESTAMP				9999l

#define SPC_TIMESTAMP_CMD			47000l
#define		TS_RESET					0l
#define		TS_MODE_DISABLE				10l
#define		TS_MODE_STARTRESET			11l
#define		TS_MODE_STANDARD			12l
#define		TS_MODE_REFCLOCK			13l
#define		TS_MODE_TEST5555			90l
#define		TS_MODE_TESTAAAA			91l
#define		TS_MODE_ZHTEST				92l

#define SPC_TIMESTAMP_STATUS		47010l
#define		TS_FIFO_EMPTY				0l
#define		TS_FIFO_LESSHALF			1l
#define		TS_FIFO_MOREHALF			2l
#define		TS_FIFO_OVERFLOW			3l

#define	SPC_TIMESTAMP_COUNT			47020l
#define SPC_TIMESTAMP_STARTTIME		47030l
#define SPC_TIMESTAMP_FIFO			47040l

#define SPC_TIMESTAMP_RESETMODE     47050l
#define     TS_RESET_POS               10l
#define     TS_RESET_NEG               20l



// ----- Extra I/O module -----
#define SPC_XIO_DIRECTION			47100l
#define		XD_CH0_INPUT				0l
#define		XD_CH0_OUTPUT				1l
#define		XD_CH1_INPUT				0l
#define		XD_CH1_OUTPUT				2l
#define		XD_CH2_INPUT				0l
#define		XD_CH2_OUTPUT				4l
#define SPC_XIO_DIGITALIO			47110l
#define SPC_XIO_ANALOGOUT0			47120l
#define SPC_XIO_ANALOGOUT1			47121l
#define SPC_XIO_ANALOGOUT2			47122l
#define SPC_XIO_ANALOGOUT3			47123l
#define SPC_XIO_WRITEDACS			47130l



// ----- Star-Hub -----
#define SPC_STARHUB_CMD				48000l
#define		SH_INIT						0l	// Internal use: Initialisation of Starhub
#define		SH_AUTOROUTE				1l	// Internal use: Routing of Starhub
#define		SH_INITDONE					2l	// Internal use: End of Init
#define		SH_SYNCSTART				3l	// Internal use: Synchronisation

#define SPC_STARHUB_STATUS			48010l

#define SPC_STARHUB_ROUTE0			48100l	// Routing Information for Test
#define SPC_STARHUB_ROUTE99			48199l	// ...




// ----- Gain and Offset Adjust DAC's -----
#define SPC_ADJ_START				50000l

#define SPC_ADJ_LOAD				50000l
#define SPC_ADJ_SAVE				50010l
#define		ADJ_DEFAULT					0l
#define		ADJ_USER0					1l
#define		ADJ_USER1					2l
#define		ADJ_USER2					3l
#define		ADJ_USER3					4l
#define		ADJ_USER4					5l
#define		ADJ_USER5					6l
#define		ADJ_USER6					7l
#define		ADJ_USER7					8l

#define SPC_ADJ_AUTOADJ				50020l
#define		ADJ_ALL						0l     
#define		ADJ_CURRENT					1l

#define SPC_ADJ_SET					50030l
#define SPC_ADJ_FAILMASK            50040l

#define SPC_ADJ_OFFSET0				51000l
#define SPC_ADJ_OFFSET999			51999l

#define SPC_ADJ_GAIN0				52000l
#define SPC_ADJ_GAIN999				52999l

#define SPC_ADJ_CORRECT0			53000l
#define SPC_ADJ_CORRECT999			53999l

#define SPC_ADJ_XIOOFFS0            54000l
#define SPC_ADJ_XIOOFFS1            54001l
#define SPC_ADJ_XIOOFFS2            54002l
#define SPC_ADJ_XIOOFFS3            54003l

#define SPC_ADJ_XIOGAIN0            54010l
#define SPC_ADJ_XIOGAIN1            54011l
#define SPC_ADJ_XIOGAIN2            54012l
#define SPC_ADJ_XIOGAIN3            54013l

#define SPC_ADJ_END					59999l



// ----- FIFO Control -----
#define SPC_FIFO_BUFFERS			60000l			// number of FIFO buffers
#define SPC_FIFO_BUFLEN				60010l			// len of each FIFO buffer
#define SPC_FIFO_BUFCOUNT			60020l			// number of FIFO buffers tranfered until now
#define SPC_FIFO_BUFMAXCNT			60030l			// number of FIFO buffers to be transfered (0=continuous)
#define SPC_FIFO_BUFADRCNT          60040l          // number of FIFO buffers allowed
#define SPC_FIFO_BUFREADY           60050l          // fifo buffer ready register (same as SPC_COMMAND + SPC_FIFO_BUFREADY0...)
#define SPC_FIFO_BUFADR0			60100l			// adress of FIFO buffer no. 0
#define SPC_FIFO_BUFADR1			60101l			// ...
#define SPC_FIFO_BUFADR2			60102l			// ...
#define SPC_FIFO_BUFADR3			60103l			// ...
#define SPC_FIFO_BUFADR4			60104l			// ...
#define SPC_FIFO_BUFADR5			60105l			// ...
#define SPC_FIFO_BUFADR6			60106l			// ...
#define SPC_FIFO_BUFADR7			60107l			// ...
#define SPC_FIFO_BUFADR8			60108l			// ...
#define SPC_FIFO_BUFADR9			60109l			// ...
#define SPC_FIFO_BUFADR10			60110l			// ...
#define SPC_FIFO_BUFADR11			60111l			// ...
#define SPC_FIFO_BUFADR12			60112l			// ...
#define SPC_FIFO_BUFADR13			60113l			// ...
#define SPC_FIFO_BUFADR14			60114l			// ...
#define SPC_FIFO_BUFADR15			60115l			// ...
#define SPC_FIFO_BUFADR255			60355l			// last



// ----- Filter -----
#define SPC_FILTER					100000l



// ----- Pattern -----
#define SPC_PATTERNENABLE			110000l
#define SPC_READDIGITAL				110100l



// ----- Miscellanous -----
#define SPC_MISCDAC0				200000l
#define SPC_MISCDAC1				200010l
#define SPC_FACTORYMODE				200020l
#define SPC_DIRECTDAC				200030l
#define SPC_NOTRIGSYNC				200040l
#define SPC_DSPDIRECT				200100l
#define SPC_DMAPHYSICALADR          200110l
#define SPC_XYZMODE					200200l
#define SPC_INVERTDATA				200300l
#define SPC_GATEMARKENABLE			200400l
#define SPC_EXPANDINT32				200500l
#define SPC_NOPRETRIGGER			200600l
#define SPC_RELAISWAITTIME			200700l
#define SPC_DACWAITTIME             200710l
#define SPC_ILAMODE					200800l
#define SPC_NMDGMODE                200810l
#define SPC_ENHANCEDSTATUS			200900l
#define SPC_OVERRANGEBIT			201000l
#define SPC_2CH8BITMODE				201100l
#define SPC_12BITMODE				201200l
#define SPC_HOLDLASTSAMPLE          201300l
#define SPC_CKSYNC0					202000l
#define SPC_CKSYNC1					202001l
#define SPC_DISABLEMOD0				203000l
#define SPC_DISABLEMOD1				203010l
#define SPC_ENABLEOVERRANGECHECK    204000l
#define SPC_OVERRANGESTATUS         204010l
#define SPC_BITMODE					205000l

#define SPC_READBACK				206000l
#define SPC_STOPLEVEL1				206010l
#define SPC_STOPLEVEL0				206020l
#define SPC_DIFFMODE				206030l
#define SPC_DACADJUST	            206040l

#define SPC_MULTI					220000l
#define SPC_DOUBLEMEM				220100l
#define SPC_MULTIMEMVALID			220200l
#define SPC_BANK					220300l
#define SPC_GATE					220400l
#define SPC_RELOAD					230000l
#define SPC_USEROUT					230010l
#define SPC_WRITEUSER0				230100l
#define SPC_WRITEUSER1				230110l
#define SPC_READUSER0				230200l
#define SPC_READUSER1				230210l
#define SPC_MUX						240000l
#define SPC_ADJADC					241000l
#define SPC_ADJOFFS0				242000l
#define SPC_ADJOFFS1				243000l
#define SPC_ADJGAIN0				244000l
#define SPC_ADJGAIN1				245000l
#define SPC_READEPROM				250000l
#define SPC_WRITEEPROM				250010l
#define SPC_DIRECTIO				260000l
#define SPC_DIRECT_MODA				260010l
#define SPC_DIRECT_MODB				260020l
#define SPC_DIRECT_EXT0				260030l
#define SPC_DIRECT_EXT1				260031l
#define SPC_DIRECT_EXT2				260032l
#define SPC_DIRECT_EXT3				260033l
#define SPC_DIRECT_EXT4				260034l
#define SPC_DIRECT_EXT5				260035l
#define SPC_DIRECT_EXT6				260036l
#define SPC_DIRECT_EXT7				260037l
#define SPC_MEMTEST					270000l
#define SPC_NODMA					275000l
#define SPC_NOCOUNTER				275010l
#define SPC_NOSCATTERGATHER			275020l
#define SPC_RUNINTENABLE			290000l
#define SPC_XFERBUFSIZE				295000l
#define SPC_CHLX					295010l
#define SPC_SPECIALCLOCK			295100l
#define SPC_STARTDELAY				295110l
#define SPC_BASISTTLTRIG			295120l
#define SPC_TIMEOUT					295130l
#define SPC_LOGDLLCALLS				299999l






// ----- PCK400 -----
#define SPC_FREQUENCE				300000l
#define SPC_DELTAFREQUENCE			300010l
#define SPC_PINHIGH					300100l
#define SPC_PINLOW					300110l
#define SPC_PINDELTA				300120l
#define SPC_STOPLEVEL				300200l
#define SPC_PINRELAIS				300210l
#define SPC_EXTERNLEVEL				300300l



// ----- PADCO -----
#define SPC_COUNTER0				310000l
#define SPC_COUNTER1				310001l
#define SPC_COUNTER2				310002l
#define SPC_COUNTER3				310003l
#define SPC_COUNTER4				310004l
#define SPC_COUNTER5				310005l
#define SPC_MODE0					310100l
#define SPC_MODE1					310101l
#define SPC_MODE2					310102l
#define SPC_MODE3					310103l
#define SPC_MODE4					310104l
#define SPC_MODE5					310105l
#define		CM_SINGLE					1l
#define		CM_MULTI					2l
#define		CM_POSEDGE					4l
#define		CM_NEGEDGE					8l
#define		CM_HIGHPULSE				16l
#define		CM_LOWPULSE					32l



// ----- PAD1616 -----
#define SPC_SEQUENCERESET			320000l
#define SPC_SEQUENCEADD				320010l
#define		SEQ_IR_10000MV				0l
#define		SEQ_IR_5000MV				1l
#define		SEQ_IR_2000MV				2l
#define		SEQ_IR_1000MV				3l
#define		SEQ_IR_500MV				4l
#define		SEQ_CH0						0l
#define		SEQ_CH1						8l
#define		SEQ_CH2						16l
#define		SEQ_CH3						24l 
#define		SEQ_CH4						32l
#define		SEQ_CH5						40l 
#define		SEQ_CH6						48l
#define		SEQ_CH7						56l
#define		SEQ_CH8						64l
#define		SEQ_CH9						72l
#define		SEQ_CH10					80l
#define		SEQ_CH11					88l
#define		SEQ_CH12					96l
#define		SEQ_CH13					104l
#define		SEQ_CH14					112l
#define		SEQ_CH15					120l
#define		SEQ_TRIGGER					128l
#define		SEQ_START					256l



// ----- Option CA -----
#define SPC_CA_MODE					330000l
#define		CAMODE_OFF					0l
#define		CAMODE_CDM					1l
#define		CAMODE_KW					2l
#define		CAMODE_OT					3l
#define     CAMODE_CDMMUL				4l
#define SPC_CA_TRIGDELAY			330010l
#define SPC_CA_CKDIV				330020l
#define SPC_CA_PULS					330030l
#define SPC_CA_CKMUL				330040l
#define SPC_CA_DREHZAHLFORMAT		330050l
#define		CADREH_4X4					0l
#define		CADREH_1X16					1l
#define SPC_CA_KWINVERT				330060l
#define SPC_CA_OUTA					330100l
#define SPC_CA_OUTB					330110l
#define		CAOUT_TRISTATE				0l
#define		CAOUT_LOW					1l
#define		CAOUT_HIGH					2l
#define		CAOUT_CDM					3l
#define		CAOUT_OT					4l
#define		CAOUT_KW					5l
#define		CAOUT_TRIG					6l
#define		CAOUT_CLK					7l
#define		CAOUT_KW60					8l
#define		CAOUT_KWGAP					9l
#define     CAOUT_TRDLY                 10l
#define		CAOUT_INVERT				16l



// ----- Hardware registers (debug use only) -----
#define SPC_REG0x00					900000l
#define SPC_REG0x02					900010l
#define SPC_REG0x04					900020l
#define SPC_REG0x06					900030l
#define SPC_REG0x08					900040l
#define SPC_REG0x0A					900050l
#define SPC_REG0x0C					900060l
#define SPC_REG0x0E					900070l

#define SPC_DEBUGREG0				900100l
#define SPC_DEBUGREG15				900115l
#define SPC_DEBUGVALUE0				900200l
#define SPC_DEBUGVALUE15			900215l

#define SPC_MI_ISP					901000l
#define		ISP_TMS_0					0l
#define		ISP_TMS_1					1l
#define		ISP_TDI_0					0l
#define		ISP_TDO_1					2l


#define SPC_EE_RWAUTH               901100l
#define SPC_EE_REG                  901110l
#define SPC_EE_RESETCOUNTER         901120l

// ----- Test Registers -----
#define SPC_TEST_BASE               902000l
#define SPC_TEST_LOCAL_START        902100l
#define SPC_TEST_LOCAL_END          902356l
#define SPC_TEST_PLX_START          902400l
#define SPC_TEST_PLX_END            902656l
