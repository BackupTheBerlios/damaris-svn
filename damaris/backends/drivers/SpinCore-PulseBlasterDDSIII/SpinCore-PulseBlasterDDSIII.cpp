/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#include "SpinCore-PulseBlasterDDSIII.h"
#include <cmath>
#include <typeinfo>
#include <iterator>
#include "core/states.h"

/*********************************************************************

                    PulseBlasterDDSIIICommand

*********************************************************************/

PulseBlasterDDSIIICommand::PulseBlasterDDSIIICommand() {
  instruction=SpinCorePulseBlaster::CONTINUE;
  length=0;
  rx_phase_reg=0;
  rx_enable=SpinCorePulseBlasterDDSIII::ANALOG_OFF;
  tx_phase_reg=0;
  tx_enable=SpinCorePulseBlasterDDSIII::ANALOG_OFF;
  freq_reg=0;
  ttls=0;
  program=NULL;
}

PulseBlasterDDSIIICommand::PulseBlasterDDSIIICommand(const PulseBlasterDDSIIICommand& orig): PulseBlasterCommand(orig) {
  rx_phase_reg=orig.rx_phase_reg;
  rx_enable=orig.rx_enable;
  tx_phase_reg=orig.tx_phase_reg;
  tx_enable=orig.tx_enable;
  freq_reg=orig.freq_reg;
}


PulseBlasterDDSIIICommand::PulseBlasterDDSIIICommand(PulseBlasterDDSIIIProgram& p,
						     double _frequency,
						     double _rx_phase, SpinCorePulseBlasterDDSIII::analog_state _rx_enable,
						     double _tx_phase, SpinCorePulseBlasterDDSIII::analog_state _tx_enable,
						     int _ttls, double _length)
  :PulseBlasterCommand(p,_ttls,_length) {
  rx_phase_reg=0;
  tx_phase_reg=0;
  freq_reg=0;
  // allocate frequency and phases only if necessary
  if (_rx_enable==SpinCorePulseBlasterDDSIII::ANALOG_ON || _tx_enable==SpinCorePulseBlasterDDSIII::ANALOG_ON) {
    freq_reg=p.get_frequency_regno(_frequency);
    if (_rx_enable==SpinCorePulseBlasterDDSIII::ANALOG_ON)
      rx_phase_reg=p.get_rx_phase_regno(_rx_phase);
    if (_tx_enable==SpinCorePulseBlasterDDSIII::ANALOG_ON)
      tx_phase_reg=p.get_tx_phase_regno(_tx_phase);
  }
  rx_enable=_rx_enable;
  tx_enable=_tx_enable;
}


int PulseBlasterDDSIIICommand::write_to_file(FILE* out, size_t indent) const {

  if (program==NULL) throw pulse_exception("PulseBlasterDDSIII: Command not associated with Program");
  fprintf(out,"%s<instruction frequency=\"%d\" rxphase=\"%d\" rxenable=\"%d\" txphase=\"%d\" txenable=\"%d\" ttls=\"%d\" ",
	  std::string(indent,' ').c_str(),freq_reg,rx_phase_reg,rx_enable,tx_phase_reg,tx_enable,ttls);
  switch(instruction) {
  case SpinCorePulseBlaster::LOOP:
    fprintf(out,"inst=\"LOOP\" instdata=\"%d\"",loop_count);
    break;
  case SpinCorePulseBlaster::LONG_DELAY:
    fprintf(out,"inst=\"LONG_DELAY\" instdata=\"%d\"",loop_count);
    break;
  case SpinCorePulseBlaster::BRANCH:
    fprintf(out,"inst=\"BRANCH\" instdata=\"%d\"",std::distance(program->begin(),jump));
    break;
  case SpinCorePulseBlaster::END_LOOP:
    fprintf(out,"inst=\"END_LOOP\" instdata=\"%d\"",std::distance(program->begin(),jump));
    break;
  case SpinCorePulseBlaster::JSR:
    fprintf(out,"inst=\"JSR\" instdata=\"%d\"",std::distance(program->begin(),jump));
    break;
  case SpinCorePulseBlaster::CONTINUE:
    fprintf(out,"inst=\"CONTINUE\"");
    break;
  case SpinCorePulseBlaster::STOP:
    fprintf(out,"inst=\"STOP\"");
    break;
  case SpinCorePulseBlaster::RTS:
    fprintf(out,"inst=\"RTS\"");
    break;
  case SpinCorePulseBlaster::WAIT:
    fprintf(out,"inst=\"WAIT\"");
    break;
  default:
    fprintf(out,"inst=\"%d\" instdata=\"UNKNOWN\"",instruction);
  }
  fprintf(out," length=\"%g\"/>\n",1.0/program->internal_clock_freq*length);
  return 1;
}

