#include <ostream>

class Process {
public:
	Process(int, int, int, int, int, int, int);

	int pid;
	int maxBurst;
	int burst;
	int arrival;
	int priority;
	int deadline;
	int io;
};

struct rtsCmp {
	bool operator()(const Process&, const Process&);
};

struct mfqsCmp {
	bool operator()(const Process&, const Process&);
};

struct whsCmp {
	bool operator()(const Process&, const Process&);
};

std::ostream& operator<<(std::ostream&, const Process&);
