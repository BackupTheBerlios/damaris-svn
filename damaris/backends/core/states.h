/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#ifndef STATES_H
#define STATES_H

#include <list>
#include <bitset>

/** \defgroup states Devices States
    \brief different states of devices are defined

    Pulse devices are driven by \link state_sequent sequences\endlink of \link state states \endlink,
    that can be nested and repeated.
    So an appropriate data structure is a tree of sequences, that can contain states or sequences.
    Each sequence can be repeated. States have a defined time length and define the state of all
    outputs of the pulse device once. So the length of a sequence is the sum of all state durations
    and the length of the nested sequences. If the sequence is repeated, the duration has to be
    multiplicated with the numbers of loops.
    \n
    ToDo: State sections that allow parallel sequences inside are an intresting extension of this concept

    Inside the \link state states \endlink, the state_atom objects define the state. There are several classes
    derived from state_atom .
    \n
    ToDo: We could define a default state, that is used as template for the states in the sequences.

    This definition results in a tree structure, that is traversed <b>"depth-first"</b>. A class for that traversal
    is the state_iterator .

 */
//@{

/**
   \brief describes the state of one device
   the duration of the state is given by the surrounding state section
*/
class state_atom {
 public:
  /** no time, no repetiton */
  virtual state_atom* copy_new() const=0;

  virtual ~state_atom() {}
};

//@}

/**
   \defgroup derived_stateatom several derived state_atom classes
   \ingroup states
 */
//@{

/**
   channels of a multichannel device
*/
typedef std::bitset<32> channel_array;

/**
   status of ttl lines
*/
class ttlout: public state_atom {
 public:
  /** some reference to the device */
  int id;
  /** the ttl levels */
  channel_array ttls;
  
  /** default: all off */
  ttlout() {
    id=0;
    ttls=0;
  }

  ttlout(const ttlout& orig) {
    id=orig.id;
    ttls=orig.ttls;
  }

  virtual state_atom* copy_new() const {
    return new ttlout(*this);
  }

  virtual ~ttlout() {}
};

/**
   definition of analog output
*/
class analogout: public state_atom {
 public:
  /** some reference to the device */
  int id;
  /** amplitude voltage in V, as fallback 0 means off, !=0 means on */
  double amplitude;
  /** phase in degree */
  double phase;
  /** frequency in Hz*/
  double frequency;

  /** default: all of */
  analogout() {
    id=0;
    amplitude=0.0;
    phase=0.0;
    frequency=0.0;
  }

  analogout(const analogout& orig) {
    id=orig.id;
    amplitude=orig.amplitude;
    phase=orig.phase;
    frequency=orig.frequency;
  }

  virtual state_atom* copy_new() const {
    return new analogout(*this);
  }

  virtual ~analogout() {}
};


/**
   definition of a analog input
 */
class analogin: public state_atom {
 public:
  /** a reference to the device */
  int id;
  /** sample frequency */
  double sample_frequency;
  /** samples each channel */
  size_t samples;
  /** which channels to record */
  channel_array channels;

  /** sensitivity in volts*/
  double sensitivity;

  /** resolution in bit per sample */
  size_t resolution;

  /** default: do nothing */
  analogin() {
    id=0;
    sample_frequency=0;
    samples=0;
    sensitivity=0;
    resolution=0;
  }

  analogin(const analogin& orig) {
    id=orig.id;
    sample_frequency=orig.sample_frequency;
    samples=orig.samples;
    sensitivity=orig.sensitivity;
    resolution=orig.resolution;
  }

  virtual state_atom* copy_new() const {
    return new analogin(*this);
  }

  virtual ~analogin() {}

};

//@}

/**
   \ingroup states
 */
//@{

class state: public state_atom, public std::list<state_atom*> {
 public:
  /** \brief how long in seconds this state should be effective */
  double length;

  /** \brief points to parent state

      the root document has NULL, no cyclic references allowed
  */
  const state* parent;

  /**
     initializes an empty state
   */
  state(double _length, const state* my_parent=NULL): state_atom(), std::list<state_atom*>(), length(_length), parent(my_parent) {
  }
  
