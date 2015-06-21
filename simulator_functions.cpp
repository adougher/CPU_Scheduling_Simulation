#include "Event.h"
#include "Process.h"
#include "Thread.h"
#include "Burst.h"
#include "Scheduler.h"
#include "simulator_functions.h"
#include <fstream>
#include <vector>
#include <string>
#include <getopt.h>
#include <cstdlib>
#include <iostream>
#include <queue>
#include <cmath>

using namespace std;

void simfunc::print_usage( )
{
  cout << "./simulator: Usage: ./simulator [-d -v -h] <filename.txt> | [--additional_details --verbose --help] <filename.txt>\n";
  cout << "  '-d' and '--additional_details' Output additional information about thread arrival time, service time, etc.\n";
  cout << "  '-v' and '--verbose' Output verbose information about every single state-changing event in the simulation.\n";
  cout << "  '-h' and '--help' Display a help message about these flags and exit.\n";
}

void simfunc::get_options( bool& additional_details, bool& verbose, bool& help, int argc, char** argv, string &file )
{
  int opt=0;
  //Specifying the expected options
  //The options d, s and h expect numbers as argument
  static struct option long_options[] = 
  {
      {"additional_details",     no_argument, 0,  'd' },
      {           "verbose",     no_argument, 0,  'v' },
      {              "help",     no_argument, 0,  'h' },
      {                   0,               0, 0,   0  }
  };

  int long_index =0;
  while ((opt = getopt_long(argc, argv,"dvh",
                long_options, &long_index )) != -1) 
  {
    switch (opt) 
    {
      case 'd' : additional_details = 1;
                 break;
      case 'v' : verbose=1;
                 break;
      case 'h' : help = 1;
                 break;
      default: print_usage();
                 exit(EXIT_FAILURE);
    }
  }
  if( help==1 || optind+1 != argc )
  {
    print_usage( );
    exit(EXIT_SUCCESS);
  }
  else
  {
    file=string(argv[argc-1]);
  }
}

void simfunc::get_procs_from_file( string &file, int &thread_switch_overhead,
                                   int &proc_switch_overhead, vector<Process*> &procs,
                                   priority_queue<Event*, vector<Event*>, 
                                                  EventComparator> &events )
{
  FILE* pfile=fopen(file.c_str( ),"r");
  if( pfile == NULL )
  {
    cerr << "The simulation file '" << file << "' cannot be opened." << endl;
    exit(EXIT_FAILURE);
  }

  // scan for global info
  int num_procs;
  fscanf(pfile,"%d %d %d",
         &num_procs,&thread_switch_overhead,&proc_switch_overhead);
  int pid,type,arrival_time,num_threads,num_bursts,burst_length;

  // Get procs
  for( int i=0; i<num_procs; i++ )
  {
    fscanf(pfile,"%d %d %d",
           &pid,&type,&num_threads);
    Process* process;
    switch(type)
    {
      case 0: process=new Process(pid,Process::SYSTEM);
              process->priority=0;
              process->CPU_extra_time=2;
              process->offset=0;
        break;
      case 1: process=new Process(pid,Process::INTERACTIVE);
              process->priority=1;
              process->CPU_extra_time=0;
              process->offset=1;
        break;
      case 2: process=new Process(pid,Process::NORMAL);
              process->priority=2;
              process->CPU_extra_time=0;
              process->offset=2;
        break;
      case 3: process=new Process(pid,Process::BATCH);
              process->priority=3;
              process->CPU_extra_time=0;
              process->offset=3;
        break;
      default:
        cerr << "File read error. Invalid Process type." << endl;
        exit(EXIT_FAILURE);
        break;
    }
    // get threads for procs[i]
    for( int j=0; j<num_threads; j++ )
    {
      fscanf(pfile,"%d %d",
             &arrival_time,&num_bursts);
      Thread* thread=new Thread(arrival_time,j,process);
      thread->priority=thread->process->priority;
      thread->current_burst=0;
      for( int k=0; k<2*num_bursts-1; k++ )
      {
        fscanf(pfile,"%d",&burst_length);
        if( k%2==0 )
        {
          thread->bursts.push_back(new Burst(Burst::CPU,burst_length,0));
        }
        else
        {
          thread->bursts.push_back(new Burst(Burst::IO,burst_length,0));
        }
      }
      thread->service_time=thread->get_service_time( );
      thread->io_time=thread->get_io_time( );
      events.push(new Event(arrival_time,Event::THREAD_ARRIVED,thread));
      process->threads.push_back( thread );
      process->threads_finished=0;
    }
    procs.push_back( process );
  }
  fclose( pfile );
}

