/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#include <unistd.h>
#include <cstdio>
#include <signal.h>
#include "core/core_config.h"
#include "core/core_exception.h"

int main(int argc, const char** argv) {

  if (argc<2 || argc>3) {
    fprintf(stderr,"%s [--restart|--quit|--abort] config file\n",argv[0]);
    return 1;
  }
  /* find a config file */
  if (access(argv[argc-1],R_OK)!=0) {
    fprintf(stderr,"%s could not read file %s\n",argv[0],argv[argc-1]);
    return 1;
  }

  pid_t the_pid;
  try {
    /* parse file */
    the_pid=core_state(std::string(argv[argc-1])).pid;
  }
  catch(core_exception e) {
    fprintf(stderr,e.what());
    return 1;
  }

  int signal_to_send=SIGUSR1;

  if (argc==3) {
    if (strcmp(argv[1],"--restart")==0)
      signal_to_send=SIGUSR1;
    else if (strcmp(argv[1],"--quit")==0)
      signal_to_send=SIGQUIT;
    else if (strcmp(argv[1],"--abort")==0)
      signal_to_send=SIGTERM;
    else {
      fprintf(stderr, "%s allowed arguments are --restart, --quit and --abort\n",argv[0]);
      return 1;
    }
      
  }
  /* send signal */
  if (0!=kill(the_pid, signal_to_send)) {
    fprintf(stderr, "could not send restart signal to process %d\n",the_pid);
    return  1;
  }
  return 0;
}
