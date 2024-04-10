#include <iostream>
#include <fstream>
#include <unistd.h>
#include <assert.h>
#include "computer.hpp"
#include "lvdc.hpp"

// An emulator inspired by a historic computer that piloted rockets
// Due to missing information, a lot of guesswork is present.
// Don't fly a rocket with this code.

/*
Instructions not yet implemented: MPY, MPH, DIV, EXM, PIO

Other missing features:
	HOP save
	P-Q register
	Duplex mode
	Highly accurate performance using timers
	Program loading from file (should try to match format of yaOBC)
	Interrupts

Missing information:
	Instruction counter overflow (currently wraps around)
	Arithmetic overflow (currently wraps around)
	Behavior with concurrent MPY and/or DIV (neither are currently implemented at all)
	Behavior under duplex mode with mismatched but paired modules

	I assumed that it starts by using address 0 in sector 0, module 0 as a HOP constant.
	Page 2-105, item 2-249 in the LVDC manual supports this behavior.
	However, other sources seem to say that execution of code simply begins there.
	I should look into that and change this accordingly.

Software:
 For now it includes a setup_test() method which loads a hardcoded test program.
 An assembler would be nice.
 In the future maybe I'll write a FORTH for it, although memory being split
 into 128 banks of 256 bytes, the lack of a hardware stack, and the lack of
 addressing modes will make this tricky. Self-modifying code seems necessary.
*/

// Addresses have 8 bits for within a sector and the 9th bit selects either the address is used for residual memory
// A9 is the least significant bit of the address (adjacent to the opcode) but A8 is the most significant bit (8, 7, 6, ..., 1, 9, opcode)

LVDC::LVDC(int verbosity, int brkpt, int run_fast, char *fname){
			cycles = 0;
			verb = verbosity;
			fast = run_fast;
			breakpoint = brkpt;

			// Zero mem
			for(int addr=0; addr < NUM_MODULES * MODULE; addr++){
				mem[0][addr]=0;
				mem[1][addr]=0;
			}

			setup_test(); // TODO This is a temporary function for debugging

			// HOP with address 000
			dload(0X0, &transfer); // Load HOP constant
			std::cout << "\nBeginning execution with address 000 HOP Constant: " << transfer << std::endl;
			LVDC::hop(); // Uses transfer register from here
};



// Converts a 26 bit signed int (residing within a uint32_t) to a proper 32-bit signed int
int32_t LVDC::to_signed_int(uint32_t word){
	uint32_t sign = ((word>>25) & 0X1);
	if(sign){ // Negative two's complement
		return -((word ^ 0X3FFFFFF) + 0X1);
	}else{ // Positive
		 return word;
	}
}

// Show all register state
void LVDC::show_regs(){
//	std::cout << "REGISTER DUMP:" << std::endl;
	std::cout << "  acc:      " << to_signed_int(acc) << "  ";
	std::cout << "  ic:       " << ic << std::endl;
	std::cout << "  dsector:  " << unsigned(dsector) << "  ";
	std::cout << "  isector:  " << unsigned(isector) << std::endl;
	std::cout << "  dmodule:  " << unsigned(dmodule) << "  ";
	std::cout << "  imodule:  " << unsigned(imodule) << std::endl;
	std::cout << "  syllable: " << unsigned(syllable) << std::endl;
	std::cout << "  effective internal instruction address: " << ((imodule*MODULE)+(isector*SECTOR)+ic) << std::endl;
}

// Sets the data sector
// Used by CDS, HOP
// Takes a 4 bit value
void LVDC::set_dsector(uint16_t x){
	assert(!(x>>4));
	dsector = x;
}

// Sets the instruction sector
// Used by HOP
// Takes a 4 bit value
void LVDC::set_isector(uint16_t x){
	assert(!(x>>4));
	isector = x;
}

// Stores a full 26 bit word into an address in the current sector
// Takes a 9 bit address including a "residual bit" in the least signififcant position
void LVDC::dstore(uint16_t addr, uint32_t src){
	assert(!(addr>>9));
	assert(!(src>>26));
	// Syllable 1 is more significant than syllable 0
	if(addr & 1){	// Residual memory
		mem[0][(dmodule*MODULE)+(0XF*SECTOR)+(addr>>1)] = (src & 0X1FFF);
		mem[1][(dmodule*MODULE)+(0XF*SECTOR)+(addr>>1)] = (src>>13);
	}else{
		mem[0][(dmodule*MODULE)+(dsector*SECTOR)+(addr>>1)] = (src & 0X1FFF);
		mem[1][(dmodule*MODULE)+(dsector*SECTOR)+(addr>>1)] = (src>>13);
	}
}

