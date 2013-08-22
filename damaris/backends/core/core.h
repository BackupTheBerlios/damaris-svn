/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#ifndef CORE_H
#define CORE_H

#include "core/job.h"
#include "core/result.h"
#include "core/job_receiver.h"
#include "core/core_exception.h"
#include "core/core_config.h"
#include "machines/hardware.h"
#include <string>
#include <time.h>

/**
\mainpage

\section sec_introduction Introduction to DAMARIS backends

DAMARIS stands for DArmstadt MAgnetic Resonance Instrument Software. There is a homepage avaialbe at http://www.fkp.physik.tu-darmstadt.de/damaris/

The project started as study for a common software base for different spectrometer hardware setups.

The first time of this project was determined by
- the need of software for SpinCore and TiePie Hardware,
- and the wish for a "better" and unified software for our spectrometers.

Next step is a functional raw spectrometer control for the mobile NMR spectrometer. Main contributors are Achim and Holger.

\section sec_guide_doc Guide to the documentation
- For the user interface programers the modules about \link job jobs\endlink, \link result results\endlink
  and their \link xmlstateinterface xml representation \endlink are useful.
- For the spectrometer maintainer the hardware interface and drivers section is recommended. Each spectrometer ends up
  in \link machines machine specification\endlink. This program contains the <b>main</b> procedure.
- The system programer and the people who would like to see how everything is linked together, should visit the core class
  implementation and the \link design design considerations\endlink.

\section sec_source_code Source Code
The source code of this project is available from CVS at svn://element.fkp.physik.tu-darmstadt.de/share/SoftwareDevelopment/svnrepository/damaris and also accessible via a web frontend http://element.fkp.physik.tu-darmstadt.de/cgi-bin/viewcvs.cgi/damaris/
*/

/**
\page design General Design Considerations

The central ability of a spectormeter driver is to preform a single shot experiment. The selection of the
pulse sequence, the repitition and also the graphical monitoring is beyond the concept of a driver.
So the frontend-backend approach should be suitable. It is base of the picture:

\image html software_design_overview_scaled.png

To the left in grey some spectormeters are depicted. Each machine-driver can control one. This machine-driver shown to
the left in blue is called core. It recieves jobs in form of a file, processes the necessary informations and performs
an experiment. After receiving the data from an analog digital converter and waiting for the pulse driver to quit, the
next experiment is taken from a queue.

On the right side some possible sources for these jobs are listed. This is the users side. In contrast to other
universal spectrometer control software, it is not intended to have only one program, that does everything.
One can identify different tasks in NMR programs:

- The hardware drivers perform the experiment.
- The monitor functions provide a direct view on the data, to check, if the instrument is running properly.
- The data reduction unit processes data to get the essential measurement data.
- The evaluation routines derive the relevant physical quantity
- The measurement schedule tells the spectrometer, what to do next

The programs, except the hardware driver, control the experiment without dealing with the details of spectrometer setup.
\todo Gating and dead time should be set by driver. Sofar the experiment file must contain these informations.

The users side should be dominated by the users focus, monitoring single shots or scheduling long measurment sequences.

*/


/** works on jobs and gains results or errors */
class core {

 public:
  /** 1: if a fatal error occurs
     or just a quit command
     0: go on
  */
  int quit_mainloop;

  /**
   */
  static volatile int triggered_restart;

  /**
   */
  static volatile int quit_signal;

  /**
   */
  static volatile int term_signal;

  /**
     the path configuration....
   */
  core_config the_configuration;

  /**
     the time of instantiation
   */
  time_t start_time;

  /**
    counts the recieved jobs
    counting starts from 0
  */
  size_t job_counter;

  /**
     keeps the parser on standby
   */
  job_receiver* job_parser;

  /**
     polls for the job files
   */
  job* wait_for_input();

  /**
     do the experiment or some other things....
   */
  result* do_the_job(job& the_job);

  /**
     tell the result by writing to another file
     \exception core_exception if job no does not fit to result no
   */
  int write_to_output(const job& job_done, const result& result_gained);

  /**
   */
  hardware* the_hardware;

 public:
  /**
     setup the hardware and control structure for experiments
     \param configuration an initial configuration, e.g. derived by parameter parsing
  */
  core(const core_config& configuration);

  /**
    writes the actual configuration to file when mainloop is started
  */
  virtual int write_state() const;

  /**
   */
  virtual int remove_state() const;

  /**
     execute incomming instructions
   */
  virtual int run();

  /**
     return the cores name
    */
  virtual const std::string& core_name() const=0;

  /**
     switch off hardware, end core
   */
  virtual ~core();

  char* _cwd;

};

#endif
