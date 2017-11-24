#include <iostream>
#include <fstream>
#include <set>
#include <iomanip>
#include <limits>
#include <ios>

#include "Process.hpp"

void rts(std::set<Process, rtsCmp> processes) {
	int time = 0;
	int TWT = 0;
	int TTT = 0;
	int NP = 0;
	while (!processes.empty()) {
		std::set<Process, rtsCmp>::iterator p = processes.begin();
		// Get the next unexpired process
		while (p->deadline < (time + p->burst)) {
			processes.erase(p);
			p = processes.begin();
		}

		if (time >= p->arrival) {
#ifdef DEBUG
			std::cout
				<< std::setw(4)
				<< time
				<< " -- Running "
				<< p->pid
				<< " with remaining burst: "
				<< std::setw(2)
				<< p->burst
				<< std::endl;
#endif
			Process tmp = *p;
			processes.erase(p);
			if (--tmp.burst > 0) {
				processes.insert(tmp);
			} else {
				NP++;
				TTT += time - tmp.arrival;
			}
		}
		time++;
	}
	int AWT = TWT / NP;
	int ATT = TTT / NP;
	std::cout << "AWT: " << AWT << "	ATT: " << ATT << "	NP: " << NP << std::endl;
}

int main() {
	std::ifstream input("9_processes");

	std::set<Process, rtsCmp> processes;

	int pid
		, burst
		, arrival
		, priority
		, deadline
		, io;

	input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	while (input >> pid >> burst >> arrival >> priority >> deadline >> io) {
		Process p(pid, burst, arrival, priority, deadline, io);
		processes.insert(p);
	}

#ifdef DEBUG
	for (Process p : processes) {
		std::cout << p << std::endl;
	}
#endif

	rts(processes);

	return 0;
}
