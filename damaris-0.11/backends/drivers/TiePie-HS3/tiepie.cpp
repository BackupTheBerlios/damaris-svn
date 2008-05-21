/*
 * C file for linking a TiePie hardware dll to the application
 */


#include "tiepie.h"
#include <windows.h>

/*
 * Init / Exit
 */
 INITINSTRUMENT            InitInstrument            = NULL;
 EXITINSTRUMENT            ExitInstrument            = NULL;
/*
 * Information retrieval
 */
 GETSERIALNUMBER           GetSerialNumber           = NULL;
 GETCALIBRATIONDATE        GetCalibrationDate        = NULL;
 GETMAXRECORDLENGTH        GetMaxRecordLength        = NULL;
 GETMAXSAMPLEFREQUENCY     GetMaxSampleFrequency     = NULL;
 GETDCLEVELSTATUS          GetDCLevelStatus          = NULL;
 GETSQUAREWAVEGENSTATUS    GetSquareWaveGenStatus    = NULL;
 GETFUNCTIONGENSTATUS      GetFunctionGenStatus      = NULL;
 GETFUNCGENMAXAMPLITUDE    GetFuncGenMaxAmplitude    = NULL;
 GETAVAILABLESENSITIVITIES GetAvailableSensitivities = NULL;
 GETAVAILABLERESOLUTIONS   GetAvailableResolutions   = NULL;
 GETNRCHANNELS             GetNrChannels             = NULL;

/*
 * Set / Get instrument status
 */
 GETRESOLUTION             GetResolution             = NULL;
 SETRESOLUTION             SetResolution             = NULL;
 GETMEASUREMODE            GetMeasureMode            = NULL;
 SETMEASUREMODE            SetMeasureMode            = NULL;

 SETSENSITIVITY            SetSensitivity            = NULL;
 GETSENSITIVITY            GetSensitivity            = NULL;
 GETAUTORANGING            GetAutoRanging            = NULL;
 SETAUTORANGING            SetAutoRanging            = NULL;
 SETCOUPLING               SetCoupling               = NULL;
 GETCOUPLING               GetCoupling               = NULL;
 GETDCLEVEL                GetDcLevel                = NULL;
 SETDCLEVEL                SetDcLevel                = NULL;

 GETRECORDLENGTH           GetRecordLength           = NULL;
 SETRECORDLENGTH           SetRecordLength           = NULL;
 GETPOSTSAMPLES            GetPostSamples            = NULL;
 SETPOSTSAMPLES            SetPostSamples            = NULL;
 GETSAMPLEFREQUENCY        GetSampleFrequency        = NULL;
 SETSAMPLEFREQUENCY        SetSampleFrequency        = NULL;

 GETEXTERNALCLOCK          GetExternalClock          = NULL;
 SETEXTERNALCLOCK          SetExternalClock          = NULL;

 GETTRIGGERSOURCE          GetTriggerSource          = NULL;
 SETTRIGGERSOURCE          SetTriggerSource          = NULL;
 GETTRIGGERMODE            GetTriggerMode            = NULL;
 SETTRIGGERMODE            SetTriggerMode            = NULL;
 GETTRIGGERLEVEL           GetTriggerLevel           = NULL;
 SETTRIGGERLEVEL           SetTriggerLevel           = NULL;
 GETTRIGGERHYS             GetTriggerHys             = NULL;
 SETTRIGGERHYS             SetTriggerHys             = NULL;
 SETPXITRIGGERENABLES      SetPXITriggerEnables      = NULL;
 GETPXITRIGGERENABLES      GetPXITriggerEnables      = NULL;
 SETPXITRIGGERSLOPES       SetPXITriggerSlopes       = NULL;
 GETPXITRIGGERSLOPES       GetPXITriggerSlopes       = NULL;
 GETTRIGGERTIMEOUT         GetTriggerTimeOut         = NULL;
 SETTRIGGERTIMEOUT         SetTriggerTimeOut         = NULL;
/*
 *
 */
 SETDIGITALOUTPUTS         SetDigitalOutputs         = NULL;
 GETDIGITALOUTPUTS         GetDigitalOutputs         = NULL;
/*
 * perform measurements
 */
 STARTMEASUREMENT          StartMeasurement          = NULL;
/*
 * retrieve data
 */
 GETMEASUREMENTRAW         GetMeasurementRaw         = NULL;
 GETONEMEASUREMENTRAW      GetOneMeasurementRaw      = NULL;

 GETMEASUREMENT            GetMeasurement            = NULL;
 GETONEMEASUREMENT         GetOneMeasurement         = NULL;

 GETDIGITALINPUTVALUES      GetDigitalInputValues     = NULL;
 GETONEDIGITALVALUE         GetOneDigitalValue        = NULL;


