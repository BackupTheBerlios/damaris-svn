#include "core/stopwatch.h"
#include <unistd.h>
#include <cstdio>

int main() {
  stopwatch s;

  for (size_t i=0;i<10;++i) {
    s.start();
    usleep(100000);
    s.stop();
    fprintf(stdout,"time elapsed: %gs\n",s.elapsed());
  }
  return 0;
}
