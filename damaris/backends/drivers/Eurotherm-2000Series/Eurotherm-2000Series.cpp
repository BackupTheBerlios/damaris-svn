/* **************************************************************************

 Author: Holger Stork, Achim Gaedke
 Created: January 2005

****************************************************************************/

#include "Eurotherm-2000Series.h"
#include <cstdio>
#include <cmath>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <locale.h>

Eurotherm2000Series::Eurotherm2000Series(const std::string& dev_name, int dev_address, int failure_status_mask): tempcont() {
  /*
    initialization of serial device
  */
  device_name=dev_name;
  failure_mask=failure_status_mask;
  serial_dev=open(dev_name.c_str(),O_RDWR|O_NONBLOCK);
  if (serial_dev<0)
    throw Eurotherm2000Series_error("could not open serial device");
  // set attributes now
  termios the_attributes;
  the_attributes.c_iflag=0;
  the_attributes.c_oflag=0;
  // 7 bit per byte, even parity, reciever enabled
  the_attributes.c_cflag=CS7|PARENB|CREAD;
  the_attributes.c_lflag=0;
  // baud rate
  cfsetospeed(&the_attributes,B19200);
  cfsetispeed(&the_attributes,B19200);
  if(tcsetattr(serial_dev,TCSANOW,&the_attributes)!=0)
    throw Eurotherm2000Series_error("could not configure serial line");
  address=dev_address;
  (void)setlocale(LC_NUMERIC,"C");
  fp_format="%05.1f";
  int_format="%05d.";
  hex_format=">%04x";
  std::map<std::string,std::string> config;
  // configure for °K output
  config["Q1"]="0002.";
  // configure for nnn.n format output/precision
  config["QD"]="0001.";
  configure(config);
  set_history_stepsize(1);
  device_failed=0;
}

void Eurotherm2000Series::configure(const std::map<std::string,std::string>& config) {
  std::string error_message;
  try {
    set_value("IM","0002.");
    for (std::map<std::string,std::string>::const_iterator i=config.begin();i!=config.end();++i) {
      try { set_value(i->first,i->second); }
      catch (Eurotherm2000Series_error e){error_message+=i->first+"->"+i->second+", ";}
    }
    set_value("IM","0000.");
    // read superficial zero byte
    char buffer[1];
    while (read(serial_dev,buffer,1)<=0) sleep(1);
    // wait for restarted communication
    int timeout=10;
    while (1) {
      if (timeout<0) throw Eurotherm2000Series_error("could not establish connection after configuring");
      try {get_summary_status();} catch(Eurotherm2000Series_error){--timeout;sleep(1); continue;}
      break;
    }
  }
  catch (Eurotherm2000Series_error e) {
    throw Eurotherm2000Series_error("got error while configuration:"+e);
  }
  if (error_message.size()!=0) {
    throw Eurotherm2000Series_error("Could not set "+error_message.substr(0,error_message.size()-1));
  }
 
}

Eurotherm2000Series::~Eurotherm2000Series() {
  /**
     close serial device, clean up
  */
  close(serial_dev);
}

void Eurotherm2000Series::read_value(const std::string& param_name, std::string& return_value) const{
  // assemble message without channel
  std::string message;
  message+=4;
  message+='0'+(address/10);
  message+='0'+(address/10);
  message+='0'+(address%10);
  message+='0'+(address%10);
  message+=param_name;
  message+=5;
  // write message to device
  pthread_mutex_lock((pthread_mutex_t*)&device_lock);
  write(serial_dev,message.c_str(),message.size());
  // read answer
  unsigned char got_byte=0;
  return_value.clear();
  // sleep minimum latency time for parameter reading
  timespec read_latency;
  read_latency.tv_sec=0; read_latency.tv_nsec=2000000;
  nanosleep(&read_latency,NULL);
  // now use shorter steps
  int timeout=100;
  read_latency.tv_nsec=1000000;
  while (return_value.size()<2 || return_value[return_value.size()-2]!=3) {
    int result=read(serial_dev,&got_byte,1);
    if (result<=0) {
      if (result<0 && errno!=EAGAIN) {
	return_value.clear();
	pthread_mutex_unlock((pthread_mutex_t*)&device_lock);
	throw Eurotherm2000Series_error("read_value: error while reading");
      }
      if (timeout<0) {
	return_value.clear();
	pthread_mutex_unlock((pthread_mutex_t*)&device_lock);
	throw Eurotherm2000Series_error("read_value: timeout while reading");
      }
      // go on sleeping for a while
      nanosleep(&read_latency,NULL);
      timeout--;
      continue;
    }
    if (return_value.size()==0 && got_byte==4) throw Eurotherm2000Series_error("read_value: invalid register");
    return_value+=got_byte;
  }  
  pthread_mutex_unlock((pthread_mutex_t*)&device_lock);

  if (0) {
    for (std::string::const_iterator i=return_value.begin();i!=return_value.end();++i)
      fprintf(stderr,"%02x",*i);
    fprintf(stderr,"\n");
  }
  // test bcc
  unsigned char bcc=return_value[1];
  for (size_t i=2;i<return_value.size()-1;++i) bcc^=return_value[i];
  if (bcc!=got_byte) {
    return_value.clear();
    throw Eurotherm2000Series_error("read_value: bcc missmatch");
  }

  // compare C1 and C2
  if (return_value.substr(1,param_name.size())!=param_name) {
    return_value.clear();
    throw Eurotherm2000Series_error("read_value: register name echo mismatch");
  }
  // cut protocol bytes
  return_value.erase(0,3);
  return_value.resize(return_value.size()-2);
  return;
}

