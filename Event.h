#pragma once
#include "Thread.h"

struct Event
{
  enum Type {
    THREAD_ARRIVED,
    THREAD_FINISHED,
    PROCESS_TERMINATED,
    THREAD_DISPATCHED,
    THREAD_CPU_BURST_STARTED,
    THREAD_IO_BURST_STARTED,
    THREAD_IO_BURST_FINISHED,
    THREAD_PREEMPTED
  };

  Event( int time, Type type, Thread* thread ):
         time(time), type(type), thread(thread) { };

  int time; 
  Type type;
  Thread* thread;
};
