#ifndef _tiepie
#define _tiepie

/*
 * This file contains the declarations for the low level routines
 * for the TiePie engineering instrument dll's
 */

#include <windows.h>


/*
 * declarations to match pascal-types to c-types
 */
 typedef unsigned char      byte;
 typedef unsigned int       dword;
 typedef unsigned short int word;
 typedef signed   long int  integer;

/*
 * The instruments
 */
 typedef enum { dtHandyprobe2,
                dtHandyscope2,
                dtHandyscope3,
                dtHS508,
                dtHS801,
                dtTP112,
                dtTP208,
                dtTP508,
                dtTP801,
                dtPCI801
              } TDeviceTypes;

/*
 * Error-codes
 */
 #define E_INVALID_VALUE   0x0020
 #define E_INVALID_CHANNEL 0x0010
 #define E_NO_GENERATOR    0x0008
 #define E_NOT_SUPPORTED   0x0004
 #define E_NOT_INITIALISED 0x0002
 #define E_NO_HARDWARE     0x0001
 #define E_NO_ERRORS       0x0000
/*
 * channel indicators
 */
 #define Ch1 1
 #define Ch2 2
/*
 * Measure mode settings
 */
 #define mmCh1             1
 #define mmCh2             2
 #define mmCh1Ch2          3
/*
 * Coupling settings
 */
 #define ctAC              0
 #define ctDC              1
/*
 * Trigger sources
 */
 #define tsCh1             0
 #define tsCh2             1
 #define tsExternal        2
 #define tsAnd             3
 #define tsOr              4
 #define tsNo              5
 #define tsPxiExt          6
 #define tsXor             7
 #define tsAnalogExt       8
/*
 * Trigger modes
 */
 #define tmRising          0
 #define tmFalling         1
 #define tmInWindow        2
 #define tmOutWindow       3
 #define tmTVLine          4
 #define tmTVFrameOdd      5
 #define tmTVFrameEven     6
/*
 * Function generator signal types
 */
 #define stSine            0
 #define stTriangle        1
 #define stSquare          2
 #define stDC              3
 #define stNoise           4


bool _export OpenDLL( TDeviceTypes Device );

/*
 * Declarations for functions from dynamically loaded librarie(s)
 *
 * Init / Exit
 */
typedef word  (WINAPI    *INITINSTRUMENT           ) (word    );
extern  word  (__stdcall *InitInstrument           ) (word    );

typedef word  (WINAPI    *EXITINSTRUMENT           ) (void    );
extern  word  (__stdcall *ExitInstrument           ) (void    );

/*
 * Information retrieval
 */

typedef word  (WINAPI    *GETSERIALNUMBER          ) (dword*  );
extern  word  (__stdcall *GetSerialNumber          ) (dword*  );

typedef word  (WINAPI    *GETCALIBRATIONDATE       ) (dword*  );
extern  word  (__stdcall *GetCalibrationDate       ) (dword*  );

typedef dword (WINAPI    *GETMAXRECORDLENGTH       ) (void    );
extern  dword (__stdcall *GetMaxRecordLength       ) (void    );

typedef dword (WINAPI    *GETMAXSAMPLEFREQUENCY    ) (void    );
extern  dword (__stdcall *GetMaxSampleFrequency    ) (void    );

typedef word  (WINAPI    *GETDCLEVELSTATUS         ) (void    );
extern  word  (__stdcall *GetDCLevelStatus         ) (void    );

typedef word  (WINAPI    *GETSQUAREWAVEGENSTATUS   ) (void    );
extern  word  (__stdcall *GetSquareWaveGenStatus   ) (void    );

typedef word  (WINAPI    *GETFUNCTIONGENSTATUS     ) (void    );
extern  word  (__stdcall *GetFunctionGenStatus     ) (void    );

typedef word  (WINAPI    *GETFUNCGENMAXAMPLITUDE   ) (double* );
extern  word  (__stdcall *GetFuncGenMaxAmplitude   ) (double* );

typedef word  (WINAPI    *GETAVAILABLESENSITIVITIES) (double* );
extern  word  (__stdcall *GetAvailableSensitivities) (double* );

typedef word  (WINAPI    *GETAVAILABLERESOLUTIONS  ) (double* );
extern  word  (__stdcall *GetAvailableResolutions  ) (double* );

/*
 * Instrument setup
 */

typedef word  (WINAPI    *GETRESOLUTION            ) (byte*        );
extern  word  (__stdcall *GetResolution            ) (byte*        );

typedef word  (WINAPI    *SETRESOLUTION            ) (byte         );
extern  word  (__stdcall *SetResolution            ) (byte         );

typedef word  (WINAPI    *GETMEASUREMODE           ) (byte*        );
extern  word  (__stdcall *GetMeasureMode           ) (byte*        );

typedef word  (WINAPI    *SETMEASUREMODE           ) (byte         );
extern  word  (__stdcall *SetMeasureMode           ) (byte         );



