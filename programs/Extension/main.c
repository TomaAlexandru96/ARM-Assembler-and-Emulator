#if __STDC_VERSION__ >= 199901L
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 500
#endif /* __STDC_VERSION__ */

#include <SDL.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

/*General Constants*/
#define NANOSECONDS_IN_A_SECOND 1000000000.0f
#define MILISECONDS_IN_A_SECOND 1000000.0f
#define FPS 16
#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 600
#define WHITE_RGB_CODE 255

//Prototypes
float convert_time(struct timespec *ts);
void start_time(void);
void end_time(void);
float convert_time(struct timespec *ts);
void init(void);
void handle(void);
void draw(void);

//Global variables
SDL_Surface *screen;
float time_measure = 0.0f;
float delta_time = 0.0f;

int main(int argc, char **argv) {
  if(SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
  }

  screen = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, 0, SDL_HWSURFACE | SDL_DOUBLEBUF);
  if(!screen) {
    fprintf(stderr, "Could not set video mode: %s\n", SDL_GetError());
  }

  int active = 1;
  //Main loop
  while (active) {
    SDL_Event ev;
    //Handle events
    while (SDL_PollEvent(&ev)) {
      if(ev.type == SDL_QUIT) {
        active = 0;
      }
    }
    //Start time
    start_time();
    //Handle game
    handle();
    //Draw game
    draw();
    //End time
    end_time();
  }

  //Exit
  SDL_FreeSurface(screen);
  SDL_Quit();
}


/*Convert from timespec to float*/
float convert_time(struct timespec *ts) {
  float accu;
  accu = (float) ts->tv_sec;
  //Seconds that have gone by
  accu *= NANOSECONDS_IN_A_SECOND;
  accu += (float) ts->tv_nsec;
  //Nanoseconds that have gone by
  accu /= MILISECONDS_IN_A_SECOND;
  return accu;
}

/*Start time*/
void start_time(void) {
  struct timespec ts;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
  time_measure = convert_time(&ts);
}

/*End time*/
void end_time(void) {
  struct timespec ts;
  float delta;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
  delta = convert_time(&ts) - time_measure;
  //Found "distance" in time
  delta_time =  delta / (1000.0f / FPS);
}

void handle() {

}

void draw(void) {
  //Clear screen
  SDL_FillRect(screen, NULL, SDL_MapRGBA(screen->format, 0, 0, 0, 255));

  SDL_LockSurface(screen);
  /*
	int i,rank,x,y;
	Uint32 *pixel;
	rank = demo_screen->pitch / sizeof(Uint32);
	pixel = (Uint32*) demo_screen->pixels;
	// Draw all dots
	for(i = 0; i < NUM_DOTS; i++) {
		// Rasterize position as integer
		x = (int) (demo_dots[i].x);
		y = (int) (demo_dots[i].y);
		// Set pixel
		pixel[x+y*rank] = SDL_MapRGBA(demo_screen->format, demo_dots[i].red, demo_dots[i].green,0,255);
	}
  */

  SDL_UnlockSurface(screen);

  //Show screen
  SDL_Flip(screen);
}
