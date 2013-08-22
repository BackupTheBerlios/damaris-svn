#include "xml_result.h"
#include <cstdlib>
#include <xercesc/util/Base64.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOMWriter.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>

/* ***************************************************************************************************

here starts the reader implementation

*****************************************************************************************************/
#ifndef SIZETPRINTFLETTER
#  ifndef __LP64__
#    define SIZETPRINTFLETTER "u"
#  else
#    define SIZETPRINTFLETTER "lu"
#  endif
#endif

void start_element_stub(void* userdata,const XML_Char* name, const XML_Char** attrs );
void end_element_stub(void* userdata,const XML_Char* name);
void data_stub(void* userdata,  const XML_Char* data, int len);

/**
   table for translation base64 -> 0 to 63
*/
static char base64_table[] = {
  -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
  -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
  -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,62, -1,-1,-1,63,
  52,53,54,55, 56,57,58,59, 60,61,-1,-1, -1, 0,-1,-1,
  -1, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
  15,16,17,18, 19,20,21,22, 23,24,25,-1, -1,-1,-1,-1,
  -1,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,
  41,42,43,44, 45,46,47,48, 49,50,51,-1, -1,-1,-1,-1
};


class xml_result_handler {
  /**
   */
  int alloc_stepsize;
  /**
     maximum allocated size of buffer list
   */
  size_t bufferno_max;
  /**
     actual or new sample buffer
   */
  size_t bufferno;
  /**
     buffers
   */
  short int** buffers;
  /**
     number of samples (2 values for one sample) in each buffer, if actual buffer, this is the expected number
   */
  size_t* sampleno;
  /**
   */
  double* frequencies;
  /**
     maximum allocated size of last sample buffer in bytes
   */
  size_t bufferlength;
  /**
     position of in actual sample buffer
   */
  size_t bufferpos;
  /**
     base64 byte buffer
   */
  char base64buffer[4];
  /**
     the length of content in base64buffer
   */
  int base64buffer_length;
  /**
     1 if we are inside a adcdata section, 2 if we are saving a description
   */
  int section_state;
  /**
     error messages, that are found
   */
  std::string error_message;
  /**
     descripton sections
   */
  std::string description_text;

public:
  xml_result_handler(XML_Parser p) {
    /* set event handlers */
    alloc_stepsize=1<<10;
    XML_SetElementHandler(p,start_element_stub,end_element_stub);
    XML_SetCharacterDataHandler(p,data_stub);
    XML_SetUserData(p,this);

    /* allocate some buffers */
    sampleno=(size_t*)malloc(alloc_stepsize*sizeof(size_t));
    buffers=(short int**)malloc(alloc_stepsize*sizeof(short int*));
    frequencies=(double*)malloc(alloc_stepsize*sizeof(double));
    if (sampleno==NULL || buffers==NULL) {
      if (sampleno!=NULL) free(sampleno);
      if (buffers!=NULL) free(buffers);
      if (frequencies!=NULL) free(frequencies);
      /* todo throw something */
      return;
    }
    bufferno=0;
    buffers[0]=NULL;
    bufferno_max=alloc_stepsize;
    section_state=0;
  }

  void start_element(const XML_Char* name, const XML_Char** attrs) {
    if (strcmp(name,"adcdata")==0) {
      /* is there enough space in array? */
      if (bufferno>=bufferno_max) {
	bufferno_max+=alloc_stepsize;
	buffers=(short int**)realloc((void*)buffers,bufferno_max*sizeof(short int*));
	sampleno=(size_t*)realloc((void*)sampleno,bufferno_max*sizeof(size_t));
	frequencies=(double*)realloc((void*)frequencies,bufferno_max*sizeof(double));
	if (buffers==NULL || sampleno==NULL || frequencies==NULL) fprintf(stderr,"could not reallocate the buffer");
      }
      /* todo read sample no*/
      const XML_Char** pos=attrs;
      while (*pos!=NULL) {
	if (strcmp(*pos,"samples")==0 || strcmp(*pos,"s")==0) {
	  sampleno[bufferno]=strtoul(*(pos+1),NULL,0);
	}
	else if (strcmp(*pos,"frequency")==0 || strcmp(*pos,"f")==0) {
	  frequencies[bufferno]=strtod(*(pos+1),NULL);
	}
	pos+=2;
      }
      if (sampleno[bufferno]!=0) {
	bufferlength=sampleno[bufferno]*2*sizeof(short int);
	/* fresh buffer */
	buffers[bufferno]=(short int*)malloc(bufferlength);
      }
      else {
	bufferlength=0;
	buffers[bufferno]=NULL;
      }
      bufferpos=0;
      /* todo check result */
      base64buffer_length=0;
      section_state=1;
    }
    if (strcmp(name,"description")) {
      section_state=3;
    }
    if (strcmp(name,"error")==0) {
      section_state=2;
    }
    if (section_state==3) {
      description_text+="<";
      description_text+=name;
      const XML_Char** attr=attrs;
      while(*attr!=NULL) {
	description_text+=" ";
	description_text+=*(attr++);
	description_text+="='";
	description_text+=*(attr++);
	description_text+="'";
      }
      description_text+=">";
    } 
  }

