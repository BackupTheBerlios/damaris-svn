/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#include "states.h"
#include "xml_states.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>

void xml_state_reader_startelement_handler(xml_state_reader* self,
					   const XML_Char *name,
					   const XML_Char **atts ) {
  self->start_element(name,atts);
}

void xml_state_reader_endelement_handler(xml_state_reader* self,
					   const XML_Char *name) {
  self->end_element(name);
}

const XML_Char* xml_state_reader::search_attribute(const XML_Char** atts,const XML_Char* attr_name) const {
  if (atts==NULL || atts[0]==NULL) return NULL;
  size_t attr_idx=0;
  while (strcmp(atts[attr_idx],attr_name)!=0) {
    attr_idx+=2;
    if (atts[attr_idx]==NULL) return NULL;
  }
  return atts[attr_idx+1];
}

const XML_Char* xml_state_reader::search_attributes(const XML_Char** atts, ...) const {
  if (atts==NULL || atts[0]==NULL) return NULL;
  va_list names;
  va_start(names,atts);
  const XML_Char* attr_name=va_arg(names, const XML_Char*);
  while (attr_name!=NULL) {
    size_t attr_idx=0;
    while (atts[attr_idx]!=NULL && strcmp(atts[attr_idx],attr_name)!=0)
      attr_idx+=2;
    if (atts[attr_idx]!=NULL) {
      va_end(names);
      return atts[attr_idx+1];
    }
    attr_name=va_arg(names, const XML_Char*);
  }
  va_end(names);
  return NULL;
}



