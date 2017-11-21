#include <iostream>
#include <fstream>
#include <vector>
#include <set>

#include "Process.hpp"

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
	/*	std::cout << "pid: " << p.pid << "\n";
		std::cout << "burst: " << p.burst << "\n";
		std::cout << "arrival: " << p.arrival << "\n";
		std::cout << "priority: " << p.priority << "\n";
		std::cout << "deadline: " << p.deadline << "\n";
		std::cout << "io: " << p.io << "\n";*/
		processes.insert(p);
	}

	for(Process p : processes) {
		std::cout << p << "\n";
	}

	return 0;
}
