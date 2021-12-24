#pragma once

// For display buffers
typedef struct Color {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} Color;

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