state_atom* xml_state_reader::state_factory(const XML_Char *name,
					   const XML_Char **atts) const {
  if (strcmp(name,"state")==0) {
    state* s=new state(1);
    const XML_Char* length=search_attributes(atts,"length","time",(XML_Char*)NULL);
    if (length!=NULL) s->length=strtod(length,NULL);
    return s;
  }
  if (strcmp(name,"sequent")==0) {
    state_sequent* s=new state_sequent();
    const XML_Char* repeat=search_attribute(atts,"repeat");
    if (repeat!=NULL)
      s->repeat=strtoul(repeat,0,0);
    else
      s->repeat=1;
    return s;
  }
  if (strcmp(name,"parallel")==0) {
    return new state_parallel();
  }
  if (strcmp(name,"ttlout")==0) {
    ttlout* ttls=new ttlout();
    const XML_Char* id=search_attributes(atts,"id","i",(XML_Char*)NULL);
    if (id!=NULL) ttls->id=strtol(id,NULL,0);
    const XML_Char* value=search_attribute(atts,"value");
    if (value!=NULL) ttls->ttls=strtoul(value,NULL,0);
    const XML_Char* channel=search_attribute(atts,"channel");
    if (channel!=NULL) {
      const XML_Char* state=search_attribute(atts,"state");
      if (state!=NULL) {
	size_t number=strtoul(channel,0,0);
	char state_char=(state)[0];
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
    }
    return (state_atom*)ttls;
  }
  if (strcmp(name,"analogout")==0) {
    analogout* aout=new analogout();
    const XML_Char* channel=search_attributes(atts,"channel","c","id","i",(XML_Char*)NULL);
    const XML_Char* frequency=search_attributes(atts,"frequency","f",(XML_Char*)NULL);
    const XML_Char* phase=search_attributes(atts,"phase","p",(XML_Char*)NULL);
    const XML_Char* amplitude=search_attributes(atts,"amplitude","a",(XML_Char*)NULL);
    if (channel!=NULL) aout->id=strtoul(channel,NULL,0);
    if (frequency!=NULL) aout->frequency=strtod(frequency,NULL);
    if (phase!=NULL) aout->phase=strtod(phase,NULL);
    if (amplitude!=NULL)
      aout->amplitude=strtod(amplitude,NULL);
    else
      aout->amplitude=1.0;
    return (state_atom*)aout;
  }
  if (strcmp(name,"analogin")==0) {
    analogin* ain=new analogin();
    const XML_Char* id=search_attributes(atts,"id","i",(XML_Char*)NULL);
    const XML_Char* frequency=search_attributes(atts,"f","frequency",(XML_Char*)NULL);
    const XML_Char* samples=search_attributes(atts,"s","samples",(XML_Char*)NULL);
    const XML_Char* channels=search_attributes(atts,"c","channels",(XML_Char*)NULL);
    const XML_Char* sensitivity=search_attributes(atts,"sen","sensitivity",(XML_Char*)NULL);
    const XML_Char* resolution=search_attributes(atts,"r","res","resolution",(XML_Char*)NULL);
    if (frequency!=NULL) ain->sample_frequency=strtod(frequency,NULL);
    if (samples!=NULL) ain->samples=strtoul(samples,NULL,0);
    if (id!=NULL) ain->id=strtoul(id,NULL,0);
    if (channels!=NULL)
      ain->channels=strtoul(channels,NULL,0);
    else
      ain->channels=3;
    if (sensitivity!=NULL)
      ain->sensitivity=strtod(sensitivity,NULL);
    else
      ain->sensitivity=5.0;
    if (resolution!=NULL)
      ain->resolution=strtoul(resolution,NULL,0);
    else
      ain->resolution=12;
    return (state_atom*)ain;
  }
  fprintf(stderr,"unknown tag %s found\n",name);
  return NULL;
}

void xml_state_reader::start_element(const XML_Char *name,
				     const XML_Char **atts) {
  state_atom* new_element=state_factory(name,atts);
  if (root==NULL) 
    root=new_element;
  if (new_element!=NULL && !pending_elements.empty()) {
    state* container=dynamic_cast<state*>(pending_elements.back());
    if (container!=NULL)
      container->push_back(new_element);
    else
      fprintf(stderr,"found invalid nested element %s\n",name);
  }
  pending_elements.push_back(new_element);
}

void xml_state_reader::end_element(const XML_Char *name) {
  pending_elements.pop_back();
}

xml_state_reader::xml_state_reader() {
  root=(state_atom*)NULL;
  parser=XML_ParserCreate((XML_Char*)NULL);
  XML_SetUserData(parser,(void*)this);
  XML_SetElementHandler(parser,
			(XML_StartElementHandler)xml_state_reader_startelement_handler,
			(XML_EndElementHandler)xml_state_reader_endelement_handler);
}

state_atom* xml_state_reader::read_from_string(const std::string& data) {
  if (!XML_Parse(parser,data.c_str(), data.size(),1)) {
    /* handle parse error */
    fprintf(stderr,"error while parsing\n");
    delete root;
    root=NULL;
    return NULL;
  }
  state_atom* return_value=root;
  root=NULL;
  return return_value;
}

state_atom* xml_state_reader::read_from_file(const std::string& filename) {
  int docfd=open(filename.c_str(),O_RDONLY);
  for (;;) {
    const size_t BUFF_SIZE=1<<12;
    int bytes_read;
    void *buff = XML_GetBuffer(parser, BUFF_SIZE);
    if (buff == NULL) {
      fprintf(stderr, "could not get the buffer from XML Parser\n");
      close(docfd);
      return NULL;
    }

    bytes_read = read(docfd, buff, BUFF_SIZE);
    if (bytes_read < 0) {
      /* handle error */
      fprintf(stderr,"error while reading\n");
      if (root!=NULL) delete root;
      root=NULL;
      break;
    }

    if (! XML_ParseBuffer(parser, bytes_read, bytes_read == 0)) {
      /* handle parse error */
      fprintf(stderr,"error while parsing\n");
      if (root!=NULL) delete root;
      root=NULL;
      break;
    }

    // peaceful end
    if (bytes_read == 0) break;
  };
  close(docfd);

  state_atom* return_value=root;
  root=NULL;
  return return_value;
}

xml_state_reader::~xml_state_reader(){
  XML_ParserFree(parser);
  if (root!=NULL) delete root;
}

int xml_state_writer::write_states(FILE* output, const state_atom& states_to_write, int add_header, int indent_size) {
  if (add_header) {
    fprintf(output,"<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
  }

  std::string indent_string(indent_size,' ');

  const state_sequent* ss=dynamic_cast<const state_sequent*>(&states_to_write);
  if (ss!=NULL) {
    if (ss->repeat==1)
      fprintf(output,"%s<sequent>\n",indent_string.c_str());
    else
      fprintf(output,"%s<sequent repeat=\"%u\">\n",indent_string.c_str(),ss->repeat);
    for (state::const_iterator i=ss->begin();i!=ss->end(); i++) {
      write_states(output,**i,0,indent_size+indent_increase);
    }
    fprintf(output,"%s</sequent>\n",indent_string.c_str());
    return 1;
  }
  const state_parallel* sp=dynamic_cast<const state_parallel*>(&states_to_write);
  if (sp!=NULL){
    fprintf(output,"%s<parallel>\n",indent_string.c_str());
    for (state::const_iterator i=sp->begin();i!=sp->end(); i++) {
      write_states(output,**i,0,indent_size+indent_increase);
    }
    fprintf(output,"%s</parallel>\n",indent_string.c_str());
    return 1;
  }
  const state* st=dynamic_cast<const state*>(&states_to_write);
  if (st!=NULL) {
    fprintf(output,"%s<state length=\"%g\">\n",indent_string.c_str(),st->length);
    for (state::const_iterator i=st->begin();i!=st->end(); i++) {
      write_states(output,**i,0,indent_size+indent_increase);
    }
    fprintf(output,"%s</state>\n",indent_string.c_str());
    return 1;
  }
  const ttlout* ttlo=dynamic_cast<const ttlout*>(&states_to_write);
  if (ttlo!=NULL) {
    fprintf(output,"%s<ttlout value=\"0x%lx\"/>\n",
	    indent_string.c_str(),
	    ttlo->ttls.to_ulong());
    return 1;
  }
  const analogout* ao=dynamic_cast<const analogout*>(&states_to_write);
  if (ao!=NULL) {
    fprintf(output,
	    "%s<analogout id=\"%d\" frequency=\"%g\" amplitude=\"%g\" phase=\"%g\"/>\n",
	    indent_string.c_str(),
	    ao->id,
	    ao->frequency,
	    ao->amplitude,
	    ao->phase);
    return 1;
  }
  const analogin* ai=dynamic_cast<const analogin*>(&states_to_write);
  if (ai!=NULL) {
    fprintf(output,
	    "%s<analogin id=\"%d\" samples=\"%u\" sample_frequency=\"%g\" channels=\"%lu\" sensitivity=\"%g\" resolution=\"%u\"/>\n",
	    indent_string.c_str(),
	    ai->id,
	    ai->samples,
	    ai->sample_frequency,
	    ai->channels.to_ulong(),
	    ai->sensitivity,
	    ai->resolution);
    return 1;
  }
  fprintf(output,"%s<!-- something missing -->\n",indent_string.c_str());
  return 0;
}
