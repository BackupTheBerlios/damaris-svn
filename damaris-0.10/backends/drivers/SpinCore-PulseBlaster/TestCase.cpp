#include <SpinCore-PulseBlaster.h>
#include <unistd.h>

class PB_test: SpinCorePulseBlasterLowlevel {

public:
  PB_test(): SpinCorePulseBlasterLowlevel() {}

  int testprog(int n=1) {

    write_register(0x00,0x00);
    write_register(0x02,0x0a);
    write_register(0x03,0x00);
    write_register(0x04,0x00);
    // do nothing in first place (no loops in first place!!!!)
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x06);
    // pulse 0.6 us
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x07);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x39);
    // wait 7.7 us
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,n&0xff);
    write_register(0x06,0xff); // change 0xff to 0xfe
    // trigger
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x07);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x50);
    write_register(0x06,0x0c);
    // clear flags
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x06);
    // stop
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x01);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x06);
    // end programming
    write_register(0x07,0x00);
    // run
    write_register(0x01,0x00);

    sleep(1);

    return 0;
  }

  int testinterval(unsigned long int tics) {
    write_register(0x00,0x00);
    write_register(0x02,0x0a);
    write_register(0x03,0x00);
    write_register(0x04,0x00);
    // do nothing in first place (no loops in first place!!!!)
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x06);
    // pulse 0.1 us
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x07);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0xfe);
    // wait tics time units
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    // delay (32 bit)
    write_register(0x06,(tics>>24)&0xff);
    write_register(0x06,(tics>>16)&0xff);
    write_register(0x06,(tics>>8)&0xff);
    write_register(0x06,tics&0xff);
    // trigger
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x07);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0xfe);
    // clear flags
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x06);
    // stop
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x01);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x00);
    write_register(0x06,0x06);
    // end programming
    write_register(0x07,0x00);
    // run
    write_register(0x01,0x00);

    sleep(1);

    return 0;
    
  }

  ~PB_test(){}
};


int main(int argc, char* argv[]) {
  int n=9;
  if (argc>1 && argv[1]!=0) {
    n=strtoul(argv[1],NULL,0);
  }
  printf("%d\n",n);
  return PB_test().testinterval(n);
}
