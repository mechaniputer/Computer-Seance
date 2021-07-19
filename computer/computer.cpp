#include <iostream>
#include <assert.h>
#include <tuple>
#include <ncurses.h>
#include "computer.hpp"

#define RAQ_MASK_OPC uint8_t(0b11100011)
#define RAQ_ACC (regs[0])
#define RAQ_X (regs[1])
#define RAQ_Y (regs[2])
#define RAQ_STACK (regs[3])

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

// The "Raquette" is the standard instructional machine for the University.
// TODO support smaller memory configurations than 64k
// TODO Add some assertions on memory bounds, contents, etc
Raquette::Raquette(uint8_t *init_contents, int len_contents) {
	unsigned tmp; // For intermediate values below
	int eff_addr = 0; // Effective address of interrupt vector
	width_bytes = 1;
	num_words = 0xFFFF+1;
	num_regs = 4; // A(cc), X(ind), Y(ind), S(tack ptr)

	delete(regs);
	delete(memory);
	regs = new uint8_t[(width_bytes) * (num_regs)];
	memory = new uint8_t[(width_bytes) * (num_words)];
	pc = 0;

	// 7 processor flags
	flag_c = false;
	flag_z = false;
	flag_i = false;
	flag_d = false;
	flag_b = false;
	flag_v = false;
	flag_n = false;

	// Zero regs
	for (int i=0; i < (num_regs * width_bytes); i++) {
		regs[i] = uint8_t(0);
	}

	// RAQ_STACK should be FF for 01FF stack base (and it decrements)
	RAQ_STACK = 0xFF;

	//  Zero all mem to start with
	// Note that this include the reset vector
	for(int i=0; i < (width_bytes * num_words); i++){
		memory[i] = uint8_t(0);
	}
	// Load or zero memory
	if (init_contents) {
		// TODO replace with improved helper that supports load to any location
		for(int i=0; i < len_contents; i++){
			memory[i] = init_contents[i];
		}
	}

	// Now initialize PC by reading reset vector from FFFC-FFFD
	tmp = memory[0xFFFD]; // tmp is an unsigned int with room for shifts
	eff_addr = (tmp << 8) + memory[0xFFFC];
	std::cout << "Reset vector " << std::hex << eff_addr << std::dec << std::endl;
	pc = eff_addr;
}

// Takes the current byte with the opcode part masked out
// Determines the addressing mode
// Returns a tuple of: The effective address of the current instruction and the amount to increase pc
std::tuple<int, int> Raquette::aModeHelper(uint8_t thisbyte) {
	uint8_t amode = (thisbyte & ~RAQ_MASK_OPC); // Zero-mask opcode bits
	int opbytes = 0; // Remember how much to increase PC
	int eff_addr = 0; // Effective address of current instruction
	unsigned tmp, tmp2; // For intermediate values below

	switch (amode){
		case uint8_t(0b00001000): // 08 Immediate
			// The next byte is the operand
			eff_addr = pc+1;
			opbytes = 2;
			break;

		case uint8_t(0b00000100): // 04 Zero page
			// The next byte is an address. Prepend it with 00.
			eff_addr = memory[pc+1];
			opbytes = 2;
			break;

		case uint8_t(0b00010100): // 14 Zero page, X
			// The next byte is an address. Prepend it with 00 and add the contents of the X register to it.
			eff_addr = ((memory[pc+1] + RAQ_X) % 0xFF); // Wrap around if sum of base and reg exceeds 0xFF
			opbytes = 2;
			break;

		case uint8_t(0b00001100): // 0C Absolute
			// The next two bytes specify a little endian address.
			tmp = memory[pc+2]; // tmp is an unsigned int with room for shifts
			eff_addr = (tmp << 8) + memory[pc+1];
			opbytes = 3;
			break;

		case uint8_t(0b00011100): // 1C Absolute, X
			// TODO check bounds
			// The next two bytes specify an address. Add the contents of the X register to it. (Little Endian!)
			tmp = memory[pc+2]; // tmp is an unsigned int with room for shifts
			eff_addr = (tmp << 8) + memory[pc+1] + RAQ_X;
			opbytes = 3;
			break;

		case uint8_t(0b00011000): // 18 Absolute, Y
			// TODO check bounds
			// The next two bytes specify an address. Add the contents of the Y register to it. (Little Endian!)
			tmp = memory[pc+2]; // tmp is an unsigned int with room for shifts
			eff_addr = (tmp << 8) + memory[pc+1] + RAQ_Y;
			opbytes = 3;
			break;

		case uint8_t(0b00000000): // 00 (Indirect, X)
			// TODO check bounds
			// The next byte is an address. Prepend it with 00, add the contents of X to it, and get the two-byte address from that memory location.
			tmp = memory[pc+1] + RAQ_X; // First address
			tmp2 = memory[tmp+1]; // MSB of second address
			eff_addr = (tmp2 << 8) + memory[tmp]; // Plus LSB of second address
			opbytes = 2;
			break;

		case uint8_t(0b00010000): // 10 (Indirect), Y
			// TODO check bounds
			// The next byte is an address. Prepend it with 00, get the two-byte address from that memory location, and add the contents of Y to it.
			tmp = memory[pc+1]; // tmp is addr of 2-byte addr
			tmp2 = memory[tmp+1]; // MSB
			eff_addr = (tmp2 << 8) + memory[tmp]; // Add LSB for full two-byte address
			eff_addr += RAQ_Y; // Add Y
			opbytes = 2;
			break;

		default: // Literally impossible
			assert(0);
			break;
	}
	return std::make_tuple(eff_addr, opbytes);
}