void simfunc::print_details( std::vector<Process*> &procs, unsigned &time, unsigned &inefficiency, 
                             unsigned &not_utilized, bool additional_details )
{
  size_t num_sys_threads=0, num_int_threads=0;
  size_t num_std_threads=0, num_btc_threads=0;
  float avg_response_sys=0, avg_turnaround_sys=0;
  float avg_response_int=0, avg_turnaround_int=0;
  float avg_response_std=0, avg_turnaround_std=0;
  float avg_response_btc=0, avg_turnaround_btc=0;
  // Compute totals by thread type for stats
  for( size_t i=0; i < procs.size( ); i++ )
  {
    if( procs[i]->type == Process::SYSTEM )
    {
      num_sys_threads+=procs[i]->threads.size( );
    }
    if( procs[i]->type == Process::INTERACTIVE )
    {
      num_int_threads+=procs[i]->threads.size( );
    }
    if( procs[i]->type == Process::NORMAL )
    {
      num_std_threads+=procs[i]->threads.size( );
    }
    if( procs[i]->type == Process::BATCH )
    {
      num_btc_threads+=procs[i]->threads.size( );
    }
  }
  for( size_t i=0; i < procs.size( ); i++ )
  {
    for( size_t j=0; j < procs[i]->threads.size( ); j++ )
    {
      // Print individual thread info if specified
      if( additional_details )
      {
        cout << endl << endl << "Thread " << procs[i]->threads[j]->tid << " of process "
             << procs[i]->threads[j]->process->pid << ":" << endl;
        cout << "     Arrival Time: " << procs[i]->threads[j]->arrival_time << endl;
        cout << "    Response Time: " << procs[i]->threads[j]->response_time << endl;
        cout << "     Service Time: " << procs[i]->threads[j]->service_time << endl;
        cout << "          IO Time: " << procs[i]->threads[j]->io_time << endl;
        cout << "      Finish Time: " << procs[i]->threads[j]->finish_time << endl;
        cout << "  Turnaround Time: " << procs[i]->threads[j]->finish_time-procs[i]->threads[j]->arrival_time << endl;
      }
      // Compute averages
      if( procs[i]->type == Process::SYSTEM )
      {
        avg_response_sys+=(1.0*procs[i]->threads[j]->response_time/num_sys_threads);
        avg_turnaround_sys+=(1.0*(procs[i]->threads[j]->finish_time-procs[i]->threads[j]->arrival_time)/num_sys_threads);
      }
      if( procs[i]->type == Process::INTERACTIVE )
      {
        avg_response_int+=(1.0*procs[i]->threads[j]->response_time/num_int_threads);
        avg_turnaround_int+=(1.0*(procs[i]->threads[j]->finish_time-procs[i]->threads[j]->arrival_time)/num_int_threads);
      }
      if( procs[i]->type == Process::NORMAL )
      {
        avg_response_std+=(1.0*procs[i]->threads[j]->response_time/num_std_threads);
        avg_turnaround_std+=(1.0*(procs[i]->threads[j]->finish_time-procs[i]->threads[j]->arrival_time)/num_std_threads);
      }
      if( procs[i]->type == Process::BATCH )
      {
        avg_response_btc+=(1.0*procs[i]->threads[j]->response_time/num_btc_threads);
        avg_turnaround_btc+=(1.0*(procs[i]->threads[j]->finish_time-procs[i]->threads[j]->arrival_time)/num_btc_threads);
      }
    }
  }
  cout << endl << endl << "       Average response time (System threads):   " << avg_response_sys << endl;
  cout << "     Average turnaround time (System threads):   " << avg_turnaround_sys << endl;
  cout << "  Average response time (Interactive threads):   " << avg_response_int << endl;
  cout << "Average turnaround time (Interactive threads):   " << avg_turnaround_int << endl;
  cout << "       Average response time (Normal threads):   " << avg_response_std << endl;
  cout << "     Average turnaround time (Normal threads):   " << avg_turnaround_std << endl;
  cout << "        Average response time (Batch threads):   " << avg_response_btc << endl;
  cout << "      Average turnaround time (Batch threads):   " << avg_turnaround_btc << endl <<endl;

  cout << "             Total Time:   " << time << endl;
  cout << "        CPU  efficiency:   " << round(10000.0*(time-inefficiency)/time)/100.0 << "%" << endl;
  cout << "        CPU utilization:   " << round(10000.0*(time-not_utilized)/time)/100.0 << "%" << endl << endl;
}