  void end_element(const XML_Char* name) {
    if (strcmp(name,"adcdata")==0) {
      /* empty base64 buffer */
      if (bufferpos+base64buffer_length-1>=bufferlength) {
	bufferlength+=alloc_stepsize;
	buffers[bufferno]=(short int*)realloc(buffers[bufferno],bufferlength);
	/* todo reallocate check */
      }

      if (base64buffer_length>1) {
	unsigned char s=base64buffer[0];
	s<<=2;
	s|=(base64buffer[1]>>4);
	((char*)buffers[bufferno])[bufferpos]=s;
	bufferpos++;
	if (base64buffer_length>2) {
	  s=base64buffer[1]&15;
	  s<<=4;
	  s|=(base64buffer[2]>>2);
	  ((char*)buffers[bufferno])[bufferpos]=s;
	  bufferpos++;
	  if (base64buffer_length>3) {
	    s=base64buffer[2]&3;
	    s<<=6;
	    s|=base64buffer[3];
	    ((char*)buffers[bufferno])[bufferpos]=s;
	    bufferpos++;
	  }
	}
      }
      /* check right sample number */
      if (bufferpos!=sampleno[bufferno]*2*sizeof(short int)) {
	sampleno[bufferno]=bufferpos/(2*sizeof(short int));
      }
      /* prepare for next buffer */
      bufferlength=0;
      bufferno++;
      section_state=0;
    }
    if (strcmp(name,"error")==0) {
      if (!error_message.empty() && error_message[error_message.size()-1]!='\n') {
	error_message.push_back('\n');
      }
      section_state=0;
    }
    if (section_state==3) {
      description_text+="<";
      description_text+=name;
      description_text+="/>";
    }
    if (strcmp(name,"description")) {
      section_state=0;
    }
  }

  void data(const XML_Char* the_data, int len) {
    if (section_state==1) {
      int pos=0;
      while (pos<len) {
	/* fill buffer */
	while (base64buffer_length<4 && pos<len) {
	  if (((unsigned char)the_data[pos])<128) {
	    char value=base64_table[(unsigned char)the_data[pos]];
	    if (value!=-1) {
	      base64buffer[base64buffer_length]=value;
	      base64buffer_length++;
	    }
	  }
	  pos++;
	}
	/* interprete buffer, that has 4*6 bits=3 byte */
	if (base64buffer_length==4) {
	  /* check the space */
	  if (bufferpos+3>=bufferlength) {
	    /* to do reallocate */
	    bufferlength+=alloc_stepsize;
	    buffers[bufferno]=(short int*)realloc(buffers[bufferno],bufferlength);
	    if (buffers[bufferno]==NULL) fprintf(stderr,"could not enlarge the sample buffer\n");
	  }
	  /* read three bytes from four letters */
	  unsigned char s=base64buffer[0];
	  s<<=2;
	  s|=(base64buffer[1]>>4);
	  ((char*)buffers[bufferno])[bufferpos]=s;
	  bufferpos++;
	  s=base64buffer[1]&15;
	  s<<=4;
	  s|=(base64buffer[2]>>2);
	  ((char*)buffers[bufferno])[bufferpos]=s;
	  bufferpos++;
	  s=base64buffer[2]&3;
	  s<<=6;
	  s|=base64buffer[3];
	  ((char*)buffers[bufferno])[bufferpos]=s;
	  bufferpos++;
	  base64buffer_length=0;
	}
      }
    }
    else if (section_state==3) {
      description_text+=the_data;
    }
    else if (section_state==2) {
      error_message+=the_data;
    }
  }

