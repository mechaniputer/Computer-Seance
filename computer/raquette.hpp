#pragma once

class Raquette: public Computer {
	public:


	class RaqDisk {
		public:
		// 35 tracks of 16 sectors of 256 bytes
		uint8_t disk[35][16][256];
		uint8_t stepperPhase;
		uint8_t halftrack;
		bool stepper_p0;
		bool stepper_p1;
		bool stepper_p2;
		bool stepper_p3;
		RaqDisk();
		uint8_t stepper();
		bool spinning;
		uint8_t drive; // 1 or 2
	};

	// 7 processor status flags:
	bool flag_c, flag_z, flag_i, flag_d, flag_b, flag_v, flag_n;
	char dispBuf[192][280];
	RaqDisk disk; // Assumed to be in slot 6 for now
	Raquette(uint8_t *init_contents = nullptr, int len_contents = 0);
	// TODO reset (for resetting regs and pc)
	std::tuple<int, int> aModeHelper(uint8_t thisbyte);
	uint8_t cycleCountHelper(uint8_t byte);
	uint8_t rolHelper(uint8_t byte);
	uint8_t rorHelper(uint8_t byte);
	void dispHelper(int eff_addr);
	void softSwitchesHelper(int eff_addr);
	void branchHelper();
	int step(bool verbose = false);
	int runMicroSeconds(unsigned int microseconds);
	void show_regs();
	void consoleSession();
	bool renderScreen();

	bool screen_update;
	bool graphics_mode;
	bool full_screen;
	bool page_two;
	bool hi_res;

	// Pixel rows are in reverse order
	const uint8_t charset[0x40*7] = {
	0x70,0x80,0xba,0xaa,0xba,0x8a,0x70, // @
	0x88,0x88,0x88,0xf8,0x88,0x88,0x70, // A
	0xf0,0x88,0x88,0xf0,0x88,0x88,0xf0, // B
	0x70,0x88,0x80,0x80,0x80,0x88,0x70, // C
	0xf0,0x88,0x88,0x88,0x88,0x88,0xf0, // D
	0xfa,0x80,0x80,0xf0,0x80,0x80,0xf8, // E
	0x80,0x80,0x80,0xf0,0x80,0x80,0xf8, // F
	0x70,0x88,0x98,0x80,0x80,0x88,0x70, // G
	0x88,0x88,0x88,0xf8,0x88,0x88,0x88, // H
	0xf8,0x20,0x20,0x20,0x20,0x20,0xf8, // I
	0x70,0x88,0x08,0x08,0x08,0x08,0x78, // J
	0x88,0x88,0x90,0xe0,0x90,0x88,0x88, // K
	0xf8,0x80,0x80,0x80,0x80,0x80,0x80, // L
	0x88,0x88,0x88,0x88,0xa8,0xd8,0x88, // M
	0x88,0x88,0x98,0xa8,0xc8,0x88,0x88, // N
	0x70,0x88,0x88,0x88,0x88,0x88,0x70, // O
	0x80,0x80,0x80,0xf0,0x88,0x88,0xf0, // P
	0x68,0x90,0xa8,0x88,0x88,0x88,0x70, // Q
	0x88,0x88,0x88,0xf0,0x88,0x88,0xf0, // R
	0xf0,0x08,0x08,0x70,0x80,0x80,0x78, // S
	0x20,0x20,0x20,0x20,0x20,0x20,0xf8, // T
	0x70,0x88,0x88,0x88,0x88,0x88,0x88, // U
	0x20,0x50,0x88,0x88,0x88,0x88,0x88, // V
	0x88,0xd8,0xa8,0x88,0x88,0x88,0x88, // W
	0x88,0x88,0x50,0x20,0x50,0x88,0x88, // X
	0x20,0x20,0x20,0x20,0x50,0x88,0x88, // Y
	0xf8,0x80,0x40,0x20,0x10,0x0a,0xf8, // Z
	0x30,0x20,0x20,0x20,0x20,0x20,0x30, // [
	0x00,0x08,0x10,0x20,0x40,0x80,0x00, // \ slash
	0x60,0x20,0x20,0x20,0x20,0x20,0x60, // ]
	0x00,0x00,0x00,0x00,0x88,0x50,0x20, // ^
	0xf8,0x00,0x00,0x00,0x00,0x00,0x00, // _
	0x00,0x00,0x00,0x00,0x00,0x00,0x00, // (blank)
	0x20,0x00,0x20,0x20,0x20,0x20,0x20, // !
	0x00,0x00,0x00,0x00,0x00,0x50,0x50, // "
	0x50,0x50,0xfa,0x50,0xfa,0x50,0x50, // #
	0x20,0xf0,0x2a,0x70,0xa0,0x7a,0x20, // $
	0x18,0x98,0x40,0x20,0x10,0xca,0xc0, // %
	0x68,0x90,0xfa,0x40,0xa0,0xa0,0x40, // &
	0x00,0x00,0x00,0x00,0x00,0x20,0x20, // '
	0x10,0x20,0x40,0x40,0x40,0x20,0x10, // (
	0x40,0x20,0x10,0x10,0x10,0x20,0x40, // )
	0x20,0xa8,0x70,0x20,0x70,0xa8,0x20, // *
	0x00,0x20,0x20,0xfa,0x20,0x20,0x00, // +
	0x40,0x20,0x20,0x00,0x00,0x00,0x00, // ,
	0x00,0x00,0x00,0xf8,0x00,0x00,0x00, // -
	0x20,0x00,0x00,0x00,0x00,0x00,0x00, // .
	0x00,0x80,0x40,0x20,0x10,0x08,0x00, // /
	0x70,0x88,0xc8,0xa8,0x98,0x88,0x70, // 0
	0x70,0x20,0x20,0x20,0x20,0x60,0x20, // 1
	0xf8,0x40,0x20,0x10,0x08,0x88,0x70, // 2
	0x70,0x88,0x08,0x30,0x08,0x88,0x70, // 3
	0x10,0x10,0x10,0xf8,0x90,0x50,0x30, // 4
	0xf0,0x08,0x08,0xf0,0x80,0x80,0xf8, // 5
	0x70,0x88,0x88,0xf0,0x80,0x80,0x70, // 6
	0x40,0x40,0x40,0x20,0x10,0x08,0xf8, // 7
	0x70,0x88,0x88,0x70,0x88,0x88,0x70, // 8
	0x70,0x08,0x08,0x78,0x88,0x88,0x70, // 9
	0x00,0x20,0x00,0x00,0x00,0x20,0x00, // :
	0x40,0x20,0x20,0x00,0x00,0x20,0x00, // ;
	0x10,0x20,0x40,0x80,0x40,0x20,0x10, // <
	0x00,0x00,0xf8,0x00,0xf8,0x00,0x00, // =
	0x40,0x20,0x10,0x08,0x10,0x20,0x40, // >
	0x20,0x00,0x20,0x20,0x10,0x88,0x70, // ?
	};
};

