#ifndef SPECTRUM_MI40XXSERIES
#define SPECTRUM_MI40XXSERIES

#include <string>
#include <vector>
#include "core/states.h"
#include "core/constants.h"
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

#ifndef SPC_DEBUG
# define SPC_DEBUG 0
#endif


/**
   \defgroup spectrummi40xxadc Spectrum MI-40xx ADC
   \ingroup drivers
   \brief Driver for the Spectrum MI-40xx Series analog-to-digital converter (acquisition) boards

   @{
 */

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

    /**
        Type of the card. This is important for the number of allowed channels, etc.
    */
    int cardType;

    /** stuff concerning fifo acquisition */
    size_t fifobufferno;
    /** stuff concerning fifo acquisition */
    size_t fifobufferlen;
    /** stuff concerning fifo acquisition */
    size_t fifo_minimal_size;
    /** stuff concerning fifo acquisition */
    std::vector<short int*> fifobuffers;
    /** the extra data thread should store things to that buffer */
    short int* fifo_adc_data;
    pthread_attr_t timeout_pt_attrs;
    pthread_t timeout_pt;


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
        /* sampling frequency in Hz */
        double samplefreq;
        /* acquired data description */
        DataManagementNode* data_structure;
        /* a timeout for acquiring data in s*/
        double timeout;
        /** configured input impedance in Ohm*/
        double* impedance;
        /** configured input sensitivity in V*/
        double* sensitivity;
        /** coupling 0=DC else AC */
        int coupling;
        /** external reference clock **/
        int ext_reference_clock;

        /** offsets for each channel in % of sensitivity **/
        int* offset;

        /** \brief Number of used channels for this run */
        int lSetChannels;
        /** \brief Bitmap for currently enabled channels */
        channel_array qwSetChEnableMap;

        /** print data for debug purpose  */
        void print(FILE* f);



        Configuration() {
            samplefreq=0;
            timeout=0;
            impedance=NULL;
            sensitivity=NULL;
            offset = NULL;
            coupling=0;
            ext_reference_clock=0;
            data_structure=NULL;
            lSetChannels=0;
            qwSetChEnableMap=0;
        }

        Configuration(const Configuration& orig) {
            samplefreq=orig.samplefreq;
            timeout=orig.timeout;
            impedance=orig.impedance;
            sensitivity=orig.sensitivity;
            coupling=orig.coupling;
            ext_reference_clock=orig.ext_reference_clock;
            qwSetChEnableMap = orig.qwSetChEnableMap;
            lSetChannels = orig.lSetChannels;
            if (orig.data_structure==NULL) data_structure=NULL;
            else data_structure=new DataManagementNode(*orig.data_structure);
        }

        ~Configuration() {
            if (data_structure!=NULL) delete data_structure;
        }
    };

    /**
       these settings are copied and modified at need to derive effective settings
     */
    Configuration default_settings;

    /**
       only available with started measurement
     */
    Configuration* effective_settings;

    short int* split_adcdata_recursive(short int* data, const DataManagementNode& structure, adc_results& result_splitted);

    void collect_config_recursive(state_sequent& exp, SpectrumMI40xxSeries::Configuration& settings);

    bool IsChannelMaskLegal(int mask);

public:
    SpectrumMI40xxSeries(const ttlout& t_line, float impedance=1e6, int ext_reference_clock=(int)100e6);

    virtual void sample_after_external_trigger(double rate, size_t samples, double sensitivity=5.0, size_t resolution=14);

    virtual void set_daq(state & exp);


    /**
       for syncronization purposes with sample clock
     */
    double get_sample_clock_frequency() const;

    /**
       get results from transient recorder
       timeout: 0.0 return immediately
    */
    virtual result* get_samples(double timeout=0.0);

    /**
       timeout issues during fifo acquisition
     */
    int TimeoutThread();

    virtual ~SpectrumMI40xxSeries();
};

/**
   @}
*/

#endif
