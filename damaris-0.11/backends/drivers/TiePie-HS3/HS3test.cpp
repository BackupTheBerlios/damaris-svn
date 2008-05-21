#include <stdio.h>
#include "tiepie.h"
#if 0 
# include "tiepie.cpp"
#endif

#include "TiePie-HS3.h"

int test1() {
  word retval;
  dword serialno;
  if (1!=OpenDLL(dtHandyscope3)) {
    printf("failed loading dlls\n");
    return 1;
  }

  retval=InitInstrument(0);
  if (retval!=NO_ERROR) {
    printf("failed to initialize the instrument, code=%d\n", retval);
    CloseDLL();
    return 1;
  }

  retval=GetSerialNumber(&serialno);
  if (retval!=NO_ERROR) {
    printf("could not read serial number, code=%d\n", retval);
    ExitInstrument();
    CloseDLL();
    return 1;
  }

  printf("serial no %d\n", serialno);

  ExitInstrument();
  CloseDLL();
  return 0;
}


int test2() {

  try {
    TiePieHS3 adc;
    dword serialno;
    word retval=GetSerialNumber(&serialno);
    if (retval!=NO_ERROR) throw ADC_exception("could not get serialno");
    printf("serial number %d\n", serialno);
  }
  catch (ADC_exception e) {
    printf("ADC exception: %s\n", e.c_str());
    return 1;
  }
  return 0;
}

int main() {
  return test2();
}
