/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/

#include <string>
#include <stdarg.h>
#include <cmath>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOMWriter.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include "core/job.h"
#include "core/core.h"
#include "core/stopwatch.h"
#include "core/xml_states.h"

result* quit_job::do_it(core* c) {
  c->quit_mainloop=1;
  return new result(job_no);
}

result* pause_job::do_it(core* c) {
  /* care of SIGNALS */
  pause();
  c->triggered_restart=0;
  return new result(job_no);
}

result* restart_job::do_it(core* c) {
  c->triggered_restart=1;
  c->job_counter=0;
  return new result(job_no);
}

wait_job::wait_job(const size_t n, const XERCES_CPP_NAMESPACE_QUALIFIER DOMNamedNodeMap* attrs): control(n) {
  XMLCh* attr_name=XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode("time");
  XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* length_attr=attrs->getNamedItem(attr_name);
  XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&attr_name);
  if (length_attr==NULL) {
    attr_name=XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode("length");
    length_attr=attrs->getNamedItem((XMLCh*)"time");
    XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&attr_name);
    if (length_attr==NULL)
      throw job_exception("length or time attribute required");
  }
  char* length_data=XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode(length_attr->getNodeValue());
  char* parse_end=NULL;
  sec=strtod(length_data,&parse_end);
  int length_check=strlen(length_data)-(parse_end-length_data);
  XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&length_data);
  if (length_check!=0 || sec<=0)
    throw job_exception("wait: length attribute requires number>0");
}


result* wait_job::do_it(core* c) {
  struct timespec to_sleep;
  to_sleep.tv_sec=(time_t)floor(sec);
  to_sleep.tv_nsec=(long int)floor(1e9*(sec-to_sleep.tv_sec));
  while (c->term_signal==0) {
    struct timespec remaining_time;
    int result=nanosleep(&to_sleep,&remaining_time);
    if (result==0) break;
    to_sleep.tv_sec=remaining_time.tv_sec;
    to_sleep.tv_nsec=remaining_time.tv_nsec;
  }
  /* care of SIGNALS */
  if (c->term_signal!=0) return NULL;
  return new result(job_no);
}

