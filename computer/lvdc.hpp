#pragma once

#define STARTHOPCONST 0X1<<7 // HOP constant for address 1 syllable 0 in sector 0, module 0
#define ENDHOPCONST 0X3+(0X3FF<<16) + (0XFF<<7) + (0X1<<6) + (0XF<<2)

#define HOP (0X0)
#define SUB (0X2)
#define TNZ (0X4)
#define AND (0X6)
#define ADD (0X7)
#define TRA (0X8)
#define XOR (0X9)
#define STO (0XB)
#define TMI (0XC)
#define RSU (0XD)
#define CDS (0XE) // A9 = 0
#define SHF (0X1E) // A8 = 0, A9 = 1
#define SH_CLR (0X0<<5)
#define SH_R1 (0X1<<5)
#define SH_R2 (0X2<<5)
#define SH_L1 (0X10<<5)
#define SH_L2 (0X20<<5)
#define CLA (0XF)


#define SYLLABLE 13
#define WORD 26 // Not including parity bits
#define MODULE 4096 // One module is 4098 words
#define SECTOR 256
#define NUM_MODULES 8 // 8 is the max number of modules



// Addresses have 8 bits for within a sector and the 9th bit selects either the address is used for residual memory
// A9 is the least significant bit of the address (adjacent to the opcode) but A8 is the most significant bit (8, 7, 6, ..., 1, 9, opcode)
class LVDC: public Computer {
	public:
		uint64_t cycles; // Counts the number of cycles
		int verb; // verbosity level
		int fast; // Flag for performance emulation
		unsigned breakpoint; // instruction counter to cause execution to halt
		void test();
		int step();
		int hop();
		void setup_test(); // TODO This is a temporary function for debugging
		LVDC(int verbosity, int brkpt, int run_fast, char *fname);

		uint32_t acc; // Accumulator
		uint32_t ic; // Instruction Counter
		uint8_t dsector; // Data sector selector (4 bits are needed to select between the 16 sectors per module)
		uint8_t isector; // Instruction sector selector (4 bits are needed to select between the 16 sectors per module)
		uint8_t dmodule; // Data module selector (there are 8 modules in a fully equipped system, requiring 3 bits)
		uint8_t imodule; // Instruction module selector (there are 8 modules in a fully equipped system, requiring 3 bits)
		uint8_t mpycount; // Timer for multiply operation // TODO What happens if several MPY ops are concurent?
		uint8_t divcount; // Timer for divide operation // TODO What happens if several DIV ops are concurent?
		uint8_t syllable; // 0 or 1 to select syllable
		uint8_t isimdup, dsimdup; // TODO Figure out how to use this

	// The memory
	uint16_t mem[2][NUM_MODULES * MODULE]; // 32768 words, 65536 syllables, minus special addresses
	uint32_t transfer; // For storing operands from memory

	// Converts a 26 bit signed int (residing within a uint32_t) to a proper 32-bit signed int
	int32_t to_signed_int(uint32_t word);
	// Show all register state
	void show_regs();
	// Sets the data sector
	// Used by CDS, HOP
	// Takes a 4 bit value
	void set_dsector(uint16_t x);
	// Sets the instruction sector
	// Used by HOP
	// Takes a 4 bit value
	void set_isector(uint16_t x);
	// Stores a full 26 bit word into an address in the current sector
	// Takes a 9 bit address including a "residual bit" in the least signififcant position
	void dstore(uint16_t addr, uint32_t src);
	// Stores a word to any address in any sector in any module
	// Should not be used in execution
	// Takes an 8 bit address, does not use residual bit
	void dstore_absolute(uint16_t storeaddr, uint16_t storesector, uint16_t storemodule, uint32_t src);
	// Loads a 13 bit instruction into a register argument
	void iload(uint16_t *dest);
	// Loads a full 26 bit word into a register argument
	// addr is an instruction syllable right-shifted by 4
	// dest is a pointer to an external 32 bit register
	void dload(uint16_t addr, uint32_t *dest);
	// Sets the syllable to either 0 or 1 based on argument
	void set_syllable(uint16_t sylnew);
	// Takes 8 bit value, no A9 residual bit
	void set_ic(uint16_t icnew);
	void set_imodule(uint16_t imodnew);
	void set_dmodule(uint16_t dmodnew);
	// TODO when duplex support is added, should probably add a check for even number of modules being present
	void set_isimdup(uint16_t simdupmode);
	// TODO when duplex support is added, should probably add a check for even number of modules being present
	void set_dsimdup(uint16_t simdupmode);
	void inc_ic();

};