/*
 * advanced measurements
 */
 ADC_START                 ADC_Start                 = NULL;
 ADC_RUNNING               ADC_Running               = NULL;
 ADC_ABORT                 ADC_Abort                 = NULL;
 ADC_TRIGGERED             ADC_Triggered             = NULL;
 ADC_READY                 ADC_Ready                 = NULL;
 ADC_FORCETRIG             ADC_ForceTrig             = NULL;
 ADC_GETDATA               ADC_GetData               = NULL;
 ADC_GETDATAVOLT           ADC_GetDataVolt           = NULL;
 ADC_GETDATACH             ADC_GetDataCh             = NULL;
 ADC_GETDATAVOLTCH         ADC_GetDataVoltCh         = NULL;
/*
 * streaming
 */
 SETRECEIVERHANDLE         SetReceiverHandle         = NULL;
 GETMESSAGEID              GetMessageID              = NULL;
 SETTRANSFERMODE           SetTransferMode           = NULL;
 GETTRANSFERMODE           GetTransferMode           = NULL;

 SETINSTRUMENTCONFIG       SetInstrumentConfig       = NULL;

 SETSQUAREWAVEGENFREQUENCY SetSquareWaveGenFrequency = NULL;
 GETSQUAREWAVEGENFREQUENCY GetSquareWaveGenFrequency = NULL;

 SETFUNCGENSIGNALTYPE      SetFuncGenSignalType      = NULL;
 GETFUNCGENSIGNALTYPE      GetFuncGenSignalType      = NULL;
 SETFUNCGENAMPLITUDE       SetFuncGenAmplitude       = NULL;
 GETFUNCGENAMPLITUDE       GetFuncGenAmplitude       = NULL;
 SETFUNCGENDCOFFSET        SetFuncGenDCOffset        = NULL;
 GETFUNCGENDCOFFSET        GetFuncGenDCOffset        = NULL;
 SETFUNCGENSYMMETRY        SetFuncGenSymmetry        = NULL;
 GETFUNCGENSYMMETRY        GetFuncGenSymmetry        = NULL;
 SETFUNCGENFREQUENCY       SetFuncGenFrequency       = NULL;
 GETFUNCGENFREQUENCY       GetFuncGenFrequency       = NULL;
 FILLFUNCGENMEMORY         FillFuncGenMemory         = NULL;
 SETFUNCGENOUTPUTON        SetFuncGenOutputOn        = NULL;
 GETFUNCGENOUTPUTON        GetFuncGenOutputOn        = NULL;
 FUNCGENBURST              FuncGenBurst              = NULL;
 SETFUNCGENTRIGSOURCE      SetFuncGenTrigSource      = NULL;
 GETFUNCGENTRIGSOURCE      GetFuncGenTrigSource      = NULL; 


 HINSTANCE TiePieDLL;