experiment::experiment(size_t n, XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* exp_data ): job(n){
  // create result-document
  XMLCh* core_impl_name=XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode("core");
  XMLCh* doc_name=XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode("description");
  XERCES_CPP_NAMESPACE_QUALIFIER DOMImplementation* impl=XERCES_CPP_NAMESPACE_QUALIFIER DOMImplementationRegistry::getDOMImplementation(core_impl_name);
  XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&core_impl_name);
  XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* description_tags;
  description_tags=impl->createDocument();//NULL, doc_name, NULL);
  XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&doc_name);

  // get description by name
  XMLCh* description_name=XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode("description");
  XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* one_child=exp_data->getFirstChild();

  int found_description=0;
  while (one_child!=NULL) {
    XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* next_child=one_child->getNextSibling();
    if (one_child->getNodeType()==XERCES_CPP_NAMESPACE_QUALIFIER DOMNode::ELEMENT_NODE &&
	XERCES_CPP_NAMESPACE_QUALIFIER XMLString::compareIString(one_child->getNodeName(),description_name)==0) {
      // remove this one from list
      if (found_description==0) {
	  XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* new_child=description_tags->importNode(one_child, 1);
	  description_tags->appendChild(new_child);
	  //XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* old_node=description->replaceChild(new_child, description->getDocumentElement());
	  //old_node->release();
	  found_description=1;
      }
      else {
	fprintf(stderr,"experiment: found more than one description section, ignoring\n");
      }
      exp_data->removeChild(one_child);
      one_child->release();
    }
    one_child=next_child;
  }
  XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&description_name);
  if (description_tags->getDocumentElement()!=NULL) {
      XMLCh tempStr[100];
      XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode("LS", tempStr, 99);
      XERCES_CPP_NAMESPACE_QUALIFIER DOMImplementation *impl2=XERCES_CPP_NAMESPACE_QUALIFIER DOMImplementationRegistry::getDOMImplementation(tempStr);
      XERCES_CPP_NAMESPACE_QUALIFIER DOMWriter *theSerializer=((XERCES_CPP_NAMESPACE_QUALIFIER  DOMImplementationLS*)impl2)->createDOMWriter();  
      XERCES_CPP_NAMESPACE_QUALIFIER MemBufFormatTarget mem;
      theSerializer->writeNode(&mem,*(description_tags->getDocumentElement()));
      theSerializer->release();
      description=std::string((char*)mem.getRawBuffer(),mem.getLen());
      mem.reset();
  }
  if (description_tags!=NULL) {description_tags->release();}
  
  // create state tree from rest of dom
  one_child=exp_data->getFirstChild();
  std::list<state*> found_states;
  while (one_child!=NULL) {
    if (one_child->getNodeType()==XERCES_CPP_NAMESPACE_QUALIFIER DOMNode::ELEMENT_NODE) {
      state_atom* new_one=state_factory(dynamic_cast<XERCES_CPP_NAMESPACE_QUALIFIER DOMElement*>(one_child));
      if (new_one!=NULL) {
       
	state* new_state=dynamic_cast<state*>(new_one);
	
	if (new_state!=NULL) {
        found_states.push_back(new_state);
    }
	else {
	  fprintf(stderr,"experiment: did find something other than a state... forgetting\n");
	  delete new_one;
	}
      }
    }
    one_child=one_child->getNextSibling();
  }

  // save found states...
  if (found_states.empty()) {
    experiment_states=NULL;
  }
  else if (found_states.size()==1) {
    experiment_states=found_states.front();
  }
  else {
    state_sequent* new_sequent=new state_sequent;
    new_sequent->repeat=1;
    for (std::list<state*>::iterator i=found_states.begin();i!=found_states.end();++i) {
      new_sequent->push_back(*i);
      (**i).parent=new_sequent;
    }
    experiment_states=new_sequent;
  }
}

char* experiment::get_parameter(const XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* element, ...) {
  if (element==NULL) return NULL;
  va_list names;
  va_start(names,element);
  const char* attr_name=va_arg(names, const char*);
  const XERCES_CPP_NAMESPACE_QUALIFIER DOMAttr* attr=NULL;
  while (attr_name!=NULL) {
    XMLCh* xml_name=XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode(attr_name);
    attr=element->getAttributeNode(xml_name);
    XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&xml_name);
    if (attr!=NULL) break;
    attr_name=va_arg(names, const char*);
  }
  va_end(names);
  if (attr==NULL) return NULL;
  return XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode(attr->getValue());
}