void simfunc::run_simulation( std::vector<Process*> &procs, std::priority_queue<Event*,
                               std::vector<Event*>, EventComparator> &events,
                               int &thread_switch_overhead, int &process_switch_overhead,
                               bool verbose, bool additional_details )
{
  // Initialize scheduler with 10 queues
  Scheduler scheduler( 10 );
  
  // Variables used to compute total time and CPU inefficiency and utilization
  unsigned time=0, inefficiency=0, not_utilized=0, temp=0;

  // Used for first dispatch 
  unsigned dispatch_time=1;

  // Used to determine if thread or process switch overhead is used
  Process* last_proc_on_CPU=NULL;

  // Used in special case of all processes being IO bound
  bool all_IO;

  // base time slice before preemption
  int time_slice=2;

  // temp variable
  Thread* next_thread;

  // The simulation!!!
  while( !events.empty( ) )
  {
    Event* next_event=events.top( );
    switch( next_event->type )
    {
      case Event::THREAD_ARRIVED: 
        time=next_event->time;
        events.pop( );
        if( verbose )
        {
          cout << "At time " << time  << ": Thread " << next_event->thread->tid << " of process " 
               << next_event->thread->process->pid << " arrived." << endl;
        }
        if( scheduler.is_empty( ) )
        {
          events.push(new Event(time,Event::THREAD_DISPATCHED,next_event->thread));
          // First dispatch, don't use switch overhead
          if( last_proc_on_CPU == NULL )
          { 
            // Idel time at beginning of simulation
            not_utilized+=time;
            inefficiency+=dispatch_time;
            events.push(new Event(time+=dispatch_time,Event::THREAD_CPU_BURST_STARTED,next_event->thread));
          }
          else
          {
            // thread switch
            if( last_proc_on_CPU == next_event->thread->process )
            {
              inefficiency+=thread_switch_overhead;
              events.push(new Event(time+=thread_switch_overhead,Event::THREAD_CPU_BURST_STARTED,next_event->thread));
            }
            // process switch
            else
            {
              inefficiency+=process_switch_overhead;
              events.push(new Event(time+=process_switch_overhead,Event::THREAD_CPU_BURST_STARTED,next_event->thread));
            }
          }
          scheduler.insert_thread( next_event->thread );
        }
        else
        {
          scheduler.insert_thread( next_event->thread );
        }
        break;
      case Event::THREAD_FINISHED:
        time=next_event->time;
        events.pop( );
        last_proc_on_CPU=next_event->thread->process;
        // collect finish time stat
        next_event->thread->finish_time=time;
        if( verbose )
        {
          cout << "At time " << time << ": Thread " << next_event->thread->tid << " of process " 
               << next_event->thread->process->pid << " finished." << endl;
        }
        // increment threads finished
        next_event->thread->process->threads_finished++;
        if( next_event->thread->process->threads_finished == next_event->thread->process->threads.size( ) )
        {
          events.push(new Event(time,Event::PROCESS_TERMINATED,next_event->thread));
        }
        scheduler.remove_thread( next_event->thread );
        // Dispatch another thread is available
        if( !scheduler.is_empty( ) )
        {
          next_thread=scheduler.get_next_thread( );
          events.push(new Event(time,Event::THREAD_DISPATCHED,next_thread));
          // update inefficiency and schedule next event
          if( next_thread->process == next_event->thread->process )
          {
            inefficiency+=thread_switch_overhead;
            events.push(new Event(time+thread_switch_overhead,Event::THREAD_CPU_BURST_STARTED,next_thread));
          }
          else
          {
            inefficiency+=process_switch_overhead;
            events.push(new Event(time+process_switch_overhead,Event::THREAD_CPU_BURST_STARTED,next_thread));
          }
        }
        else
        {
          // This will be used for computing CPU utilization
          temp=next_event->time;
        }
        break;
      case Event::PROCESS_TERMINATED:
        time=next_event->time;
        events.pop( );
        if( verbose )
        {
          cout << "At time " << time << ": Process " << next_event->thread->process->pid << " finished." << endl;
        }
        break;
      case Event::THREAD_DISPATCHED:
        time=next_event->time;
        events.pop( );
        if( verbose )
        {
          cout << "At time " << time << ": Thread " << next_event->thread->tid << " of process "
               << next_event->thread->process->pid << " dispatched." << endl;
        }
        // There was idle time from all threads being IO bound,
        // no threads being in the system.
        if( temp != 0 )
        {
          not_utilized+=(time-temp);
          temp=0;
        }
        break;
      case Event::THREAD_CPU_BURST_STARTED:
        time=next_event->time;
        events.pop( );
        if( verbose )
        {
          // Output depends on if thread is continuing a burst
          if( next_event->thread->bursts[next_event->thread->current_burst]->preempted == 0 )
          {
            cout << "At time " << time << ": Thread " << next_event->thread->tid << " of process " << next_event->thread->process->pid 
                 << " started CPU burst " << (next_event->thread->current_burst)/2+1 << "." << endl;
          }
          else
          {
            cout << "At time " << time << ": Thread " << next_event->thread->tid << " of process " << next_event->thread->process->pid
                 << " continued CPU burst " << (next_event->thread->current_burst)/2+1 << "." << endl;
          }
        }
        if( next_event->thread->current_burst==0 && next_event->thread->bursts[0]->preempted==0 )
        {
          // First CPU burst, update response time
          next_event->thread->response_time=time-next_event->thread->arrival_time;
        }
        // This is the time slice, which is determined by the thread's type and priority
        // Reflects a 2^i multilevel feedback, where the time slice for system threads
        // is one greater than for the other types.  It also includes an offset
        // so that threads that are initially placed in a lower priority queue
        // do not get the benefit of a greater time slice.
        if( pow(time_slice+next_event->thread->process->CPU_extra_time,
               (next_event->thread->priority-next_event->thread->process->offset)) >= 
             next_event->thread->bursts[next_event->thread->current_burst]->length )
        {
          time+=(next_event->thread->bursts[next_event->thread->current_burst])->length;
          next_event->thread->current_burst++;
          // Determine if all bursts completed
          if( next_event->thread->current_burst == next_event->thread->bursts.size( ) )
          {
            events.push(new Event(time,Event::THREAD_FINISHED,next_event->thread));
          }
          else
          {
            events.push(new Event(time,Event::THREAD_IO_BURST_STARTED,next_event->thread));
          }
        }
        else
        {
          // Update burst length if preemption occured
          next_event->thread->bursts[next_event->thread->current_burst]->length-=
            pow(time_slice+next_event->thread->process->CPU_extra_time,
               (next_event->thread->priority-next_event->thread->process->offset));
          next_event->thread->bursts[next_event->thread->current_burst]->preempted=1;
          // schedule next event
          time+=(pow(time_slice+next_event->thread->process->CPU_extra_time,
                (next_event->thread->priority-next_event->thread->process->offset)));
          events.push(new Event(time,Event::THREAD_PREEMPTED,next_event->thread));
        }
        break;
      case Event::THREAD_IO_BURST_STARTED:
        time=next_event->time;
        events.pop( );
        last_proc_on_CPU=next_event->thread->process;
        if( verbose )
        {
          cout << "At time " << time << ": Thread " << next_event->thread->tid << " of process " << next_event->thread->process->pid 
               << " started IO burst " << (next_event->thread->current_burst)/2+1 << "." << endl;
        }
        // thread no longer Ready
        scheduler.remove_thread( next_event->thread );
        // Dispatch another if available
        if( !scheduler.is_empty( ) )
        {
          events.push(new Event(time,Event::THREAD_DISPATCHED,scheduler.get_next_thread( )));
          if( next_event->thread->process == scheduler.get_next_thread( )->process )
          {
            inefficiency+=thread_switch_overhead;
            events.push(new Event(time+thread_switch_overhead,Event::THREAD_CPU_BURST_STARTED,scheduler.get_next_thread( )));
          }
          else
          {
            inefficiency+=process_switch_overhead;
            events.push(new Event(time+process_switch_overhead,Event::THREAD_CPU_BURST_STARTED,scheduler.get_next_thread( )));
          }
        }
        else
        {
          temp=next_event->time;
        }
        events.push(new Event(time+(next_event->thread->bursts[next_event->thread->current_burst])->length,
                                  Event::THREAD_IO_BURST_FINISHED,next_event->thread));
        break;
      case Event::THREAD_IO_BURST_FINISHED:
        time=next_event->time;
        events.pop( );
        if( verbose )
        {
          cout << "At time " << time << ": Thread " << next_event->thread->tid << " of process " << next_event->thread->process->pid 
               << " finished IO burst " << next_event->thread->current_burst/2+1 << "." << endl;
        }
        // this determines if the CPU was sitting idle while IO bursts occured
        all_IO=scheduler.is_empty( );
        next_event->thread->current_burst++;
        scheduler.insert_thread( next_event->thread );
        // If no currently running thread, must schedule the one associated with this event
        if( all_IO )
        {
          events.push(new Event(time,Event::THREAD_DISPATCHED,scheduler.get_next_thread( )));
          if( scheduler.get_next_thread( )->process == next_event->thread->process )
          {
            inefficiency+=thread_switch_overhead;     
            events.push(new Event(time+thread_switch_overhead,Event::THREAD_CPU_BURST_STARTED,scheduler.get_next_thread( )));
          }
          else
          {
            inefficiency+=process_switch_overhead;
            events.push(new Event(time+process_switch_overhead,Event::THREAD_CPU_BURST_STARTED,scheduler.get_next_thread( )));
          }
        }
        break;
      case Event::THREAD_PREEMPTED:
        time=next_event->time;
        last_proc_on_CPU=next_event->thread->process;
        events.pop( );
        if( verbose )
        {
          cout << "At time " << time << ": Thread " << next_event->thread->tid << " of process " 
               << next_event->thread->process->pid << " preempted." << endl;
        }
        // remove the thread, update its priority, and reinsert it into 
        // the scheduler
        scheduler.remove_thread( next_event->thread );
        if( next_event->thread->priority < scheduler.num_queues-1 )
        {
          next_event->thread->priority++;
        }
        scheduler.insert_thread( next_event->thread );
        // Only use a switch overhead, if there was another thread
        // in the scheduler
        if( scheduler.get_next_thread( ) != next_event->thread )
        {
          events.push(new Event(time,Event::THREAD_DISPATCHED,scheduler.get_next_thread( )));
          if( next_event->thread->process == scheduler.get_next_thread( )->process )
          {
            inefficiency+=thread_switch_overhead;
            events.push(new Event(time+thread_switch_overhead,Event::THREAD_CPU_BURST_STARTED,scheduler.get_next_thread( )));
          }
          else
          {
            inefficiency+=process_switch_overhead;
            events.push(new Event(time+process_switch_overhead,Event::THREAD_CPU_BURST_STARTED,scheduler.get_next_thread( )));
          }
        }
        else
        {
          // No overhead
          events.push(new Event(time+dispatch_time,Event::THREAD_CPU_BURST_STARTED,scheduler.get_next_thread( )));
        }
        break;
    }
  }
  print_details( procs, time, inefficiency, not_utilized, additional_details );
}