  ~xml_result_handler() {
    if (sampleno!=NULL && buffers!=NULL) {
      size_t pos=0;
      while (pos<=bufferno) {
	if (buffers[pos]!=NULL) free(buffers[pos]);
	pos++;
      }
    }
    if (sampleno!=NULL) free(sampleno);
    if (buffers!=NULL) free(buffers);
  }


  /**
     print number of samples and frequency of the obtained data
  */
  void print_info(FILE* f) {
    if (buffers==NULL || sampleno==NULL) {
      fprintf(f,"nothing saved\n");
      return;
    }
    for (size_t i=0; i<bufferno; i++) {
      fprintf(f,"%" SIZETPRINTFLETTER ": %" SIZETPRINTFLETTER " samples %f Hz frequency\n",i,sampleno[i],frequencies[i]);
      if (buffers[i]!=NULL) {
	for (size_t j=0; j<sampleno[i]; j++) {
	  short int a=buffers[i][j*2];
	  short int b=buffers[i][j*2+1];
	  fprintf(f,"%hd %hd\n",a,b);
	}
      }
    }
  }


};

void start_element_stub(void* userdata,const XML_Char* name, const XML_Char** attrs ) {
  ((xml_result_handler*)userdata)->start_element(name,attrs);
}

void end_element_stub(void* userdata,const XML_Char* name) {
  ((xml_result_handler*)userdata)->end_element(name);
}

void data_stub(void* userdata,  const XML_Char* data, int len ) {
  ((xml_result_handler*)userdata)->data(data,len);
}


xml_result_reader::xml_result_reader() {
  jobfilenamepattern="";
}

xml_result_reader::~xml_result_reader() {
}

xml_result_reader::xml_result_reader(const std::string& jobpattern) {
  jobfilenamepattern=jobpattern;
}

/* ToDo error handling */
result* xml_result_reader::read(size_t no) {
  if (jobfilenamepattern=="") return NULL;
  char* tmp_name=(char*)malloc(jobfilenamepattern.size()+50);
  if (tmp_name==NULL) return NULL;
  snprintf(tmp_name,jobfilenamepattern.size()+50,jobfilenamepattern.c_str(),no);
  result* new_result=NULL;
  new_result=read(std::string(tmp_name));
  free(tmp_name);
  return new_result;
}

result* xml_result_reader::read(const std::string& jobfilename) {
  /*open file*/
  FILE* result_file=fopen(jobfilename.c_str(),"rb");
  if (result_file==NULL) return NULL;

  XML_Parser parser=XML_ParserCreate(NULL);
  if (parser==NULL) {
    fclose(result_file);
    return NULL;
  }

  xml_result_handler h(parser);

  for (;;) {
    const size_t BUFF_SIZE=1<<10;
    int bytes_read;
    void *buff = XML_GetBuffer(parser, BUFF_SIZE);
    if (buff == NULL) {
      fprintf(stderr, "could not get the buffer from XML Parser\n");
      fclose(result_file);
      return NULL;
    }

    bytes_read = fread(buff, 1, BUFF_SIZE, result_file);
    if (bytes_read < 0) {
      /* handle error */
      fprintf(stderr,"error while reading\n");
      break;
    }

    if (! XML_ParseBuffer(parser, bytes_read, bytes_read == 0)) {
      /* handle parse error */
      fprintf(stderr,"error while parsing line %d: %s\n",static_cast<int>(XML_GetCurrentLineNumber(parser)),XML_ErrorString(XML_GetErrorCode(parser)));
      break;
    }

    // peaceful end
    if (bytes_read == 0) break;
  }

  fclose(result_file);
  return NULL;
}

/* ************************************************************************************************

here starts the writer implementation

**************************************************************************************************/

xml_result_writer::xml_result_writer(data_save_mode_type how) {
  resultfilename_pattern="";
  data_save_mode=how;
}

xml_result_writer::xml_result_writer(const std::string& pattern, data_save_mode_type how) {
  resultfilename_pattern=pattern;
  data_save_mode=how;  
}