state_atom* experiment::state_factory(const XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* element) {
  if (element==NULL) return NULL;

  char* my_name=XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode(element->getNodeName());

  if(strcasecmp(my_name,"state")==0) {
    XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&my_name);
    // read parameters
    char* length_string=get_parameter(element,"length","time","t","l",NULL);
    if (length_string==NULL) throw job_exception("state requires length");
    char* length_string_end;
    double length=strtod(length_string,&length_string_end);
    int result=strlen(length_string)-(length_string_end-length_string);
    // here, we could search for us, s or ms, d or min
    XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&length_string);
    // but now we do not accept anything
    if (result!=0) throw job_exception("state requires length as floating point value");
    if (length<0) throw job_exception("state's length must be non-negative");
    state* new_state=new state(length);
    new_state->parent=NULL;

    // read states defintions
    XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* one_child=element->getFirstChild();
    while (one_child!=NULL) {
      if (one_child->getNodeType()==XERCES_CPP_NAMESPACE_QUALIFIER DOMNode::ELEMENT_NODE) {
	state_atom* new_one=state_factory(dynamic_cast<XERCES_CPP_NAMESPACE_QUALIFIER DOMElement*>(one_child));
	if (new_one!=NULL) new_state->push_back(new_one);
      }
      one_child=one_child->getNextSibling();
    }
    return new_state;
  }
  else if(strcasecmp(my_name,"sequent")==0) {
    XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&my_name);
    // read parameters
    size_t repeat=1;
    // read parameters
    char* repeat_string=get_parameter(element,"repeat","n",NULL);
    if (repeat_string!=NULL) {
      char* repeat_string_end;
      double repeat_d=strtod(repeat_string,&repeat_string_end);
      int result=strlen(repeat_string)-(repeat_string_end-repeat_string);
      XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&repeat_string);
      // but now we do not accept anything
      if (result!=0 || repeat_d<0) throw job_exception("sequent requires loop count as a nonnegative integer value");
      repeat=(size_t)floor(repeat_d);
      if (1.0*repeat-repeat_d!=0) fprintf(stderr,"rounding non integer towards lower integer");
    }
    state_sequent* new_sequent=new state_sequent(repeat);
    new_sequent->parent=NULL;
    
    // read substates and subsequences
    XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* one_child=element->getFirstChild();
    while (one_child!=NULL) {
      if (one_child->getNodeType()==XERCES_CPP_NAMESPACE_QUALIFIER DOMNode::ELEMENT_NODE) {
	state_atom* new_one=state_factory(dynamic_cast<XERCES_CPP_NAMESPACE_QUALIFIER DOMElement*>(one_child));
	if (new_one!=NULL) {
	  state* new_state=dynamic_cast<state*>(new_one);
	  if (new_state!=NULL) {
	    new_state->parent=new_sequent;
	    new_sequent->push_back(new_one);
	  }
	  else {
	    fprintf(stderr,"experiment: found nonstate in sequent element\n");
	    delete new_one;
	  }
	}
      }
      one_child=one_child->getNextSibling();
    }
    return new_sequent;
  }
  else if(strcasecmp(my_name,"ttlout")==0) {
    XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&my_name);
    // read parameters
    ttlout* ttls=new ttlout();
    char* id=get_parameter(element,"id","i",NULL);
    if (id!=NULL) {
      ttls->id=strtol(id,NULL,0);
      XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&id);      
    }
    char* value=get_parameter(element,"value",NULL);
    if (value!=NULL) {
      ttls->ttls=strtoul(value,NULL,0);
      // todo: another error message...
      XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&value);
    }
    char* channel=get_parameter(element,"channel",NULL);
    if (channel!=NULL) {
      char* state_value=get_parameter(element,"state",NULL);
      if (state_value!=NULL) {
	size_t number=strtoul(channel,NULL,0);
	char state_char=(state_value)[0];
	fprintf(stderr, "been here\n");
	XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&state_value);
	if (state_char=='h' || state_char=='1')
	  ttls->ttls.set(number,1);
	else
	  ttls->ttls.set(number,0);
      }
      else {
	fprintf(stderr,"found invalid ttl state: ignoring\n");
	delete ttls;
	ttls=NULL;
      }
      XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&channel);
    }
    return (state_atom*)ttls;
  }
  else if(strcasecmp(my_name,"analogout")==0) {
    XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&my_name);
    analogout* aout=new analogout();
    // read parameters
    char* channel=get_parameter(element,"channel","c","id","i",(char*)NULL);
    if (channel!=NULL) {
      aout->id=strtoul(channel,NULL,0);
      XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&channel);
    }
    char* frequency=get_parameter(element,"frequency","f",(char*)NULL);
    if (frequency!=NULL) {
      aout->frequency=strtod(frequency,NULL);
      XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&frequency);
    }
    char* dac_value=get_parameter(element,"dac_value","d",(char*)NULL);
    if (dac_value!=NULL) {
      aout->dac_value=strtol(dac_value,NULL,0);
      XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&dac_value);
    }

    char* phase=get_parameter(element,"phase","p",(char*)NULL);
    if (phase!=NULL) {
      aout->phase=strtod(phase,NULL);
      XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&phase);
    }
    char* amplitude=get_parameter(element,"amplitude","a",(char*)NULL);
    if (amplitude!=NULL) {
      aout->amplitude=strtod(amplitude,NULL);
      XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&amplitude);
    }
    else
      aout->amplitude=1.0;
    return aout;
  }
  else if(strcasecmp(my_name,"analogin")==0) {
    XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&my_name);
    analogin* ain=new analogin();
    // read parameters
    char* id=get_parameter(element,"id","i",(char*)NULL);
    if (id!=NULL) {
      ain->id=strtoul(id,NULL,0);
      XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&id);
    }
    char* frequency=get_parameter(element,"f","frequency",(char*)NULL);
    if (frequency!=NULL) {
      ain->sample_frequency=strtod(frequency,NULL);
      XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&frequency);
      if (ain->sample_frequency<0) {
    	  delete ain;
    	  throw job_exception("frequency must be non-negative");
      }
    }
    
    char* samples=get_parameter(element,"s","samples",(char*)NULL);
    if (samples!=NULL) {
    	char* samples_startpos=samples;
    	while (*samples_startpos!=0 && isspace(*samples_startpos)) ++samples_startpos;
    	if (*samples_startpos=='-') {
    		XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&samples);
    		delete ain;
    		throw job_exception("number of samples must be non-negative");
    	}
    	ain->samples=strtoul(samples,NULL,0);
    	XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&samples);
    }
    
    char* channels=get_parameter(element,"c","channels",(char*)NULL);
    if (channels!=NULL) {
    	ain->channels = channel_array(strtoul(channels,NULL,0));
		XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&channels);
		ain->nchannels = ain->channels.count();
    } else {
    	ain->channels = channel_array(ADC_M2I_DEFAULT_CHANNELS);
    	ain->nchannels = ain->channels.count();
    }

    ain->sensitivity = new double[ain->nchannels]; // initialize sensitivity array
    ain->offset = new int[ain->nchannels]; // initialize offset array (offset is in % of sensitivity)
    ain->impedance = new double[ain->nchannels]; // initialize impedance array
    // get parameters for each channel
    for (int i = 0; i < ain->nchannels; i++) {
    	if (ain->channels[i] == true) { // check if channel bit is true

    		/* get sensitivity */
			char buffer1[100];
			char buffer2[100];
			sprintf(buffer1, "sen%i", i);
			sprintf(buffer2, "sensitivity%i", i);
			char* sensitivity = get_parameter(element, buffer1, buffer2,(char*)NULL);
			if (sensitivity!=NULL) {
				ain->sensitivity[i] = strtod(sensitivity,NULL);
				XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&sensitivity);
			} else {
				ain->sensitivity[i] = ADC_M2I_DEFAULT_SENSITIVITY; // set to default
			}

			/* get offset */
			sprintf(buffer1, "offset%i", i);
			char* offset = get_parameter(element, buffer1, (char*)NULL);
			if (offset!=NULL) {
				ain->offset[i] = strtol(offset, NULL, 0);
				XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&offset);
			} else {
				ain->offset[i] = ADC_M2I_DEFAULT_OFFSET; // default is 0
			}

			/* get impedance */
			sprintf(buffer1, "impedance%i", i);
			char* impedance = get_parameter(element, buffer1, (char*)NULL);
			if (impedance!=NULL) {
				ain->impedance[i] = strtoul(impedance, NULL, 0);
				XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&impedance);
			} else {
				ain->impedance[i] = ADC_M2I_DEFAULT_IMPEDANCE; // default is 1 MOhm
			}
		} else { // channel is not enabled, set to defaults
			ain->sensitivity[i] = ADC_M2I_DEFAULT_SENSITIVITY;
			ain->offset[i] = ADC_M2I_DEFAULT_OFFSET;
			ain->impedance[i] = ADC_M2I_DEFAULT_IMPEDANCE;
		}
		
    }
    
    char* resolution=get_parameter(element,"r","res","resolution",(char*)NULL);
    if (resolution!=NULL) {
    	ain->resolution=strtoul(resolution,NULL,0);
    	XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&resolution);
    }
    else
    	ain->resolution=ADC_M2I_DEFAULT_RESOLUTION; // was 12
    
    return (state_atom*)ain;
  }
  
  return NULL;
}