// Stores a word to any address in any sector in any module
// Should not be used in execution
// Takes an 8 bit address, does not use residual bit
void LVDC::dstore_absolute(uint16_t storeaddr, uint16_t storesector, uint16_t storemodule, uint32_t src){
	assert(!(storeaddr>>8));
	assert(!(storesector>>4));
	assert(!(storemodule>>3));
	assert(storemodule < NUM_MODULES); // Important to check, because NUM_MODULES is configurable
	assert(!(src>>26));
	mem[0][(storemodule*MODULE)+(storesector*SECTOR)+(storeaddr)] = (src & 0X1FFF);
	mem[1][(storemodule*MODULE)+(storesector*SECTOR)+(storeaddr)] = (src>>13);
}

// Loads a 13 bit instruction into a register argument
void LVDC::iload(uint16_t *dest){
	// Residual A9 is not available for addressing instructions.
	*dest = mem[syllable][(imodule*MODULE)+(isector*SECTOR)+ic];
}

// Loads a full 26 bit word into a register argument
// addr is an instruction syllable right-shifted by 4
// dest is a pointer to an external 32 bit register
void LVDC::dload(uint16_t addr, uint32_t *dest){
	uint32_t s0, s1;
	if(addr & 1){	// Residual memory
		s0 = mem[0][(dmodule*MODULE)+(0XF*SECTOR)+(addr>>1)];
		s1 = mem[1][(dmodule*MODULE)+(0XF*SECTOR)+(addr>>1)];
	}else{
		s0 = mem[0][(dmodule*MODULE)+(dsector*SECTOR)+(addr>>1)];
		s1 = mem[1][(dmodule*MODULE)+(dsector*SECTOR)+(addr>>1)];
	}
	s1 = s1<<13; // Syllable 1 is the more significant half
	*dest = ((s0 + s1) & 0X3FFFFFF);
}

// Sets the syllable to either 0 or 1 based on argument
void LVDC::set_syllable(uint16_t sylnew){
	assert(0==(sylnew>>1)); // Must be either 0 or 1
	syllable = sylnew;
}

// Takes 8 bit value, no A9 residual bit
void LVDC::set_ic(uint16_t icnew){
	assert(0==(icnew>>8)); // Catch invalid input
	ic = icnew;
}

void LVDC::set_imodule(uint16_t imodnew){
	assert(0==(imodnew>>3));
	assert(imodnew < NUM_MODULES); // Important to check, because NUM_MODULES is configurable
	imodule = imodnew;
}

void LVDC::set_dmodule(uint16_t dmodnew){
	assert(0==(dmodnew>>3));
	assert(dmodnew < NUM_MODULES); // Important to check, because NUM_MODULES is configurable
	dmodule = dmodnew;
}

// TODO when duplex support is added, should probably add a check for even number of modules being present
void LVDC::set_isimdup(uint16_t simdupmode){
	assert(0==(simdupmode>>1)); // Must be either 0 or 1
	isimdup = simdupmode;
	if(0 != isimdup){
		std::cout << "ERROR: Duplex operation currently unsupported\n";
		assert(0);
	}
}

// TODO when duplex support is added, should probably add a check for even number of modules being present
void LVDC::set_dsimdup(uint16_t simdupmode){
	assert(0==(simdupmode>>1)); // Must be either 0 or 1
	dsimdup = simdupmode;
	if(0 != dsimdup){
		std::cout << "ERROR: Duplex operation currently unsupported\n";
		assert(0);
	}
}

void LVDC::inc_ic(){
	ic++; // TODO What should really happen when this overflows? I assume it wraps but for all I know it should catch on fire.
	ic = (ic & 0XFF);
}




