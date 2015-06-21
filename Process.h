#pragma once
#include "Thread.h"
#include <vector>
#include <stdlib.h>

struct Process
{
  enum Type {
    SYSTEM,
    INTERACTIVE,
    NORMAL,
    BATCH
  };

  Process( int pid, Type type ):
           pid(pid),type(type) { };
  ~Process( );

  int pid;
  Type type;
  size_t priority;                     // Initially determined by type, then by preemptions
  size_t offset;                       // Used for balance in 2^i multilevel
  size_t threads_finished;             // a variable to determine when the process is finished
  size_t CPU_extra_time;               // Extra time for time slice based on type
  std::vector<Thread*> threads;
};