bool _export OpenDLL( TDeviceTypes Device )
{
/*
 * Check is a dll was already opened. If so, close it and open the new one
 */
 if (TiePieDLL != NULL)
 {
   FreeLibrary( TiePieDLL );
   TiePieDLL = NULL;
 } // if

 /*
  * select the correct DLL
  */
  switch (Device)
  {
    case dtHandyprobe2 : TiePieDLL = LoadLibrary( "hp2.dll"    );
                         break;
    case dtHandyscope2 : TiePieDLL = LoadLibrary( "hs2.dll"    );
                         break;
    case dtHandyscope3 : TiePieDLL = LoadLibrary( "hs3.dll"    );
                         break;
    case dtHandyscope4 : TiePieDLL = LoadLibrary( "hs4.dll"    );
                         break;
    case dtHS508       : TiePieDLL = LoadLibrary( "hs508.dll"  );
                         break;
    case dtHS801       : TiePieDLL = LoadLibrary( "hs801.dll"  );
                         break;
    case dtTP112       : TiePieDLL = LoadLibrary( "tp112.dll"  );
                         break;
    case dtTP208       : TiePieDLL = LoadLibrary( "tp208.dll"  );
                         break;
    case dtTP508       : TiePieDLL = LoadLibrary( "tp508.dll"  );
                         break;
    case dtTP801       : TiePieDLL = LoadLibrary( "tp801.dll"  );
                         break;
    case dtPCI801      : TiePieDLL = LoadLibrary( "pci801.dll" );
                         break;
  } // switch

  if (TiePieDLL != NULL)
  {
    InitInstrument            = (INITINSTRUMENT)            GetProcAddress( TiePieDLL, "InitInstrument"            );
    ExitInstrument            = (EXITINSTRUMENT)            GetProcAddress( TiePieDLL, "ExitInstrument"            );

    GetCalibrationDate        = (GETCALIBRATIONDATE)        GetProcAddress( TiePieDLL, "GetCalibrationDate"        );
    GetSerialNumber           = (GETSERIALNUMBER)           GetProcAddress( TiePieDLL, "GetSerialNumber"           );
    GetMaxSampleFrequency     = (GETMAXSAMPLEFREQUENCY)     GetProcAddress( TiePieDLL, "GetMaxSampleFrequency"     );
    GetMaxRecordLength        = (GETMAXRECORDLENGTH)        GetProcAddress( TiePieDLL, "GetMaxRecordLength"        );
    GetDCLevelStatus          = (GETDCLEVELSTATUS)          GetProcAddress( TiePieDLL, "GetDCLevelStatus"          );
    GetSquareWaveGenStatus    = (GETSQUAREWAVEGENSTATUS)    GetProcAddress( TiePieDLL, "GetSquareWaveGenStatus"    );
    GetFunctionGenStatus      = (GETFUNCTIONGENSTATUS)      GetProcAddress( TiePieDLL, "GetFunctionGenStatus"      );
    GetFuncGenMaxAmplitude    = (GETFUNCGENMAXAMPLITUDE)    GetProcAddress( TiePieDLL, "GetFuncGenMaxAmplitude"    );
    GetAvailableResolutions   = (GETAVAILABLERESOLUTIONS)   GetProcAddress( TiePieDLL, "GetAvailableResolutions"   );
    GetAvailableSensitivities = (GETAVAILABLESENSITIVITIES) GetProcAddress( TiePieDLL, "GetAvailableSensitivities" );
    GetNrChannels             = (GETNRCHANNELS)             GetProcAddress( TiePieDLL, "GetNrChannels"             );

    GetResolution             = (GETRESOLUTION)             GetProcAddress( TiePieDLL, "GetResolution"             );
    SetResolution             = (SETRESOLUTION)             GetProcAddress( TiePieDLL, "SetResolution"             );

    GetMeasureMode            = (GETMEASUREMODE)            GetProcAddress( TiePieDLL, "GetMeasureMode"            );
    SetMeasureMode            = (SETMEASUREMODE)            GetProcAddress( TiePieDLL, "SetMeasureMode"            );

    StartMeasurement          = (STARTMEASUREMENT)          GetProcAddress( TiePieDLL, "StartMeasurement"          );

    GetMeasurementRaw         = (GETMEASUREMENTRAW)         GetProcAddress( TiePieDLL, "GetMeasurementRaw"         );
    GetOneMeasurementRaw      = (GETONEMEASUREMENTRAW)      GetProcAddress( TiePieDLL, "GetOneMeasurementRaw"      );
    GetMeasurement            = (GETMEASUREMENT)            GetProcAddress( TiePieDLL, "GetMeasurement"            );
    GetOneMeasurement         = (GETONEMEASUREMENT)         GetProcAddress( TiePieDLL, "GetOneMeasurement"         );
    GetDigitalInputValues     = (GETDIGITALINPUTVALUES)     GetProcAddress( TiePieDLL, "GetDigitalInputValues"     );
    GetOneDigitalValue        = (GETONEDIGITALVALUE)        GetProcAddress( TiePieDLL, "GetOneDigitalValue"        );

    ADC_Start                 = (ADC_START)                 GetProcAddress( TiePieDLL, "ADC_Start"                 );
    ADC_Running               = (ADC_RUNNING)               GetProcAddress( TiePieDLL, "ADC_Running"               );
    ADC_Abort                 = (ADC_ABORT)                 GetProcAddress( TiePieDLL, "ADC_Abort"                 );
    ADC_Triggered             = (ADC_TRIGGERED)             GetProcAddress( TiePieDLL, "ADC_Triggered"             );
    ADC_Ready                 = (ADC_READY)                 GetProcAddress( TiePieDLL, "ADC_Ready"                 );
    ADC_ForceTrig             = (ADC_FORCETRIG)             GetProcAddress( TiePieDLL, "ADC_ForceTrig"             );
    ADC_GetData               = (ADC_GETDATA)               GetProcAddress( TiePieDLL, "ADC_GetData"               );
    ADC_GetDataVolt           = (ADC_GETDATAVOLT)           GetProcAddress( TiePieDLL, "ADC_GetDataVolt"           );
    ADC_GetDataCh             = (ADC_GETDATACH)             GetProcAddress( TiePieDLL, "ADC_GetDataCh"             );
    ADC_GetDataVoltCh         = (ADC_GETDATAVOLTCH)         GetProcAddress( TiePieDLL, "ADC_GetDataVoltCh"         );

    SetReceiverHandle         = (SETRECEIVERHANDLE)         GetProcAddress( TiePieDLL, "SetReceiverHandle"         );
    GetMessageID              = (GETMESSAGEID)              GetProcAddress( TiePieDLL, "GetMessageID"              );
    SetTransferMode           = (SETTRANSFERMODE)           GetProcAddress( TiePieDLL, "SetTransferMode"           );
    GetTransferMode           = (GETTRANSFERMODE)           GetProcAddress( TiePieDLL, "GetTransferMode"           );
    
    GetRecordLength           = (GETRECORDLENGTH)           GetProcAddress( TiePieDLL, "GetRecordLength"           );
    SetRecordLength           = (SETRECORDLENGTH)           GetProcAddress( TiePieDLL, "SetRecordLength"           );
    GetPostSamples            = (GETPOSTSAMPLES)            GetProcAddress( TiePieDLL, "GetPostSamples"            );
    SetPostSamples            = (SETPOSTSAMPLES)            GetProcAddress( TiePieDLL, "SetPostSamples"            );
    GetSampleFrequency        = (GETSAMPLEFREQUENCY)        GetProcAddress( TiePieDLL, "GetSampleFrequency"        );
    SetSampleFrequency        = (SETSAMPLEFREQUENCY)        GetProcAddress( TiePieDLL, "SetSampleFrequency"        );
    GetExternalClock          = (GETEXTERNALCLOCK)          GetProcAddress( TiePieDLL, "GetExternalClock"          );
    SetExternalClock          = (SETEXTERNALCLOCK)          GetProcAddress( TiePieDLL, "SetExternalClock"          );

    SetSensitivity            = (SETSENSITIVITY)            GetProcAddress( TiePieDLL, "SetSensitivity"            );
    GetSensitivity            = (GETSENSITIVITY)            GetProcAddress( TiePieDLL, "GetSensitivity"            );
    GetAutoRanging            = (GETAUTORANGING)            GetProcAddress( TiePieDLL, "GetAutoRanging"            );
    SetAutoRanging            = (SETAUTORANGING)            GetProcAddress( TiePieDLL, "SetAutoRanging"            );
    SetCoupling               = (SETCOUPLING)               GetProcAddress( TiePieDLL, "SetCoupling"               );
    GetCoupling               = (GETCOUPLING)               GetProcAddress( TiePieDLL, "GetCoupling"               );
    GetDcLevel                = (GETDCLEVEL)                GetProcAddress( TiePieDLL, "GetDcLevel"                );
    SetDcLevel                = (SETDCLEVEL)                GetProcAddress( TiePieDLL, "SetDcLevel"                );

    GetTriggerSource          = (GETTRIGGERSOURCE)          GetProcAddress( TiePieDLL, "GetTriggerSource"          );
    SetTriggerSource          = (SETTRIGGERSOURCE)          GetProcAddress( TiePieDLL, "SetTriggerSource"          );
    GetTriggerMode            = (GETTRIGGERMODE)            GetProcAddress( TiePieDLL, "GetTriggerMode"            );
    SetTriggerMode            = (SETTRIGGERMODE)            GetProcAddress( TiePieDLL, "SetTriggerMode"            );
    GetTriggerLevel           = (GETTRIGGERLEVEL)           GetProcAddress( TiePieDLL, "GetTriggerLevel"           );
    SetTriggerLevel           = (SETTRIGGERLEVEL)           GetProcAddress( TiePieDLL, "SetTriggerLevel"           );
    GetTriggerHys             = (GETTRIGGERHYS)             GetProcAddress( TiePieDLL, "GetTriggerHys"             );
    SetTriggerHys             = (SETTRIGGERHYS)             GetProcAddress( TiePieDLL, "SetTriggerHys"             );
    GetPXITriggerEnables      = (GETPXITRIGGERENABLES)      GetProcAddress( TiePieDLL, "GetPXITriggerEnables"      );
    SetPXITriggerEnables      = (SETPXITRIGGERENABLES)      GetProcAddress( TiePieDLL, "SetPXITriggerEnables"      );
    GetPXITriggerSlopes       = (GETPXITRIGGERSLOPES)       GetProcAddress( TiePieDLL, "GetPXITriggerSlopes"       );
    SetPXITriggerSlopes       = (SETPXITRIGGERSLOPES)       GetProcAddress( TiePieDLL, "SetPXITriggerSlopes"       );
    GetTriggerTimeOut         = (GETTRIGGERTIMEOUT)         GetProcAddress( TiePieDLL, "GetTriggerTimeOut"         );
    SetTriggerTimeOut         = (SETTRIGGERTIMEOUT)         GetProcAddress( TiePieDLL, "SetTriggerTimeOut"         );

    SetDigitalOutputs         = (SETDIGITALOUTPUTS)         GetProcAddress( TiePieDLL, "SetDigitalOutputs"         );
    GetDigitalOutputs         = (GETDIGITALOUTPUTS)         GetProcAddress( TiePieDLL, "GetDigitalOutputs"         );

    SetSquareWaveGenFrequency = (SETSQUAREWAVEGENFREQUENCY) GetProcAddress( TiePieDLL, "SetSquareWaveGenFrequency" );
    GetSquareWaveGenFrequency = (GETSQUAREWAVEGENFREQUENCY) GetProcAddress( TiePieDLL, "GetSquareWaveGenFrequency" );

    SetFuncGenSignalType      = (SETFUNCGENSIGNALTYPE)      GetProcAddress( TiePieDLL, "SetFuncGenSignalType"      );
    GetFuncGenSignalType      = (GETFUNCGENSIGNALTYPE)      GetProcAddress( TiePieDLL, "GetFuncGenSignalType"      );
    SetFuncGenAmplitude       = (SETFUNCGENAMPLITUDE)       GetProcAddress( TiePieDLL, "SetFuncGenAmplitude"       );
    GetFuncGenAmplitude       = (GETFUNCGENAMPLITUDE)       GetProcAddress( TiePieDLL, "GetFuncGenAmplitude"       );
    SetFuncGenDCOffset        = (SETFUNCGENDCOFFSET)        GetProcAddress( TiePieDLL, "SetFuncGenDCOffset"        );
    GetFuncGenDCOffset        = (GETFUNCGENDCOFFSET)        GetProcAddress( TiePieDLL, "GetFuncGenDCOffset"        );
    SetFuncGenSymmetry        = (SETFUNCGENSYMMETRY)        GetProcAddress( TiePieDLL, "SetFuncGenSymmetry"        );
    GetFuncGenSymmetry        = (GETFUNCGENSYMMETRY)        GetProcAddress( TiePieDLL, "GetFuncGenSymmetry"        );
    SetFuncGenFrequency       = (SETFUNCGENFREQUENCY)       GetProcAddress( TiePieDLL, "SetFuncGenFrequency"       );
    GetFuncGenFrequency       = (GETFUNCGENFREQUENCY)       GetProcAddress( TiePieDLL, "GetFuncGenFrequency"       );
    FillFuncGenMemory         = (FILLFUNCGENMEMORY)         GetProcAddress( TiePieDLL, "FillFuncGenMemory"         );
    SetFuncGenOutputOn        = (SETFUNCGENOUTPUTON)        GetProcAddress( TiePieDLL, "SetFuncGenOutputOn"        );
    GetFuncGenOutputOn        = (GETFUNCGENOUTPUTON)        GetProcAddress( TiePieDLL, "GetFuncGenOutputOn"        );
    FuncGenBurst              = (FUNCGENBURST)              GetProcAddress( TiePieDLL, "FuncGenBurst"              );
    SetFuncGenTrigSource      = (SETFUNCGENTRIGSOURCE)      GetProcAddress( TiePieDLL, "SetFuncGenTrigSource"      );
    GetFuncGenTrigSource      = (GETFUNCGENTRIGSOURCE)      GetProcAddress( TiePieDLL, "GetFuncGenTrigSource"      );

    SetInstrumentConfig       = (SETINSTRUMENTCONFIG)       GetProcAddress( TiePieDLL, "SetInstrumentConfig"       );

    return( 1 );
  } // if
  else
  {
    return( 0 );
  } // else
}; // OpenDLL




bool _export CloseDLL( void )
{
 /*
  * Check if a dll was opened. If so, close it and open the new one
  */
  if (TiePieDLL != NULL)
  {
    FreeLibrary( TiePieDLL );
    TiePieDLL = NULL;
    return( 1 );
  } // if
  else
  {
    return( 0 );
  } // else
} // CloseDLL


