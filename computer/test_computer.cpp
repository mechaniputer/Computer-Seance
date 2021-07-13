#include <iostream>
#include "computer.hpp"

using namespace std;

#define BYTES 1
#define WORDS 16
#define REGS 2
int main() {
	cout << "Testing machine with " << BYTES << "x" << WORDS << " memory and " << REGS << " registers\n";

	// Initial memory values (TODO load from file)
	char memcontents[WORDS*BYTES];
	for (int i=0; i < WORDS*BYTES; i++) {
		memcontents[i] = char(i);
	}

	// Create computer
	Computer EPICAC(BYTES, WORDS, REGS, memcontents);

	// Show and run computer
	EPICAC.dump();
	EPICAC.show_regs();
	while (!EPICAC.step()) {
		EPICAC.show_regs();
	}
	EPICAC.show_regs();

	return 0;
}
