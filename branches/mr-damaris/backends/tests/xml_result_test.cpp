#include "core/result.h"
#include "core/xml_result.h"
#include <cstdio>
#include <string>

int main(int argc, char** argv) {

  if (argc>2) {
    if (strcmp("-r",argv[1])==0) {
      xml_result_reader r;
      result* res=r.read(std::string(argv[2]));
      return 0;
    }
    if (strcmp("-w",argv[1])==0) {
      size_t samples=4096;
      short int* buffer=(short int*)malloc(sizeof(short int)*2*samples);
      for (size_t i=0; i<samples; i++) {buffer[i*2]=(short int)i; buffer[i*2+1]=(short int)i;}
      adc_result(0,samples,buffer).write_to_file(std::string(argv[2]),adc_result::base64);
      return 0;
    }
  }
  
  fprintf(stderr,"%s: [-r|-w] filename\n",argv[0]);
  return 1;

}
