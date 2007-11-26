#include "SpinCore-PulseBlaster.h"
#include "PulseBlasterProgram.h"
#include "core/xml_states.h"

/* ***************************************************************************************

                                 PulseBlasterCommand

**************************************************************************************** */

PulseBlasterCommand::PulseBlasterCommand() {
  ttls=0;
  instruction=SpinCorePulseBlaster::CONTINUE;
  length=0;
  loop_count=0;
  program=NULL;
}

PulseBlasterCommand::PulseBlasterCommand(const PulseBlasterCommand& orig) {
  instruction=orig.instruction;
  length=orig.length;
  ttls=orig.ttls;
  program=orig.program;
  switch(instruction) {
      case SpinCorePulseBlaster::LOOP:
      case SpinCorePulseBlaster::LONG_DELAY:
	loop_count=orig.loop_count;
	jump=program->end();
	break;
      case SpinCorePulseBlaster::BRANCH:
      case SpinCorePulseBlaster::END_LOOP:
      case SpinCorePulseBlaster::JSR:
	jump=orig.jump;
	loop_count=0;
	break;
      default:
	loop_count=0;
	jump=program->end();
  }
}

PulseBlasterCommand::PulseBlasterCommand(PulseBlasterProgram& p) {
    ttls=0;
    instruction=SpinCorePulseBlaster::CONTINUE;
    length=p.minimum_interval;
    program=&p;
    loop_count=0;
    jump=p.end();
}

PulseBlasterCommand::PulseBlasterCommand(PulseBlasterProgram& p, int _ttls, double _length) {
  ttls=_ttls;
  loop_count=0;
  instruction=SpinCorePulseBlaster::CONTINUE;
  program=&p;
  jump=p.end();
  length=(size_t)floor(_length*p.internal_clock_freq+0.5); // graceful rounding
}

/* ***************************************************************************************

                                 PulseBlasterProgram

**************************************************************************************** */


PulseBlasterProgram::PulseBlasterProgram(const PulseBlasterProgram& orig): std::list<PulseBlasterCommand*>() {
    // do not copy the list. This must be done by the derived class...
    internal_clock_freq=orig.internal_clock_freq;
    minimum_interval=orig.minimum_interval;
}

int PulseBlasterProgram::insert_ref(PulseBlasterProgram::iterator target_pos,
				    PulseBlasterProgram::const_iterator start_copy,
				    PulseBlasterProgram::const_iterator end_copy) {
  insert(target_pos,start_copy,end_copy);
  iterator i=target_pos;
  for(const_iterator orig_i=end_copy;orig_i!=start_copy;--orig_i) {
    if ((**i).instruction==SpinCorePulseBlaster::END_LOOP || (**i).instruction==SpinCorePulseBlaster::BRANCH || (**i).instruction==SpinCorePulseBlaster::JSR) {
      (**i).jump=i;
      advance(i,distance(start_copy,(**orig_i).jump)-distance(start_copy,orig_i));
    }
    --i;
  }
  return 0;
}

PulseBlasterProgram::~PulseBlasterProgram() {
    while (!empty()) {
	if (back()!=NULL) delete back();
	pop_back();
    }
}

