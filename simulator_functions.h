#include "Event.h"
#include "Process.h"
#include <vector>
#include <string>
#include <queue>

#ifndef SIMULATOR_FUNCTIONS_H
#define SIMULATOR_FUNCTIONS_H

namespace simfunc
{
  // means of comparing events
  struct EventComparator {
    bool operator()(Event* e1, Event* e2) {
      return e1->time >= e2->time;
    }
  };

  // Prints optional flags and their functionality if -h or --help passed
  void print_usage( );

  // sets the options and retrieves the filename
  void get_options( bool& additional_details, bool& verbose, bool& help, 
                    int argc, char** argv, std::string &file );

  // If file open succeeds, retrieves the info from it
  void get_procs_from_file( std::string &file, int &thread_switch_overhead,
                            int &proc_switch_overhead, std::vector<Process*> &procs,
                            std::priority_queue<Event*, std::vector<Event*>,
                                                EventComparator> &events );

  // Prints statistics and optional stats if specified
  void print_details( std::vector<Process*> &procs, unsigned &time, 
                      unsigned &inefficiency, unsigned &not_utilized,
                      bool additional_details );

  // Main driver. Prints events and times if verbose flag passed
  void run_simulation( std::vector<Process*> &procs, std::priority_queue<Event*,
                       std::vector<Event*>, EventComparator> &events,
                       int &thread_switch_overhead, int &proc_switch_overhead,
                       bool verbose, bool additional_details );
};
#endif