typedef word  (WINAPI    *SETSENSITIVITY           ) (byte, double*);
extern  word  (__stdcall *SetSensitivity           ) (byte, double*);

typedef word  (WINAPI    *GETSENSITIVITY           ) (byte, double*);
extern  word  (__stdcall *GetSensitivity           ) (byte, double*);

typedef word  (WINAPI    *GETCOUPLING              ) (byte, byte*  );
extern  word  (__stdcall *GetCoupling              ) (byte, byte*  );

typedef word  (WINAPI    *SETCOUPLING              ) (byte, byte   );
extern  word  (__stdcall *SetCoupling              ) (byte, byte   );

typedef word  (WINAPI    *GETDCLEVEL               ) (byte, double*);
extern  word  (__stdcall *GetDcLevel               ) (byte, double*);

typedef word  (WINAPI    *SETDCLEVEL               ) (byte, double );
extern  word  (__stdcall *SetDcLevel               ) (byte, double );

typedef word  (WINAPI    *GETAUTORANGING           ) (byte, byte*  );
extern  word  (__stdcall *GetAutoRanging           ) (byte, byte*  );

typedef word  (WINAPI    *SETAUTORANGING           ) (byte, byte   );
extern  word  (__stdcall *SetAutoRanging           ) (byte, byte   );



typedef dword (WINAPI    *GETRECORDLENGTH          ) (void         );
extern  dword (__stdcall *GetRecordLength          ) (void         );

typedef word  (WINAPI    *SETRECORDLENGTH          ) (dword        );
extern  word  (__stdcall *SetRecordLength          ) (dword        );


typedef dword (WINAPI    *GETPOSTSAMPLES           ) (void         );
extern  dword (__stdcall *GetPostSamples           ) (void         );

typedef word  (WINAPI    *SETPOSTSAMPLES           ) (dword        );
extern  word  (__stdcall *SetPostSamples           ) (dword        );

typedef dword (WINAPI    *GETSAMPLEFREQUENCY       ) (void         );
extern  dword (__stdcall *GetSampleFrequency       ) (void         );

typedef word  (WINAPI    *SETSAMPLEFREQUENCY       ) (dword*       );
extern  word  (__stdcall *SetSampleFrequency       ) (dword*       );



typedef word  (WINAPI    *GETTRIGGERSOURCE         ) (byte*        );
extern  word  (__stdcall *GetTriggerSource         ) (byte*        );

typedef word  (WINAPI    *SETTRIGGERSOURCE         ) (byte         );
extern  word  (__stdcall *SetTriggerSource         ) (byte         );

typedef word  (WINAPI    *GETTRIGGERMODE           ) (byte*        );
extern  word  (__stdcall *GetTriggerMode           ) (byte*        );

typedef word  (WINAPI    *SETTRIGGERMODE           ) (byte         );
extern  word  (__stdcall *SetTriggerMode           ) (byte         );

typedef word  (WINAPI    *GETTRIGGERLEVEL          ) (byte, double*);
extern  word  (__stdcall *GetTriggerLevel          ) (byte, double*);

typedef word  (WINAPI    *SETTRIGGERLEVEL          ) (byte, double );
extern  word  (__stdcall *SetTriggerLevel          ) (byte, double );

typedef word  (WINAPI    *GETTRIGGERHYS            ) (byte, double*);
extern  word  (__stdcall *GetTriggerHys            ) (byte, double*);

typedef word  (WINAPI    *SETTRIGGERHYS            ) (byte, double );
extern  word  (__stdcall *SetTriggerHys            ) (byte, double );

typedef word  (WINAPI    *SETPXITRIGGERENABLES     ) (byte         );
extern  word  (__stdcall *SetPXITriggerEnables     ) (byte         );

typedef word  (WINAPI    *GETPXITRIGGERENABLES     ) (byte*        );
extern  word  (__stdcall *GetPXITriggerEnables     ) (byte*        );

typedef word  (WINAPI    *SETPXITRIGGERSLOPES      ) (byte         );
extern  word  (__stdcall *SetPXITriggerSlopes      ) (byte         );

typedef word  (WINAPI    *GETPXITRIGGERSLOPES      ) (byte*        );
extern  word  (__stdcall *GetPXITriggerSlopes      ) (byte*        );

typedef dword (WINAPI    *GETTRIGGERTIMEOUT        ) (void         );
extern  dword (__stdcall *GetTriggerTimeOut        ) (void         );

typedef word  (WINAPI    *SETTRIGGERTIMEOUT        ) (dword        );
extern  word  (__stdcall *SetTriggerTimeOut        ) (dword        );

typedef word  (WINAPI    *STARTMEASUREMENT         ) (void         );
extern  word  (__stdcall *StartMeasurement         ) (void         );


typedef word  (WINAPI    *GETMEASUREMENTRAW        ) (word*, word* );
extern  word  (__stdcall *GetMeasurementRaw        ) (word*, word* );

