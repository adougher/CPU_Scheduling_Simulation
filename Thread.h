#pragma once
#include "Burst.h"
#include <cstdlib>
#include <vector>
#include <stdlib.h>

struct Process;

struct Thread
{
  Thread( size_t arrival_time, int tid, Process* process ):
           arrival_time(arrival_time),tid(tid),process(process) { };
  ~Thread( );

  // Computes the service time as the sum of CPU bursts
  size_t get_service_time( )
  {
    size_t service_time=0;
    for( size_t i=0; i<bursts.size( ); i++ )
    {
      if( i%2==0 )
      {
        service_time+=bursts[i]->length;
      }
    }
    return service_time;
  }
  
  // Computes the io time as the sum of IO bursts
  size_t get_io_time( )
  {
    size_t io_time=0;
    for( size_t i=0; i<bursts.size( ); i++ )
    {
      if( i%2==1 )
      {
        io_time+=bursts[i]->length;
      }
    }
    return io_time;
  }

  // Set during file read and simulation
  size_t arrival_time, response_time, finish_time;

  size_t service_time, io_time;

  int tid;
  std::vector<Burst*> bursts;
  Process* process;
  size_t priority; 
  size_t current_burst;  // Keeps track of which burst is next to run
};
