/* **************************************************************************

 Author: Achim Gaedke
 Created: March 2005

****************************************************************************/
#include "core/core.h"
#include "SpinCore-PulseBlaster24Bit.h"
#include <cmath>
#include <typeinfo>
#include <iterator>

#ifndef SIZETPRINTFLETTER
#  ifndef __LP64__
#    define SIZETPRINTFLETTER "u"
#  else
#    define SIZETPRINTFLETTER "lu"
#  endif
#endif

/*********************************************************************

                    PulseBlaster24BitCommand

*********************************************************************/

PulseBlaster24BitCommand::PulseBlaster24BitCommand(): PulseBlasterCommand() {
}

PulseBlaster24BitCommand::PulseBlaster24BitCommand(const PulseBlaster24BitCommand& orig): PulseBlasterCommand(orig) {}

PulseBlaster24BitCommand::PulseBlaster24BitCommand(PulseBlaster24BitProgram& p): PulseBlasterCommand(p) {}

PulseBlaster24BitCommand::PulseBlaster24BitCommand(PulseBlaster24BitProgram& p,
						   int _ttls, double _length)
  :PulseBlasterCommand(p, _ttls, _length)
{}

int PulseBlaster24BitCommand::write_to_file(FILE* out, size_t indent) const {

  if (program==NULL) throw pulse_exception("PulseBlaster24Bit: Command not associated with Program");
  fprintf(out,"%s<instruction ttls=\"0x%06x\" ",
	  std::string(indent,' ').c_str(),ttls);
  switch(instruction) {
  case SpinCorePulseBlaster::LOOP:
    fprintf(out,"inst=\"LOOP\" loops=\"%d\"",loop_count);
    break;
  case SpinCorePulseBlaster::LONG_DELAY:
    fprintf(out,"inst=\"LONG_DELAY\" loops=\"%d\"",loop_count);
    break;
  case SpinCorePulseBlaster::BRANCH:
    fprintf(out,"inst=\"BRANCH\" address=\"%" SIZETPRINTFLETTER "\"",std::distance(program->begin(),jump));
    break;
  case SpinCorePulseBlaster::END_LOOP:
    fprintf(out,"inst=\"END_LOOP\" address=\"%" SIZETPRINTFLETTER "\"",std::distance(program->begin(),jump));
    break;
  case SpinCorePulseBlaster::JSR:
    fprintf(out,"inst=\"JSR\" address=\"%" SIZETPRINTFLETTER "\"",std::distance(program->begin(),jump));
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
  fprintf(out," length=\"%g\"/>\n",(1.0*length)/program->internal_clock_freq);
  return 1;
}

/*********************************************************************

                    PulseBlaster24BitProgram

*********************************************************************/

PulseBlaster24BitProgram::PulseBlaster24BitProgram(): PulseBlasterProgram() {
  internal_clock_freq=100.0e6;
  minimum_interval=9;
  ttl_device_id=0;
}

PulseBlaster24BitProgram::PulseBlaster24BitProgram(const SpinCorePulseBlaster24Bit& pb24) {
    internal_clock_freq=pb24.clock;
    minimum_interval=pb24.shortest_pulse;
    ttl_device_id=pb24.ttl_device_id;
}

PulseBlaster24BitProgram::PulseBlaster24BitProgram(const PulseBlaster24BitProgram& orig): PulseBlasterProgram() {
    ttl_device_id=orig.ttl_device_id;
    // be sure, there are no old list items left!
    clear();
    const_iterator orig_i;
    for (orig_i=orig.begin(); orig_i!=orig.end(); ++orig_i) {
	const PulseBlaster24BitCommand* the_command=dynamic_cast<const PulseBlaster24BitCommand*>(*orig_i);
	if (the_command==NULL) {
	    throw SpinCorePulseBlaster_error("wrong command class or NULL pointer found in program");
	}
	PulseBlaster24BitCommand* new_one=new PulseBlaster24BitCommand(*the_command);
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


PulseBlasterCommand* PulseBlaster24BitProgram::create_command(const state& the_state) {
    if (typeid(the_state)==typeid(state_sequent)||typeid(the_state)==typeid(state_parallel))
	throw pulse_exception("PulseBlasterCommands can only be created from states");
    if (the_state.length < minimum_interval)
	throw pulse_exception("PulseBlaster state too short: t<90ns");
    // all states or-ed together
    int the_ttls=0;
    for (state::const_iterator i=the_state.begin();i!=the_state.end();++i) {
	const ttlout* to=dynamic_cast<const ttlout*>(*i);
	if (to!=NULL && to->id==ttl_device_id) {
	    the_ttls|=to->ttls.to_ulong();
	}
    }
    //fprintf(stderr, "state length=%g ttls=%x\n",the_state.length, the_ttls);
    return new PulseBlaster24BitCommand(*this, the_ttls, the_state.length);
}

PulseBlasterCommand* PulseBlaster24BitProgram::create_command(const PulseBlasterCommand* orig) {
    PulseBlaster24BitCommand* new_command=NULL;
    if (orig==NULL) {
	new_command=new PulseBlaster24BitCommand();
	new_command->program=this;
	new_command->length=minimum_interval;
    }
    else {
	const PulseBlaster24BitCommand* command24Bit=dynamic_cast<const PulseBlaster24BitCommand*>(orig);
	if (command24Bit==NULL) throw pulse_exception("wrong PulseBlasterCommand class");
	new_command=new PulseBlaster24BitCommand(*command24Bit);
    }
    return new_command;
}


int PulseBlaster24BitProgram::write_to_file(FILE* out, size_t indent) const {
  std::string indent_string(indent,' ');
  fprintf(out,"%s<PulseBlaster24BitProgram>\n",indent_string.c_str());

  for(const_iterator i=begin();i!=end();++i) {
    if (*i==NULL) throw SpinCorePulseBlaster_error("NULL pointer found in command list");
    (**i).write_to_file(out, indent+2);
  }
  fprintf(out,"%s</PulseBlaster24BitProgram>\n",indent_string.c_str());
  return 1;
}


/*********************************************************************

                    SpinCorePulseBlaster24Bit

*********************************************************************/

void SpinCorePulseBlaster24Bit::write_command(unsigned char* data, int flags, opcode inst, unsigned int inst_data, size_t delay) {
    if (inst>8)
	throw SpinCorePulseBlaster_error("instruction code not known");
        
    // Output, Control Word 1st Byte
    data[0]=(flags&0xff0000)>>16;
    // Output, Control Word 2nd Byte
    data[1]=(flags&0xff00)>>8;
    // Output, Control Word 3rd Byte
    data[2]=flags&0xff;
    // Data Field 1st Byte
    data[3]=(inst_data&0x0ff000)>>12;
    // Data Field 2nd Byte
    data[4]=(inst_data&0xff0)>>4;
    // Data Field 3rd Byte and opcode
    data[5]=(inst_data&0xf)<<4|(inst&0xf);
    // Delay Count 1st Byte
    data[6]=(delay&0xff000000)>>24;
    // Delay Count 2nd Byte
    data[7]=(delay&0xff0000)>>16;
    // Delay Count 3rd Byte
    data[8]=(delay&0xff00)>>8;
    // Delay Count 4th Byte
    data[9]=(delay&0xff);
    /* *** BUG-FIX (ToDo: clean solution) ***
       there is a nasty error in pulseblaster, affecting all states with 4th byte
       equal 0xff and delay >255. In this case reduce state for 10ns.
    */
    if (0 && data[9]==0xff && delay>0xff)
      data[9]=0xfe;
}



void SpinCorePulseBlaster24Bit::write_command(unsigned char* data, const PulseBlasterCommand& command) {
    const PulseBlaster24BitCommand* command24Bit=dynamic_cast<const PulseBlaster24BitCommand*>(&command);
    if (command24Bit==NULL)
	throw SpinCorePulseBlaster_error("found wrong command class in PulseBlasterProgram");

    if (command24Bit->program==NULL) throw SpinCorePulseBlaster_error("Command not associated with Program");
    unsigned int inst_data=0;
    switch(command24Bit->instruction) {
	case SpinCorePulseBlaster::CONTINUE:
	case SpinCorePulseBlaster::STOP:
	case SpinCorePulseBlaster::WAIT:
	case SpinCorePulseBlaster::RTS:
	    // no parameter
	    break;
	case SpinCorePulseBlaster::LOOP:
	    inst_data=command24Bit->loop_count-1;
	    break;
	case SpinCorePulseBlaster::LONG_DELAY:
	    inst_data=command24Bit->loop_count-2;
	    break;
	case SpinCorePulseBlaster::BRANCH:
	case SpinCorePulseBlaster::END_LOOP:
	case SpinCorePulseBlaster::JSR:
	    inst_data=std::distance(command24Bit->program->begin(),command24Bit->jump);
	    break;
	default:
	    throw SpinCorePulseBlaster_error("instruction code not known");
    }
    if (command24Bit->length<3)
	    throw SpinCorePulseBlaster_error("delay length too small!");
    unsigned int delay=(unsigned int)command24Bit->length-3;
    write_command(data,command24Bit->ttls,command24Bit->instruction,inst_data,delay);
}

void SpinCorePulseBlaster24Bit::write_to_device(const PulseBlaster24BitProgram& p) {
  std::string program;
  //Begin pulse program
  for (PulseBlaster24BitProgram::const_iterator c=p.begin(); c!=p.end();++c) {
    char command[10];
    write_command((unsigned char*)command,**c);
    program.append(command, (size_t)10);
  }
  set_program(program);
  // End of programming registers and pulse program
  set_initialized();
}

PulseBlasterProgram* SpinCorePulseBlaster24Bit::create_program(state& exp) {
    PulseBlaster24BitProgram* prog=new PulseBlaster24BitProgram();
    // the user's code
    prog->append_sequence(exp);
    return prog;
}

void SpinCorePulseBlaster24Bit::run_pulse_program(const PulseBlasterProgram& p) {
    const PulseBlaster24BitProgram* prog=dynamic_cast<const PulseBlaster24BitProgram*>(&p);
    if (prog==NULL)
	throw SpinCorePulseBlaster_error("found wrong program class in SpinCorePulseBlaster24Bit method");
    write_to_device(*prog);
    start();
}
