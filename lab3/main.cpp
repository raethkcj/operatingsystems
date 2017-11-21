#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <iomanip>

#include "Process.hpp"

void rts(std::set<Process> processes) {
	int time = 0;
	while(!processes.empty()) {
		std::set<Process>::iterator p = processes.begin();
		// Get the next unexpired process
		while(p->deadline < (time + p->burst)) {
			processes.erase(p);
			p = processes.begin();
		}

		if(time >= p->arrival) {
			std::cout << std::setw(4) << time << " -- Running " << p->pid << " with remaining burst: " << std::setw(2) << p->burst << std::endl;
			Process tmp = *p;
			processes.erase(p);
			if(--tmp.burst > 0) {
				processes.insert(tmp);
			}
		}
		time++;
	}
}

int main() {
	std::ifstream input("9_processes");

	std::set<Process> processes;

	int pid
		, burst
		, arrival
		, priority
		, deadline
		, io;

	while (input >> pid >> burst >> arrival >> priority >> deadline >> io) {
		Process p(pid, burst, arrival, priority, deadline, io);
		processes.insert(p);
	}

	for(Process p : processes) {
		std::cout << p << "\n";
	}

	rts(processes);

	return 0;
}
