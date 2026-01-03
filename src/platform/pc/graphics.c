#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "SDL2/SDL.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Event event;
SDL_Rect rect_window;

extern void initWindow() {
	window = SDL_CreateWindow("SNLS", 0, 0, 640, 480, 0);
	renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED);


	return;
}
extern void pollWindow() {
	SDL_SetRenderDrawColor(renderer, 0x10, 0x10, 0x10, 0xFF);
	SDL_RenderClear(renderer);

	SDL_RenderPresent(renderer);

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				exit(1);
			break;
		}
	}
	return;
};


