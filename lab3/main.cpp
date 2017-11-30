#include <iostream>
#include <fstream>
#include <set>
#include <iomanip>
#include <limits>
#include <ios>
#include <cstring> // strcmp()
#include <vector>
#include <deque>
#include <map>
#include <algorithm> // not bubble sort
#include <string> // argument checks
#include <cmath> // pow()

#include "Process.hpp"

void tick(int*, std::vector<Process>*);

// Real-Time Scheduler
void rts(std::set<Process, RtsCmp> processes, bool isSoft) {
	int time = 0;
	int TWT = 0;
	int TTT = 0;
	int NP = 0;
	while (!processes.empty()) {
		std::set<Process, RtsCmp>::iterator p = processes.begin();
		// Get the next unexpired process
		while (p != processes.end() && p->deadline < (time + p->burst)) {
			if(!isSoft) {
				std::cout << "Process " << p->pid << " did not finish. Exiting due to hard RTS." << std::endl;
				exit(1);
			} else {
				std::cout << "Reporting process " << p->pid << " because it would not finish." << std::endl;
			}
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
// TODO: Aging
// TODO: Sanitize invalid values
void mfqs(std::set<Process, MfqsCmp> processes, int nQueues, int maxQuantum, int ageThresh) {
	int time = 0;
	long TWT = 0;
	long TTT = 0;
	int NP = 0;
	std::vector<Process> queues[nQueues];
	bool queuesEmpty = true;

	while (!(processes.empty() && queuesEmpty)) {
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
		int quantum = maxQuantum;
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
					//++time;
					tick(&time, &queues[nQueues - 1]);
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
				//++time;
				tick(&time, &queues[nQueues - 1]);
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
				//++time;
				tick(&time, &queues[nQueues - 1]);
			}
		}
		queuesEmpty = true;
		for (int j = 0; queuesEmpty && j < nQueues; ++j)
			queuesEmpty = queues[j].empty();
	}
	long AWT = TWT / NP;
	long ATT = TTT / NP;
	std::cout
		<< "AWT: " << AWT
		<< "	ATT: " << ATT
		<< "	NP: " << NP
		<< std::endl;
}

// Windows Hybrid Scheduler
void whs(std::set<Process, WhsCmp> processes, int quantum, int ageThreshold) {
	std::vector<std::deque<Process>> queues;
	queues.resize(100);

	std::map<int, Process> io_queue;	

	int time = 0;
	long TTT = 0;
	long TWT = 0;
	int NP = 0;
	// While any of the queues are not empty
	while(!processes.empty() || !io_queue.empty() || std::any_of(queues.begin(), queues.end(), [](const std::deque<Process>& q) { return !q.empty(); })) {
		// Put all the arrived processes in their initial queues
		std::set<Process, WhsCmp>::iterator p = processes.begin();
		while(!processes.empty() && p != processes.end() && time >= p->arrival) {
			queues[p->priority].push_back(*p);
			p = processes.erase(p);
		}
		// Get the highest priority non-empty queue
		std::vector<std::deque<Process>>::reverse_iterator queue = queues.rbegin();
		while(queue->empty() && queue != queues.rend()) {
			queue++;
		}
		if (queue == queues.rend()) {
			time++;
			int i = 1;
			std::map<int, Process>::iterator io_p = io_queue.begin();
			while(io_p != io_queue.end()) {
				io_p->second.age += i;
				if (io_p->second.age >= io_p->second.io) {
					io_p->second.age = 0;
					Process process = io_p->second;
					io_queue.erase(io_p);
					queues[process.priority].push_back(process);
					io_p = io_queue.begin();
				} else {
					io_p++;
				}
			}
			continue;
		}
		Process process = queue->front();
		queue->pop_front();
		// i = elapsed time during this time quantum
		int i;
		// Run up to the time quantum, process done, or process IO
		for(i = 0; process.burst > 0 && i < quantum - 1; i++) {
#ifdef DEBUG
			std::cout
				<< std::setw(4)
				<< time
				<< " -- Running "
				<< process.pid
				<< " with priority "
				<< process.priority
				<< " and remaning burst "
				<< process.burst
				<< std::endl;
#endif
			process.burst--;
			time++;
		}
		std::pair<int, Process> io_pair;
		if (process.burst > 0) {
			if (process.io > 0) {
				// Go do IO, setting new priority and resetting age
				process.age = 0;
				if (process.initPriority >= 50) {
					process.priority = (process.priority+process.io > 99) ? 99 : process.priority + process.io;
				} else {
					process.priority = (process.priority+process.io > 49) ? 49 : process.priority + process.io;
				}
				io_pair = std::make_pair(process.pid, process);
			} else {
				// Finish the time quantum
				process.burst--;
				time++;
				i++;
			}
		}

		// Age bottom 10 process queues, potentially promote
		for(int j = 0; j < 10; j++) {
			std::deque<Process>::iterator p = queues[j].begin();
			while(p != queues[j].end()) {
				p->age += i;
				if (p->age >= ageThreshold) {
					// Promote
					Process process = *p;
					p = queues[j].erase(p);
#ifdef DEBUG
					std::cout
						<< "AGE: Process "
						<< process.pid
						<< " moving from priority "
						<< process.priority
						<< " to " ;
#endif
					process.age = 0;
					if (process.initPriority >= 50) {
						process.priority = (process.priority+10 > 99) ? 99 : process.priority + 10;
					} else {
						process.priority = (process.priority+10 > 49) ? 49 : process.priority + 10;
					}
#ifdef DEBUG
					std::cout << process.priority << std::endl;
#endif
					queues[process.priority].push_back(process);
				} else {
					p++;
				}
			}
		}

		std::map<int, Process>::iterator io_p = io_queue.begin();
		while(io_p != io_queue.end()) {
			io_p->second.age += i;
			if (io_p->second.age >= io_p->second.io) {
				io_p->second.age = 0;
				Process process = io_p->second;
				io_queue.erase(io_p);
				queues[process.priority].push_back(process);
				io_p = io_queue.begin();
			} else {
				io_p++;
			}
		}
		if (process.burst > 0 && process.io > 0) {
#ifdef DEBUG
			std::cout << "IO: " << io_pair.second << std::endl;
#endif
			io_queue.insert(io_pair);
		}

		// Only demote if there's remaining burst and we didn't do IO
		if (process.burst > 0 && i == quantum) {
			// Quantum expired, need to demote (but not below initPriority)
#ifdef DEBUG
			std::cout
				<< "DEMOTE: Demoted process "
				<< process.pid
				<< " by "
				<< quantum
				<< " from "
				<< process.priority
				<< " to ";
#endif
			process.priority -= quantum;
			if (process.priority < process.initPriority) process.priority = process.initPriority;
#ifdef DEBUG
			std::cout << process.priority << std::endl;
#endif
			// Reset aging timer
			process.age = 0;
			queues[process.priority].push_back(process);
		} else if (process.burst <= 0) {
			// Done!
			NP++;
			TTT += time - process.arrival;
			TWT += time - process.arrival - process.maxBurst; 
		}
	}
	long AWT = TWT / NP;
	long ATT = TTT / NP;
	std::cout
		<< "AWT: " << AWT
		<< "	ATT: " << ATT
		<< "	NP: " << NP
		<< std::endl;
}

// Increment time and age queue (so pass in bottom queue!)
void tick(int *time, std::vector<Process> *queue) {
	++*time;
	for (std::vector<Process>::iterator i = queue->begin();
		 i != queue->end();
		 ++i) {
		++(i->age);
	}
}

void usageExit(std::string program) {
	std::cout << "usage: " << program << " rts <infile>" << std::endl;
	std::cout << "    " << program
			  << " mfqs <nQueues> <maxQuantum> <ageThresh> <infile>" << std::endl;
	std::cout << "    " << program
			  << " whs <maxQuantum> <ageThresh> <infile>" << std::endl;
	std::cout << "For debug mode, run make clean, then make debug" << std::endl;
	exit(1);
}

int main(int argc, char **argv) {
	// Check arguments
	bool isRts
		, isMfqs
		, isWhs;

	int nQueues;
	int maxQuantum;
	int ageThresh;
	std::string filename;

	if (argc >= 3) {
		isRts = strcmp(argv[1], "rts") == 0;
		isMfqs = strcmp(argv[1], "mfqs") == 0;
		isWhs = strcmp(argv[1], "whs") == 0;

		if (isRts)
			filename = argv[2];
		else if (isMfqs && argc == 6) {
			nQueues = atoi(argv[2]);
			maxQuantum = atoi(argv[3]);
			ageThresh = atoi(argv[4]);
			filename = argv[5];
		} else if (isWhs && argc == 5) {
			maxQuantum = atoi(argv[2]);
			ageThresh = atoi(argv[3]);
			filename = argv[4];
		} else
			usageExit(argv[0]);
	} else
		usageExit(argv[0]);

	std::ifstream input(filename);
	if (!input.good()) {
		std::cout
			<< argv[0] << ": error: "
			<< filename << ": No such file or directory" << std::endl;
		exit(1);
	}

	if (isMfqs && (nQueues < 2 || nQueues > 5)) {
		std::cout
			<< argv[0] << ": error: "
			<< nQueues << ": nQueues must be between 2 and 5" << std::endl;
		exit(1);
	}

	if (isMfqs && maxQuantum < pow(2, nQueues - 1)) {
		std::cout
			<< argv[0] << ": error: "
			<< maxQuantum << ": maxQuantum must be enough to be halved each queue "
			<< "( 2^(nQueues-1) )" << std::endl;
		exit(1);
	}

	if (!isRts && ageThresh <= 0) {
		std::cout
			<< argv[0] << ": error: "
			<< ageThresh << ": ageThresh must be greater than 0" << std::endl;
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
		Process p(pid, burst, burst, arrival, priority, priority, deadline, io);
		
		if (isRts)
			rtsProcesses.insert(p);
		else if (isMfqs)
			mfqsProcesses.insert(p);
		else if (isWhs)
			whsProcesses.insert(p);
	}

#ifdef DEBUG
	if (isRts)
		for (Process p : rtsProcesses)
			std::cout << p << std::endl;		
	else if (isMfqs)
		for (Process p : mfqsProcesses)
			std::cout << p << std::endl;		
	else if (isWhs)
		for (Process p : whsProcesses)
			std::cout << p << std::endl;		
#endif

	if (isRts)
		rts(rtsProcesses, true); // TODO Get isSoft from user
	else if (isMfqs)
		mfqs(mfqsProcesses, nQueues, maxQuantum, ageThresh);
	else if (isWhs)
		whs(whsProcesses, maxQuantum, ageThresh);

	return 0;
}
