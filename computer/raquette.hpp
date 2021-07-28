#pragma once

class Raquette: public Computer {
	public:
	// 7 processor status flags:
	bool flag_c, flag_z, flag_i, flag_d, flag_b, flag_v, flag_n;
	Raquette(uint8_t *init_contents = nullptr, int len_contents = 0);
	// TODO reset (for resetting regs and pc)
	std::tuple<int, int> aModeHelper(uint8_t thisbyte);
	uint8_t rolHelper(uint8_t byte);
	uint8_t rorHelper(uint8_t byte);
	void softSwitchesHelper(int eff_addr);
	void branchHelper();
	int step(bool verbose = false);
	void show_regs();
	void show_screen();
};
