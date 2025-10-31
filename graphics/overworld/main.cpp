#include <iostream>
#include <fstream>
#include <SDL2/SDL.h>
#include <unistd.h>

#define WINDOW_WIDTH 600
#define TIME_STEP (1)

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

	SDL_Event event;
	SDL_Renderer *renderer;
	SDL_Window *window;

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_WIDTH, 0, &window, &renderer);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	SDL_TimerID kbd_timer_id = SDL_AddTimer(TIME_STEP, kbd_callbackfunc, 0);
	SDL_TimerID display_timer_id = SDL_AddTimer(TIME_STEP*51, display_callbackfunc, 0);

	bool quit = false;
	while (!quit) {
		SDL_WaitEvent(&event);
		if(event.type == SDL_QUIT){
			quit = true;
			break;
		// Display Callback
		}else if(event.user.code==2){
			// TODO Display sprites
			SDL_RenderPresent(renderer);
		}
		SDL_UpdateWindowSurface(window);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;
}
