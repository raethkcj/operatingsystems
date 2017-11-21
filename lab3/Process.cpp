#include "Process.hpp"
#include <iomanip>

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

bool Process::operator<(Process other) const {
	return arrival < other.arrival;
}

std::ostream& operator<<(std::ostream &strm, const Process &p) {
	return strm << "Process(" << std::setw(6) << p.pid 
		<< ", Bst:" << std::setw(2) << p.burst 
		<< ", Arv:" << std::setw(4) << p.arrival 
		<< ", Pri:" << std::setw(2) << p.priority 
		<< ", Dln:" << std::setw(4) << p.deadline 
		<< ", Io:" << p.io 
		<< ")";
}
