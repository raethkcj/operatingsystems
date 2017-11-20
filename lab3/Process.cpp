#include "Process.hpp"

Process::Process(
        int pid
		, int burst
		, int arrival
		, int priority
		, int deadline
		, int io
    ):
	    pid(pid)
		, burst(burst)
		, arrival(arrival)
		, priority(priority)
		, deadline(deadline)
		, io(io) {
}
