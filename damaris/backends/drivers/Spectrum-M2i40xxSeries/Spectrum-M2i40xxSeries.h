// todo: Multi-Platform support is not implemented.
// todo: fifo if needed


#ifndef SPECTRUM_M2i40XXSERIES
#define SPECTRUM_M2i40XXSERIES

#define SPCM_MAX_AIRANGE    8
#define SPCM_MAX_AICHANNEL  16

#include <string>
#include <vector>
#include <math.h>
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
# include "include/spcm_drv.h"
#endif

#if !(defined __linux__)
# error "sorry, expecting linux"
#endif

#ifndef SPC_DEBUG
# define SPC_DEBUG 1
#endif

/**
  \defgroup spectrum2i40xxseries Spectrum M2i40xx ADC
  \ingroup drivers
  \brief The driver for the Spectrum M2i.40xx series analog to digital converter.

  The M2i.4021/4022 we use supports sampling with up to 20 MS/s on 2/4 channels. This driver requires the GatedSampling option to be installed.
  @{
*/

class SpectrumM2i40xxSeries_error: public ADC_exception {
 	public:
		SpectrumM2i40xxSeries_error(const std::string& message): ADC_exception(message) {}
};


/**
   \brief The driver class
 */
class SpectrumM2i40xxSeries: public ADC {

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
    /** the extra data thread should store things to that buffer */
    short int* fifo_adc_data;
    pthread_attr_t timeout_pt_attrs;
    pthread_t timeout_pt;


# if defined __linux__
  // ----- include the easy ioctl commands from the driver -----
#  include "include/spcioctl.inc"
# endif

    /**
     \brief Stores card settings and information
     */
    class Configuration {
    public:
		/** sampling frequency in Hz */
		double samplefreq;
		/** Data returned by the ADC during the different gating periods is stored in a nested list */
		DataManagementNode* data_structure;
		/** a timeout for acquiring data in s*/
		double timeout;
		/** configured input impedance for each channel in Ohm*/
		double* impedance;
		/** configured input sensitivity for each channel in V*/
		double* sensitivity;
		/** coupling 0=DC else AC */
		int coupling;
		/** external reference clock **/
		int ext_reference_clock;
		/** offsets for each channel in % of sensitivity **/
		int* offset;

		// Information about the card copied from the Spectrum example program

		/** \brief The card's driver handle as returned by the Spectrum driver open function */
		drv_handle hDrv;
		/** \brief The card's ID
		 * Typically 0 if no other card is installed. The device is created as /dev/spcm0
		 */
		int32 lCardID;

		/** \brief The card's type as listed in the manual */
		int32 lCardType;
		/** \brief Serial Number */
		int32 lSerialNumber;
		/** \brief Installed on-board memory in bytes */
		int64 llInstMemBytes;
		/** \brief Bitmap with installed card features */
		int32 lFeatureMap;
		/** \brief Number of channels (analog or digital) */
		int32 lMaxChannels;
		/** \brief Number of bytes for each sample (analog data) */
		int32 lBytesPerSample;
		/** \brief Minimum sampling rate */
		int32 lMinSampleRate;
		/** \brief Maximum sampling rate */
		int32 lMaxSampleRate;
		/** \brief Number of installed modules for data sorting algorithm */
		int32 lModulesCount;

		/** \brief Library version */
		int32 lLibVersion;
		/** \brief Kernel version */
		int32 lKernelVersion;

		/** \brief Main control firmware version */
		int32 lCtrlFwVersion;
		/** \brief Base hardware version */
		int32 lBaseHwVersion;
		/** \brief Module hardware version */
		int32 lModHwVersion;
		/** \brief Module firmware version */
		int32 lModFwVersion;


		// current settings
		/** \brief Bitmap for currently enabled channels */
		channel_array qwSetChEnableMap;
		/** \brief Programmed memory size */
		int llSetMemsize;
		/** \brief Number of used channels for this run */
		int lSetChannels;
		/** \brief Currently active oversampling factor */
		int32 lOversampling;


		// Analog Input settings
		/** \brief Resolution of analog input channel */
		int32   lResolution;
		int32   lMaxADCValue;                   // maximum range, normally 2^(Resolution-1) but can be limited
		int32   lPathCount;                     // number of input paths
		int32   lRangeCount;                    // number of analog input ranges
		int32   lRangeMin[SPCM_MAX_AIRANGE];    // analog input ranges
		int32   lRangeMax[SPCM_MAX_AIRANGE];    // ...
		bool    bInputTermAvailable;            // input termination available
		bool    bDiffModeAvailable;             // differential mode available
		bool    bACCouplingAvailable;           // AC/DC coupling softwar selectable
		bool    bBWLimitAvailable;              // bandwidth limit available
		bool    bOffsPercentMode;               // offset programmed in percent of range



		/** print data for debug purpose  */
		void print(FILE* f);

		bool bOpenCard();
		char* PrintInfo();

		Configuration() {
			samplefreq=0;
			timeout=0;
			impedance = NULL;
			sensitivity = NULL;
			offset = NULL;
			coupling=0;
			ext_reference_clock=0;
			data_structure=NULL;
			hDrv = NULL;

			lCardID = 0;
			lSetChannels = 0;
			lBytesPerSample = 0;
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
			if (orig.data_structure==NULL)
				data_structure=NULL;
			else
				data_structure=new DataManagementNode(*orig.data_structure);
		}
      
		~Configuration() {
			if (data_structure!=NULL)
				delete data_structure;
		}
    };

	/**
	 \brief Default settings for the ADC card.
	 These settings are copied and modified at need to derive effective settings
	*/
	Configuration default_settings;

	/**
	 \brief The settings actually used for a measurement.
	 Once a measurement is started, user settings may override default settings
	*/
	Configuration* effective_settings;

	short int* split_adcdata_recursive(short int* data, const DataManagementNode& structure, adc_results& result_splitted);

	/**
	 \brief Collects the settings required for the current job.
	 */
	void collect_config_recursive(state_sequent& exp, SpectrumM2i40xxSeries::Configuration& settings);

public:

	SpectrumM2i40xxSeries(const ttlout& t_line, int ext_reference_clock=(int)100e6);

	virtual void sample_after_external_trigger(double rate, size_t samples, double sensitivity=5.0, size_t resolution=14);

	virtual void set_daq(state & exp);


	/**
	 for syncronization purposes with sample clock
	*/
	//double get_sample_clock_frequency() const;

	/**
	 get results from transient recorder
	 timeout: 0.0 return immediately
	*/
	virtual result* get_samples(double timeout=0.0);

	/**
	 timeout issues during fifo acquisition
	*/
	//int TimeoutThread();

	virtual ~SpectrumM2i40xxSeries();
};

/** @{
 */

#endif
