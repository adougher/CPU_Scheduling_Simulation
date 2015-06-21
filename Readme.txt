Andrew Dougherty
CSCI 442, Spring 2014
Project 3

Process.h:
This class contains a type which determines initial priority and time slice,
an integer identifier, a vector of Threads and a counter to keep track of the
number of finished threads. It also contains an offset variable that 
allows threads to be placed initially into a lower priority queue based
on their type, without gaining the advantage of a greater time slice
in the 2^i multi-level feedback algorithm.

Thread.h:
This class contains an integer arrival time, response time and finish_time,
which are set during the file read and simulation.  It also contains an 
integer identifier, a vector of Bursts, a priority inherited from its
parent process and a counter to keep track of which burst it is on. It
also contains functions for returning the service and IO times of the 
thread, based on its vector of Bursts.

Burst.h:
This class contains the burst length, type and whether it has been prrempted.
The preempted bool is used for verbose output purposes.

Scheduler.h:
This class contains a vector of vectors, the number of which is determined by
the constructor.  Each vector represents a simple FIFO queue, and the index
of the vector represents the queue's priority. It also has a member variable
to keep track of the number of queues.  It also contains functions for inserting
a thread into the scheduler, deleting a thread from the scheduler and a
function to determine if the scheduler is empty.

Event.h:
This class contains a type, a time, and a pointer to the thread associated
with the event.  A priority queue based on the time member is used in
the simulation.

simulator_functions.h:
This is the header file for the simulation's core functionality.  It 
contains a struct for doing Event comparisons

simulator_functions.cpp:
This is the implementation file for the simulation's core functionality.  It
includes functions for parsing command line arguments, for retrieving info
from a valid input file and using it to populate all of the necessary data
structures, for actually running the simulation using the event priority
queue, and for printing statistics and other information based on the 
command line flags.

simulator.cpp:
This is the driver for the simulation.

****************************************************************

The scheduling algorithm I chose to implement is the 2^i multi-level 
feedback algorithm.  With each preemption, a thread is demoted in 
priority to the back of the next-lower queue. To help minimize
starvation, the time slice for queue i is 2^(i+1), where i ranges
from 0 to 9. 

I modified this basic approach to accomadate the different thread 
types. I assumed that the repsonse time for system threads should
be lower than the response time for interactive threads, which should 
be lower than the response time for normal threads, which should
be lower than the response time for batch threads. I therefore
started each type of thread in queue 0,1,2,3 respectively. But
this will automatically give a greater time slice to interactive,
normal and batch threads. I therefore also included an offset
variable with each process, based on its type, so that the time 
slice is actually determined by 2^(i+1-offset). This way, a batch
process starts of with the same time slice as an interactive thread
even though it is in a lower priority queue.

I also assumed that system threads should have a lower turnaround
time that the other types. I therefore made its base time slice
4 instead of 2, so that its slice is 4^(i+1-offset), where as
described above, its offset is zero. This seems to have produced 
the desired results for rnadomly generated files.


1.Does your simulator include switch overhead for the first ready state to running state transi-
tion? Explain.

Yes it does, but I assumed it was less than either a thread or process switch.
There would have to be some overhead associated with loading the first thread
and this will affect the efficiecy calculation.


2.Does your simulator include switch overhead if a thread moves from ready state to running
state and the CPU is idle? Explain.

In my code, I keep track of which process and thread was last loaded onto
the CPU.  The appropriate overhead occurs even if the CPU is idle. This
can be either due to all threads being IO bound, or no threads being in the
system.


3.Does your simulator include switch overhead if a thread moves from running state to blocked
state and the ready queue is empty? Explain.

This event does not incur any overhead in my code. I assumed that the context for the
thread remains on the CPU in this case, until the next ready thread.  This
situation does however decrease the CPU utilization.


4.Does your simulation include switch overhead if a thread is interrupted (due to a time slice)
and either the ready queue is empty or the thread has the highest priority? Explain.

For the first case, I treated the situation just as in question 3.  But in the 
second case, I assumed that because the process and thread context are 
already loaded onto the CPU, there is no overhead to having the thread
run again, for the same time slice.


5.Does your scheduling algorithm give appropriate priority to each of the different types of
threads? How does the priority of a thread affect the decisions made by your scheduler?

As explained in the paragraphs above, the threads are assigned initial
priorities based on their types. This determines which level of the 
2^i multi-level feedback queue they are inserted into. It also determines
the offset in the time slice exponent, so that lower priority thread
types don't automatically recieve a larger time slice. 

Other than that, the thread priorities are decremented by one with each
preemption, and the algorithm differs in no way from the classic 2^i
multi-level feedback.

