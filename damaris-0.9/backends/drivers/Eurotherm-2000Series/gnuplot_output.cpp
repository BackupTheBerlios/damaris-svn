#include "Eurotherm-2000Series.h"
#include <cstdio>

int main() {
  try {
    Eurotherm2000Series e("/dev/ttyS1",2);
    while (1) {
      if (e.device_failure()) return 1;
      sleep(2);
      temp_history* h=e.get_history(0);
      printf("plot '-' w l\n");
      for (temp_history::const_reverse_iterator i=h->rbegin(); i!=((temp_history::const_reverse_iterator)h->rend()); ++i)
	printf("%f\n",*i);
      delete h;
      printf("e\n");
      fflush(stdout);
    }
  }
  catch (Eurotherm2000Series_error e) {
    fprintf(stderr, "error: %s",e.c_str());
    return 1;
  }
  return 0;
}
