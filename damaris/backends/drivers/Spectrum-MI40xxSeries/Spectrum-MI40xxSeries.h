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


// ----- main task -----
class SpectrumMI40xxSeries: public ADC {
  int deviceno;

  double samplefreq;
  size_t sampleno;
  DataManagementNode* data_structure;
  double timeout;
  size_t fifobufferno;
  size_t fifobufferlen;
  size_t fifo_minimal_size;
  int device_id;
  ttlout trigger_line;
  std::vector<short int*> fifobuffers;

  // configured input impedance
  double impedance;


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
