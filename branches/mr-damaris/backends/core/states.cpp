/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#include "states.h"
#include "xml_states.h"
state* state_parallel::copy_flat(size_t enroll) const {
  return copy_new();
}

state* state_sequent::copy_flat(size_t max_enroll_loop) const {
  state_sequent* flat=new state_sequent;
  flat->repeat=repeat;
  if (flat->repeat==0)
    return flat;

  // start copying everything...
  for (const_iterator i=begin(); i!=end(); ++i) {
    const state* this_state=dynamic_cast<const state*>(*i);
    // make the state flat, if sequence is no loop
    if (this_state!=NULL) {
      state* new_flat=this_state->copy_flat(max_enroll_loop);
      state_sequent* ss=dynamic_cast<state_sequent*>(new_flat);
      // handle the sequence
      if (ss!=NULL) {
	// skip this part
	if (ss->repeat==0) {
	  delete ss;
	  continue;
	}
	// do not use extra sequent section
	if (ss->repeat==1) {
	  while (!ss->empty()) {
	    flat->push_back(ss->front());
	    ss->pop_front();
	  }
	  delete ss;
	  continue;
	}
	flat->push_back(new_flat);
	continue;
      }
      state_parallel* sp=dynamic_cast<state_parallel*>(new_flat);
      if (sp!=NULL) {
	flat->push_back(new_flat);
	continue;
      }
      // if this is a state, check length...
      if (new_flat->length==0.0)
	delete new_flat;
      else
	flat->push_back(new_flat);
      continue;
    }
    // simply append other things
    flat->push_back((**i).copy_new());
  } // for
  
  // now enrol own loop
  if (repeat>1 && repeat<=max_enroll_loop) {
    flat->repeat=1;
    std::list<state_atom*> orig_list(*flat);
    for (size_t loop=1;loop<repeat;++loop)
      flat->insert(flat->end(),orig_list.begin(),orig_list.end());
  }

  return flat;
}

state* state_iterator::get_state() {
  if (subsequence_stack.empty()) return NULL; // at the end of traversal
  // find a state or a subsequence in actual subsequence
  state* next_one=NULL;
  for (state_sequent::const_iterator i=subsequence_stack.back().subsequence_pos;
       i!=subsequence_stack.back().subsequence->end();
       ++i) {
    next_one=dynamic_cast<state*>(*i);
    if (next_one!=NULL) break;
  }

  // this subsequence is finished, so go on with the upper one
  if (next_one==NULL) {
    double substate_time=subsequence_stack.back().elapsed_time*subsequence_stack.back().subsequence->repeat;
    subsequence_stack.pop_back();
    if (subsequence_stack.empty()) {
      total_time=substate_time;
      return NULL;
    }
    subsequence_stack.back().elapsed_time+=substate_time;
    ++subsequence_stack.back().subsequence_pos;
    return get_state();
  }
  
  // find out, if this is a subsequence
  state_sequent* next_sequent=dynamic_cast<state_sequent*>(next_one);
  if (next_sequent!=NULL) {
    if (next_sequent->repeat!=0) {
      subsequence_iterator next_level={next_sequent,next_sequent->begin(),0};
      subsequence_stack.push_back(next_level);
    }
    else {
      ++(subsequence_stack.back().subsequence_pos);
    }
    return get_state();
  }
  // this is a state
  return next_one;
}
