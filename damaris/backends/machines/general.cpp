/***************************************************************************

 Author: Markus Rosenstihl
 Created: June 2010

****************************************************************************/
#include "machines/hardware.h"
#include "core/core.h"
#include "drivers/PTS-Synthesizer/PTS.h"
#include "drivers/Spectrum-MI40xxSeries/Spectrum-MI40xxSeries.h"
#include "drivers/SpinCore-PulseBlaster24Bit/SpinCore-PulseBlaster24Bit.h"
#include <glib.h>



/**
   \defgroup General NMR Spectrometer
   \ingroup machines
   Uses:
     \li Spincore Pulseblaster 24 Bit
     \li Spectrum MI4021 with gated sampling option (PB ref. clock is fed to Ext.Clock)
     \li Programmed Test Sources PTS frequency synthesizer with phase control
   This backend uses a configuration file for setting the various lines and configurations.

   \par Starting the hardware
   This procedure should assure the correct initialisation of the hardware:
   \li Switch off main switches of SpinCore Pulseblaster and Computer (the main switch of the computer is at the rear)
   \li Switch on Computer and start Windows or Linux

   @{
*/



class general_hardware: public hardware {

  PTS* my_pts;
  SpinCorePulseBlaster24Bit* my_pulseblaster;
  SpectrumMI40xxSeries* my_adc;

public:
  general_hardware(){
      int SYSCONF=0;
      int USRCONF=0;
      GKeyFile* cfg_file;
      GError *error=NULL;
      char cfg_name[512];
      const gchar* usr_configdir;
      const gchar* const*  sys_configdirs;
      sys_configdirs = g_get_system_config_dirs(); 
      usr_configdir = g_get_user_config_dir();
      cfg_file = g_key_file_new();
      for (int i=0; i<100; i++) {
        if (sys_configdirs[i]==NULL) break;
        else {
          snprintf(cfg_name, 512,"%s/damaris/backend.conf", sys_configdirs[i]);
          fprintf(stdout, "reading backend configuration file (system: %i): %s...", i,cfg_name);
          if (!g_key_file_load_from_file (cfg_file, cfg_name, G_KEY_FILE_NONE, &error)){
            if (error->code != 4) {
              fprintf(stdout, "found!\n");
              g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, error->message);
            }
            else
              printf("not found!\n");
          }
          else
            SYSCONF=1;
          error=NULL; // reset error
        }
        
      }
      snprintf(cfg_name, 512,"%s/damaris/backend.conf", usr_configdir);
      fprintf(stdout, "reading backend configuration file (user): %s...", cfg_name);
      if (!g_key_file_load_from_file (cfg_file, cfg_name, G_KEY_FILE_NONE, &error)){
        if (error->code != 4)
          g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, error->message);
        else
          printf("not found!\n");
        error = NULL;
      }
      else
        USRCONF=1;
      if ( !(SYSCONF|USRCONF))
        throw(core_exception("configuration failed!\n"));
      printf("done!\n");
      /* configure ADC card */
      ttlout trigger;
      trigger.id 		= g_key_file_get_integer(cfg_file, "ADC","id", &error);
      if (error) g_error(error->message);
      error = NULL;
      //g_error(error->message);
      trigger.ttls 		= 1<<g_key_file_get_integer(cfg_file, "ADC", "trigger_line", &error); 
      if (error) g_error(error->message);
      error = NULL;
      int ext_reference_clock 	= (int) g_key_file_get_double(cfg_file, "ADC", "refclock", &error); // 50 MHz from PB24 SP17; defaults to 100MHz (PB24 SP 2)
      if (error) g_error(error->message);
      printf("reclock %i", ext_reference_clock);
      error = NULL;
      double impedance 		= g_key_file_get_double(cfg_file, "ADC", "impedance", &error); // Ohm ( or 50 Ohm)
      if (error) g_error(error->message);

      error = NULL;
      my_adc=new SpectrumMI40xxSeries(trigger, impedance, ext_reference_clock);
      
      /* configure PulseBlaster */
      int pb_id = g_key_file_get_integer(cfg_file, "PB", "id", &error);
      if (error) g_error(error->message);
      error = NULL;
      double pb_refclock = g_key_file_get_double(cfg_file, "PB", "refclock", &error);
      if (error) g_error(error->message);
      error = NULL;
      int pb_sync = 1<<g_key_file_get_integer(cfg_file, "PB", "sync_line", &error);
      if (error) g_error(error->message);
      error = NULL;

      my_pulseblaster=new SpinCorePulseBlaster24Bit(pb_id, pb_refclock, pb_sync);

      /* configure PTS */
      int pts_id = g_key_file_get_integer(cfg_file, "PTS", "id", &error);
      if (error) g_error(error->message);
      error = NULL;
      my_pts=new PTS_latched(pts_id);
      // PTS 500 has 0.36 ;  PTS 310 has 0.225 degrees/step
      my_pts->phase_step=g_key_file_get_double(cfg_file, "PTS", "phase_stepsize", &error);
      if (error) g_error(error->message);
      error = NULL;

      // publish devices
      the_pg=my_pulseblaster;
      the_adc=my_adc;
      the_fg=my_pts;
  }

  result* experiment(const state& exp) {
    result* r=NULL;
    for(size_t tries=0; r==NULL && core::term_signal==0 &&  tries<102; ++tries) {
      state* work_copy=exp.copy_flat();
      if (work_copy==NULL) return new error_result(1,"could create work copy of experiment sequence");
      try {
	if (the_fg!=NULL)
	  the_fg->set_frequency(*work_copy);
	if (the_adc!=NULL)
	  the_adc->set_daq(*work_copy);
	// the pulse generator is necessary
	my_pulseblaster->run_pulse_program_w_sync(*work_copy, my_adc->get_sample_clock_frequency());
	// wait for pulse generator
	the_pg->wait_till_end();
	// after that, the result must be available
	if (the_adc!=NULL)
	  r=the_adc->get_samples();
	else
	  r=new adc_result(1,0,NULL);
      }
      catch (frequ_exception e) {
	r=new error_result(1,"frequ_exception: "+e);
      }
      catch (ADC_exception e) {
	if (e!="ran into timeout!" || tries>=100)
	  r=new error_result(1,"ADC_exception: "+e);
      }
      catch (pulse_exception p) {
	r=new error_result(1,"pulse_exception: "+p);
      }
      delete work_copy;
      if (core::quit_signal!=0) break;
    }
    return r;
  }

  virtual ~general_hardware() {
    if (the_adc!=NULL) delete the_adc;
    if (the_fg!=NULL) delete the_fg;
    if (the_pg!=NULL) delete the_pg;
  }

};

/**
   \brief brings standard core together with the general NMR hardware
*/
class general_core: public core {
  std::string the_name;
public:
    general_core(const core_config& conf): core(conf) {
	the_hardware=new general_hardware();
	the_name="berta core";
  }
  virtual const std::string& core_name() const {
  	return the_name;
  }
};

/**
   @}
 */

int main(int argc, const char** argv) {
  int return_result=0;
  try {
      core_config my_conf(argv, argc);
      // setup input and output
      general_core my_core(my_conf);
      // start core application
      my_core.run();
  }
  catch(ADC_exception ae) {
    fprintf(stderr,"adc: %s\n",ae.c_str());
    return_result=1;
  }
  catch(core_exception ce) {
    fprintf(stderr,"core: %s\n",ce.c_str());
    return_result=1;
  }
  catch(pulse_exception pe) {
    fprintf(stderr,"pulse: %s\n",pe.c_str());
    return_result=1;
  }
  return return_result;
}
