#include <iostream>
#include <assert.h>
#include <tuple>
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
	num_words = 0xFFFF;
	num_regs = 4; // A(cc), X(ind), Y(ind), S(tack ptr)

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
	std::cout << "Reset vector " << eff_addr << std::endl;
	pc = eff_addr;
}

// Takes the current byte with the opcode part masked out
// Determines the addressing mode
// Returns a tuple of: The effective address of the current instruction and the amount to increase pc
std::tuple<int, int> Raquette::aModeHelper(uint8_t thisbyte) {
	uint8_t amode = (thisbyte & ~RAQ_MASK_OPC); // Zero-mask opcode bits
	int opbytes = 0; // Remember how much to increase PC
	int eff_addr = 0; // Effective address of current instruction
	unsigned tmp; // For intermediate values below

	switch (amode){
		case uint8_t(0b00001000): // 08 Immediate
			std::cout << "Immediate ";
			// The next byte is the operand
			eff_addr = pc+1;
			opbytes = 2;
			break;

		case uint8_t(0b00000100): // 04 Zero page
			std::cout << "Zero page ";
			// The next byte is an address. Prepend it with 00.
			eff_addr = memory[pc+1];
			opbytes = 2;
			break;

		case uint8_t(0b00010100): // 14 Zero page, X
			std::cout << "Zero page, X ";
			// The next byte is an address. Prepend it with 00 and add the contents of the X register to it.
			eff_addr = memory[pc+1] + RAQ_X;
			opbytes = 2;
			break;

		case uint8_t(0b00001100): // 0C Absolute
			std::cout << "Absolute ";
			// The next two bytes specify a little endian address.
			tmp = memory[pc+2]; // tmp is an unsigned int with room for shifts
			eff_addr = (tmp << 8) + memory[pc+1];
			opbytes = 3;
			break;

		case uint8_t(0b00011100): // 1C Absolute, X
			std::cout << "Absolute, X ";
			// TODO The next two bytes specify an address. Add the contents of the X register to it. (Little Endian!)
			opbytes = 3;
			break;

		case uint8_t(0b00011000): // 18 Absolute, Y
			std::cout << "Absolute, Y ";
			// TODO The next two bytes specify an address. Add the contents of the Y register to it. (Little Endian!)
			opbytes = 3;
			break;

		case uint8_t(0b00000000): // 00 (Indirect, X)
			std::cout << "(Indirect, X) ";
			// TODO The next byte is an address. Prepend it with 00, add the contents of X to it, and get the two-byte address from that memory location.
			opbytes = 2;
			break;

		case uint8_t(0b00010000): // 10 (Indirect), Y
			std::cout << "(Indirect), Y ";
			// TODO The next byte is an address. Prepend it with 00, get the two-byte address from that memory location, and add the contents of Y to it.
			opbytes = 2;
			break;

		default: // Literally impossible
			assert(0);
			break;
	}
	return std::make_tuple(eff_addr, opbytes);
}

// ISA based on MOS 6502
// aaabbbcc. The aaa and cc bits determine the opcode, and the bbb bits determine the addressing mode.
// Instruction format: bits 0-2 and 6-7 determine opcode. bits 3-5 determine addressing mode.
// Little-endian (least sig byte first)
// 7 processor flags: flag_c flag_z flag_i flag_d flag_b flag_v flag_n
int Raquette::step() {
	assert(pc >= 0);
	if (pc >= num_words) {
		return 1; // Already out of bounds
	}

	unsigned tmp; // For intermediate values below
	int eff_addr, opbytes;
	uint8_t thisbyte = memory[pc];

	opbytes = 0;
	switch (thisbyte) {
		case uint8_t(0x00): // BRK
			std::cout << "BRK\n";
			flag_b = true;
			// TODO push stuff to stack, do interrupt
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
			std::cout << "LDA " << eff_addr << " pc+=" << opbytes << std::endl;
			RAQ_ACC = memory[eff_addr];
			flag_z = (RAQ_ACC == 0); // Zero flag if zero
			flag_n = ((RAQ_ACC & 0b10000000) != 0); // Negative flag if sign bit set
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
			std::cout << "ADC " << eff_addr << " pc+=" << opbytes << std::endl;

			tmp = RAQ_ACC + memory[eff_addr];
			if (flag_c) tmp++;
			// TODO Handle decimal mode

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
			// TODO Implement SBC
			// TODO Handle decimal mode
			std::tie(eff_addr, opbytes) = aModeHelper(thisbyte);
			std::cout << "SBC " << eff_addr << " pc+=" << opbytes << std::endl;
			break;

		case uint8_t(0x85): // STA
		case uint8_t(0x95):
		case uint8_t(0x8D):
		case uint8_t(0x9D):
		case uint8_t(0x99):
		case uint8_t(0x81):
		case uint8_t(0x91):
			std::tie(eff_addr, opbytes) = aModeHelper(thisbyte);
			std::cout << "STA " << eff_addr << " pc+=" << opbytes << std::endl;
			memory[eff_addr] = RAQ_ACC;
			break;

		case uint8_t(0x4C): // JMP (absolute)
			// The next two bytes specify a little endian address.
			tmp = memory[pc+2]; // tmp is an unsigned int with room for shifts
			eff_addr = (tmp << 8) + memory[pc+1];
			std::cout << "Absolute JMP " << std::hex << eff_addr << std::dec << std::endl;
			pc = eff_addr;
			break;

		case uint8_t(0x6C): // JMP (indirect)
			// TODO The next two bytes specify a little endian address containing a little endian address
			eff_addr = 0;
			std::cout << "Indirect JMP " << std::hex << eff_addr << std::dec << std::endl;
			pc = eff_addr;
			break;

		case uint8_t(0x90): // BCC TODO
			break;
		case uint8_t(0xB0): // BCS TODO
			break;
		case uint8_t(0xF0): // BEQ TODO
			break;
		case uint8_t(0x30): // BMI TODO
			break;
		case uint8_t(0xD0): // BNE TODO
			break;
		case uint8_t(0x10): // BPL TODO
			break;
		case uint8_t(0x50): // BVC TODO
			break;
		case uint8_t(0x70): // BVS TODO
			break;

		case uint8_t(0x38): // SEC
			std::cout << "SEC\n";
			flag_c = true;
			opbytes = 1;
			break;

		case uint8_t(0xF8): // SED
			std::cout << "SED\n";
			flag_d = true;
			opbytes = 1;
			break;

		case uint8_t(0x78): // SEI
			std::cout << "SEI\n";
			flag_i = true;
			opbytes = 1;
			break;

		case uint8_t(0x18): // CLC
			std::cout << "CLC\n";
			flag_c = false;
			opbytes = 1;
			break;

		case uint8_t(0xD8): // CLD
			std::cout << "CLD\n";
			flag_d = false;
			opbytes = 1;
			break;

		case uint8_t(0x58): // CLI
			std::cout << "CLI\n";
			flag_i = false;
			opbytes = 1;
			break;

		case uint8_t(0xB8): // CLV
			std::cout << "CLV\n";
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
