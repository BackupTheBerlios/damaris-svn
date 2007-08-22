/*
  Achim Gaedke, TU Darmstadt, September 2004
  add_endline filename

  this tool appends an appropriate endline sequence, if file does not end with a \n or \r
  the added sequence is determined by the first line end sequence (\n, \r, \n\r or \r\n)
  if no line break is found, \n will be used
*/

#include <cstdio>

int main(int argc, char** argv) {
  /* get filename and open file for reading */
  if (argc!=2) {
    fprintf(stderr,"%s filename\n adds endline/carrige return to c code if necessary\n",argv[0]);
    return 1;
  }
  FILE* inspect_file=fopen(argv[1],"r+");
  if (inspect_file==NULL) {
    fprintf(stderr, "%s: could not open file %s",argv[0],argv[1]);
    return 1;
  }
  /* determine, if file ends without newline/carrige return */
  fseek(inspect_file, -2, SEEK_END);
  
  char lineend[3];
  size_t charread=fread(lineend, 1, 2, inspect_file);
  // nothing inside? fileend?
  if (charread==0) return 0;
  // is this a file with line end?
  if (lineend[charread-1]=='\n' || lineend[charread-1]=='\r') {
    fclose(inspect_file);
    return 0;
  }

  /* if not, determine cr/nl file type */
  fseek(inspect_file, 0, SEEK_SET);
  while (1==(charread=fread(lineend,1,1,inspect_file)) && lineend[0]!='\r' && lineend[0]!='\n') {}
  if (charread!=1) {
    // could not find a newline, just using \n
    lineend[0]='\n';
    lineend[1]='\0';
  }
  else {
    charread=fread(lineend+1,1,1,inspect_file);
    if (charread!=1 || lineend[0]==lineend[1] || (lineend[1]!='\n' && lineend[1]!='\r'))
      lineend[1]='\0';
    else
      lineend[2]='\0';
  }
  fclose(inspect_file);

  /* fix file by appending cr/nl */
  inspect_file=fopen(argv[1],"a+");
  if (inspect_file==NULL) {
    fprintf(stderr,"%s could not open file %s for appending newlines\n",argv[0],argv[1]);
  }
  fprintf(inspect_file,lineend);
  fclose(inspect_file);

  return 0;
}