int xml_result_writer::write_to_file(const std::string& filename, const result* res) const {
  // find the way to write
  const adc_result* adc_res=dynamic_cast<const adc_result*>(res);
  if (adc_res!=NULL) {
    write_adc_to_file(filename,adc_res);
    return 0;
  }
  const adc_results* adc_ress=dynamic_cast<const adc_results*>(res);
  if (adc_ress!=NULL) {
    write_adcs_to_file(filename, adc_ress);
    return 0;
  }
  const configuration_results* config_ress=dynamic_cast<const configuration_results*>(res);
  if (config_ress!=NULL) {
    write_configuration_results_to_file(filename, *config_ress);
    return 0;
  }
    
  const error_result* err_res=dynamic_cast<const error_result*>(res);
  if (err_res!=NULL) write_error_to_file(filename,err_res);
    else
      write_unknown_to_file(filename, res);
  return 0;
}

int xml_result_writer::write_unknown_to_file(const std::string& filename, const result* res) const {
  FILE* out=fopen(filename.c_str(),"w");
  if (out==0) fprintf(stderr,"could not open file %s\n",filename.c_str());
  fprintf(out,"<?xml version=\"1.0\"?>\n");
  fprintf(out,"<result job=\"%" SIZETPRINTFLETTER "\">\n",res->job_no);
  if (res==NULL)
      fprintf(out,"<!-- got NULL pointer result... -->\n");
  else
      fprintf(out,"<!-- don't know how to print result... -->\n");
  fprintf(out,"</result>\n");
  fclose(out);
  return 0;
}


int xml_result_writer::write_error_to_file(const std::string& filename, const error_result* res) const {
  /* write an extra message to stderr */
  fprintf(stderr,"job %" SIZETPRINTFLETTER ": %s\n",res->job_no,res->error_message.c_str());
  FILE* out=fopen(filename.c_str(),"w");
  if (out==0) {
    fprintf(stderr,"could not open file %s\n",filename.c_str());
    return 0;
  }
  fprintf(out,"<?xml version=\"1.0\"?>\n");
  fprintf(out,"<result job=\"%" SIZETPRINTFLETTER "\">\n",res->job_no);
  fprintf(out," <error>%s</error>\n",res->error_message.c_str());
  fprintf(out,"</result>");
  fclose(out);
  return 0;
}