int Eurotherm2000Series::set_value(const std::string& param_name, const std::string& value) {
  // comunicate with device
  std::string message;
  message+=4; //EOT
  message+='0'+(address/10);
  message+='0'+(address/10);
  message+='0'+(address%10);
  message+='0'+(address%10);
  message+=2; //STX
  message+=param_name.substr(0,2);
  message+=value;
  message+=3; //ETX

  // calculate bcc byte
  unsigned char bcc=message[6];
  for (size_t i=7;i<message.size();++i) bcc^=message[i];
  message+=bcc; //BCC

  pthread_mutex_lock(&device_lock);
  write(serial_dev,message.c_str(),message.size());
  // sleep minimum latency time for value setting
  timespec write_latency;
  write_latency.tv_sec=0; write_latency.tv_nsec=5000000;
  nanosleep(&write_latency,NULL);
  // wait shorter while polling
  write_latency.tv_nsec=1000000;
  int timeout=1000;
  unsigned char got_byte;
  int result;
  while ((result=read(serial_dev,&got_byte,1))<=0) {
    if (result<0 && errno!=EAGAIN) {
      pthread_mutex_unlock(&device_lock);
      throw Eurotherm2000Series_error("set_value: read error");
    }
    if (timeout<0) {
      pthread_mutex_unlock(&device_lock);
      throw Eurotherm2000Series_error("set_value: timeout occurred");
    }
    nanosleep(&write_latency,NULL);
    timeout--;
  }
  pthread_mutex_unlock(&device_lock);
  if (got_byte!=6) throw Eurotherm2000Series_error("set_value: negative acknoledge");
  return 0;
}


double Eurotherm2000Series::get_temperature() const {
  int summary_status=get_summary_status();
  if (failure_mask&summary_status) {
    char message_buffer[10];
    snprintf(message_buffer,10,"%4x",summary_status);
    throw Eurotherm2000Series_error(std::string("sensor/heater/temperature range fault: ")+message_buffer);
  }
  std::string answer;
  read_value("PV",answer);
  return strtod(answer.c_str(),NULL);
}

double Eurotherm2000Series::set_setpoint(double ct) {
  const char* value;
  char buffer[6];
  snprintf(buffer,6,fp_format.c_str(),ct);
  set_value("SL",value);
  // extra wait, until the get_setpoint function returns the same value
  timespec write_latency;
  write_latency.tv_sec=0; write_latency.tv_nsec=50000000;
  nanosleep(&write_latency,NULL);  
  write_latency.tv_nsec=100000000;
  int i=100;
  while(fabs(get_setpoint()-ct)>1e-4) {
    if (i<0) throw Eurotherm2000Series_error("could not confirm setpoint value settings, timeout");
    nanosleep(&write_latency,NULL);
    --i;
  }
  return ct;
}

double Eurotherm2000Series::get_setpoint() const {
  std::string answer;
  read_value("SL",answer);
  return strtod(answer.c_str(),NULL);
}

int Eurotherm2000Series::get_summary_status() const {
  std::string answer;
  read_value("SO",answer);
  if (answer[0]!='>') throw Eurotherm2000Series_error("could not handle status data format \'"+answer+"\'");
  answer.erase(0,1);
  return strtol(answer.c_str(),NULL,16);
}

void Eurotherm2000Series::get_setpoint_limits(double& min, double& max) const {
  std::string answer;
  read_value("LS",answer);
  min=strtod(answer.c_str(),NULL);
  read_value("HS",answer);
  max=strtod(answer.c_str(),NULL);  
}
