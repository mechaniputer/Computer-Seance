#include <iostream>
#include <fstream>
#include "computer.hpp"
#include "raquette.hpp"

#define BASEBYTES 2
#define BASEWORDS 16
#define BASEREGS 1

void test_base_computer(){
	std::cout << "Testing base computer with " << BASEBYTES << "x" << BASEWORDS << " memory and " << BASEREGS << " registers\n";

	// Initial memory values
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

// This is a temporary debugging test that expects a proprietary ROM that we cannot not include in the repo.
// We will have our own FOSS ROM eventually.
void test_raq_romfile(){
//	std::ifstream infile("./software/raquette/OUT.BIN", std::ios::binary | std::ios::in);
//	std::ifstream infile("A2ROM.BIN", std::ios::binary | std::ios::in);
//	std::ifstream infile("apple.rom", std::ios::binary | std::ios::in);
	std::ifstream infile("../software/raquette/rom/raq_rom.bin", std::ios::binary | std::ios::in);
	if(!infile){
		std::cout << "Cannot open ROM file\n";
		return;
	}
	//get length of file
	infile.seekg(0, std::ios::end);
	size_t length = infile.tellg();
	infile.seekg(0, std::ios::beg);

	char * buffer = new char[length];
	std::cout << "Opened rom of length " << length << std::endl;
	infile.read(buffer, length);

	uint8_t raq_rom_arr[0xFFFF];
	for (unsigned i=0; i < 0xD000; i++) {
		raq_rom_arr[i] = 0x00; // Zero low mem
	}
	// TODO merge this functionality into raquette class
	for (unsigned i=0; i < length; i++) {
//		raq_rom_arr[i+0xD000] = buffer[i];
		raq_rom_arr[1+i+(0xFFFF-length)] = buffer[i];
	}

	delete [] buffer;

	Raquette raquette(raq_rom_arr, 0xFFFF+1);

	raquette.show_regs();
	raquette.consoleSession();
//	while (!raquette.step()) {
//	for(int erg =0; erg<5; erg++){
//		raquette.step(true);
//		raquette.show_regs();
//	}
//	raquette.show_regs();
}

void test_raq_all(){
	std::ifstream infile("../software/raquette/functionalTest/6502_functional_test.bin", std::ios::binary | std::ios::in);
	if(!infile){
		std::cout << "Cannot open ROM file\n";
		return;
	}
	//get length of file
	infile.seekg(0, std::ios::end);
	size_t length = infile.tellg();
	infile.seekg(0, std::ios::beg);

	char * buffer = new char[length];
	std::cout << "Opened file of length " << length << std::endl;
	infile.read(buffer, length);

	// Zero low mem
	uint8_t raq_rom_arr[0xFFFF];
	uint8_t tmp;
	// Copy in ROM
	for (unsigned i=0; i < length; i++) {
		tmp = buffer[i];
		raq_rom_arr[i] = tmp;
	}

	delete [] buffer;

	Raquette raquette(raq_rom_arr, 0xFFFF+1);

	// Have to manually set PC for this test suite
	std::cout << "Manually set PC to 0x400\n";
	raquette.pc = 0x0400;

	int prevpc = 0xFFFFF;
	int numsteps = 0;
	while(true){
		numsteps++;
		if(raquette.step(false)) break;
		if(raquette.pc == prevpc) break;
		prevpc = raquette.pc;
	}
	raquette.show_regs();
	std::cout << "Executed " << numsteps << " instructions\n";

}

int main() {
//	test_base_computer();
	test_raq_romfile(); // Loads a 12K ROM file into high mem and runs it
//	test_raq_all(); // Loads a ~13k functional test ROM file to 0x0400 and runs it

	return 0;
}
