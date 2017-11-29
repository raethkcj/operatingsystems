#include <iostream>
#include <fstream>
#include <set>
#include <iomanip>
#include <limits>
#include <ios>
#include <cstring> // strcmp()
#include <vector>

#include "Process.hpp"

// Real-Time Scheduler
// TODO: Account for both soft and hard RT environments
// TODO: Report any process that cannot be scheduled (in soft RT only)
// TODO: Wait time
void rts(std::set<Process, RtsCmp> processes) {
	int time = 0;
	int TWT = 0;
	int TTT = 0;
	int NP = 0;
	while (!processes.empty()) {
		std::set<Process, RtsCmp>::iterator p = processes.begin();
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
				// Process is done
				NP++;
				TTT += time - tmp.arrival;
				TWT += time - tmp.arrival - tmp.maxBurst;
			}
		}
		time++;
	}
	int AWT = TWT / NP;
	int ATT = TTT / NP;
	std::cout
		<< "AWT: " << AWT
		<< "	ATT: " << ATT
		<< "	NP: " << NP
		<< std::endl;
}

// Multi-level Feedback Queue Scheduler
// TODO: Prompt user for # of queues (up to 5), and time limit before
//     process begins to age -- prompt here or in main?
// TODO: Wait time
// TODO: Lower (1?) and upper (5) limits for nQueues
// TODO: Account for I/O
// TODO: Aging
// TODO: Tiebreakers
// TODO: Sanitize invalid values
void mfqs(std::set<Process, MfqsCmp> processes) {
	int time = 0;
	int TWT = 0;
	int TTT = 0;
	int NP = 0;
	int nQueues = 5;
	std::vector<Process> queues[nQueues];

	while (!processes.empty()) {
		std::set<Process, MfqsCmp>::iterator sProc = processes.begin();
		// Can we just use processes.empty() here?
		if (sProc != processes.end() && time >= sProc->arrival) {
			// Put in first queue
			queues[0].push_back(*sProc);
			// It's in the queue now, no need to keep it in the set
			processes.erase(sProc);
		}
		// Run one process from nonempty RR queue
		std::vector<Process>::iterator qProcs[nQueues];
		int quantum = 20;
		int i = 0;
		bool ran = false;
		while (i < (nQueues - 1) && !ran) {
			qProcs[i] = queues[i].begin();

			if (!queues[i].empty()) {
				// Run
#ifdef DEBUG
				std::cout
					<< std::setw(4)
					<< time
					<< " -- Running "
					<< qProcs[i]->pid
					<< " from queue "
					<< i
					<< " (RR) with remaining burst: "
					<< qProcs[i]->burst
					<< ", and remaining quantum: "
					<< quantum
					<< std::endl;
#endif
				ran = true;
				bool done = false;
				while (!done) {
					++time;
					if (--qProcs[i]->burst > 0) {
						if (--quantum <= 0) {
							done = true;
							// Process incomplete, but quantum depleted. Move to next queue
							queues[i + 1].push_back(*qProcs[i]);
							queues[i].erase(qProcs[i]);
						}
					} else {
						done = true;
						// Process complete. Remove from master set and current queue
						++NP;
						TTT += time - qProcs[i]->arrival;
#ifdef DEBUG
						std::cout
							<< std::setw(4)
							<< time
							<< " -- Complete "
							<< qProcs[i]->pid
							<< " with turnaround: "
							<< time - qProcs[i]->arrival
							<< ", and wait: "
							<< time - qProcs[i]->arrival - qProcs[i]->maxBurst
							<< std::endl;
#endif
						// Add to wait time
						TWT += time - qProcs[i]->arrival - qProcs[i]->maxBurst;
						queues[i].erase(qProcs[i]);
					}
				}
			}
			++i;
			quantum /= 2;
		}
		// Already ran? You're done!
		if (!ran) {
			// If all RR queues empty, look in FIFO queue (last)
			if (!queues[i].empty()) {
				// Run
				qProcs[i] = queues[i].begin();
#ifdef DEBUG
				std::cout
					<< std::setw(4)
					<< time
					<< " -- Running "
					<< qProcs[i]->pid
					<< " from queue "
					<< i
					<< " (FIFO) with remaining burst: "
					<< qProcs[i]->burst
					<< std::endl;
#endif
				ran = true;
				++time;
				if (--qProcs[i]->burst <= 0) {
					++NP;
					TTT += time - qProcs[i]->arrival;
#ifdef DEBUG
					std::cout
						<< std::setw(4)
						<< time
						<< " -- Complete "
						<< qProcs[i]->pid
						<< " with turnaround: "
						<< time - qProcs[i]->arrival
						<< ", and wait: "
						<< time - qProcs[i]->arrival - qProcs[i]->maxBurst
						<< std::endl;
#endif
					// Add to wait time
					TWT += time - qProcs[i]->arrival - qProcs[i]->maxBurst;
					queues[i].erase(qProcs[i]);
				}
			} else {
				// This should only happen if all queues are empty
				++time;
			}
		}
	}
	int AWT = TWT / NP;
	int ATT = TTT / NP;
	std::cout
		<< "AWT: " << AWT
		<< "	ATT: " << ATT
		<< "	NP: " << NP
		<< std::endl;
}

// Windows Hybrid Scheduler
void whs(std::set<Process, WhsCmp> processes) {
	std::cout << "WHS coming soon!" << std::endl;
}

// TODO: Take 2nd argument for filename, maybe more for MFQS and WHS parameters
int main(int argc, char **argv) {
	// Check arguments
	bool isRts
		, isMfqs
		, isWhs;

	if (argc != 3
		|| ((isRts = strcmp(argv[1], "rts") != 0)
			&& (isMfqs = strcmp(argv[1], "mfqs") != 0)
			&& (isWhs = strcmp(argv[1], "whs") != 0))
		) {
		std::cout << "usage: " << argv[0] << " rts|mfqs|whs <infile>" << std::endl;
		std::cout << "For debug mode, run make clean, "
				  << "then make debug" << std::endl;
		exit(1);
	}

	std::ifstream input(argv[2]);
	if (!input.good()) {
		std::cout
			<< argv[0] << ": error: "
			<< argv[2] << ": No such file or directory" << std::endl;
		exit(1);
	}

	std::set<Process, RtsCmp> rtsProcesses;
	std::set<Process, MfqsCmp> mfqsProcesses;
	std::set<Process, WhsCmp> whsProcesses;

	int pid
		, burst
		, arrival
		, priority
		, deadline
		, io;

	// Ignore headings
	input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	while (input >> pid >> burst >> arrival >> priority >> deadline >> io) {
		Process p(pid, burst, burst, arrival, priority, deadline, io);
		
		if (!isRts)
			rtsProcesses.insert(p);
		else if (!isMfqs)
			mfqsProcesses.insert(p);
		else if (!isWhs)
			whsProcesses.insert(p);
	}

#ifdef DEBUG
	if (!isRts)
		for (Process p : rtsProcesses)
			std::cout << p << std::endl;		
	else if (!isMfqs)
		for (Process p : mfqsProcesses)
			std::cout << p << std::endl;		
	else if (!isWhs)
		for (Process p : whsProcesses)
			std::cout << p << std::endl;		
#endif

	if (!isRts)
		rts(rtsProcesses);
	else if (!isMfqs)
		mfqs(mfqsProcesses);
	else if (!isWhs)
		whs(whsProcesses);

	return 0;
}
