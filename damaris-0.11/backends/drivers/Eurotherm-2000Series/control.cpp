#include "Eurotherm-2000Series.h"
#include <cstdlib>
#include <cstdio>

int main(int argc, char** argv) {

  try {
    Eurotherm2000Series e("/dev/ttyS0", 2);
    double upper;
    double lower;
    e.get_setpoint_limits(lower,upper);
    fprintf(stderr, "setpoint limits %f %f\n",lower,upper);
    
    while (1) {
      char buffer[100];
      char* nextline=fgets(buffer,sizeof(buffer),stdin);
      if (nextline==NULL) {
	break;
      }
      if (nextline[0]==0) {
	break;
      }
      else if (nextline[0]==10 || nextline[0]==12) {
	// print the temperature and setpoint
	double t=e.get_temperature();
	double s=e.get_setpoint();
	printf("%f %f\n", t,s); 
      }
      else if (nextline[0]=='h') {
	//print history
	temp_history* h=e.get_history(0);
	if (h==NULL) break;
	for (temp_history::const_reverse_iterator i=h->rbegin();
	     i!=((temp_history::const_reverse_iterator)h->rend());
	     ++i)
	  printf("%f\n",*i);
	delete h;
	printf("\n");
      }
      else if (nextline[0]=='a') {
	// autotune
	e.set_value("AT","0000.");
	e.set_value("mA","0000.");
      }
      else if (nextline[0]=='r') {
	// reset
	e.reset();
      }
      else {
	// get number and set setpoint
	char* lineend=NULL;
	double s=strtod(nextline,&lineend);
	if (nextline!=lineend) {
	  fprintf (stderr,"new setpoint %f...",s);
	  fflush(stderr);
	  e.set_setpoint(s);
	  fprintf (stderr,"...done\n");
	}
      }

    }

    
  }
  catch (Eurotherm2000Series_error e) {
    fprintf(stderr,"%s\n",e.c_str());
    return 1;
  }
  return 0;

}
