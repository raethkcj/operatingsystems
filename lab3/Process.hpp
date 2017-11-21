#include <ostream>

class Process {
public:
	Process(int, int, int, int, int, int);

	int pid;
	int burst;
	int arrival;
	int priority;
	int deadline;
	int io;

	bool operator<(Process) const;
};

std::ostream& operator<<(std::ostream&, const Process&);