typedef word  (WINAPI    *GETONEMEASUREMENTRAW     ) (int, word*, word* );
extern  word  (__stdcall *GetOneMeasurementRaw     ) (int, word*, word* );

typedef word  (WINAPI    *GETMEASUREMENT           ) (double*, double* );
extern  word  (__stdcall *GetMeasurement           ) (double*, double* );

typedef word  (WINAPI    *GETONEMEASUREMENT        ) (int, double*, double* );
extern  word  (__stdcall *GetOneMeasurement        ) (int, double*, double* );


typedef word  (WINAPI    *ADC_START                ) (void         );
extern  word  (__stdcall *ADC_Start                ) (void         );

typedef word  (WINAPI    *ADC_RUNNING              ) (void         );
extern  word  (__stdcall *ADC_Running              ) (void         );

typedef word  (WINAPI    *ADC_ABORT                ) (void         );
extern  word  (__stdcall *ADC_Abort                ) (void         );

typedef word  (WINAPI    *ADC_TRIGGERED            ) (void         );
extern  word  (__stdcall *ADC_Triggered            ) (void         );

typedef word  (WINAPI    *ADC_READY                ) (void         );
extern  word  (__stdcall *ADC_Ready                ) (void         );

typedef word  (WINAPI    *ADC_GETDATA              ) (word*, word* );
extern  word  (__stdcall *ADC_GetData              ) (word*, word* );





typedef word  (WINAPI    *SETFUNCGENSIGNALTYPE     ) (word         );
extern  word  (__stdcall *SetFuncGenSignalType     ) (word         );

typedef word  (WINAPI    *GETFUNCGENSIGNALTYPE     ) (word*        );
extern  word  (__stdcall *GetFuncGenSignalType     ) (word*        );

typedef word  (WINAPI    *SETFUNCGENAMPLITUDE      ) (double       );
extern  word  (__stdcall *SetFuncGenAmplitude      ) (double       );

typedef word  (WINAPI    *GETFUNCGENAMPLITUDE      ) (double*      );
extern  word  (__stdcall *GetFuncGenAmplitude      ) (double*      );

typedef word  (WINAPI    *SETFUNCGENDCOFFSET       ) (double       );
extern  word  (__stdcall *SetFuncGenDCOffset       ) (double       );

typedef word  (WINAPI    *GETFUNCGENDCOFFSET       ) (double*      );
extern  word  (__stdcall *GetFuncGenDCOffset       ) (double*      );

typedef word  (WINAPI    *SETFUNCGENSYMMETRY       ) (double       );
extern  word  (__stdcall *SetFuncGenSymmetry       ) (double       );

typedef word  (WINAPI    *GETFUNCGENSYMMETRY       ) (double*      );
extern  word  (__stdcall *GetFuncGenSymmetry       ) (double*      );

typedef word  (WINAPI    *SETFUNCGENFREQUENCY      ) (double*      );
extern  word  (__stdcall *SetFuncGenFrequency      ) (double*      );

typedef word  (WINAPI    *GETFUNCGENFREQUENCY      ) (double*      );
extern  word  (__stdcall *GetFuncGenFrequency      ) (double*      );

typedef word  (WINAPI    *FILLFUNCGENMEMORY        ) (word, word*  );
extern  word  (__stdcall *FillFuncGenMemory        ) (word, word*  );

typedef word  (WINAPI    *SETFUNCGENOUTPUTON       ) (word         );
extern  word  (__stdcall *SetFuncGenOutputOn       ) (word         );

typedef word  (WINAPI    *GETFUNCGENOUTPUTON       ) (word*        );
extern  word  (__stdcall *GetFuncGenOutputOn       ) (word*        );



/*

word (WINAPI*SetDigitalOutputs) (byte);
word (WINAPI*GetDigitalOutputs) (byte*);

word (WINAPI*GetDigitalInputValues) ( word* );
word (WINAPI*GetOneDigitalValue) (word, word* );


double (WINAPI*GetSquareWaveGenFrequency) (void );
word (WINAPI*SetSquareWaveGenFrequency) (double*);

word (WINAPI*SetFuncGenSignalType) (word );
word (WINAPI*GetFuncGenSignalType) (word*);

word (WINAPI*SetFuncGenAmplitude) (double );
word (WINAPI*GetFuncGenAmplitude) (double*);

word (WINAPI*SetFuncGenDCOffset) (double );
word (WINAPI*GetFuncGenDCOffset) (double*);

word (WINAPI*SetFuncGenSymmetry) (double );
word (WINAPI*GetFuncGenSymmetry) (double*);

word (WINAPI*SetFuncGenFrequency) (double*);
word (WINAPI*GetFuncGenFrequency) (double*);

word (WINAPI*FillFuncGenMemory) (word, word*);

word (WINAPI*SetFuncGenOutputOn) (word );
word (WINAPI*GetFuncGenOutputOn) (word*);





*/

#endif