result* experiment::do_it(hardware* hw) {
  if (experiment_states==NULL) {
    return new error_result(job_no,"did not find any states in job file");
  }
  fprintf(stderr,"performing experiment job no %" SIZETPRINTFLETTER ", number of exps is %i\n",job_no, experiment_states->size());
  result* data=hw->experiment(*experiment_states);
  fprintf(stderr,"finished experiment job no %" SIZETPRINTFLETTER "\n\n",job_no);
  if (data==NULL)
    return new error_result(job_no,"did not get a result from method result* hardware::experiment(const state&) ");
  data->description=description;
  data->job_no=job_no;
    
  return data;
}

configuration::configuration(size_t n, XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* conf_data ):job(n) {
  XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* one_child=conf_data->getFirstChild();

  while (one_child!=NULL) {
    if (one_child->getNodeType()==XERCES_CPP_NAMESPACE_QUALIFIER DOMNode::ELEMENT_NODE) {
      // found an element
      configuration_device_section new_one;
      // copy name
      char* dev_name=XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode(one_child->getNodeName());
      new_one.name=dev_name;
      XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&dev_name);
      
      // copy attributes
      XERCES_CPP_NAMESPACE_QUALIFIER DOMNamedNodeMap* dev_attribs=one_child->getAttributes();
      if (dev_attribs!=NULL) {
	
	XMLSize_t i=0;
	while (1) {
	  XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* dev_attrib=dev_attribs->item(i);
	  if (dev_attrib==NULL) break;
	  char* dev_attrib_name=XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode(dev_attrib->getNodeName());
	  char* dev_attrib_value=XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode(dev_attrib->getNodeValue());
	  new_one.attributes[dev_attrib_name]=dev_attrib_value;
	  XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&dev_attrib_name);
	  XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&dev_attrib_value);
	  dev_attrib->getNodeValue();
	  ++i;
	}
      }

      // now copy string contents
      new_one.data="";
      XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* child_child=one_child->getFirstChild();
      while (child_child!=NULL) {
	if (child_child->getNodeType()==XERCES_CPP_NAMESPACE_QUALIFIER DOMNode::TEXT_NODE ||
	    child_child->getNodeType()==XERCES_CPP_NAMESPACE_QUALIFIER DOMNode::CDATA_SECTION_NODE) {
	  char* text_data=XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode(child_child->getNodeValue());
	  new_one.data.append(text_data);
	  XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&text_data);	  
	}
	child_child=child_child->getNextSibling();
      }
      configuration_changes.push_back(new_one);
    }
    one_child=one_child->getNextSibling();
  }  
}

result* configuration::do_it(hardware* hw) {
  result* res=hw->configure(configuration_changes);
  configuration_result* cres=dynamic_cast<configuration_result*>(res);
  if (cres!=NULL)
    cres->job_no=job_no;
  else {
    configuration_results* cress=dynamic_cast<configuration_results*>(res);
    if (cress!=NULL) {
      cress->job_no=job_no;
      for(configuration_results::iterator i=cress->begin(); i!=cress->end(); ++i)
	(*i)->job_no=job_no;
    }
  }
  return res;
}

void configuration_device_section::print(FILE* f) const {
  fprintf(f,"name: %s\n",name.c_str());
  for (std::map<const std::string, std::string>::const_iterator j=attributes.begin();
       j!=attributes.end();
       ++j) {
    fprintf(f,"  %s: %s\n",j->first.c_str(),j->second.c_str());
  } // j
  fprintf(f,"data: %s\n", data.c_str());
}

void configuration::print(FILE* f) const {
  for (std::list<configuration_device_section>::const_iterator i=configuration_changes.begin();
       i!=configuration_changes.end();
       ++i) {
    i->print(f);
  } // i
}
