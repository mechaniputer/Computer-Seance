#include <iostream>
#include <fstream>
#include "../computer/computer.hpp"
#include "../computer/raquette.hpp"
#include <SDL2/SDL.h>
#include <unistd.h>

#define WINDOW_WIDTH 600


void bigPixel(SDL_Renderer *renderer, int x, int y, int size){
	for(int row=0; row<size; row++){
		for(int subpix=0; subpix < size; subpix++){
			SDL_RenderDrawPoint(renderer, (x*size)-subpix, (y*size)-row);
		}
	}
	return;
}

int main(void) {

	// TODO Place this in class Raquette
//	std::ifstream infile("../computer/A2ROM.BIN", std::ios::binary | std::ios::in);
	std::ifstream infile("../computer/apple.rom", std::ios::binary | std::ios::in);
	if(!infile){
		std::cout << "Cannot open ROM file\n";
		exit(0);
	}
	//get length of file
	infile.seekg(0, std::ios::end);
	size_t length = infile.tellg();
	infile.seekg(0, std::ios::beg);

	char * buffer = new char[length];
	std::cout << "Opened file of length " << length << std::endl;
	infile.read(buffer, length);

	uint8_t raq_rom_arr[0xFFFF];
	for (unsigned i=0; i < 0xD000; i++) {
		raq_rom_arr[i] = 0x00; // Zero low mem
	}
	for (unsigned i=0; i < length; i++) {
		raq_rom_arr[i+0xD000] = buffer[i];
	}

	delete [] buffer;

	Raquette raquette(raq_rom_arr, 0xFFFF+1);

	SDL_Event event;
	SDL_Renderer *renderer;
	SDL_Window *window;

	SDL_Init(SDL_INIT_VIDEO);
	SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_WIDTH, 0, &window, &renderer);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	bool quit = false;
	int lctrl = 0, rctrl = 0;
	int ctrl_state = 0;

	int prevpc = 0xFFFFF;
	int numsteps = 0;

	while (!quit) {
		SDL_PollEvent(&event);
		switch (event.type)
		{
		case SDL_QUIT:
			quit = true;
			break;
		case SDL_TEXTINPUT:
			ctrl_state = lctrl || rctrl;
			printf("%s, ctrl %s\n", event.text.text, (ctrl_state) ? "pressed" : "released");
			break;
		case SDL_KEYDOWN:
			if(event.key.keysym.sym == SDLK_RCTRL) { rctrl = 1; printf("erg\n");}
			else if(event.key.keysym.sym == SDLK_LCTRL) { lctrl = 1; printf("erg\n");}
			break;
		case SDL_KEYUP:
			if(event.key.keysym.sym == SDLK_RCTRL) { rctrl = 0; }
			else if(event.key.keysym.sym == SDLK_LCTRL) { lctrl = 0; }
			break;
		}
		SDL_UpdateWindowSurface(window);

		// Run the computer
		numsteps++;
		if(raquette.step(false)) break;
		if(raquette.pc == prevpc) break;
		prevpc = raquette.pc;

		// Draw the screen
		if(numsteps%20000 == 0){
			raquette.renderScreen();
			for(int i=0; i<192; i++){
				for(int j = 0; j<280; j++){
					SDL_SetRenderDrawColor(renderer, raquette.dispBuf[i][j].r, raquette.dispBuf[i][j].g, raquette.dispBuf[i][j].b, 0);
					bigPixel(renderer, j, i, (WINDOW_WIDTH/(40*7)));
				}
			}
			SDL_RenderPresent(renderer);
		}else if(numsteps%300 == 0){
			usleep(4000);
		}
	}
	std::cout << "Executed " << numsteps << " instructions\n";

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;
}