/*********************************************************************

                    PulseBlasterDDSIIIProgram

*********************************************************************/


PulseBlasterDDSIIIProgram::PulseBlasterDDSIIIProgram() {
  freq_accuracy=1e-6;
  phase_accuracy=1e-3;
  internal_clock_freq=100.0e6;
  minimum_interval=9;
}

PulseBlasterDDSIIIProgram::PulseBlasterDDSIIIProgram(const SpinCorePulseBlasterDDSIII& pbddsiii) {
  freq_accuracy=1e-6;
  phase_accuracy=1e-3;
  internal_clock_freq=pbddsiii.clock;
  minimum_interval=pbddsiii.shortest_pulse;
}

PulseBlasterDDSIIIProgram::PulseBlasterDDSIIIProgram(const PulseBlasterDDSIIIProgram& orig): PulseBlasterProgram(orig) {
  freq_accuracy=orig.freq_accuracy;
  phase_accuracy=orig.phase_accuracy;
  frequency_registers=orig.frequency_registers;
  rx_phase_registers=orig.rx_phase_registers;

  clear();
  const_iterator orig_i;
  for (orig_i=orig.begin(); orig_i!=orig.end(); ++orig_i) {
    const PulseBlasterDDSIIICommand* the_command=dynamic_cast<const PulseBlasterDDSIIICommand*>(*orig_i);
    if (the_command==NULL) {
      throw SpinCorePulseBlaster_error("wrong command class or NULL pointer found in program");
    }
    PulseBlasterDDSIIICommand* new_one=new PulseBlasterDDSIIICommand(*the_command);
    new_one->program=this;
    push_back(new_one);
  }
  // set correct references...
  orig_i=orig.begin();
  for(iterator i=begin();i!=end();++i) {
    (**i).program=this;
    if ((**i).instruction==SpinCorePulseBlaster::END_LOOP || (**i).instruction==SpinCorePulseBlaster::BRANCH || (**i).instruction==SpinCorePulseBlaster::JSR) {
      (**i).jump=i;
      advance((**i).jump,distance(orig.begin(),(**orig_i).jump)-distance(orig.begin(),orig_i));
    }
    ++orig_i;
  }
}

int PulseBlasterDDSIIIProgram::write_to_file(FILE* out, size_t indent) const {
  std::string indent_string(indent,' ');
  fprintf(out,"%s<PulseBlasterDDSIIIProgram>\n",indent_string.c_str());

  if (!frequency_registers.empty()) {
    fprintf(out,"%s  <frequencies>",indent_string.c_str());
    for (size_t i=0; i<frequency_registers.size(); i++)
      fprintf(out," %g",frequency_registers[i]);
    fprintf(out," </frequencies>\n");
  }

  if (!rx_phase_registers.empty()) {
    fprintf(out,"%s  <rxphases>",indent_string.c_str());
    for (size_t i=0;i<rx_phase_registers.size();i++)
      fprintf(out," %g",rx_phase_registers[i]);
    fprintf(out," </rxphases>\n");
  }

  if (!tx_phase_registers.empty()) {
    fprintf(out,"%s  <txphases>",indent_string.c_str());
    for (size_t i=0;i<tx_phase_registers.size();i++)
      fprintf(out," %g",tx_phase_registers[i]);
    fprintf(out," </txphases>\n");
  }

  for(const_iterator i=begin();i!=end();++i) {
    (**i).write_to_file(out,indent+2);
  }
  fprintf(out,"%s</PulseBlasterDDSIIIProgram>\n",indent_string.c_str());
  return 1;
}

int PulseBlasterDDSIIIProgram::get_frequency_regno(double f) {
  size_t i=0;
  while (i<frequency_registers.size() && fabs(frequency_registers[i]-f)>freq_accuracy) ++i;
  if (i==frequency_registers.size())
    frequency_registers.push_back(f);
  return i;
}

int PulseBlasterDDSIIIProgram::get_rx_phase_regno(double p) {
  size_t i=0;
  while (i<rx_phase_registers.size() && fabs(rx_phase_registers[i]-p)>phase_accuracy) ++i;
  if (i==rx_phase_registers.size())
    rx_phase_registers.push_back(p);
  return i;

}