// TODO This is a temporary function for debugging
void LVDC::setup_test(){
	// Manchester baby program
	dstore_absolute(0, 0, 0, STARTHOPCONST);  // 00: HOP constant to begin execution at address 1
	dstore_absolute(1, 0, 0, SHF);                     // 01: LDN 24, load the number to be factored
	dstore_absolute(2, 0, 0, SUB+(0X18<<5)+(0X1<<4));  // 01_A:
	dstore_absolute(3, 0, 0, STO+(0X1A<<5)+(0X1<<4));  // 02: STO 26, store initial -b test value in line 26
	dstore_absolute(4, 0, 0, SHF);                     // 03: LDN 26, load +b value
	dstore_absolute(5, 0, 0, SUB+(0X1A<<5)+(0X1<<4));  // 03_A:
	dstore_absolute(6, 0, 0, STO+(0X1B<<5)+(0X1<<4));  // 04: STO 27, store initial +b test value in line 27
	dstore_absolute(7, 0, 0, SHF);                     // 05: LDN 23, load number to be factored
	dstore_absolute(8, 0, 0, SUB+(0X17<<5)+(0X1<<4));  // 05_A:
	dstore_absolute(9, 0, 0, SUB+(0X1B<<5)+(0X1<<4)); // 06: SUB 27 Subtracts latest +b test value from current ACC value
	dstore_absolute(10, 0, 0, TMI+(0XC<<5));           // 07: Skip to 12 if negative ACC
	dstore_absolute(11, 0, 0, TRA+(0X9<<5)); // 08: TRA back two instructions (not negative yet)
	dstore_absolute(12, 0, 0, SUB+(0X1A<<5)+(0X1<<4)); // 09: SUB 26 subtract current -b value (addition)
	dstore_absolute(13, 0, 0, STO+(0X19<<5)+(0X1<<4)); // 10: STO 25, Stores the calculated overshoot difference value in line 25
	dstore_absolute(14, 0, 0, SHF);                     // 11: LDN 25
	dstore_absolute(15, 0, 0, SUB+(0X19<<5)+(0X1<<4));  // 11_A:
	dstore_absolute(16, 0, 0, TMI+(0X12<<5));          // 12: Skip to 18 if negative ACC
	dstore_absolute(17, 0, 0, TRA+(0X40<<5));          // 13: Solution is found, hop to display loop at 64 (answer is in 27 of residual)
	dstore_absolute(18, 0, 0, SHF);                     // 14: LDN 26, load last tested b value as a positive ACC value
	dstore_absolute(19, 0, 0, SUB+(0X1A<<5)+(0X1<<4));  // 14_A:
	dstore_absolute(20, 0, 0, SUB+(0X15<<5)+(0X1<<4)); // 15: Sub 1
	dstore_absolute(21, 0, 0, STO+(0X1B<<5)+(0X1<<4)); // 16: STO 27, Store new +b test value in line 27
	dstore_absolute(22, 0, 0, SHF);                     // 17: LDN 27, Load new -b test value into ACC
	dstore_absolute(23, 0, 0, SUB+(0X1B<<5)+(0X1<<4));  // 17_A:
	dstore_absolute(24, 0, 0, STO+(0X1A<<5)+(0X1<<4)); // 18: STO 26, Stores the calculated overshoot difference value in line 25
	dstore_absolute(25, 0, 0, TRA+(0X7<<5));           // 19: TRA 7, Execute subtractions from line 5 again using new test b value.

	// Done
	dstore_absolute(64, 0, 0, CLA+(0X1B<<5)+(0X1<<4)); // load result
	dstore_absolute(65, 0, 0, TRA+(0X41<<5)); // self loop

	// Constants ( in residual sector)
	dstore_absolute(21, 15, 0, 0X1); // 1
	dstore_absolute(22, 15, 0, 0X4); // 4
	dstore_absolute(23, 15, 0, ((0X1<<18) ^ (0X3FFFFFF))+0X1); // Neg form of number to be factored
	dstore_absolute(24, 15, 0, 0X3FFFF); // Number to be factored minus 1
}

