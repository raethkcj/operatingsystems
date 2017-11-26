#include <iostream>
#include <fstream>
#include <set>
#include <iomanip>
#include <limits>
#include <ios>
#include <cstring> // strcmp()

#include "Process.hpp"

// Real-Time Scheduler
// TODO: Account for both soft and hard RT environments
// TODO: Report any process that cannot be scheduled (in soft RT only)
void rts(std::set<Process, rtsCmp> processes) {
	int time = 0;
	int TWT = 0;
	int TTT = 0;
	int NP = 0;
	while (!processes.empty()) {
		std::set<Process, rtsCmp>::iterator p = processes.begin();
		// Get the next unexpired process
		while (p != processes.end() && p->deadline < (time + p->burst)) {
			processes.erase(p++);
		}

		if (p != processes.end() && time >= p->arrival) {
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

// Multi-level Feedback Queue Scheduler
// TODO: Prompt user for # of queues (up to 5), and time limit before
//     process begins to age -- prompt here or in main?
void mfqs(std::set<Process, rtsCmp> processes) {
	std::cout << "MFQS coming soon!" << std::endl;
}

// Windows Hybrid Scheduler
void whs(std::set<Process, rtsCmp> processes) {
	std::cout << "WHS coming soon!" << std::endl;
}

int main(int argc, char **argv) {
	// Check arguments
	bool isRts
		, isMfqs
		, isWhs;

	if (argc != 2
		|| ((isRts = strcmp(argv[1], "rts") != 0)
			&& (isMfqs = strcmp(argv[1], "mfqs") != 0)
			&& (isWhs = strcmp(argv[1], "whs") != 0))
		) {
		std::cout << "usage: " << argv[0] << " rts|mfqs|whs" << std::endl;
		std::cout << "For debug mode, run make clean, ";
		std::cout << "then make debug" << std::endl;
		exit(1);
	}

	std::ifstream input("9_processes");

	std::set<Process, rtsCmp> processes;

	int pid
		, burst
		, arrival
		, priority
		, deadline
		, io;

	// Ignore headings
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

	if (!isRts)
		rts(processes);
	else if (!isMfqs)
		mfqs(processes);
	else if (!isWhs)
		whs(processes);

	return 0;
}