int PulseBlasterDDSIIIProgram::get_tx_phase_regno(double p) {
  size_t i=0;
  while (i<tx_phase_registers.size() && fabs(tx_phase_registers[i]-p)>phase_accuracy) ++i;
  if (i==tx_phase_registers.size())
    tx_phase_registers.push_back(p);
  return i;
}

PulseBlasterCommand* PulseBlasterDDSIIIProgram::create_command(const state& the_state) {
  if (typeid(the_state)!=typeid(state))
    throw pulse_exception("tried to make a state from a sequence");

  // todo: state's defaults

  double length=the_state.length;
  double state_frequency=0;
  unsigned long ttls=0;
  double rx_phase=0;
  SpinCorePulseBlasterDDSIII::analog_state rx_enable=SpinCorePulseBlasterDDSIII::ANALOG_OFF;
  double tx_phase=0;
  SpinCorePulseBlasterDDSIII::analog_state tx_enable=SpinCorePulseBlasterDDSIII::ANALOG_OFF;

  for (state::const_iterator i=the_state.begin(); i!=the_state.end(); ++i) {
    // collect states information
    const ttlout* to=dynamic_cast<const ttlout*>(*i);
    if (to!=NULL  && to->id==0) {
      ttls|=to->ttls.to_ulong();
      continue;
    }
    // add frequency information
    const analogout* ao=dynamic_cast<const analogout*>(*i);
    if (ao!=NULL && ao->id<2) {
      if ((rx_enable!=SpinCorePulseBlasterDDSIII::ANALOG_OFF ||
	   tx_enable!=SpinCorePulseBlasterDDSIII::ANALOG_OFF) &&
	  fabs(state_frequency-ao->frequency)>freq_accuracy)
	throw pulse_exception("only one frequency for analog outputs possible");
      state_frequency=ao->frequency;
      switch(ao->id) {
      case 0:
	if (rx_enable!=SpinCorePulseBlasterDDSIII::ANALOG_OFF) throw pulse_exception("rx channel already set");
	// rx is identified with channel 0
	rx_phase=ao->phase;
	rx_enable=SpinCorePulseBlasterDDSIII::ANALOG_ON;
	break;
      case 1:
	if (tx_enable!=SpinCorePulseBlasterDDSIII::ANALOG_OFF) throw pulse_exception("tx channel already set");
	// tx is identified with channel 1
	tx_phase=ao->phase;
	tx_enable=SpinCorePulseBlasterDDSIII::ANALOG_ON;
	break;
      }
      continue;
    }
  }
  return new PulseBlasterDDSIIICommand(*this,state_frequency,rx_phase,rx_enable,tx_phase,tx_enable,ttls,length);
}

PulseBlasterCommand* PulseBlasterDDSIIIProgram::create_command(const PulseBlasterCommand* orig) {
  PulseBlasterDDSIIICommand* new_command=NULL;
  if (orig==NULL) {
    new_command=new PulseBlasterDDSIIICommand();
    new_command->program=this;
    new_command->length=minimum_interval;
  }
  else {
    const PulseBlasterDDSIIICommand* commandDDSIII=dynamic_cast<const PulseBlasterDDSIIICommand*>(orig);
    if (commandDDSIII==NULL) throw pulse_exception("wrong PulseBlasterCommand class in PulseBlasterDDSIIIProgram method");
    new_command=new PulseBlasterDDSIIICommand(*commandDDSIII);
  }
  return new_command;
}


/*********************************************************************

                    SpinCorePulseBlasterDDSIII

*********************************************************************/

SpinCorePulseBlasterDDSIII::SpinCorePulseBlasterDDSIII(double the_clock): SpinCorePulseBlaster(10,the_clock)  {
  freq_regno=16;
  phase_regno=16;
}

void SpinCorePulseBlasterDDSIII::set_registers(int device, unsigned int register_size, double multiplier, const std::vector<double>& values) {
  if (values.size()>register_size)
    throw SpinCorePulseBlaster_error("to many data for registers");
  unsigned char data[4*register_size];
  
  for (unsigned int reg=0; reg<register_size; ++reg) {
    if (values.size()>reg) {
      double temp=values[reg]*multiplier;
      if (temp<0 || temp>pow(2,32))
	throw SpinCorePulseBlaster_error("invalid data value");
      unsigned int val=(unsigned int)temp;
      data[reg*4]=(val&0xff000000)>>24;
      data[reg*4+1]=(val&0xff0000)>>16;
      data[reg*4+2]=(val&0xff00)>>8;
      data[reg*4+3]=val&0xff;	
    }
    else {
      data[reg*4]=0;
      data[reg*4+1]=0;
      data[reg*4+2]=0;
      data[reg*4+3]=0;
    }
  }
  write_register(0,0); // dev reset
  write_register(2,4); // bytes per word
  write_register(3,device); // dev to program
  write_register(4,0); //reset address counter
  write_data(data,4*register_size);
}

