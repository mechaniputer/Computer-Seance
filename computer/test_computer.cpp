#include <iostream>
#include <fstream>
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

void test_raq_disp(){
	std::cout << "Testing Raquette computer display output\n";

	// Displays "RAQUETTE 64" on second row of display
	// All other positions should display "*"
	uint8_t raq_test_disp[0xFFFF];
	for (unsigned i=0x0400; i < 0x07FF; i++) { // Note that this also fills the 8 ignored bytes
		raq_test_disp[i] = 0xAA; // char '*'
	}
	raq_test_disp[0x048E] = 0xD2; // char R
	raq_test_disp[0x048F] = 0x81; // char A
	raq_test_disp[0x0490] = 0xD1; // char Q
	raq_test_disp[0x0491] = 0xD5; // char U
	raq_test_disp[0x0492] = 0xC5; // char E
	raq_test_disp[0x0493] = 0xD4; // char T
	raq_test_disp[0x0494] = 0xD4; // char T
	raq_test_disp[0x0495] = 0xC5; // char E
	raq_test_disp[0x0496] = 0xA0; // char sp
	raq_test_disp[0x0497] = 0xB6; // char 6
	raq_test_disp[0x0498] = 0xB4; // char 4

	Raquette raquette(raq_test_disp, 0xFFFF);

	raquette.dumpmem(0x0480,40); // second row
	raquette.show_screen();

	return;
}

// This is a temporary debugging test that expects a proprietary ROM that we cannot not include in the repo.
// We will have our own FOSS ROM eventually.
void test_raq_romfile(){
	std::ifstream infile("A2ROM.BIN", std::ios::binary | std::ios::in);
	if(!infile){
		std::cout << "Cannot open ROM file\n";
		return;
	}
	//get length of file
	infile.seekg(0, std::ios::end);
	size_t length = infile.tellg();
	infile.seekg(0, std::ios::beg);

	char * buffer = new char[length];
	std::cout << "length " << length << std::endl;
	infile.read(buffer, length);

	uint8_t raq_rom_arr[0xFFFF];
	for (unsigned i=0; i < 0xD000; i++) {
		raq_rom_arr[i] = 0x00; // Zero low mem
	}
	for (unsigned i=0; i < length; i++) {
		raq_rom_arr[i+0xD000] = buffer[i];
		std::cout << std::hex << i+0xD000 << std::dec << std::endl;
	}

	delete [] buffer;

	Raquette raquette(raq_rom_arr, 0xFFFF+1);
	raquette.show_regs();
	int numstep = 0;

	raquette.memory[0x0] = 0xDE;
	raquette.memory[0x1] = 0xAD;
	raquette.memory[0x2] = 0xBE;
	raquette.memory[0x3] = 0xEF;
	raquette.memory[0x5] = 0xC0;
	raquette.memory[0x6] = 0xFF;
	raquette.memory[0x7] = 0xEE;
//	raquette.memory[0xC000] = (0x0D | 0b10000000);

	while (!raquette.step(true)) {
		numstep++;
		raquette.show_regs();
		std::cout << "step " << numstep << std::endl;
		if(raquette.pc == 0xFD21) raquette.show_screen(); // FD21 is the keyboard loop
		if(raquette.pc < 0xF800){
			if((numstep % 1000) == 0) raquette.show_screen();
			raquette.memory[0xC000] = 0x0; // Clear keyboard just in case
//			std::cin.get();
		}
	}
	numstep++;
	raquette.show_regs();

	raquette.dumpmem(raquette.pc,8);
//	raquette.dumpmem(0x01F9,8);
	raquette.show_screen();
	std::cout << "Ran for " << numstep << " steps\n";
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

	uint8_t raq_sbc_test[8];
	raq_sbc_test[0] = 0x38; // SEC
//	raq_sbc_test[0] = 0x18; // CLC
	raq_sbc_test[1] = 0xA9; // LDA Immediate
	raq_sbc_test[2] = 0xd0; // 
	raq_sbc_test[3] = 0xE9; // SBC Immediate
	raq_sbc_test[4] = 0x30; //
	raq_sbc_test[5] = 0x00;
	raq_sbc_test[6] = 0x00;
	raq_sbc_test[7] = 0x00;

	Raquette raquette(raq_sbc_test, 8);

	raquette.dumpmem(0x0,8);
	raquette.show_regs();
	while (!raquette.step(true)) {
		raquette.show_regs();
	}
	raquette.show_regs();
	raquette.dumpmem(0x0,8);

	return;
}

int main() {
//	test_base_computer();
//	test_raq_computer();
//	test_raq_disp();
	test_raq_romfile(); // Loads a 12K ROM file into high mem and runs it

	return 0;
}
