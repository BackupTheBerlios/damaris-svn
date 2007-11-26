#include <cstdio>
#include <string>
#include <vector>
#include <cmath>
#include "SpinCore-PulseBlaster.h"

class SpinCorePulseBlaster24Bit: public SpinCorePulseBlaster {
public:
  SpinCorePulseBlaster24Bit (double the_clock=100e6): SpinCorePulseBlaster(10,the_clock) {
  }
  
  void append_command(std::string& data, int flags, opcode inst, int inst_data, double length) {
    unsigned char command[command_length];

    if (inst>8)
      throw SpinCorePulseBlaster_error("instruction code not known");

    double tmpdelay=(clock*length-3.0);
    if (tmpdelay<(shortest_pulse-3))
      throw SpinCorePulseBlaster_error("pulse is shorter than allowed");
    if (tmpdelay>(pow(2,32)-1))
      throw SpinCorePulseBlaster_error("pulse is longer than allowed");
    unsigned int delay=(unsigned int)tmpdelay;

    // Output, Control Word 1st Byte
    command[0]=(flags&0xff0000)>>16;
    // Output, Control Word 2nd Byte
    command[1]=(flags&0xff00)>>8;
    // Output, Control Word 3rd Byte
    command[2]=flags&0xff;
    // Data Field 1st Byte
    command[3]=(inst_data&0x0ff000)>>12;
    // Data Field 2nd Byte
    command[4]=(inst_data&0xff0)>>4;
    // Data Field 3rd Byte and opcode
    command[5]=(inst_data&0xf)<<4|(inst&0xf);
    // Delay Count 1st Byte
    command[6]=(delay&0xff000000)>>24;
    // Delay Count 2nd Byte
    command[7]=(delay&0xff0000)>>16;
    // Delay Count 3rd Byte
    command[8]=(delay&0xff00)>>8;
    // Delay Count 4th Byte
    command[9]=(delay&0xff);

    data.append((char*)command,command_length);
  }

};


int main() {

  try {
    SpinCorePulseBlaster24Bit p;
    std::string program;
    p.append_command(program,1,SpinCorePulseBlaster::LOOP,5,2e-6);
    p.append_command(program,3,SpinCorePulseBlaster::END_LOOP,0,1e-6);
    p.append_command(program,4,SpinCorePulseBlaster::CONTINUE,0,1e-6);
    p.append_command(program,0,SpinCorePulseBlaster::CONTINUE,0,1e0);
    p.append_command(program,1,SpinCorePulseBlaster::STOP,0,1e-6);
    p.set_program(program);
    p.set_initialized();
    p.start();
  }
  catch(SpinCorePulseBlaster_error e) {
    fprintf(stderr, "%s\n", e.c_str());
  }

  return 0;
}