// Sets new value of pc (without increment by 2)
void Raquette::branchHelper(){
	if (memory[pc+1]&0b10000000){ // It is negative
		pc = pc - (((~memory[pc+1])+1) & 0b01111111);
	}else{ // It is positive
		pc = pc + (memory[pc+1] & 0b01111111);
	}
}

// ISA based on MOS 6502
// aaabbbcc. The aaa and cc bits determine the opcode, and the bbb bits determine the addressing mode.
// Instruction format: bits 0-2 and 6-7 determine opcode. bits 3-5 determine addressing mode.
// Little-endian (least sig byte first)
// 7 processor flags: flag_c flag_z flag_i flag_d flag_b flag_v flag_n
int Raquette::step(bool verbose) {
	assert(pc >= 0);
	if (pc >= num_words) {
		if(verbose) std::cout << "PC out of bounds\n";
		return 1; // Already out of bounds
	}

	unsigned tmp, tmp2; // For intermediate values below
	int eff_addr, opbytes;
	uint8_t thisbyte = memory[pc];

	opbytes = 0;
	switch (thisbyte) {
		case uint8_t(0x00): // BRK
			std::cout << "BRK\n";
			flag_b = true;
			// TODO push stuff to stack, do interrupt
			// Push MSB of PC
			// Push LSB of PC
			// Push status register
			// Set interrupt disable
			// Load PC from IRQ interrupt vector at 0xFFFE and 0xFFFF
			return 1;
			break;

		case uint8_t(0xA9): // LDA
		case uint8_t(0xA5):
		case uint8_t(0xB5):
		case uint8_t(0xAD):
		case uint8_t(0xBD):
		case uint8_t(0xB9):
		case uint8_t(0xA1):
		case uint8_t(0xB1):
			std::tie(eff_addr, opbytes) = aModeHelper(thisbyte);
			if(verbose) std::cout << "LDA " << std::hex << eff_addr << std::dec << " pc+=" << opbytes << std::endl;
			RAQ_ACC = memory[eff_addr];
			flag_z = (RAQ_ACC == 0); // Zero flag if zero
			flag_n = ((RAQ_ACC & 0b10000000) != 0); // Negative flag if sign bit set
			break;

		case uint8_t(0xA2): // LDX Immediate
			opbytes=2;
			eff_addr = pc+1;
			if(verbose) std::cout << "LDX Immediate " << std::hex << eff_addr << std::dec << " pc+=" << opbytes << std::endl;
			RAQ_X = memory[eff_addr];
			flag_z = (RAQ_X == 0); // Zero flag if zero
			flag_n = ((RAQ_X & 0b10000000) != 0); // Negative flag if sign bit set
			break;

		case uint8_t(0xA6): // LDX Zero Page
			opbytes = 2;
			// The next byte is an address. Prepend it with 00.
			eff_addr = memory[pc+1];
			if(verbose) std::cout << "LDX zero page " << std::hex << eff_addr << std::dec << " pc+=" << opbytes << std::endl;
			RAQ_X = memory[eff_addr];
			flag_z = (RAQ_X == 0); // Zero flag if zero
			flag_n = ((RAQ_X & 0b10000000) != 0); // Negative flag if sign bit set
			break;

		case uint8_t(0xB6): // LDX Zero Page, Y
			assert(0);
			break;

		case uint8_t(0xAE): // Absolute
			assert(0);
			break;

		case uint8_t(0xBE): // Absolute, Y
			assert(0);
			break;

		case uint8_t(0xA0): // LDY Immediate
			opbytes=2;
			eff_addr = pc+1;
			if(verbose) std::cout << "LDY Immediate " << std::hex << eff_addr << std::dec << " pc+=" << opbytes << std::endl;
			RAQ_Y = memory[eff_addr];
			flag_z = (RAQ_Y == 0); // Zero flag if zero
			flag_n = ((RAQ_Y & 0b10000000) != 0); // Negative flag if sign bit set
			break;

		case uint8_t(0xA4): // LDY Zero Page
			// The next byte is an address. Prepend it with 00.
			eff_addr = memory[pc+1];
			if(verbose) std::cout << "LDY zero page " << std::hex << eff_addr << std::dec << " pc+=" << opbytes << std::endl;
			RAQ_Y = memory[eff_addr];
			flag_z = (RAQ_Y == 0); // Zero flag if zero
			flag_n = ((RAQ_Y & 0b10000000) != 0); // Negative flag if sign bit set
			opbytes = 2;
			break;

		case uint8_t(0xB4): // LDY Zero Page, X
			// The next byte is an address. Prepend it with 0 and add X to it.
			eff_addr = memory[pc+1] + RAQ_X;
			if(verbose) std::cout << "LDY zero page " << std::hex << eff_addr << std::dec << " pc+=" << opbytes << std::endl;
			RAQ_Y = memory[eff_addr];
			flag_z = (RAQ_Y == 0); // Zero flag if zero
			flag_n = ((RAQ_Y & 0b10000000) != 0); // Negative flag if sign bit set
			opbytes = 2;
			break;

		case uint8_t(0xAC): // LDY Absolute
			assert(0);
			break;

		case uint8_t(0xBC): // LDY Absolute, X
			assert(0);
			break;

		case uint8_t(0x86): // STX Zero Page
			// The next byte is an address. Prepend it with 00.
			eff_addr = memory[pc+1];
			memory[eff_addr] = (uint8_t) RAQ_X;
			opbytes = 2;
			if(verbose) std::cout << "STX zero page " << std::hex << eff_addr << std::dec << " pc+=" << opbytes << std::endl;
			break;

		case uint8_t(0x96): // STX Zero Page, Y
			// The next byte is an address. Prepend it with 00 and add the contents of the Y register to it.
			eff_addr = ((memory[pc+1] + RAQ_Y) % 0xFF); // Wrap around if sum of base and reg exceeds 0xFF
			memory[eff_addr] = (uint8_t) RAQ_X;
			opbytes = 2;
			if(verbose) std::cout << "STX zero page, Y " << std::hex << eff_addr << std::dec << " pc+=" << opbytes << std::endl;
			break;

		case uint8_t(0x8E): // STX Absolute
			assert(0);
			break;

		case uint8_t(0x84): // STY Zero Page
			// The next byte is an address. Prepend it with 00.
			eff_addr = memory[pc+1];
			memory[eff_addr] = (uint8_t) RAQ_Y;
			opbytes = 2;
			if(verbose) std::cout << "STY zero page " << std::hex << eff_addr << std::dec << " pc+=" << opbytes << std::endl;
			break;

		case uint8_t(0x94): // STY Zero Page, X
			// The next byte is an address. Prepend it with 00 and add the contents of the X register to it.
			eff_addr = ((memory[pc+1] + RAQ_X) % 0xFF); // Wrap around if sum of base and reg exceeds 0xFF
			memory[eff_addr] = (uint8_t) RAQ_Y;
			opbytes = 2;
			if(verbose) std::cout << "STY zero page, X " << std::hex << eff_addr << std::dec << " pc+=" << opbytes << std::endl;
			break;

		case uint8_t(0x8C): // STY Absolute
			assert(0);
			break;

		case uint8_t(0xAA): // TAX
			if(verbose) std::cout << "TAY\n";
			RAQ_X = RAQ_ACC;
			flag_z = (RAQ_X == 0);
			flag_n = ((RAQ_X & 0b10000000) != 0); // Negative flag if sign bit set
			opbytes = 1;
			break;

		case uint8_t(0xA8): // TAY
			if(verbose) std::cout << "TAY\n";
			RAQ_Y = RAQ_ACC;
			flag_z = (RAQ_Y == 0);
			flag_n = ((RAQ_Y & 0b10000000) != 0); // Negative flag if sign bit set
			opbytes = 1;
			break;

		case uint8_t(0x8A): // TXA
			if(verbose) std::cout << "TXA\n";
			RAQ_ACC = RAQ_X;
			flag_z = (RAQ_ACC == 0);
			flag_n = ((RAQ_ACC & 0b10000000) != 0); // Negative flag if sign bit set
			opbytes = 1;
			break;

		case uint8_t(0x98): // TYA
			if(verbose) std::cout << "TYA\n";
			RAQ_ACC = RAQ_Y;
			flag_z = (RAQ_ACC == 0);
			flag_n = ((RAQ_ACC & 0b10000000) != 0); // Negative flag if sign bit set
			opbytes = 1;
			break;

		case uint8_t(0x9A): // TXS
			if(verbose) std::cout << "TXS\n";
			RAQ_STACK = RAQ_X;
			opbytes = 1;
			break;


		case uint8_t(0xCA): // DEX
			if(verbose) std::cout << "DEX\n";
			RAQ_X = RAQ_X - 1;
			flag_z = (RAQ_X == 0);
			flag_n = ((RAQ_X & 0b10000000) != 0); // Negative flag if sign bit set
			opbytes = 1;
			break;

		case uint8_t(0x88): // DEY
			if(verbose) std::cout << "DEY\n";
			RAQ_Y = RAQ_Y - 1;
			flag_z = (RAQ_Y == 0);
			flag_n = ((RAQ_Y & 0b10000000) != 0); // Negative flag if sign bit set
			opbytes = 1;
			break;

		case uint8_t(0xE6): // INC Zero Page
			if(verbose) std::cout << "INC Zero Page\n";
			eff_addr = memory[pc+1];
			memory[eff_addr] += 1;
			flag_z = (memory[eff_addr] == 0); // Zero flag if zero
			flag_n = ((memory[eff_addr] & 0b10000000) != 0); // Negative flag if sign bit set
			opbytes = 2;
			break;

		case uint8_t(0xF6): // INC Zero Page, X
			if(verbose) std::cout << "INC Zero Page, X\n";
			eff_addr = memory[pc+1] + RAQ_X;
			memory[eff_addr] += 1;
			flag_z = (memory[eff_addr] == 0); // Zero flag if zero
			flag_n = ((memory[eff_addr] & 0b10000000) != 0); // Negative flag if sign bit set
			opbytes = 2;
			break;

		case uint8_t(0xEE): // INC Absolute
			if(verbose) std::cout << "INC AbsoluteX\n";
			return 1;
			break;

		case uint8_t(0xFE): // INC Absolute, X
			if(verbose) std::cout << "INC Absolute, X\n";
			return 1;
			break;

		case uint8_t(0xC6): // DEC Zero Page
			if(verbose) std::cout << "DEC Zero Page\n";
			eff_addr = memory[pc+1];
			memory[eff_addr] -= 1;
			flag_z = (memory[eff_addr] == 0); // Zero flag if zero
			flag_n = ((memory[eff_addr] & 0b10000000) != 0); // Negative flag if sign bit set
			opbytes = 2;
			break;

		case uint8_t(0xD6): // DEC Zero Page, X
			if(verbose) std::cout << "DEC Zero Page, X\n";
			return 1;
			break;

		case uint8_t(0xCE): // DEC Absolute
			if(verbose) std::cout << "DEC AbsoluteX\n";
			return 1;
			break;

		case uint8_t(0xDE): // DEC Absolute, X
			if(verbose) std::cout << "DEC Absolute, X\n";
			return 1;
			break;

		case uint8_t(0xE8): // INX
			if(verbose) std::cout << "INX\n";
			RAQ_X += 1;
			flag_z = (RAQ_X == 0); // Zero flag if zero
			flag_n = ((RAQ_X & 0b10000000) != 0); // Negative flag if sign bit set
			opbytes = 1;
			break;

		case uint8_t(0xC8): // INY
			if(verbose) std::cout << "INY\n";
			RAQ_Y += 1;
			flag_z = (RAQ_Y == 0); // Zero flag if zero
			flag_n = ((RAQ_Y & 0b10000000) != 0); // Negative flag if sign bit set
			opbytes = 1;
			break;

		case uint8_t(0x69): // ADC
		case uint8_t(0x65):
		case uint8_t(0x75):
		case uint8_t(0x6D):
		case uint8_t(0x7D):
		case uint8_t(0x79):
		case uint8_t(0x61):
		case uint8_t(0x71):
			std::tie(eff_addr, opbytes) = aModeHelper(thisbyte);
			if(verbose) std::cout << "ADC " << std::hex << eff_addr << std::dec << " pc+=" << opbytes << std::endl;
			tmp = RAQ_ACC + memory[eff_addr] + (flag_c ? 1 : 0);

			// TODO Handle decimal mode
			if(flag_d) {
				assert(0);
			}
			flag_c = (tmp > 0xFF); // Carry flag
			flag_v = (((RAQ_ACC ^ tmp) & (memory[eff_addr] ^ tmp) & 0x80) != 0); // Overflow flag if sign bit is incorrect
			RAQ_ACC = tmp & 0xFF; // Assign final value
			flag_z = (RAQ_ACC == 0); // Zero flag if zero
			flag_n = ((RAQ_ACC & 0b10000000) != 0); // Negative flag if sign bit set
			break;

		case uint8_t(0xE9): // SBC
		case uint8_t(0xE5):
		case uint8_t(0xF5):
		case uint8_t(0xED):
		case uint8_t(0xFD):
		case uint8_t(0xF9):
		case uint8_t(0xE1):
		case uint8_t(0xF1):
			// Note, we assume that carry is set unless the previous SBC needed a borrow
			std::tie(eff_addr, opbytes) = aModeHelper(thisbyte);
			if(verbose) std::cout << "SBC " << std::hex << eff_addr << std::dec << " pc+=" << opbytes << std::endl;
			tmp = RAQ_ACC - memory[eff_addr] - (flag_c ? 0 : 1);

			// TODO Handle decimal mode
			if(flag_d) {
				assert(0);
			}
			flag_c = (tmp < 0x100); // Carry flag
			flag_v = (((RAQ_ACC ^ tmp) & (~memory[eff_addr] ^ tmp) & 0x80) != 0); // This is the same overflow formula for ADC except operand is flipped
			RAQ_ACC = tmp & 0xFF; // Assign final value
			flag_z = (RAQ_ACC == 0); // Zero flag if zero
			flag_n = ((RAQ_ACC & 0b10000000) != 0); // Negative flag if sign bit set
			break;

		case uint8_t(0x24): // BIT Zero Page
			if(verbose) std::cout << "BIT Zero Page\n";
			eff_addr = memory[pc+1];
			// & with ACC for zero, and map bits of word in memory to flags
			tmp = RAQ_ACC & memory[eff_addr];
			flag_z = (tmp == 0); // Zero flag if zero
			flag_v = ((memory[eff_addr] & 0b01000000) != 0); // bit 6 maps to V
			flag_n = ((memory[eff_addr] & 0b10000000) != 0); // bit 7 maps to N
			opbytes = 2;
			break;

		case uint8_t(0x2C): // BIT Absolute
			if(verbose) std::cout << "BIT Absolute\n";
			// Next two bytes are little endian address
			tmp = memory[pc+2]; // tmp is an unsigned int with room for shifts
			eff_addr = (tmp << 8) + memory[pc+1];
			// & with ACC for zero, and map bits of word in memory to flags
			tmp = RAQ_ACC & memory[eff_addr];
			flag_z = (tmp == 0); // Zero flag if zero
			flag_v = ((memory[eff_addr] & 0b01000000) != 0); // bit 6 maps to V
			flag_n = ((memory[eff_addr] & 0b10000000) != 0); // bit 7 maps to N
			opbytes = 3;
			break;

		case uint8_t(0xC9): // CMP
		case uint8_t(0xC5):
		case uint8_t(0xD5):
		case uint8_t(0xCD):
		case uint8_t(0xDD):
		case uint8_t(0xD9):
		case uint8_t(0xC1):
		case uint8_t(0xD1):
			std::tie(eff_addr, opbytes) = aModeHelper(thisbyte);
			if(verbose) std::cout << "CMP " << std::hex << eff_addr << std::dec << " pc+=" << opbytes << std::endl;
			tmp = RAQ_ACC - memory[eff_addr];
			flag_z = (RAQ_ACC == memory[eff_addr]); // Zero flag if equal
			flag_n = ((tmp & 0b10000000) != 0); // Negative flag if sign bit set
			flag_c = (RAQ_ACC >= memory[eff_addr]); // Carry flag
			break;

		case uint8_t(0xE0): // CPX Immediate
			if(verbose) std::cout << "CPX Immediate\n";
			eff_addr = pc+1;
			tmp = RAQ_X - memory[eff_addr];
			flag_z = (tmp == 0); // Zero flag if zero
			flag_n = ((tmp & 0b10000000) != 0); // Negative flag if sign bit set
			opbytes = 2;
			break;

		case uint8_t(0xE4): // CPX Zero Page
			if(verbose) std::cout << "CPX Zero Page\n";
			eff_addr = memory[pc+1];
			tmp = RAQ_X - memory[eff_addr];
			flag_z = (tmp == 0); // Zero flag if zero
			flag_n = ((tmp & 0b10000000) != 0); // Negative flag if sign bit set
			flag_c = (RAQ_X >= memory[eff_addr]); // Carry flag
			opbytes = 2;
			break;

		case uint8_t(0xEC): // CPX Absolute
			std::cout << "CPX Absolute\n";
			return 1;
			opbytes = 3;
			break;

		case uint8_t(0xC0): // CPY Immediate
			std::cout << "CPY Immediate\n";
			return 1;
			opbytes = 2;
			break;

		case uint8_t(0xC4): // CPY Zero Page
			if(verbose) std::cout << "CPY Zero Page\n";
			eff_addr = memory[pc+1];
			tmp = RAQ_Y - memory[eff_addr];
			flag_z = (tmp == 0); // Zero flag if zero
			flag_n = ((tmp & 0b10000000) != 0); // Negative flag if sign bit set
			flag_c = (RAQ_Y >= memory[eff_addr]); // Carry flag
			opbytes = 2;
			break;

		case uint8_t(0xCC): // CPY Absolute
			std::cout << "CPY Absolute\n";
			return 1;
			opbytes = 3;
			break;


		case uint8_t(0x0A): // ASL Accumulator
			if(verbose) std::cout << "ASL Accumulator\n";
			flag_c = RAQ_ACC & 0b10000000;
			tmp = RAQ_ACC<<1;
			RAQ_ACC = tmp;
			flag_z = (RAQ_ACC == 0x0);
			flag_n = RAQ_ACC & 0b10000000;
			opbytes = 1;
			break;

		case uint8_t(0x06): // ASL Zero Page
			std::cout << "ASL Zero Page\n";
			assert(0);
			break;

		case uint8_t(0x16): // ASL Zero Page, X
			// Wrap around if sum of base and reg exceeds 0xFF
			std::cout << "ASL Zero Page, X\n";
			assert(0);
			break;

		case uint8_t(0x0E): // ASL Absolute
			std::cout << "ASL Absolute\n";
			assert(0);
			break;

		case uint8_t(0x1E): // ASL Absolute, X
			std::cout << "ASL Absolute, X\n";
			assert(0);
			break;

		case uint8_t(0x4A): // LSR Accumulator
			if(verbose) std::cout << "LSR Accumulator\n";
			flag_c = RAQ_ACC & 0x1;
			tmp = RAQ_ACC>>1;
			RAQ_ACC = tmp;
			flag_z = (RAQ_ACC == 0x0);
			flag_n = false;
			opbytes = 1;
			break;

		case uint8_t(0x46): // LSR Zero Page
			eff_addr = memory[pc+1];
			tmp = memory[eff_addr];
			flag_c = tmp & 0x1;
			tmp2 = tmp>>1;
			memory[eff_addr] = tmp2;
			flag_z = (memory[eff_addr] == 0x0);
			flag_n = false;
			opbytes = 2;
			break;

		case uint8_t(0x56): // LSR Zero Page, X
			// TODO Implement, and remember status flags
			// Wrap around if sum of base and reg exceeds 0xFF
			opbytes = 2;
			assert(0);
			break;

		case uint8_t(0x4E): // LSR Absolute
			// TODO Implement, and remember status flags
			opbytes = 3;
			assert(0);
			break;

		case uint8_t(0x5E): // LSR Absolute, X
			// TODO Implement, and remember status flags
			opbytes = 3;
			assert(0);
			break;

		case uint8_t(0x2A): // ROL Accumulator
			if(verbose) std::cout << "ROL Accumulator\n";
			tmp = RAQ_ACC;
			// Rotate TMP left, carry goes into bit 0, bit 7 becomes new carry flag
			tmp2 = tmp;
			tmp <<=1;
			tmp +=(flag_c ? 1 : 0);
			flag_c = ((tmp2 & 0b10000000) != 0);
			RAQ_ACC = tmp;
			opbytes = 1;
			break;

		case uint8_t(0x26): // ROL Zero Page
			if(verbose) std::cout << "ROL Zero Page\n";
			eff_addr = memory[pc+1];
			tmp = (memory[eff_addr]);
			// Rotate TMP left, carry goes into bit 0, bit 7 becomes new carry flag
			tmp2 = tmp;
			tmp <<=1;
			tmp +=(flag_c ? 1 : 0);
			flag_c = ((tmp2 & 0b10000000) != 0);
			memory[eff_addr] = tmp;
			opbytes = 2;
			break;

		case uint8_t(0x36): // ROL Zero Page, X
			if(verbose) std::cout << "ROL Zero Page, X\n";
			opbytes = 2;
			assert(0);
			break;

		case uint8_t(0x2E): // ROL Absolute
			if(verbose) std::cout << "ROL Absolute\n";
			opbytes = 3;
			assert(0);
			break;

		case uint8_t(0x3E): // ROL Absolute, X
			if(verbose) std::cout << "ROL Absolute, X\n";
			opbytes = 3;
			assert(0);
			break;

		case uint8_t(0x29): // AND
		case uint8_t(0x25):
		case uint8_t(0x35):
		case uint8_t(0x2D):
		case uint8_t(0x3D):
		case uint8_t(0x39):
		case uint8_t(0x21):
		case uint8_t(0x31):
			std::tie(eff_addr, opbytes) = aModeHelper(thisbyte);
			if(verbose) std::cout << "AND " << std::hex << eff_addr << std::dec << " pc+=" << opbytes << std::endl;

			tmp = RAQ_ACC & memory[eff_addr];
			RAQ_ACC = tmp;
			flag_z = (RAQ_ACC == 0); // Zero flag if zero
			flag_n = ((RAQ_ACC & 0b10000000) != 0); // Negative flag if sign bit set
			break;

		case uint8_t(0x49): // EOR
		case uint8_t(0x45):
		case uint8_t(0x55):
		case uint8_t(0x4D):
		case uint8_t(0x5D):
		case uint8_t(0x59):
		case uint8_t(0x41):
		case uint8_t(0x51):
			std::tie(eff_addr, opbytes) = aModeHelper(thisbyte);
			if(verbose) std::cout << "EOR " << std::hex << eff_addr << std::dec << " pc+=" << opbytes << std::endl;

			tmp = RAQ_ACC ^ memory[eff_addr];
			RAQ_ACC = tmp;
			flag_z = (RAQ_ACC == 0); // Zero flag if zero
			flag_n = ((RAQ_ACC & 0b10000000) != 0); // Negative flag if sign bit set
			break;

		case uint8_t(0x09): // ORA
		case uint8_t(0x05):
		case uint8_t(0x15):
		case uint8_t(0x0D):
		case uint8_t(0x1D):
		case uint8_t(0x19):
		case uint8_t(0x01):
		case uint8_t(0x11):
			std::tie(eff_addr, opbytes) = aModeHelper(thisbyte);
			if(verbose) std::cout << "ORA " << std::hex << eff_addr << std::dec << " pc+=" << opbytes << std::endl;
			tmp = RAQ_ACC | memory[eff_addr];
			RAQ_ACC = tmp;
			flag_z = (RAQ_ACC == 0); // Zero flag if zero
			flag_n = ((RAQ_ACC & 0b10000000) != 0); // Negative flag if sign bit set
			break;

		case uint8_t(0x85): // STA
		case uint8_t(0x95):
		case uint8_t(0x8D):
		case uint8_t(0x9D):
		case uint8_t(0x99):
		case uint8_t(0x81):
		case uint8_t(0x91):
			std::tie(eff_addr, opbytes) = aModeHelper(thisbyte);
			if(verbose) std::cout << "STA " << std::hex << eff_addr << std::dec << " pc+=" << opbytes << std::endl;
			memory[eff_addr] = RAQ_ACC;
			break;

		case uint8_t(0x4C): // JMP (absolute)
			// The next two bytes specify a little endian address.
			tmp = memory[pc+2]; // tmp is an unsigned int with room for shifts
			eff_addr = (tmp << 8) + memory[pc+1];
			if(verbose) std::cout << "Absolute JMP " << std::hex << eff_addr << std::dec << std::endl;
			pc = eff_addr;
			opbytes = 0; // For JMP we just go directly to the address, ignoring PC increment
			break;

		case uint8_t(0x6C): // JMP (indirect)
			// The next two bytes specify a little endian address containing a little endian address
			tmp = memory[pc+2]; // tmp is an unsigned int with room for shifts
			tmp2 = (tmp << 8) + memory[pc+1];

			// Now tmp2 has the address of the second address
			// First the MSB
			tmp = memory[tmp2+1]; // tmp is an unsigned int with room for shifts

			// Now the LSB
			eff_addr = (tmp << 8) + memory[tmp2];

			if(verbose) std::cout << "Indirect JMP " << std::hex << eff_addr << std::dec << std::endl;
			pc = eff_addr;
			opbytes = 0; // For JMP we just go directly to the address, ignoring PC increment
			break;

		case uint8_t(0x20): // JSR (Absolute)
			// Push pc of next instruction minus 1 onto stack (msb first, little endian since stack is upside down)
			memory[0x100+RAQ_STACK--] = (((pc+2)>>8) & 0b11111111);
			memory[0x100+RAQ_STACK--] = ((pc+2) & 0b11111111);
			// The next two bytes specify a little endian address.
			tmp = memory[pc+2]; // tmp is an unsigned int with room for shifts
			eff_addr = (tmp << 8) + memory[pc+1];
			if(verbose) std::cout << "JSR " << std::hex << eff_addr << std::dec << std::endl;
			pc = eff_addr;
			opbytes = 0;
			break;

		case uint8_t(0x60): // RTS
			eff_addr = (( ((memory[0x100+RAQ_STACK+0x2])<<8) | (memory[0x100+RAQ_STACK+1]) ) +1); // The +1 is important and easy to miss
			if(verbose) std::cout << "return to " << std::hex << eff_addr << std::dec << std::endl;
			RAQ_STACK += 2;
			pc = eff_addr;
			opbytes = 0;
			break;

		case uint8_t(0x48): // PHA
			if(verbose) std::cout << "PHA" << std::endl;
			memory[0x100+RAQ_STACK--] = RAQ_ACC;
			opbytes = 1;
			break;

		case uint8_t(0x68): // PLA
			if(verbose) std::cout << "PLA" << std::endl;
			RAQ_STACK += 1;
			RAQ_ACC = (memory[0x100+RAQ_STACK]);
			opbytes = 1;
			break;

		case uint8_t(0x90): // BCC
			if(verbose) std::cout << "BCC ";
			if(flag_c == false){
				if(verbose) std::cout << "taken\n";
				branchHelper(); // This changes PC appropriately
			}else{
				if(verbose) std::cout << "not taken\n";
			}
			opbytes = 2; // Always increment by 2 regardless
			break;

		case uint8_t(0xB0): // BCS
			if(verbose) std::cout << "BCS ";
			if(flag_c == true){
				if(verbose) std::cout << "taken\n";
				branchHelper(); // This changes PC appropriately
			}else{
				if(verbose) std::cout << "not taken\n";
			}
			opbytes = 2; // Always increment by 2 regardless
			break;

		case uint8_t(0xF0): // BEQ
			if(verbose) std::cout << "BEQ ";
			if(flag_z == true){
				if(verbose) std::cout << "taken\n";
				branchHelper(); // This changes PC appropriately
			}else{
				if(verbose) std::cout << "not taken\n";
			}
			opbytes = 2; // Always increment by 2 regardless
			break;

		case uint8_t(0x30): // BMI
			if(verbose) std::cout << "BMI ";
			if(flag_n == true){
				if(verbose) std::cout << "taken\n";
				branchHelper(); // This changes PC appropriately
			}else{
				if(verbose) std::cout << "not taken\n";
			}
			opbytes = 2; // Always increment by 2 regardless
			break;

		case uint8_t(0xD0): // BNE
			if(verbose) std::cout << "BNE ";
			if(flag_z == false){
				if(verbose) std::cout << "taken\n";
				branchHelper(); // This changes PC appropriately
			}else{
				if(verbose) std::cout << "not taken\n";
			}
			opbytes = 2; // Always increment by 2 regardless
			break;

		case uint8_t(0x10): // BPL
			if(verbose) std::cout << "BPL ";
			if(flag_n == false){
				if(verbose) std::cout << "taken\n";
				branchHelper(); // This changes PC appropriately
			}else{
				if(verbose) std::cout << "not taken\n";
			}
			opbytes = 2; // Always increment by 2 regardless
			break;

		case uint8_t(0x50): // BVC
			if(verbose) std::cout << "BVC ";
			if(flag_v == false){
				if(verbose) std::cout << "taken\n";
				branchHelper(); // This changes PC appropriately
			}else{
				if(verbose) std::cout << "not taken\n";
			}
			opbytes = 2; // Always increment by 2 regardless
			break;

		case uint8_t(0x70): // BVS
			if(verbose) std::cout << "BVS ";
			if(flag_v == true){
				if(verbose) std::cout << "taken\n";
				branchHelper(); // This changes PC appropriately
			}else{
				if(verbose) std::cout << "not taken\n";
			}
			opbytes = 2; // Always increment by 2 regardless
			break;

		case uint8_t(0x38): // SEC
			if(verbose) std::cout << "SEC\n";
			flag_c = true;
			opbytes = 1;
			break;

		case uint8_t(0xF8): // SED
			if(verbose) std::cout << "SED\n";
			flag_d = true;
			opbytes = 1;
			break;

		case uint8_t(0x78): // SEI
			if(verbose) std::cout << "SEI\n";
			flag_i = true;
			opbytes = 1;
			break;

		case uint8_t(0x18): // CLC
			if(verbose) std::cout << "CLC\n";
			flag_c = false;
			opbytes = 1;
			break;

		case uint8_t(0xD8): // CLD
			if(verbose) std::cout << "CLD\n";
			flag_d = false;
			opbytes = 1;
			break;

		case uint8_t(0x58): // CLI
			if(verbose) std::cout << "CLI\n";
			flag_i = false;
			opbytes = 1;
			break;

		case uint8_t(0xB8): // CLV
			if(verbose) std::cout << "CLV\n";
			flag_v = false;
			opbytes = 1;
			break;

		default:
			std::cout << "Undefined instruction:" << std::hex << (int) thisbyte << std::dec << std::endl;
			return 1;
			break;
	}
	pc += opbytes;
	return !((pc > 0) && (pc < num_words));

}

