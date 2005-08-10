#ifndef SPECTRUM_MI40XXSERIES
#define SPECTRUM_MI40XXSERIES

#include <string>
#include <vector>
#include "core/states.h"
#include "drivers/ADC.h"
#include "GatedData.h"

#if defined __linux__
// ----- linux includes -----
# include <cstdio>
# include <fcntl.h>
# include <sys/ioctl.h>
# include <unistd.h>
# include "include/dlltyp.h"
# include "include/regs.h"
# include "include/spcerr.h"
#endif
#if defined __CYGWIN__
// ------- for cygwin with windows ----------
# define int16 short int
# define int32 int
# define ptr16 short int*
# include "include/regs.h"
# include "include/spcerr.h"
# include "windows.h"
#endif

#if !(defined __linux__ || defined __CYGWIN__)
# error "sorry, expecting linux or cygwin"
#endif

class SpectrumMI40xxSeries_error: public ADC_exception {
 public:
  SpectrumMI40xxSeries_error(const std::string& message): ADC_exception(message) {}
};


/**
   driver for Spectrum MI40 Series acquisition boards
 */
class SpectrumMI40xxSeries: public ADC {
    /**
       analogin device number 
    */
    int deviceno;

    /**
       ttlout triggerline device id
     */
    int device_id;
    /**
       ttlout triggerline ttlmask
     */
    ttlout trigger_line;

    /** stuff concerning fifo acquisition */
    size_t fifobufferno;
    /** stuff concerning fifo acquisition */
    size_t fifobufferlen;
    /** stuff concerning fifo acquisition */
    size_t fifo_minimal_size;
    /** stuff concerning fifo acquisition */
    std::vector<short int*> fifobuffers;

# if defined __linux__
  // ----- include the easy ioctl commands from the driver -----
#  include "include/spcioctl.inc"
# endif

# if defined __CYGWIN__
  HINSTANCE spectrum_driver_dll;
  int16 boardcount;
  int16 PCIVersion;
# define define_spectrum_function(name, returntype, ... ) typedef returntype (__attribute__((stdcall)) *name##_type)(__VA_ARGS__); name##_type name
  define_spectrum_function(SpcSetParam, int16, int16, int32, int32);
  define_spectrum_function(SpcGetParam, int16, int16, int32, int32*);
  define_spectrum_function(SpcGetData, int16, int16, int16, int32, int32, void*);
  define_spectrum_function(SpcInitPCIBoards, int16, int16*, int16*);
# endif

  class Configuration {
  public:
      /* sampling frequency */
      double samplefreq;
      /* acquired data description */
      DataManagementNode* data_structure;
      /* a timeout for acquiring data */
      double timeout;
      /** configured input impedance */
      double impedance;
      /** configured input sensitivity */
      double sensitivity;
  };

  Configuration* effective_settings;

  short int* split_adcdata_recursive(short int* data, const DataManagementNode& structure, adc_results& result_splitted);

  void collect_config_recursive(state_sequent& exp, SpectrumMI40xxSeries::Configuration& settings);

public:
  SpectrumMI40xxSeries(const ttlout& t_line);

  virtual void sample_after_external_trigger(double rate, size_t samples, double sensitivity=5.0, size_t resolution=14);

  virtual void set_daq(state & exp);


  virtual result* get_samples(double timeout=0.0);

  /**
     timeout issues during fifo acquisition
   */
  int TimeoutThread();

  virtual ~SpectrumMI40xxSeries();
};

#endif
