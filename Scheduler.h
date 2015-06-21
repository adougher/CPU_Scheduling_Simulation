#pragma once
#include "Thread.h"
#include <vector>

struct Scheduler
{
  // 10 FIFO queues for Multilevel Feedback
  Scheduler( size_t num_queues ): num_queues( num_queues ), queues(10) { };

  // Push thread to back of appropriate queue
  void insert_thread( Thread* thread )
  {
    queues[thread->priority].push_back( thread );
  }

  // Pull next thread from the front of the highest priority
  // non-empty queue
  Thread* get_next_thread( )
  {
    size_t i=0;
    while( queues[i].size( ) == 0 )
    {
      i++;
    }
    return queues[i][0];
  }
 
  // Erase thread from the scheduler (like when blocked or finished)
  void remove_thread( Thread* to_remove )
  {
    for( size_t i=0; i<queues.size( ); i++ )
    {
      for( size_t j=0; j< queues[i].size( ); j++ )
      {
        if( queues[i][j]->process==to_remove->process && queues[i][j]->tid==to_remove->tid )
        {
          queues[i].erase( queues[i].begin( )+j );
        }
      }
    }
  }

  // Returns true if all of the queues are empty
  bool is_empty( )
  {
    bool empty=1;
    for( size_t i=0; i<num_queues; i++ )
    {
      if( queues[i].size( ) > 0 )
      {
        empty=0;
      }
    }
    return empty;
  }

  size_t num_queues;
  std::vector< std::vector<Thread*> > queues;
};
