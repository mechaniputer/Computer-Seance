#include <iostream>
#include "computer.hpp"

using namespace std;

// TODO support init_contents smaller than entire mem and zero the rest
Computer::Computer(int memwidth, int memlocs, int registers, char *init_contents) {
	width_bytes = memwidth;
	num_words = memlocs;
	num_regs = registers;

	regs = new char[(width_bytes) * (num_regs)];
	memory = new char[(width_bytes) * (num_words)];
	pc = 0;

	// Zero regs
	for (int i=0; i < (num_regs * width_bytes); i++) {
		regs[i] = char(0);
	}

	// Load or zero memory
	if (init_contents) {
		for(int i=0; i < (width_bytes * num_words); i++){
			memory[i] = init_contents[i];
		}
	} else {
		for(int i=0; i < memlocs; i++){
			memory[i] = char(0);
		}
	}
}

Computer::~Computer() {
	delete[] memory;
	delete[] regs;
}

void Computer::dump() {
	if ((num_words * width_bytes) == 0) {
		return; // No memory
	}

	for (int i=0; i < num_words; i++) {
		for (int j=0; j < width_bytes; j++) {
			cout << int(memory[(i*width_bytes)+j]) << ',';
		}
		cout << endl;
	}
}

void Computer::show_regs() {
	if (num_regs < 1){
		return; // No registers
	}

	cout << "pc: " << pc << " regs: ";
	for (int i=0; i<num_regs; i++) {
		cout << '(';
		for (int j=0; j < width_bytes; j++) {
			cout << int(regs[(i*width_bytes)+j]) << ',';
		}
		cout << ')';
	}
	cout << endl;
}

// TODO add instruction set
// flow control, arithmetic, random, halt, I/O (memory mapped disp, char disp)
// Non-zero return values represent fault or halt (may add detailed error codes later)
// Note: only the first byte is considered as instructions
int Computer::step() {
	if ((pc*width_bytes) >= (width_bytes*num_words)) {
		return 1; // Already out of bounds (may be a 0-byte machine for all we know)
	}

	switch (memory[pc*width_bytes]) {
		case char(0):
			cout << "instr 0\n";
			break;
		case char(1):
			cout << "instr 1\n";
			break;
		case char(2):
			cout << "instr 2\n";
			break;
		case char(3):
			cout << "instr 3\n";
			break;
		case char(4):
			cout << "instr 4\n";
			break;
		default:
			cout << "instr ? HALT\n";
			return 1;
			break;
	}
	pc++;
	return !((pc > 0) && (pc < num_words));
}

