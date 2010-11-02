/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#ifndef XML_STATES_H
#define XML_STATES_H

#include "states.h"
#include <expat.h>
#include <cstdio>
#include <xercesc/dom/DOMElement.hpp>

/**
   \defgroup xmlstateinterface XML support for all state classes
   \ingroup states
   The state sequences consist of states and again sequences. A state is defined by a number of state_atom objects.
   This definition results in a file with nested sections, that is parsed into a tree formed by the state objects.
   The leaves of the tree are normaly state_atom objects. These state_atom objects define the state of a device.
   
   An example for a sequence definition is:
\verbatim<?xml version="1.0" encoding="ISO-8859-1"?>
<sequent repeat="3">
  <state time="3e-3">
   <ttlout value="4"/>
   <ttlout channel="2" state="0"/>
   <analogout channel="1" f="1e6" p="0"/>
   <analogout channel="2" f="1e6" p="90"/>
  </state>
  <sequent repeat="15">
    <state time="3e-3">
     <ttlout value="7"/>
     <analogout channel="1" f="1e6" p="0"/>
     <analogout channel="2" f="1e6" p="90"/>
    </state>
    <state time="1e-3">
      <analogin f="5e6" s="4096"/>
    <state/>
  </sequent>
</sequent>\endverbatim

The first line of this code is a statement of xml conformity. (More about xml can be read at:
http://www.w3.org/TR/REC-xml .)
\n
The second line defines a new subsequence of states by using the tag \<sequent repeat="3"\>.
The "repeat" value 3 is the loop count. This line effects an instatiation of a state_sequent object.
All information will be collected in this branch, it is closed on the last line by \</sequent\>.
In this example the third line provides a %state tag corresponding to a state object.
So this also line specifies the duration of this state in seconds.
This section is closed with the eigth line containing \</state\>.
A xml document can contain only one top-level-element, that is called root or document element.
Applying the xml_state_reader it is not necessary to begin with a sequent element.
\n
A xml section (representing a state or sequence in our case) is opend by a tag with a name in angle braces like
\<sequent\> and is closed with an extra slash before the name \</sequent\>. Attributes can follow in the form name="value",
attributes must be quoted (single or double quotes). If such a section has no contents there is a short form with a slash
before the closing angle, e.g. \<ttlout value="7"/\>.
\n
Of course outside these tags, text can be supplied. By now, xml_state_reader ignores this text.
Inside a %state section the definition of the state is collected. Implemented are ttlout, analogout and
analogin. One element can occurr several times. How these states are merged is machine dependent.

So a state sequence is defined by <b>one sequent</b> section, containing again serval <b>state</b> or <b>sequent</b>
sections. The <b>state</b> sections must not contain other <b>state</b> sections or <b>sequent</b> sections.
 
*/
//@{
/**
   \brief gains state sequence from an xml event stream
 */
class xml_state_reader {
  state_atom* root;
  std::list<state_atom*> pending_elements;
  XML_Parser parser;
 public:
  /**
     instantiates an expat xml parser object
   */
  xml_state_reader();
  /**
     frees the parser object
   */
  ~xml_state_reader();
  /**
     \brief parses xml file containing sequences of states
     the name of the file is the only parameter
     \return returns tree of states or NULL if an parse error occured
   */
  state_atom* read_from_file(const std::string& filename);

  state_atom* read_from_dom(const XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* e) {
    // to do work, work and work
    return NULL;
  }

  /**
     \brief parses a string containing sequences of states
     \return returns tree of states or NULL if an parse error occured
   */
  state_atom* read_from_string(const std::string& data);

  /** \brief finds an attribute in the attributes array
      \return returns the pointer to the attributes value or NULL if not in the array
   */
  const XML_Char* search_attribute(const XML_Char** atts, const XML_Char* attr) const;

  /**
     \brief looks for an attribute with different names
     the first alternative, that could be found is taken
     \return returns a pointer to the attributes value or NULL if nothing approprate is found
   */
  const XML_Char* search_attributes(const XML_Char** atts, ...) const;

  /**
     create new states from xml tags and their attributes
   */
  state_atom* state_factory(const XML_Char* name, const XML_Char** atts) const;

  /**
     start tag callback for expat parser
   */
  void start_element(const XML_Char* name,const XML_Char** atts);

  /**
     end tag callback for expat parser
   */
  void end_element(const XML_Char* name);
};

/**
   \ingroup states
   the writer outputs states to xml files
   takes care of formating
 */
class xml_state_writer {
 public:
  /// have a stack
  std::list<state_atom*> pending_elements;
  /// allow nice formating druing recursive operation
  size_t indent_offset;
  /// allow nice formating druing recursive operation
  size_t indent_increase;
  
  /// simple initialisation
  xml_state_writer(){
    indent_offset=0;
    indent_increase=2;
  };


  /**
     write states to a given output
     if add_header is !=0 a full xml document is given
   */
  int write_states(FILE* output, const state_atom& states_to_write, int add_header=0, int indent_size=0);
};
//@}


#endif
