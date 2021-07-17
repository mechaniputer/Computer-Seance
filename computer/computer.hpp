#pragma once

class Computer {
	public:
	int width_bytes, num_words, num_regs, pc;
	uint8_t *regs;
	uint8_t *memory;

	Computer(int memwidth = 1, int memlocs = 64, int registers = 2, uint8_t *init_contents = nullptr, int len_contents = 0);
	~Computer();

	void show_regs();
	void dumpmem(int addr=0, int len = 16);
	int step();
	// TODO load (for replacing entire contents of memory after init)
};

class Raquette: public Computer {
	public:
	// 7 processor status flags:
	bool flag_c, flag_z, flag_i, flag_d, flag_b, flag_v, flag_n;
	Raquette(uint8_t *init_contents = nullptr, int len_contents = 0);
	// TODO reset (for resetting regs and pc)
	std::tuple<int, int> aModeHelper(uint8_t thisbyte);
	void branchHelper();
	int step();
	void show_regs();
	void show_screen();
};