// Performs one full execution step
int LVDC::step(){
	if(breakpoint == ic){
		show_regs();
		return 1; // Signal that breakpoint has been reached
	}
	cycles++;

	uint16_t instr; // Holds current instruction
	int32_t result; // Hold math results in signed form

	// TODO Match LVDC performance more accurately using timers instead
	if(!fast) usleep(82); // LVDC cycle time was 82 microseconds

	// FETCH
	iload(&instr);
	if(verb) std::cout << ic << ": ";

	// DECODE & EXECUTE
	switch(instr & 0XF){ // Keep lower four bits
		case 0:
			dload(instr>>4, &transfer); // Load HOP constant
			if(verb) std::cout << "HOP Constant: " << to_signed_int(transfer) << " from address " << (instr>>5) << (((instr>>4)&0X1)? " of residual sector\n":"\n");
			if(LVDC::hop()){ // Uses transfer register from here
				show_regs(); // TODO print error
				return 1; // HOP failed, halt execution
			}
			break; // Don't increment IC because this is a branch
		case 1:
			if(verb) std::cout << "MPY\n";
			inc_ic();
			break;
		case 2:
			if(verb) std::cout << "SUB with address " << (instr>>5) << (((instr>>4)&1)? " of residual sector\n":"\n");;
			dload(instr>>4, &transfer); // Load operand
			// Do the actual math with 32 bit signed ints
			result = to_signed_int(acc) - to_signed_int(transfer);
			// TODO detect overflow?
			// Currently overflow just wraps around. Not sure if the original machine had an error for that.
			acc = ((uint32_t) (result & 0X3FFFFFF));
			inc_ic();
			break;
		case 3:
			if(verb) std::cout << "DIV\n";
			inc_ic();
			break;
		case 4:
			if(verb) std::cout << "TNZ " << ((0!=acc)?"taken\n":"not taken\n");
			if(acc != 0){
				set_syllable((instr>>4)&0X1); // A9 sets syllable
				set_ic(instr>>5); // Takes 8 bit address
			}else{
				inc_ic(); // Only increment IC if not taking the jump
			}
			break;
		case 5:
			if(verb) std::cout << "MPH\n";
			inc_ic();
			break;
		case 6:
			if(verb) std::cout << "AND with address " << (instr>>5) << (((instr>>4)&1)? " of residual sector\n":"\n");
			dload(instr>>4, &transfer); // Load operand
			acc = ((acc & transfer) & 0X3FFFFFF);
			inc_ic();
			break;
		case 7:
			if(verb) std::cout << "ADD with address " << (instr>>5) << (((instr>>4)&1)? " of residual sector\n":"\n");;
			dload(instr>>4, &transfer); // Load operand
			// Do the actual math with 32 bit signed ints
			result = to_signed_int(acc) + to_signed_int(transfer);
			// TODO detect overflow?
			// Currently overflow just wraps around. Not sure if the original machine had an error for that.
			acc = ((uint32_t) (result & 0X3FFFFFF));
			inc_ic();
			break;
		case 8:
			if(verb) std::cout << "TRA\n";
			set_syllable((instr>>4) & 0X1); // A9 sets syllable
			set_ic(instr>>5); // Takes 8 bit address
			break; // Don't increment IC because this is a branch
		case 9:
			if(verb) std::cout << "XOR with address " << (instr>>5) << (((instr>>4)&1)? " of residual sector\n":"\n");
			dload(instr>>4, &transfer); // Load operand
			acc = ((acc ^ transfer) & 0X3FFFFFF);
			inc_ic();
			break;
		case 10:
			if(verb) std::cout << "PIO\n";
			inc_ic();
			break;
		case 11:
			if(verb) std::cout << "STO to address " << (instr>>5) << (((instr>>4)&1)? " of residual sector\n":"\n");
			// TODO P-Q register
			// TODO HOP save feature
			dstore(instr>>4,acc);
			inc_ic();
			break;
		case 12:
			// Transfer if ACC sign is negative (aka, MSB is 1)
			if(verb) std::cout << "TMI " << ((0 != ((acc>>25) & 0X1))?"taken\n":"not taken\n");
			if(0 != ((acc>>25) & 0X1)){
				set_syllable((instr>>4)&0X1); // A9 sets syllable
				set_ic(instr>>5); // Takes 8 bit address
			}else{
				inc_ic(); // Only increment IC if not taking the jump
			}

			break;
		case 13:
			if(verb) std::cout << "RSU with address " << (instr>>5) << (((instr>>4)&1)? " of residual sector\n":"\n");;
			dload(instr>>4, &transfer); // Load operand
			// Do the actual math with 32 bit signed ints
			result = to_signed_int(transfer) - to_signed_int(acc); 
			// TODO detect overflow?
			// Currently overflow just wraps around. Not sure if the original machine had an error for that.
			acc = ((uint32_t) (result & 0X3FFFFFF));
			inc_ic();
			break;
		case 14:
			if((((instr>>4) & 0X1) + ((instr>>11) & 0X2)) == 0X1){ // A8=0; A9=1
				if(verb) std::cout << "SHF\n";
				if((instr>>5) == 0X0){ // clear acc
					acc = 0X0;
				}else if((instr>>5) == 0X1){ // LSD 1
					acc = (acc>>1);

				}else if((instr>>5) == 0X2){ // LSD 2
					acc = (acc>>2);

				}else if((instr>>5) == 0X10){ // MSD 1
					acc = ((acc<<1) & 0X3FFFFFF);

				}else if((instr>>5) == 0X20){ // MSD 2
					acc = ((acc<<2) & 0X3FFFFFF);

				}else{ // Illegal operand for SHF
					std::cout<<"ERROR: illegal SHF operand... aborting\n";
					show_regs();
					return 1; // Stop execution
				}
				inc_ic();
				break;
			}else if((((instr>>4) & 0X1) + ((instr>>11) & 0X2)) == 0X3){ // A8=1; A9=1
				if(verb) std::cout << "EXM\n";
				inc_ic();
				break;
			}else if((((instr>>4) & 0X1)) == 0X0){ // A9=0;
				// IMPORTANT NOTE: The available documentation is unclear regarding the proper order of the operand here.
				// For lack of anything conclusive, I have done what seems to make sense.
				if(verb) std::cout << "CDS\n";
				// Note that instruction occupies 5 least significant bits, not 4
				set_dsector((instr>>9)&0XF);
				set_dmodule((instr>>6)&0X7);
				set_dsimdup(((instr>>5) & 0X1));
				inc_ic();
				break;
			}else{
				std::cout<<"ERROR: instruction not recognized... aborting\n";
				show_regs();
				return 1; // Stop execution
			}
		case 15:
			if(verb) std::cout << "CLA from address " << (instr>>5) << (((instr>>4)&1)? " of residual sector\n":"\n");
			dload(instr>>4, &acc);
			inc_ic();
			break;
		default:
			std::cout<<"ERROR: unrecognized instruction... aborting\n";
			show_regs();
			return 1; // Stop execution
	}
	// TODO Decrement MPY/DIV counter, update results after 2 and 4 instructions

	if(verb) show_regs();
	return 0; // Continue execution
}

