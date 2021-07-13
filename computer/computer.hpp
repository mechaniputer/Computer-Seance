#pragma once

class Computer {
	public:
	int width_bytes, num_words, num_regs, pc;
	char *regs;
	char *memory;

	Computer(int memwidth = 1, int memlocs = 64, int registers = 2, char *init_contents = nullptr);
	~Computer();

	void show_regs();
	void dump();
	int step();
	// TODO load (for replacing entire contents of memory after init)
	// TODO reset (for resetting regs and pc)
};
