#include <iostream>

#include "Process.hpp"

int main() {
	Process p(1, 30, 0, 5, 50, 0);
	std::cout << "pid: " << p.pid << "\n";
	std::cout << "burst: " << p.burst << "\n";
	std::cout << "arrival: " << p.arrival << "\n";
	std::cout << "priority: " << p.priority << "\n";
	std::cout << "deadline: " << p.deadline << "\n";
	std::cout << "io: " << p.io << "\n";
	return 0;
}