void SpinCorePulseBlasterDDSIII::set_phase_registers(std::vector<double> rx_phases, std::vector<double> tx_phases) {
  if (tx_phases.empty())
    set_registers(2,phase_regno,1.0,std::vector<double>(1,0.0));
  else
    set_registers(2,phase_regno,pow(2,32)/360.0,tx_phases);
  if (rx_phases.empty())
    set_registers(3,phase_regno,1.0,std::vector<double>(1,0.0));
  else 
  set_registers(3,phase_regno,pow(2,32)/360.0,rx_phases);
}

void SpinCorePulseBlasterDDSIII::set_frequency_registers(const std::vector<double>& values) {
  if (values.empty())
    set_registers(1,freq_regno,pow(2,32)/clock,std::vector<double>(1,1e6));
  else
    set_registers(1,freq_regno,pow(2,32)/clock,values);
}

void SpinCorePulseBlasterDDSIII::write_command(unsigned char* data, const PulseBlasterCommand& command) {
  //void PulseBlasterDDSIII::append_command(std::string& data, int freqreg, int phasereg_tx, int output_tx, int phasereg_rx, int output_rx, int flags, opcode inst, int inst_data, double length)
  const PulseBlasterDDSIIICommand* commandDDSIII=dynamic_cast<const PulseBlasterDDSIIICommand*>(&command);
  if (commandDDSIII==NULL)
    throw SpinCorePulseBlaster_error("found wrong command class in PulseBlasterProgram");

  if (command.program==NULL) throw SpinCorePulseBlaster_error("Command not associated with Program");
  int inst_data=0;
  switch(command.instruction) {
  case SpinCorePulseBlaster::CONTINUE:
  case SpinCorePulseBlaster::STOP:
  case SpinCorePulseBlaster::WAIT:
  case SpinCorePulseBlaster::RTS:
    // no parameter
    break;
  case SpinCorePulseBlaster::LOOP:
    inst_data=command.loop_count-1;
    break;
  case SpinCorePulseBlaster::LONG_DELAY:
    inst_data=command.loop_count-2;
    break;
  case SpinCorePulseBlaster::BRANCH:
  case SpinCorePulseBlaster::END_LOOP:
  case SpinCorePulseBlaster::JSR:
    inst_data=std::distance(command.program->begin(),command.jump);
    break;
  default:
    throw SpinCorePulseBlaster_error("instruction code not known");
  }
  unsigned int delay=(unsigned int)command.length-3;

  // Output, Control Word 1st Byte
  data[0]=(commandDDSIII->freq_reg&0x0f)<<4|(commandDDSIII->tx_phase_reg&0x0f);
  // Output, Control Word 2nd Byte
  data[1]=((commandDDSIII->rx_phase_reg&0x0f)<<4)|((command.ttls&0x300)>>8);
  if (commandDDSIII->rx_enable==ANALOG_OFF) data[1]|=0x04;
  if (commandDDSIII->tx_enable==ANALOG_OFF) data[1]|=0x08;
  // Output, Control Word 3rd Byte
  data[2]=command.ttls&0xff;
  // Data Field 1st Byte
  data[3]=(inst_data&0x0ff000)>>12;
  // Data Field 2nd Byte
  data[4]=(inst_data&0xff0)>>4;
  // Data Field 3rd Byte and opcode
  data[5]=(inst_data&0xf)<<4|(command.instruction&0xf);
  // Delay Count 1st Byte
  data[6]=(delay&0xff000000)>>24;   
  // Delay Count 2nd Byte
  data[7]=(delay&0xff0000)>>16;
  // Delay Count 3rd Byte
  data[8]=(delay&0xff00)>>8;
  // Delay Count 4th Byte
  data[9]=(delay&0xff);
}

