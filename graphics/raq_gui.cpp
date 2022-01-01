#include <iostream>
#include <fstream>
#include "../computer/computer.hpp"
#include "../computer/raquette.hpp"
#include <SDL2/SDL.h>
#include <unistd.h>

#define WINDOW_WIDTH 600


void bigPixel(SDL_Renderer *renderer, int x, int y, int size){
	SDL_Rect pixel;
	pixel.x = (x*size);
	pixel.y = (y*size);
	pixel.w = size;
	pixel.h = size;
	SDL_RenderFillRect(renderer, &pixel);
	return;
}

unsigned int display_callbackfunc(Uint32 interval, void *param) {
	SDL_Event event;
    SDL_UserEvent userevent;
    userevent.type = SDL_USEREVENT;
    userevent.code = 2;
    userevent.data1 = NULL;
    userevent.data2 = NULL;
    event.type = SDL_USEREVENT;
    event.user = userevent;
    SDL_PushEvent(&event);
    return(interval);

}

unsigned int steps_callbackfunc(Uint32 interval, void *param) {
	SDL_Event event;
    SDL_UserEvent userevent;
    userevent.type = SDL_USEREVENT;
    userevent.code = 1;
    userevent.data1 = NULL;
    userevent.data2 = NULL;
    event.type = SDL_USEREVENT;
    event.user = userevent;
    SDL_PushEvent(&event);
    return(interval);
}

unsigned int kbd_callbackfunc(Uint32 interval, void *param) {
    SDL_Event event;
    SDL_UserEvent userevent;
    userevent.type = SDL_USEREVENT;
    userevent.code = 0;
    userevent.data1 = NULL;
    userevent.data2 = NULL;
    event.type = SDL_USEREVENT;
    event.user = userevent;
    SDL_PushEvent(&event);
    return(interval);
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

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_WIDTH, 0, &window, &renderer);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