int LVDC::hop(){
	// Variables for HOP constant
	uint32_t imod23, imod1, imod; // Hold the instruction module
	uint16_t isimdup, dsimdup; // Hold bits for simplex/duplex (instruction, data)
	uint16_t isect, dsect; // Hold the sector selection (instruction, data)
	uint16_t dmod; // Hold the data module selection
	uint16_t icnew; // Hold new value of instruction counter
	uint16_t sylnew; // Hold new syllable value

	// Select instruction module 0-7
	imod23 = transfer&0X3; // bits 25-26
	imod1 = transfer>>25; // bit 1
	imod = (imod23<<1)+imod1;
	//std::cout << "Instruction module selection: " << imod << std::endl;
	set_imodule(imod);

	// Select instruction simdup 0-1
	isimdup = (transfer>>24) & 0X1; // bit 2
	//std::cout << "Instruction Simplex/Duplex: " << isimdup << std::endl;
	set_isimdup(isimdup);

	// Select data sector 0-15
	dsect = (transfer>>20) & 0XF; // bits 3-6
	//std::cout << "Data sector: " << dsect << std::endl;
	set_dsector(dsect);

	// Select data module 0-7
	dmod = (transfer>>17) & 0X7; // bits 7-9
	//std::cout << "Data module: " << dmod << std::endl;
	set_dmodule(dmod);

	// Select data simdup 0-1
	dsimdup = (transfer>>16) & 0X1; // bit 10
	//std::cout << "Data Simplex/Duplex: " << dsimdup << std::endl;
	set_dsimdup(dsimdup);

	// Bit 11 must be zero. This is probably because it corresponds to A9, despite being in a different order.
	if((transfer>>15) & 0X1){
		std::cout<<"ERROR: Bit 11 of HOP constant must be zero... aborting\n";
		return 1; // Stop execution
	}

	// Select new instruction counter 0-255
	icnew = (transfer>>7) & 0XFF; // bits 12-19
	//std::cout << "New instruction counter: " << icnew << std::endl;
	set_ic(icnew);

	// Select new syllable 0-1
	sylnew = (transfer>>6) & 0X1; // bit 20
	//std::cout << "New syllable: " << sylnew << std::endl;
	set_syllable(sylnew);

	// Select new instruction sector 0-15
	isect = (transfer>>2) & 0XF; // bits 21-24
	//std::cout << "Instruction sector: " << isect << std::endl;
	set_isector(isect);

	//std::cout << std::endl;
	return 0; // Continue execution
}
