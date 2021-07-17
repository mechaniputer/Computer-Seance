#include <iostream>
#include "computer.hpp"


#define BASEBYTES 2
#define BASEWORDS 16
#define BASEREGS 1

void test_base_computer(){
	std::cout << "Testing base computer with " << BASEBYTES << "x" << BASEWORDS << " memory and " << BASEREGS << " registers\n";

	// Initial memory values (TODO load from file)
	uint8_t memcontents[BASEWORDS*BASEBYTES];
	for (unsigned i=0; i < BASEWORDS*BASEBYTES; i++) {
		memcontents[i] = i;
	}

	// Create computer
	Computer test_computer(BASEBYTES, BASEWORDS, BASEREGS, memcontents, BASEWORDS*BASEBYTES);

	// Show and run computer
	test_computer.dumpmem();
	test_computer.show_regs();
	while (!test_computer.step()) {
		test_computer.show_regs();
	}
	test_computer.show_regs();
	return;
}

void test_raq_computer(){
	std::cout << "Testing Raquette computer\n";
	uint8_t raq_asm_test[11];
	raq_asm_test[0] = 0x38; // SEC
	raq_asm_test[1] = 0xA5; // LDA zero page
	raq_asm_test[2] = 0x08; // addr 8
	raq_asm_test[3] = 0x65; // ADC zero page
	raq_asm_test[4] = 0x09; // addr 9
	raq_asm_test[5] = 0x85; // STA zero page
	raq_asm_test[6] = 0x0A; // addr 10
	raq_asm_test[7] = 0x00; // BRK
	raq_asm_test[8] = 0xD0; // x
	raq_asm_test[9] = 0x90; // y
	raq_asm_test[10] = 0x00; // z

	uint8_t raq_jmp_test[8];
	raq_jmp_test[0] = 0x4C; // JMP absolute
	raq_jmp_test[1] = 0x05;
	raq_jmp_test[2] = 0x00;
	raq_jmp_test[3] = 0x00;
	raq_jmp_test[4] = 0x00;
	raq_jmp_test[5] = 0x4C; // JMP absolute
	raq_jmp_test[6] = 0x00;
	raq_jmp_test[7] = 0xF0;

	uint8_t raq_branch_test[8];
	raq_branch_test[0] = 0x90; // BCC (forwards)
	raq_branch_test[1] = 0x04;
	raq_branch_test[2] = 0x00;
	raq_branch_test[3] = 0x00;
	raq_branch_test[4] = 0x00;
	raq_branch_test[5] = 0x00;
	raq_branch_test[6] = 0x90; // BCC (backwards)
	raq_branch_test[7] = 0xFA; // decimal -6

	Raquette raquette(raq_branch_test, 8);
	raquette.dumpmem(0x0,8);
	raquette.show_regs();
	while (!raquette.step()) {
		raquette.show_regs();
	}
	raquette.show_regs();
	raquette.dumpmem(0x0,8);
	return;
}

int main() {
//	test_base_computer();
	test_raq_computer();

	return 0;
}