int SpinCorePulseBlasterDDSIII::write_to_device(const PulseBlasterDDSIIIProgram& p) {
  set_frequency_registers(p.frequency_registers);
  set_phase_registers(p.rx_phase_registers,p.tx_phase_registers);

  std::string program;
  //Begin pulse program
  for (PulseBlasterDDSIIIProgram::const_iterator c=p.begin(); c!=p.end();++c) {
    char command[10];
    write_command((unsigned char*)command,**c);
    program.append(command, (size_t)10);
  }

  set_program(program);

  // End of programming registers and pulse program
  set_initialized();
  return 1;
}


#if 1
void SpinCorePulseBlasterDDSIII::single_pulse_program(double t_before, double t_pulse, double t_after) {

  /* design error, these definitions should be elsewhere! */
  double t_trigger=1e5; // [s] delay of trigger after pulse, i.e. dead time
  unsigned long trigger_line_mask=1<<2; // mask for trigger pulse

  PulseBlasterDDSIIIProgram p;

  // wait until gating....
  if (t_before>gate_time+10.0e-9)
    // append_state(0,0,TX_ANALOG_ON,0,RX_ANALOG_ON,0x0,t_before-t_gate);
    p.push_back(new PulseBlasterDDSIIICommand(p,frequency,0,SpinCorePulseBlasterDDSIII::ANALOG_ON,
					      90,SpinCorePulseBlasterDDSIII::ANALOG_ON,0x0,t_before-gate_time));

  // gate
  //append_state(0,0,TX_ANALOG_ON,0,RX_ANALOG_ON,gate_channel,t_gate);
  p.push_back(new PulseBlasterDDSIIICommand(p,frequency,0,SpinCorePulseBlasterDDSIII::ANALOG_ON,
					    90,SpinCorePulseBlasterDDSIII::ANALOG_ON,gate_channel,gate_time));

  // pulse
  //append_state(0,0,TX_ANALOG_ON,0,RX_ANALOG_ON,gate_channel|pulse_channel,t_pulse);
  p.push_back(new PulseBlasterDDSIIICommand(p,frequency,0,SpinCorePulseBlasterDDSIII::ANALOG_ON,
					    90,SpinCorePulseBlasterDDSIII::ANALOG_ON,gate_channel|pulse_channel,t_pulse));

  // stop pulse, close gate and trigger ADC
  //append_state(0,0,TX_ANALOG_ON,0,RX_ANALOG_ON,trigger_channel,t_trigger);
  p.push_back(new PulseBlasterDDSIIICommand(p, frequency, 0, SpinCorePulseBlasterDDSIII::ANALOG_ON,
					    90, SpinCorePulseBlasterDDSIII::ANALOG_ON, trigger_line_mask, t_trigger));

  // just send analog output for some time
  // append_state(0,0,TX_ANALOG_ON,0,RX_ANALOG_ON,0x0,t_after);
  p.push_back(new PulseBlasterDDSIIICommand(p,frequency,0,SpinCorePulseBlasterDDSIII::ANALOG_ON,
					    90,SpinCorePulseBlasterDDSIII::ANALOG_ON,0x0,t_after));

  // stop output
  //pb_inst(0,0,TX_ANALOG_OFF,0,RX_ANALOG_OFF,0x0,STOP,0,100*ns);
  PulseBlasterDDSIIICommand* c_stop=new PulseBlasterDDSIIICommand(p,frequency,0,SpinCorePulseBlasterDDSIII::ANALOG_ON,
								  90,SpinCorePulseBlasterDDSIII::ANALOG_ON,0x0,1e-7);
  c_stop->instruction=STOP;
  p.push_back(c_stop);

  run_pulse_program(p);

  duration=((t_before>gate_time)?t_before:gate_time)+t_pulse+t_after;
}
#endif

PulseBlasterProgram* SpinCorePulseBlasterDDSIII::create_program(state& exp) {
    PulseBlasterDDSIIIProgram* prog=new PulseBlasterDDSIIIProgram();
    // some initialisiation...
    prog->append_sequence(exp);
    // some reset code ...
    return prog;
}

void SpinCorePulseBlasterDDSIII::run_pulse_program(const PulseBlasterProgram& p) {
  const PulseBlasterDDSIIIProgram* prog=dynamic_cast<const PulseBlasterDDSIIIProgram*>(&p);
  if (prog==NULL)
    throw SpinCorePulseBlaster_error("found wrong program class in SpinCorePulseBlasterDDSIII method");
  write_to_device(*prog);
  prog->write_to_file(stderr);
  start();
}


