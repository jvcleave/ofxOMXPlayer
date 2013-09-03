#pragma once
#include "ofMain.h"

#include <pthread.h>

class OMXThread 
{
protected:
  pthread_attr_t      m_tattr;
  struct sched_param  m_sched_param;
  pthread_mutex_t     m_lock;
  pthread_t           m_thread;
  volatile bool       m_running;
  volatile bool       m_bStop;
private:
  static void *Run(void *arg);
public:
  OMXThread();
  virtual ~OMXThread();
  bool Create();
  virtual void Process() = 0;
  bool Running();
  pthread_t ThreadHandle();
  bool StopThread();
  void Lock();
  void UnLock();
};
