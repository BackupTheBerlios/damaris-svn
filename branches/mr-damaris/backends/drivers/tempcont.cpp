#include "tempcont.h"
#include <cmath>
#include "errno.h"

void temp_history::print_xml(FILE* f) const {
  if (step==0) {fprintf(f,"<temperature/>\n");}
  char timebuffer[30];
  ctime_r(&latest,timebuffer);
  timebuffer[strlen(timebuffer)-1]=0;
  fprintf(f,"<temperature time=\"%s\" step=\"%d\">",timebuffer,step);
  if (!empty()) {
    fprintf(f,"\n");
    for (const_iterator i=begin(); i!=end();++i) fprintf(f,"%g ",*i);
    fprintf(f,"\n");
  }
  fprintf(f,"</temperature>\n");
}

void* history_maintainance_thread(void* temperature_sensor) {
  tempcont* ts=(tempcont*)temperature_sensor;
  // wait for completely initialised structure
  ts->maintain_history();
  return NULL;
}

temp_history* tempcont::get_history(size_t seconds_back) const {
  temp_history* result_history=new temp_history();
  if (result_history==NULL) throw tempcont_error("failed to create new temp_history object");
  if (history_step!=0) {
    pthread_mutex_lock((pthread_mutex_t*)&history_lock);
    for (int i=0; ((seconds_back==0 || i*history_step<seconds_back) && i<history_used); ++i)
      result_history->push_back(history_buffer[(history_latest_index+history_length-i)%history_length]);
    pthread_mutex_unlock((pthread_mutex_t*)&history_lock);
    result_history->step=history_step;
    result_history->latest=history_latest_time;
  }
  return result_history;
}

void tempcont::set_history_stepsize(size_t step) {
  if (history_step==step) return;
  pthread_mutex_lock((pthread_mutex_t*)&history_lock);
  history_step=step;
  if (step==0)  {
    // reinitalise history
    history_used=0;
  }
  else {
    history_latest_index=0;
    history_used=1;
    history_buffer[0]=get_temperature();
    history_latest_time=time(NULL);
  }
  pthread_mutex_unlock((pthread_mutex_t*)&history_lock);
}

double tempcont::wait_setpoint_reached(double delta, size_t timeout) const {
  // poll temperature every second
  time_t end_polling=time(NULL)+timeout;
  double setpoint=get_setpoint();
  double temperature=get_temperature();
  while (fabs(temperature-setpoint)>delta && (timeout==0 || end_polling>=time(NULL))) {
    sleep(1);
    temperature=get_temperature();
  }
  return temperature;
}

void tempcont::maintain_history() {
  // wait for initialisation of derived class or failure
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
  // sleep 0.9 seconds, that assures a check every second
  timespec sleeptime;
  sleeptime.tv_sec=0;
  sleeptime.tv_nsec=800*1000*1000;
  while (1) {
    // do not maintain history until step is set to a reasonable value
    if (history_step!=0) {
      pthread_mutex_lock(&history_lock);
      time_t now=time(NULL);
      if (history_latest_time+history_step<=now) {
	double new_temp;
	try {
	  new_temp=get_temperature();
	}
	catch (tempcont_error e) {
	  pthread_mutex_unlock(&history_lock);
	  fprintf(stderr,"history maintainance thread caught exception: %s, terminating\n",e.c_str());
	  device_failed=1;
	  pthread_exit(NULL);
	}
	// append the new temperature value
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);
	history_latest_time=now;
	if (history_used==0) {
	  history_latest_index=0;
	  history_buffer[0]=new_temp;
	  history_used=1;
	}
	else {
	  history_latest_index++;
	  if (history_latest_index==history_length) history_latest_index=0;
	  history_buffer[history_latest_index]=new_temp;
	  if (history_used<history_length) history_used++;
	}
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
	pthread_mutex_unlock(&history_lock);
	// continue directly with next check
	continue;
      }
      else {
	pthread_mutex_unlock(&history_lock);
      }
    }
    nanosleep(&sleeptime,NULL);
  }
  return;
}

tempcont::tempcont() {
  // initialise history
  history_step=0;
  // one day history (1s steps)
  history_length=24*60*60;
  history_used=0;
  history_buffer=(double*)malloc(sizeof(double)*history_length);
  if (history_buffer==NULL) throw tempcont_error("failed to allocate history");
  // no failures detected
  device_failed=0;
  // create history lock
  pthread_mutexattr_t history_lock_attr;
  pthread_mutexattr_init(&history_lock_attr);
  pthread_mutex_init(&history_lock,&history_lock_attr);
  pthread_mutexattr_destroy(&history_lock_attr);
  // create device lock
  pthread_mutexattr_t device_lock_attr;
  pthread_mutexattr_init(&device_lock_attr);
  pthread_mutex_init(&device_lock,&device_lock_attr);
  pthread_mutexattr_destroy(&device_lock_attr);
  // create history update thread
  pthread_attr_t history_thread_attr;
  pthread_attr_init(&history_thread_attr);
  int result=pthread_create(&history_thread,&history_thread_attr,&history_maintainance_thread,this);
  pthread_attr_destroy(&history_thread_attr);
  // clean up, it this operation failed
  if (result!=0) {
    free(history_buffer);
    pthread_mutex_destroy(&device_lock);
    pthread_mutex_destroy(&history_lock);
    throw tempcont_error("could not create history maintainance thread");
  }
}

tempcont::~tempcont() {
  // send cancelation
  if (0!=pthread_cancel(history_thread) && errno!=EIO) {
    fprintf(stderr,"thread cancelation failed\n");
  }
  // wait for terminated thread
  if (pthread_join(history_thread,NULL)!=0){
    fprintf(stderr,"thread joining failed\n");
  };
  if (history_buffer!=NULL) free(history_buffer);
  pthread_mutex_destroy(&device_lock);
  pthread_mutex_destroy(&history_lock);
}
