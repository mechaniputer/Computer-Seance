#include <iostream>
#include <assert.h>
#include "computer.hpp"

Computer::Computer(int memwidth, int memlocs, int registers, uint8_t *init_contents, int len_contents) {
	width_bytes = memwidth;
	num_words = memlocs;
	num_regs = registers;

	regs = new uint8_t[(width_bytes) * (num_regs)];
	memory = new uint8_t[(width_bytes) * (num_words)];
	pc = 0;

	// Zero regs
	for (int i=0; i < (num_regs * width_bytes); i++) {
		regs[i] = uint8_t(0);
	}

	// Load or zero memory
	if (init_contents) {
		// TODO replace with improved helper that supports load to any location
		for(int i=0; i < len_contents; i++){
			memory[i] = init_contents[i];
		}
	} else {
		for(int i=0; i < memlocs; i++){
			memory[i] = uint8_t(0);
		}
	}
}

Computer::~Computer() {
	delete[] memory;
	delete[] regs;
}

// TODO add bounds checks
void Computer::dumpmem(int addr, int len) {
	if ((num_words * width_bytes) == 0) {
		return; // No memory
	}

	for (int i=addr; i < addr+len; i++) {
		std::cout << std::hex << i << std::dec << ": ";
		for (int j=0; j < width_bytes; j++) {
			std::cout << std::hex << (int) memory[(i*width_bytes)+j] << std::dec;
			if (j < width_bytes-1) std::cout << ',';
		}
		std::cout << std::endl;
	}
}

void Computer::show_regs() {
	if (num_regs < 1){
		return; // No registers
	}

	std::cout << "pc: " << pc << " regs: ";
	for (int i=0; i<num_regs; i++) {
		std::cout << '(';
		for (int j=0; j < width_bytes; j++) {
			std::cout << std::hex << (int) regs[(i*width_bytes)+j] << std::dec;
			if (j < width_bytes-1) std::cout << ',';
		}
		std::cout << ')';
	}
	std::cout << std::endl;
}

// Note: only the first byte is considered as instructions
// This serves as example code only for now
int Computer::step() {
	assert(pc >= 0);
	if ((pc*width_bytes) >= (width_bytes*num_words)) {
		return 1; // Already out of bounds (may be a 0-byte machine for all we know)
	}

	switch (memory[pc*width_bytes]) {
		default:
			std::cout << "Undefined instruction\n";
			return 1;
			break;
	}
	pc++;
	return !((pc > 0) && (pc < num_words));
}

