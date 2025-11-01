#include <iostream>
#include <fstream>
#include <SDL2/SDL.h>
#include <unistd.h>

#define WINDOW_WIDTH 600
#define TIME_STEP (1)

#define TILE_SIZE 64
#define MAP_WIDTH 8
#define MAP_HEIGHT 8

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

	// --- LOAD SPRITES ---
	SDL_Surface *tile_surf = SDL_LoadBMP("sprites/floortile.bmp");
	SDL_Texture *tile_tex = SDL_CreateTextureFromSurface(renderer, tile_surf);
	SDL_FreeSurface(tile_surf);

	SDL_Surface *person_surf = SDL_LoadBMP("sprites/person.bmp");
	Uint32 person_key = SDL_MapRGB(person_surf->format, 255, 255, 255);
	SDL_SetColorKey(person_surf, SDL_TRUE, person_key);
	SDL_Texture *person_tex = SDL_CreateTextureFromSurface(renderer, person_surf);
	SDL_FreeSurface(person_surf);

	SDL_Surface *table_surf = SDL_LoadBMP("sprites/table.bmp");
	Uint32 table_key = SDL_MapRGB(table_surf->format, 255, 255, 255);
	SDL_SetColorKey(table_surf, SDL_TRUE, table_key);
	SDL_Texture *table_tex = SDL_CreateTextureFromSurface(renderer, table_surf);
	SDL_FreeSurface(table_surf);

	SDL_Surface *term_surf = SDL_LoadBMP("sprites/terminal.bmp");
	Uint32 term_key = SDL_MapRGB(term_surf->format, 255, 255, 255);
	SDL_SetColorKey(term_surf, SDL_TRUE, term_key);
	SDL_Texture *term_tex = SDL_CreateTextureFromSurface(renderer, term_surf);
	SDL_FreeSurface(term_surf);

	// --- PLAYER AND CAMERA STATE ---
	int player_tile_x = 0;
	int player_tile_y = MAP_HEIGHT - 1; // lower-left corner
	float cam_x = 0, cam_y = 0;

	bool quit = false;
	while (!quit) {
		SDL_WaitEvent(&event);
		if (event.type == SDL_QUIT) {
			quit = true;
			break;

		// --- Handle Keyboard ---
		// TODO Detect obstacles
		//      Log last movement direction for interaction direction and sprite selection
		//      Add interaction buttons, menu
		} else if (event.type == SDL_KEYDOWN) {
			switch (event.key.keysym.sym) {
				case SDLK_LEFT:  if (player_tile_x > 0) player_tile_x--; break;
				case SDLK_RIGHT: if (player_tile_x < MAP_WIDTH - 1) player_tile_x++; break;
				case SDLK_UP:    if (player_tile_y > 0) player_tile_y--; break;
				case SDLK_DOWN:  if (player_tile_y < MAP_HEIGHT - 1) player_tile_y++; break;
			}
		}

		// --- Display Callback ---
		else if (event.user.code == 2) {
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderClear(renderer);

			// Center player on screen
			int center_x = WINDOW_WIDTH / 2;
			int center_y = WINDOW_WIDTH / 2;

			// Camera offset so player stays centered
			cam_x = player_tile_x * TILE_SIZE - center_x + TILE_SIZE / 2;
			cam_y = player_tile_y * TILE_SIZE - center_y + 16; // Adjust for player height

			// Draw floor tiles
			for (int y = 0; y < MAP_HEIGHT; y++) {
				for (int x = 0; x < MAP_WIDTH; x++) {
					SDL_Rect dst;
					dst.x = x * TILE_SIZE - cam_x;
					dst.y = y * TILE_SIZE - cam_y;
					dst.w = TILE_SIZE;
					dst.h = TILE_SIZE;

					// Draw only visible tiles
					if (dst.x + TILE_SIZE >= 0 && dst.x < WINDOW_WIDTH &&
						dst.y + TILE_SIZE >= 0 && dst.y < WINDOW_WIDTH)
						SDL_RenderCopy(renderer, tile_tex, NULL, &dst);
				}
			}

			// Draw table
			SDL_Rect table_rect;
			table_rect.x = 2 * TILE_SIZE - cam_x;
			table_rect.y = 2 * TILE_SIZE - cam_y;
			table_rect.w = 2 * TILE_SIZE;
			table_rect.h = TILE_SIZE;
			SDL_RenderCopy(renderer, table_tex, NULL, &table_rect);

			// Draw terminal
			SDL_Rect term_rect;
			term_rect.x = 2 * TILE_SIZE - cam_x;
			term_rect.y = 2 * TILE_SIZE - cam_y - (TILE_SIZE>>2);
			term_rect.w = TILE_SIZE;
			term_rect.h = TILE_SIZE;
			SDL_RenderCopy(renderer, term_tex, NULL, &term_rect);


			// Draw player (always centered)
			SDL_Rect player_rect = { center_x - (TILE_SIZE/2), center_y - (TILE_SIZE + (TILE_SIZE>>2)), TILE_SIZE, 2* TILE_SIZE };
			SDL_RenderCopy(renderer, person_tex, NULL, &player_rect);

			SDL_RenderPresent(renderer);
		}

		SDL_UpdateWindowSurface(window);
	}

	SDL_DestroyTexture(tile_tex);
	SDL_DestroyTexture(person_tex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;
}