int xml_result_writer::write_configuration_results_to_file(const std::string& filename, const configuration_results& ress) const {

  FILE* out=fopen(filename.c_str(),"w");
  fprintf(out,"<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
  fprintf(out,"<result job=\"%" SIZETPRINTFLETTER "\">\n<configuration>\n",ress.job_no);
  XMLCh tempStr[100];
  XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode("LS", tempStr, 99);
  XERCES_CPP_NAMESPACE_QUALIFIER DOMImplementation *impl2=XERCES_CPP_NAMESPACE_QUALIFIER DOMImplementationRegistry::getDOMImplementation(tempStr);
  XERCES_CPP_NAMESPACE_QUALIFIER DOMWriter *theSerializer=((XERCES_CPP_NAMESPACE_QUALIFIER  DOMImplementationLS*)impl2)->createDOMWriter();  
  for (configuration_results::const_iterator i=ress.begin(); i!=ress.end(); ++i)
    if ((*i)->tag->getDocumentElement()!=NULL) {
      XERCES_CPP_NAMESPACE_QUALIFIER MemBufFormatTarget mem;
      theSerializer->writeNode(&mem,*((*i)->tag->getDocumentElement()));
      fwrite((char*)mem.getRawBuffer(),sizeof(char),mem.getLen(),out);
      mem.reset();
    }
  theSerializer->release();
  
  // todo
  fprintf(out,"</configuration>\n</result>\n");
  fclose(out);
  return 0;
}

int xml_result_writer::write_adcs_to_file(const std::string& filename, const adc_results* ress) const {
  // fall back to some default mode
  data_save_mode_type how=data_save_mode;
  if (how==defaultmode) how=ascii;
  FILE* out=fopen(filename.c_str(),"w");
  if (out==0) {
    fprintf(stderr,"could not open file %s\n",filename.c_str());
    return 0;
  }
  fprintf(out,"<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
  fprintf(out,"<result job=\"%" SIZETPRINTFLETTER "\">\n",ress->job_no);
  fwrite(ress->description.c_str(),ress->description.size(),1,out);
  fprintf(out,"\n");
  int num_res=0;
  for (adc_results::const_iterator res=ress->begin(); res!=ress->end(); ++res) {
    switch (how) {
    case separate_file:
      {
	// write separate data file
	char result_filename[1<<10];
	snprintf(result_filename,sizeof(result_filename),"adc.%09" SIZETPRINTFLETTER ".%d.bin",ress->job_no,num_res);
	write_adcdata_separate(out, std::string(result_filename), *res);
      }
      break;
    case base64:
      write_adcdata_base64(out,*res);
      break;
    case csv:
      write_adcdata_formated(out,"%d; %d\n",*res);
      break;
    case ascii:
      write_adcdata_formated(out,"%d\t%d\n",*res);
      break;
    default:
      /* forget data */
      fprintf(stderr,"forgeting adc data\n");
    }
    num_res++;
  }
  fprintf(out,"</result>\n");
  fclose(out);
  return 0;
}

int xml_result_writer::write_adc_to_file(const std::string& filename, const adc_result* res) const {
  // fall back to some default mode
  data_save_mode_type how=data_save_mode;
  if (how==defaultmode) how=ascii;
  FILE* out=fopen(filename.c_str(),"w");
  if (out==0) {
    fprintf(stderr,"could not open file %s\n",filename.c_str());
    return 0;
  }
  fprintf(out,"<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
  fprintf(out,"<result job=\"%" SIZETPRINTFLETTER "\">\n",res->job_no);
  fwrite(res->description.c_str(),res->description.size(),1,out);
  fprintf(out,"\n");
  switch (how) {
  case separate_file:
    {
      // write separate data file
      write_adcdata_separate(out, filename+".bin", res);
    }
    break;
  case base64:
    write_adcdata_base64(out,res);
    break;
  case csv:
    write_adcdata_formated(out,"%d; %d\n",res);
    break;
  case ascii:
    write_adcdata_formated(out,"%d\t%d\n",res);
    break;
  default:
    /* forget data */
    fprintf(stderr,"forgeting adc data\n");
  }
  fprintf(out,"</result>\n");
  fclose(out);
  return 0;
}

int xml_result_writer::write_adcdata_separate(FILE* out, const std::string& datafilename, const adc_result* res) const {
  /* todo: error handling */
  FILE* binout=fopen(datafilename.c_str(),"wb");
  if (binout==0) {
    fprintf(stderr,"could not open file %s\n",datafilename.c_str());
    return 0;
  }
  fwrite(res->data, sizeof(short int)*res->nchannels, res->samples, binout);
  fclose(binout);
  fprintf(out,
	  "<adcdatafile path=\"%s\" samples=\"%" SIZETPRINTFLETTER "\" rate=\"%g\" channels=\"%i\"/>\n",
	  datafilename.c_str(),
	  res->samples,
	  res->sampling_frequency,
	  res->nchannels);
  return 0;
}

int xml_result_writer::write_adcdata_formated(FILE* out, const std::string& format, const adc_result* res) const {
  fprintf(out,"<adcdata samples=\"%" SIZETPRINTFLETTER "\" rate=\"%g\" channels=\"%i\">\n",res->samples,res->sampling_frequency, res->nchannels);
  for(size_t i=0;i<res->samples;++i) {
    for (int j = 0; j < res->nchannels; j++) {
        fprintf(out, format.c_str(), res->data[i*2 + j]);
    }
  }
  fprintf(out,"</adcdata>\n");
  return 0;
}

int xml_result_writer::write_adcdata_base64(FILE* out, const adc_result* res) const {
  fprintf(out,"<adcdata samples=\"%" SIZETPRINTFLETTER "\" rate=\"%g\" channels=\"%i\">\n",res->samples,res->sampling_frequency, res->nchannels);
  unsigned int base64length=0;
  XMLByte* base64buffer=XERCES_CPP_NAMESPACE_QUALIFIER Base64::encode((XMLByte*)res->data,res->samples*res->nchannels*sizeof(short int),&base64length);
  fwrite(base64buffer,1,base64length,out);
  XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&base64buffer);
  fprintf(out,"</adcdata>\n");
  return 0;
}
