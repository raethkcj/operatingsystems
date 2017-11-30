#include <ostream>

class Process {
public:
	Process();
	Process(int, int, int, int, int, int, int, int);

	int pid;
	int maxBurst;
	int burst;
	int arrival;
	int initPriority;
	int priority;
	int deadline;
	int io;

	int age;
};

struct RtsCmp {
	bool operator()(const Process&, const Process&);
};

struct MfqsCmp {
	bool operator()(const Process&, const Process&);
};

struct WhsCmp {
	bool operator()(const Process&, const Process&);
};

std::ostream& operator<<(std::ostream&, const Process&);
