#include <iomanip>

#include "Process.hpp"

Process::Process() {}

Process::Process(
        int pid
		, int maxBurst
		, int burst
		, int arrival
		, int initPriority
		, int priority
		, int deadline
		, int io
    ):
	    pid(pid)
		, maxBurst(maxBurst)
		, burst(burst)
		, arrival(arrival)
		, initPriority(initPriority)
		, priority(priority)
		, deadline(deadline)
		, io(io)
		, age(0) {
}

// Weird formatting but it works
bool RtsCmp::operator()(const Process& a, const Process& b) {
	if (a.arrival > b.arrival) { return false; }
	else if (a.arrival < b.arrival) { return true; }
	else {
		if (a.deadline > b.deadline) { return false; }
		else if (a.deadline < b.deadline) { return true; }
		else {
			return a.pid < b.pid;
		}
	}
}

bool MfqsCmp::operator()(const Process& a, const Process& b) {
	if (a.arrival > b.arrival) { return false; }
	else if (a.arrival < b.arrival) { return true; }
	else {
		return a.pid < b.pid;
	}
}

bool WhsCmp::operator()(const Process& a, const Process& b) {
	if (a.arrival > b.arrival) { return false; }
	else if (a.arrival < b.arrival) { return true; }
	else {
		return a.pid < b.pid;
	}
}

std::ostream& operator<<(std::ostream &strm, const Process &p) {
	return strm << "Process(" << std::setw(6) << p.pid
		<< ", MaxBst:" << std::setw(2) << p.maxBurst
		<< ", Bst:" << std::setw(2) << p.burst
		<< ", Arv:" << std::setw(4) << p.arrival
		<< ", Pri:" << std::setw(2) << p.priority
		<< ", Dln:" << std::setw(4) << p.deadline
		<< ", Io:" << p.io
		<< ")";
}