void Raquette::show_regs() {
	std::cout << "pc:" << std::hex << pc << std::dec
		<< "  acc:" << std::hex << (int) RAQ_ACC << std::dec
		<< "  x:" << std::hex << (int) RAQ_X << std::dec
		<< "  y:" << std::hex << (int) RAQ_Y << std::dec
		<< "  sp:" << std::hex << (int) RAQ_STACK << std::dec
		<< std::endl;
	std::cout << "Status flags: C" << flag_c
		<< " Z" << flag_z
		<< " I" << flag_i
		<< " D" << flag_d
		<< " B" << flag_b
		<< " V" << flag_v
		<< " N" << flag_n
		<< std::endl;
}

// Emulates Apple ][ text mode behavior
// The Apple ][ screen mapping was quirky.
// For example the first row starts at 0400. The next 40 bytes are shown on that row.
// Then we wrap onto the first row of the middle level of the screen (the 9th row)
// Then we wrap onto the first row of the third level and after that there are 8 unused bytes
// Then we repeat for the second rows of each of the three layers, etc.
#define RAQ_CHAR(x) (charset[x % 0x40])
void Raquette::show_screen(){
	// The Apple 2 charset includes the same characters repeated in 4 variations (dark, blinking, etc)
	// For now we only emulate one variation and map onto it from the other 4 variations using modulo division
	char charset[0x40] = {
		'@','A','B','C','D','E','F','G', 'H','I','J','K','L' ,'M','N','O',
		'P','Q','R','S','T','U','V','W', 'X','Y','Z','[','\\',']','^','_',
		' ','!','"','#','$','%','&','\'','(',')','*','+',',' ,'-','.','/',
		'0','1','2','3','4','5','6','7', '8','9',':',';','<' ,'=','>','?'};

	std::cout << "Starting screen...\n";
	initscr();
	cbreak(); // One character at a time input
	noecho(); // Only show what the machine is showing
	keypad(stdscr, TRUE); // Capture backspace, delete, arrow keys
	curs_set(0); // Invisible cursor
	WINDOW * win = newwin(24, 40, 0, 0);

	// We will print the rows in memory-order
	wmove(win, 0, 0);
	for(int i=0; i<24; i++){
		int row = (8*(i%3))+(i/3);
		int rowaddr = (0x400 + (i*40) + ((i/3)*8));
		int col = 0;
		for(int j=0; j<40; j++){
			mvwaddch(win, row, col++, RAQ_CHAR(memory[rowaddr+j]));
		}
	}

	wrefresh(win);
	char ch = wgetch(win);
	endwin();
	std::cout << "Entered " << std::hex << (int) ch << std::dec << std::endl;

	if(ch == 0xA){ // 0xA is line feed, and 0xD is CR. The Apple 2 expects the latter.
		memory[0xC000] = 0x0D | 0b10000000;
	}else{
		memory[0xC000] = ch | 0b10000000;
	}
}
