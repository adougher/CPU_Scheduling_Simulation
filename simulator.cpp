#include "Event.h"
#include "Process.h"
#include "Thread.h"
#include "Burst.h"
#include "simulator_functions.h"
#include <fstream>
#include <vector>
#include <string>
#include <getopt.h>
#include <cstdlib>
#include <iostream>
#include <queue>

using namespace std;
using namespace simfunc;

int main( int argc, char** argv )
{
  bool additional_details=0,verbose=0,help=0;  // Command line flags
  string file;
  get_options( additional_details, verbose, help, argc, argv, file );
  int thread_switch_overhead, proc_switch_overhead;
  vector<Process*> procs;
  priority_queue<Event*, vector<Event*>,EventComparator> events;

  // extract all info from file
  get_procs_from_file( file, thread_switch_overhead,proc_switch_overhead, 
                       procs,events );

  run_simulation( procs, events,thread_switch_overhead, proc_switch_overhead,
                  verbose, additional_details );
}