  state(const state& orig): state_atom(), std::list<state_atom*>(), length(orig.length), parent(orig.parent) {
    for (state::const_iterator i=orig.begin(); i!=orig.end(); i++)
      push_back((**i).copy_new());
  }

  virtual state_atom* copy_new() const {
    return new state(*this);
  }

  virtual state* copy_flat(size_t enroll=1) const {
    return new state(*this);
  }
  
  virtual ~state() {
    while (!empty()) {
      if (back()!=NULL) delete (back());
      pop_back();
    }
  }
  
};

class state_parallel: public state {
 public:
  /**
     possible time alignments
  */
  typedef enum {begin_align,end_align,center_align} states_alignment;

  /**
     define time allignments
   */
  states_alignment align;

  state_parallel(const state* my_parent=NULL, states_alignment _a=begin_align): state(0.0,my_parent), align(_a) {
  }

  state_parallel(const state_parallel& orig): state(orig) {
    align=orig.align;
  }

  virtual state* copy_new() const {
    return new state_parallel(*this);
  }

  virtual state* copy_flat(size_t enroll=4) const;

  virtual ~state_parallel() {}

};

/**
   \brief sequential states with loops
 */
class state_sequent: public state {
 public:
  /** \brief how often this state or state sequence should be repeated
  */
  size_t repeat;
  
  state_sequent(size_t _repeat=1, const state* my_parent=NULL): state(0.0,my_parent), repeat(_repeat) {
  }

  state_sequent(const state_sequent& orig): state(orig) {
    repeat=orig.repeat;
  }

  virtual state* copy_new() const {
    return new state_sequent(*this);
  }

  virtual state* copy_flat(size_t enroll=4) const;

  virtual ~state_sequent() {
      while (!empty()) {
	  if (back()!=NULL) delete (back());
	  pop_back();
      }
  }
};

/**
   \brief traverses the state tree from state to state
   this iterator leaves out the subsequence borders, but returns correct informations about time of the first
   traversal of a state and the total count of traversals.
   this is a nonconstant iterator, we have to write a const-iterator...
*/

class state_iterator {
 public:
  typedef struct {
    state_sequent* subsequence;
    state_sequent::iterator subsequence_pos;
    /// time in seconds in a substate until iterator position
    double elapsed_time;
  } subsequence_iterator;
  
  std::list<subsequence_iterator> subsequence_stack;
  /// time in seconds till iterator position for first loop run
  double total_time;

  /**
     start iteration at the beginning of a sequence
   */
  state_iterator(state_sequent& subsequence) {
    subsequence_iterator the_beginning={&subsequence,subsequence.begin(),0};
    subsequence_stack.push_back(the_beginning);
    total_time=0.0;
    (void)get_state();
  }
  
  /**
     time of end of first traversal
   */
  double get_time() const {
    if (subsequence_stack.empty()) return total_time;
    double time=0.0;
    for (std::list<subsequence_iterator>::const_iterator i=subsequence_stack.begin();
	 i!=subsequence_stack.end();
	 ++i)
      time+=i->elapsed_time;
    return time;
  }

  /**
     get count of traversals of this state
   */
  size_t get_count() const {
    size_t count=1;
    for (std::list<subsequence_iterator>::const_iterator i=subsequence_stack.begin();
	 i!=subsequence_stack.end();
	 ++i)
      count*=i->subsequence->repeat;
    return count;
  }

  /**
     advance to next state
    \return a pointer to next state or NULL if no state was following
  */
  state* next_state() {
    if (subsequence_stack.empty()) return NULL;
    subsequence_stack.back().elapsed_time+=get_state()->length;
    ++subsequence_stack.back().subsequence_pos;
    return get_state();
  }
  
  /**
     return true, if iterator is at end
   */
  int is_last() const {
    return subsequence_stack.empty();
  }
  
  /**
     get a pointer of this state
     if the iterator is actually not on a state, it will go to the next (traverse loops and sections)
     \return a pointer to the state or NULL, if no state available
   */
  state* get_state();
};

/*@}*/
#endif