#define TIME_STEP (1)
#define CPU_FACTOR (50)
// TODO Use just one callback func but set data in params
SDL_TimerID step_timer_id = SDL_AddTimer(TIME_STEP*CPU_FACTOR, steps_callbackfunc, 0);
SDL_TimerID kbd_timer_id = SDL_AddTimer(TIME_STEP, kbd_callbackfunc, 0);
SDL_TimerID display_timer_id = SDL_AddTimer(TIME_STEP*51, display_callbackfunc, 0);

	bool quit = false;
	while (!quit) {
		SDL_WaitEvent(&event);
		if(event.type == SDL_USEREVENT){
			// Keyboard Callback
			if(event.user.code == 0){
				const Uint8 *state = SDL_GetKeyboardState(NULL);
				if (state[SDL_SCANCODE_RETURN]) {
					if(raquette.memory[0xC000] != (0x0D)){
						raquette.memory[0xC000]  = (0x0D | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_A]){
					if(raquette.memory[0xC000] != ('A')){
						raquette.memory[0xC000]  = ('A' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_B]){
					if((raquette.memory[0xC000] != ('B')) && (raquette.memory[0xC000] != (0x02))){
						if((state[SDL_SCANCODE_LCTRL]) || state[SDL_SCANCODE_RCTRL]){
							raquette.memory[0xC000]  = (0x02 | 0b10000000);
						}else{
							raquette.memory[0xC000]  = ('B' | 0b10000000);
						}
					}
				}else if(state[SDL_SCANCODE_C]){
					if((raquette.memory[0xC000] != ('C')) && (raquette.memory[0xC000] != (0x03))){
						if((state[SDL_SCANCODE_LCTRL]) || state[SDL_SCANCODE_RCTRL]){
							raquette.memory[0xC000]  = (0x03 | 0b10000000);
						}else{
							raquette.memory[0xC000]  = ('C' | 0b10000000);
						}
					}
				}else if(state[SDL_SCANCODE_D]){
					if(raquette.memory[0xC000] != ('D')){
						raquette.memory[0xC000]  = ('D' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_E]){
					if(raquette.memory[0xC000] != ('E')){
						raquette.memory[0xC000]  = ('E' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_F]){
					if(raquette.memory[0xC000] != ('F')){
						raquette.memory[0xC000]  = ('F' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_G]){
					if(raquette.memory[0xC000] != ('G')){
						raquette.memory[0xC000]  = ('G' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_H]){
					if(raquette.memory[0xC000] != ('H')){
						raquette.memory[0xC000]  = ('H' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_I]){
					if(raquette.memory[0xC000] != ('I')){
						raquette.memory[0xC000]  = ('I' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_J]){
					if(raquette.memory[0xC000] != ('J')){
						raquette.memory[0xC000]  = ('J' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_K]){
					if(raquette.memory[0xC000] != ('K')){
						raquette.memory[0xC000]  = ('K' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_L]){
					if(raquette.memory[0xC000] != ('L')){
						raquette.memory[0xC000]  = ('L' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_M]){
					if(raquette.memory[0xC000] != ('M')){
						raquette.memory[0xC000]  = ('M' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_N]){
					if(raquette.memory[0xC000] != ('N')){
						raquette.memory[0xC000]  = ('N' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_O]){
					if(raquette.memory[0xC000] != ('O')){
						raquette.memory[0xC000]  = ('O' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_P]){
					if(raquette.memory[0xC000] != ('P')){
						raquette.memory[0xC000]  = ('P' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_Q]){
					if(raquette.memory[0xC000] != ('Q')){
						raquette.memory[0xC000]  = ('Q' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_R]){
					if(raquette.memory[0xC000] != ('R')){
						raquette.memory[0xC000]  = ('R' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_S]){
					if(raquette.memory[0xC000] != ('S')){
						raquette.memory[0xC000]  = ('S' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_T]){
					if(raquette.memory[0xC000] != ('T')){
						raquette.memory[0xC000]  = ('T' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_U]){
					if(raquette.memory[0xC000] != ('U')){
						raquette.memory[0xC000]  = ('U' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_V]){
					if(raquette.memory[0xC000] != ('V')){
						raquette.memory[0xC000]  = ('V' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_W]){
					if(raquette.memory[0xC000] != ('W')){
						raquette.memory[0xC000]  = ('W' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_X]){
					if(raquette.memory[0xC000] != ('X')){
						raquette.memory[0xC000]  = ('X' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_Y]){
					if(raquette.memory[0xC000] != ('Y')){
						raquette.memory[0xC000]  = ('Y' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_Z]){
					if(raquette.memory[0xC000] != ('Z')){
						raquette.memory[0xC000]  = ('Z' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_SPACE]){
					if(raquette.memory[0xC000] != (' ')){
						raquette.memory[0xC000]  = (' ' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_1]){
					if(raquette.memory[0xC000] != ('1')){
						raquette.memory[0xC000]  = ('1' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_2]){
					if(raquette.memory[0xC000] != ('2')){
						raquette.memory[0xC000]  = ('2' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_3]){
					if(raquette.memory[0xC000] != ('3')){
						raquette.memory[0xC000]  = ('3' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_4]){
					if(raquette.memory[0xC000] != ('4')){
						raquette.memory[0xC000]  = ('4' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_5]){
					if(raquette.memory[0xC000] != ('5')){
						raquette.memory[0xC000]  = ('5' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_6]){
					if(raquette.memory[0xC000] != ('6')){
						raquette.memory[0xC000]  = ('6' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_7]){
					if(raquette.memory[0xC000] != ('7')){
						raquette.memory[0xC000]  = ('7' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_8]){
					if((raquette.memory[0xC000] != ('8')) && (raquette.memory[0xC000] != ('*'))){
						if(state[SDL_SCANCODE_RSHIFT] || state[SDL_SCANCODE_LSHIFT]){
							raquette.memory[0xC000]  = ('*' | 0b10000000);
						}else{
							raquette.memory[0xC000]  = ('8' | 0b10000000);
						}
					}
				}else if(state[SDL_SCANCODE_9]){
					if((raquette.memory[0xC000] != ('9')) && (raquette.memory[0xC000] != ('('))){
						if(state[SDL_SCANCODE_RSHIFT] || state[SDL_SCANCODE_LSHIFT]){
							raquette.memory[0xC000]  = ('(' | 0b10000000);
						}else{
							raquette.memory[0xC000]  = ('9' | 0b10000000);
						}
					}
				}else if(state[SDL_SCANCODE_0]){
					if((raquette.memory[0xC000] != ('0')) && (raquette.memory[0xC000] != (')'))){
						if(state[SDL_SCANCODE_RSHIFT] || state[SDL_SCANCODE_LSHIFT]){
							raquette.memory[0xC000]  = (')' | 0b10000000);
						}else{
							raquette.memory[0xC000]  = ('0' | 0b10000000);
						}
					}

				}else if(state[SDL_SCANCODE_MINUS]){
					if((raquette.memory[0xC000] != ('-')) && (raquette.memory[0xC000] != ('_'))){
						if(state[SDL_SCANCODE_RSHIFT] || state[SDL_SCANCODE_LSHIFT]){
							raquette.memory[0xC000]  = ('_' | 0b10000000);
						}else{
							raquette.memory[0xC000]  = ('-' | 0b10000000);
						}
					}

				}else if(state[SDL_SCANCODE_COMMA]){
					if(raquette.memory[0xC000] != (',')){
						raquette.memory[0xC000]  = (',' | 0b10000000);
					}
				}else if(state[SDL_SCANCODE_SLASH]){
					if((raquette.memory[0xC000] != ('/')) && (raquette.memory[0xC000] != ('\?'))){
						if(state[SDL_SCANCODE_RSHIFT] || state[SDL_SCANCODE_LSHIFT]){
							raquette.memory[0xC000]  = ('\?' | 0b10000000);
						}else{
							raquette.memory[0xC000]  = ('/' | 0b10000000);
						}
					}

				}else if(state[SDL_SCANCODE_PERIOD]){
					if((raquette.memory[0xC000] != ('.')) && (raquette.memory[0xC000] != ('>'))){
						if(state[SDL_SCANCODE_RSHIFT] || state[SDL_SCANCODE_LSHIFT]){
							raquette.memory[0xC000]  = ('>' | 0b10000000);
						}else{
							raquette.memory[0xC000]  = ('.' | 0b10000000);
						}
					}


				}else if(state[SDL_SCANCODE_EQUALS]){
					if((raquette.memory[0xC000] != ('=')) && (raquette.memory[0xC000] != ('+'))){
						if(state[SDL_SCANCODE_RSHIFT] || state[SDL_SCANCODE_LSHIFT]){
							raquette.memory[0xC000]  = ('+' | 0b10000000);
						}else{
							raquette.memory[0xC000]  = ('=' | 0b10000000);
						}
					}
				}else if(state[SDL_SCANCODE_APOSTROPHE]){
					if((raquette.memory[0xC000] != ('\"')) && (raquette.memory[0xC000] != ('\''))){
						if(state[SDL_SCANCODE_RSHIFT] || state[SDL_SCANCODE_LSHIFT]){
							raquette.memory[0xC000]  = ('\"' | 0b10000000);
						}else{
							raquette.memory[0xC000]  = ('\'' | 0b10000000);
						}
					}
				}else if(state[SDL_SCANCODE_SEMICOLON]){
					if((raquette.memory[0xC000] != (';')) && (raquette.memory[0xC000] != (':'))){
						if(state[SDL_SCANCODE_RSHIFT] || state[SDL_SCANCODE_LSHIFT]){
							raquette.memory[0xC000]  = (':' | 0b10000000);
						}else{
							raquette.memory[0xC000]  = (';' | 0b10000000);
						}
					}

				}else if(state[SDL_SCANCODE_BACKSPACE]){
					if(raquette.memory[0xC000] != (0x08)){
						raquette.memory[0xC000]  = (0x08 | 0b10000000);
					}
				}else{
					// No keys pressed, last key read already
					if(raquette.memory[0xC000] < 0b10000000){
						raquette.memory[0xC000] = 0;
					}
				}
			// Steps callback
			}else if(event.user.code==1){
				raquette.runMicroSeconds(TIME_STEP*CPU_FACTOR*1000);
			// Display Callback
			}else if(event.user.code==2){
				if(raquette.renderScreen()){
					for(int i=0; i<192; i++){
						for(int j = 0; j<280; j++){
							// LO-RES Colors
							if(raquette.dispBuf[i][j] == 0){
								SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0); // Black
							}else if(raquette.dispBuf[i][j] == 1){
								SDL_SetRenderDrawColor(renderer, 178, 0, 98, 0); // Magenta
							}else if(raquette.dispBuf[i][j] == 2){
								SDL_SetRenderDrawColor(renderer, 2, 28, 237, 0); // Dark blue
							}else if(raquette.dispBuf[i][j] == 3){
								SDL_SetRenderDrawColor(renderer, 201, 0, 238, 0); // Purple
							}else if(raquette.dispBuf[i][j] == 4){
								SDL_SetRenderDrawColor(renderer, 34, 155, 2, 0); // Dark Green
							}else if(raquette.dispBuf[i][j] == 5){
								SDL_SetRenderDrawColor(renderer, 103, 114, 120, 0); // Grey 1
							}else if(raquette.dispBuf[i][j] == 6){
								SDL_SetRenderDrawColor(renderer, 21, 177, 234, 0); // Medium blue
							}else if(raquette.dispBuf[i][j] == 7){
								SDL_SetRenderDrawColor(renderer, 133, 135, 236, 0); // Light blue
							}else if(raquette.dispBuf[i][j] == 8){
								SDL_SetRenderDrawColor(renderer, 84, 88, 1, 0); // Brown
							}else if(raquette.dispBuf[i][j] == 9){
								SDL_SetRenderDrawColor(renderer, 225, 51, 0, 0); // Orange
							}else if(raquette.dispBuf[i][j] == 10){
								SDL_SetRenderDrawColor(renderer, 111, 109, 112, 0); // Grey 2
							}else if(raquette.dispBuf[i][j] == 11){
								SDL_SetRenderDrawColor(renderer, 224, 69, 231, 0); // Pink
							}else if(raquette.dispBuf[i][j] == 12){
								SDL_SetRenderDrawColor(renderer, 68, 246, 0, 0); // Green
							}else if(raquette.dispBuf[i][j] == 13){
								SDL_SetRenderDrawColor(renderer, 209, 216, 0, 0); // Yellow
							}else if(raquette.dispBuf[i][j] == 14){
								SDL_SetRenderDrawColor(renderer, 72, 254, 117, 0); // Aqua
							}else if(raquette.dispBuf[i][j] == 15){
								SDL_SetRenderDrawColor(renderer, 238, 231, 238, 0); // White
							// HI-RES Colors
							}else if(raquette.dispBuf[i][j] == 16){
								SDL_SetRenderDrawColor(renderer, 32, 192, 0, 0); // Green
							}else if(raquette.dispBuf[i][j] == 17){
								SDL_SetRenderDrawColor(renderer, 240, 80, 0, 0); // Orange
							}else if(raquette.dispBuf[i][j] == 18){
								SDL_SetRenderDrawColor(renderer, 160, 0, 255, 0); // Violet
							}else if(raquette.dispBuf[i][j] == 19){
								SDL_SetRenderDrawColor(renderer, 0, 128, 255, 0); // Blue
							}else{
								SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0); // Should not happen, but just in case, use black
							}
							bigPixel(renderer, 1+j, 1+i, (WINDOW_WIDTH/(40*7)));
						}
					}
				}
			SDL_RenderPresent(renderer);
			}
		}else if(event.type == SDL_QUIT){
			quit = true;
			break;
		}
/*		}else if(event.type == SDL_TEXTINPUT){
			ctrl_state = lctrl || rctrl;
			printf("%s, ctrl %s\n", event.text.text, (ctrl_state) ? "pressed" : "released");
		}else if(event.type == SDL_KEYDOWN && event.key.repeat == 0){
			if(event.key.keysym.sym == SDLK_RCTRL) { rctrl = 1; printf("erg\n");}
			else if(event.key.keysym.sym == SDLK_LCTRL) { lctrl = 1; printf("erg\n");}
			else if(event.key.keysym.sym == SDLK_RETURN){raquette.memory[0xC000] = 0x0D | 0b10000000;}
		}else if(event.type == SDL_KEYUP){
			if(event.key.keysym.sym == SDLK_RCTRL) { rctrl = 0; }
			else if(event.key.keysym.sym == SDLK_LCTRL) { lctrl = 0; }
		}
*/

		SDL_UpdateWindowSurface(window);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;
}