void PulseBlasterProgram::append_sequence(const state& the_states) {
  std::list<stackentry> the_stack;

  // recieve a work copy with partially enrolled loops and flat structures
  state* flat=the_states.copy_flat(0);


  push_back(create_command());
  iterator dummy_pos=--end();
  state_atom* this_tag=flat;

  while (1) {
    // operate on a sequence
    if (typeid(*this_tag)==typeid(state_sequent)) {
      //fprintf(stderr,"found sequence\n");
      state_sequent* sequence=dynamic_cast<state_sequent*>(this_tag);
      // throw away all elements, which were non-states
      for(state_sequent::iterator i=sequence->begin(); sequence->end()!=i;) {
	if (dynamic_cast<state*>(*i)==0) {
	  // states are expected, nothing else!
	  fprintf(stderr, "only states are allowed inside a sequent section!\n");
	  delete *i;
	  i=sequence->erase(i);
	}
	else
	  // fine
	  ++i;
      }
      // skip if empty
      if (!sequence->empty() && sequence->repeat>0) {
	if (sequence->repeat>1<<20) {
	  // nested loops as workaround
	  size_t new_count1=1<<20;
	  size_t new_count2=sequence->repeat/new_count1;
	  size_t new_count3=sequence->repeat-new_count1*new_count2;
	  // if (new_count2==1) {}
	  // if (new_count3==0) {}
	  state_sequent* new_sequence1=new state_sequent();
	  new_sequence1->repeat=new_count1;
	  state_sequent* new_sequence2=new state_sequent(*sequence);
	  new_sequence2->repeat=new_count2;
	  new_sequence1->push_back(new_sequence2);
	  state_sequent* new_sequence3=new state_sequent(*sequence);
	  new_sequence3->repeat=new_count3;
	  while (!sequence->empty()) {
	    delete sequence->front();
	    sequence->pop_front();
	  }
	  sequence->repeat=1;
	  sequence->push_back(new_sequence1);
	  sequence->push_back(new_sequence3);
	}
	// if necessary enroll loop at begin
	// prepare sequence
	if (sequence->repeat>1) {
	  while (1) {
	    state* first_state=NULL;
	    state_sequent::iterator i=sequence->begin();
	    while( i!=sequence->end()) {
	      first_state=dynamic_cast<state*>(*i);
	      if (NULL!=first_state) break;
	      ++i;
	    }
	    if (first_state!=NULL) {
	      state_sequent* first_sequence=dynamic_cast<state_sequent*>(first_state);
	      if (first_sequence!=NULL) {
		// fprintf(stderr,"enroll begin\n");
		size_t repeat=first_sequence->repeat;
		if (repeat==0) {
		  delete *i;
		  sequence->erase(i);
		}
		else if (repeat==1) {
		  sequence->insert(i,first_sequence->begin(),first_sequence->end());
		  first_sequence->clear();
		  delete *i;
		  sequence->erase(i);
		}
		else {
		  for (state_sequent::iterator j=first_sequence->begin();j!=first_sequence->end();++j)
		    sequence->insert(i,(*j)->copy_new());
		  first_sequence->repeat=repeat-1;
		}
		// check again...
		continue;
	      }
	    }
	    // nothing to be done
	    break;
	  }
	  // if necessary enroll loop at end
	  while (1) {
	    state* last_state=NULL;
	    state_sequent::iterator i=sequence->end();
	    while(i!=sequence->begin()) {
	      --i;
	      last_state=dynamic_cast<state*>(*i);
	      if (last_state!=NULL) break;
	    }
	    if (last_state!=NULL) {
	      state_sequent* last_sequence=dynamic_cast<state_sequent*>(last_state);
	      if (last_sequence!=NULL) {
		if (last_sequence->repeat==0) {
		  // forget this sequence
		  delete *i;
		  sequence->erase(i);
		}
		else if (last_sequence->repeat==1) {
		  // forget the sequence around it
		  sequence->insert(i,last_sequence->begin(),last_sequence->end());
		  last_sequence->clear();
		  delete *i;
		  sequence->erase(i);
		}
		else {
		  // fprintf(stderr,"enroll end\n");
		  last_sequence->repeat-=1;
		  i++;
		  for (state_sequent::iterator j=last_sequence->begin();j!=last_sequence->end();++j)
		    sequence->insert(i,(*j)->copy_new());
		}
		// check again
		continue;
	      }
	      break;
	    }
	  }
	}
	// step down
	// push to stack
	stackentry new_entry={sequence, sequence->begin(), --end()};
#if 0
	if (new_entry.command==the_program->end())
	  fprintf(stderr,"saved an end iterator\n");
	else
	  fprintf(stderr,"saved pos no %d\n",distance(begin(),new_entry.command));
	for (PulseBlasterProgram::iterator i=++begin();i!=end();i++)
	  i->write_to_file(stderr);
#endif
	the_stack.push_back(new_entry);
	this_tag=*(the_stack.back().subsequence_position);
	continue;
      }
    }
    // operate on a single state
    else if (typeid(*this_tag)==typeid(state)) {
      //fprintf(stderr,"found state\n");
      state* this_state=dynamic_cast<state*>(this_tag);
      if (this_state->length*internal_clock_freq>pow(2,32)-1) {
	// add a loop to state, to gain longer durations
	// numerical stability?
	// todo: avaoid unnecessary loop or state
	const double loop_state_length=42.949;
	size_t number_loops=(size_t)floor(this_state->length/loop_state_length);
	double remaining_time=this_state->length-loop_state_length*number_loops;
	if (remaining_time*internal_clock_freq<minimum_interval) {
	  // now, the remaining state might be too short
	  remaining_time+=loop_state_length;
	  --number_loops;
	}
	state_sequent* new_loop=new state_sequent;
	new_loop->repeat=number_loops;
	this_state->length=loop_state_length;
	new_loop->push_back(this_state->copy_new());
	this_state->length=remaining_time;
	if (the_stack.empty()) {
	  // in addition to this make a sequence around it
	  state_sequent* sequence_envelope=new state_sequent;
	  sequence_envelope->push_back(new_loop);
	  sequence_envelope->push_back(this_state);
	  this_state=sequence_envelope;
	}
	else {
	  // insert the loop after this state
	  state_sequent::iterator loop_pos=(state_sequent::iterator)the_stack.back().subsequence_position;
	  the_stack.back().subsequence->insert(++loop_pos,new_loop);
	}
	continue;
      }
      else {
	// simply append the state
	PulseBlasterCommand* c=create_command(*this_state);
	if (c==NULL) throw pulse_exception("failed state creation");
	push_back(c);
      }
    }
    else {
      fprintf(stderr,"droped something");
      // drop it...
    }

    // find the next state...
    while (!the_stack.empty()) {
      stackentry& last_entry=the_stack.back();
      ++last_entry.subsequence_position;
      if (last_entry.subsequence_position!=last_entry.subsequence->end()) {
	this_tag=*(the_stack.back().subsequence_position);
	break;
      }
      // finish loop...
      if (last_entry.subsequence->repeat>1) {
	++last_entry.command;
	if (last_entry.command==--end()) {
	  // special case long delay
	  back()->instruction=SpinCorePulseBlaster::LONG_DELAY;
	  back()->loop_count=last_entry.subsequence->repeat;
	}
	else {
	  // loop with several states
	  (**(last_entry.command)).instruction=SpinCorePulseBlaster::LOOP;
	  (**(last_entry.command)).loop_count=last_entry.subsequence->repeat;
	  back()->instruction=SpinCorePulseBlaster::END_LOOP;
	  back()->jump=last_entry.command;
	}
      }
      // jump to next level
      the_stack.pop_back();
    }

    if (the_stack.empty()) break;
  }

  delete flat;
  // is that ok in all cases?
  delete *dummy_pos;
  erase(dummy_pos);
}
