#include "SDL.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

/*General Constants*/
#define NANOSECONDS_IN_A_SECOND 1000000000.0f
#define MILISECONDS_IN_A_SECOND 1000000.0f
#define FPS 16
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 300
#define WHITE_RGB_CODE 255

// Global
SDL_Window *main_window;
SDL_Renderer *main_renderer;

void init_main_window() {
    SDL_Init(SDL_INIT_VIDEO);

    main_window = SDL_CreateWindow(
        "Game",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL
    );

    if (!main_window) {
        perror("Window initialization failed.");
        exit(EXIT_FAILURE);
    }
    
    // set fullscreen
    //SDL_SetWindowFullscreen(main_window, SDL_WINDOW_FULLSCREEN);

    main_renderer = SDL_CreateRenderer(main_window, -1, SDL_RENDERER_ACCELERATED);
}

void destroy_main_window() {
    // free memory
    SDL_DestroyRenderer(main_renderer);
    SDL_DestroyWindow(main_window);
    SDL_Quit();
}


SDL_Rect player;

void handle(int *active) {
    SDL_Event event;
        
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            *active = 0;
        }

        switch(event.key.keysym.sym) {
            case SDLK_ESCAPE: *active = 0;
            case SDLK_w: player.y -= 20; break;
            case SDLK_s: player.y += 20; break;
            case SDLK_a: player.x -= 20; break;
            case SDLK_d: player.x += 20; break;
            default: break;
        }
    }
}

void draw() {
    // set black screen
    SDL_SetRenderDrawColor(main_renderer, 0, 0, 0, 255);
    // clear screen
    SDL_RenderClear(main_renderer);

    SDL_SetRenderDrawColor(main_renderer, 255, 0, 0, 255);

    SDL_RenderFillRect(main_renderer, &player);

    // present screen
    SDL_RenderPresent(main_renderer);
}

int main(int argc, char **argv) {
    init_main_window();

    player.x = 0;
    player.y = 0;
    player.h = 30;
    player.w = 30;

    int active = 1;

    while (active) {
        handle(&active);
        draw();
    }

    destroy_main_window();
    exit(EXIT_SUCCESS);
}
