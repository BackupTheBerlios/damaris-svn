#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <sys/time.h>
#include <cstdlib>
#include <cmath>

/**
   \brief timing utility
   This class can take times, up to microseconds exact
 */

class stopwatch {
  /**
     time at last start call
   */
  timeval starttime;

  /**
     time at last stop call
   */
  timeval stoptime;

  /**
     to accumulate time intervals, previous interval seconds are saved
   */
  long int offset_sec;
  /**
     to accumulate time intervals, previous interval microseconds are saved
   */
  long int offset_usec;

  /**
     shift elapsed time to offset register and save new start time
   */
  void elapsed_save() const {
    /* if timer is already stoped, do nothing */
    if (timerisset(&starttime)) {
      stopwatch* noconst_this=(stopwatch*)this;
      gettimeofday(&(noconst_this->stoptime),NULL);
      /* elapsed time added to internal pointer */
      noconst_this->offset_sec+=(stoptime.tv_sec-starttime.tv_sec);
      noconst_this->offset_usec+=(stoptime.tv_usec-starttime.tv_usec);
      noconst_this->starttime.tv_sec=stoptime.tv_sec;
      noconst_this->starttime.tv_usec=stoptime.tv_usec;
      while (offset_usec<0) {
	noconst_this->offset_usec+=1000000;
	noconst_this->offset_sec-=1;
      }
      while (offset_usec>=1000000) {
	noconst_this->offset_usec-=1000000;
	noconst_this->offset_sec+=1;
      }
    }
  }

 public:

  /**
     creates stopwatch, stopped state, 0 time offset
   */
  stopwatch() {
    timerclear(&starttime);
    offset_sec=offset_usec=0;
  }

  /**
     creates stopwatch, stopped state, with time offset from double argument
   */
  stopwatch(const double& offset) {
    timerclear(&starttime);
    offset_sec=(long int)floor(offset);
    offset_usec=(long int)(1.0e6*(offset-offset_sec));
  }

  /**
     creates stopwatch, stopped state, with time offset in seconds and microseconds
   */
  stopwatch(const long int& _offset_sec, const long int& _offset_usec) {
    timerclear(&starttime);
    offset_sec=_offset_sec;
    offset_usec=_offset_usec;
  }

  stopwatch(const stopwatch& orig) {
    starttime.tv_sec=orig.starttime.tv_sec;
    starttime.tv_usec=orig.starttime.tv_usec;
    offset_sec=orig.offset_sec;
    offset_usec=orig.offset_usec;
  }

  /**
     set timer to zero and start measurement
  */
  inline void start() {
    gettimeofday(&starttime,NULL);
    offset_sec=offset_usec=0;
  }

  /**
     stop or pause time measurement
   */
  inline void stop() {
    elapsed_save();
    /* and clear this pointer */
    timerclear(&starttime);
  }

  /**
     continue time measurement
  */
  inline void cont() {
    if (!timerisset(&starttime)) gettimeofday(&starttime,NULL);
  }

  inline double elapsed() const {
    elapsed_save();
    return (1.0e-6*(double)offset_usec)+(double)offset_sec;
  }

  inline void elapsed(long int& sec, long int& usec) const {
    elapsed_save();
    sec=offset_sec;
    usec=offset_usec;
  }

};

#endif /* STOPWATCH_H */
