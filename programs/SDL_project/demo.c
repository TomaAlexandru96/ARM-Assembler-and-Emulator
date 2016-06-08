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
#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800
#define WHITE_RGB_CODE 255
/*Game dynamics constants*/
#define NUM_DOTS 1024

//Type definitions
typedef struct {
  int red;
  int green;
  //Blue is always 0(background)
  float vx;
  float vy;
  //Speed
  float x;
  float y;
  //Current position
} moving_dot;

//Prototypes
float demo_convert_time(struct timespec *ts);
void demo_start_time(void);
void demo_end_time(void);
float demo_convert_time(struct timespec *ts);
void demo_init(void);
void demo_handle(void);
void demo_draw(void);

//Global variables
SDL_Surface *demo_screen;
float demo_time_measure = 0.0f;
float demo_time_step = 0.0f;
moving_dot demo_dots[NUM_DOTS];

int main(int argc, char **argv) {
  SDL_Event ev;
  int active;
  if(SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
  }
  printf("%s\n", "SDL Initialized");
  demo_screen = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, 0, SDL_HWSURFACE|SDL_DOUBLEBUF);
  if(!demo_screen) {
    fprintf(stderr, "Could not set video mode: %s\n", SDL_GetError());
  }
  demo_init();
  active = 1;
  //Main loop
  while (active) {
    //Handle events
    while (SDL_PollEvent(&ev)) {
      if(ev.type == SDL_QUIT) {
        active = 0;
      }
    }
    //Start time
    demo_start_time();
    //Handle game
    demo_handle();
    //Clear screen
    SDL_FillRect(demo_screen,NULL,SDL_MapRGBA(demo_screen->format,0,0,255,255));
    //Draw game
    demo_draw();
    //Show screen
    SDL_Flip(demo_screen);
    //End time
    demo_end_time();
  }

 //Exit
  SDL_Quit();
}


/*Convert from timespec to float*/
float demo_convert_time(struct timespec *ts) {
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
void demo_start_time(void) {
  struct timespec ts;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
  demo_time_measure = demo_convert_time(&ts);
}

/*End time*/
void demo_end_time(void) {
  struct timespec ts;
  float delta;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
  delta = demo_convert_time(&ts) - demo_time_measure;
  //Found "distance" in time
  demo_time_step =  delta / (1000.0f / FPS);
}

/*So, remembering your mathmatics, if the engine takes longer than 16 frames
per second (1000/16 milliseconds), then the time step value becomes
greater than 1.0f. Otherwise, the faster the engine runs, the smaller
this value gets. The 16 frames per second is a magic number, if your speed is 1,
then you will move 16 units a second. You can see where this is going,
we can multiply our speeds with this value to get smooth,
frame-independant movement that video games usually have. */


/*Returns random float in interval [0.0f, 1.0f]*/
float demo_roll(void) {
  float r;
  r = (float) (rand() % RAND_MAX);
  r /= (float) (RAND_MAX - 1);
  return r;
}

/*Initialize dots*/
void demo_init() {
  for(int i = 0; i < NUM_DOTS; i++) {
    demo_dots[i].red = rand() % WHITE_RGB_CODE;
    demo_dots[i].green = rand() % WHITE_RGB_CODE;
    demo_dots[i].vx = demo_roll() * FPS - ((float)FPS / 2.0f);
    demo_dots[i].vy = demo_roll() * FPS - ((float)FPS / 2.0f);
    demo_dots[i].x = demo_roll() * WINDOW_WIDTH;
    demo_dots[i].y = demo_roll() * WINDOW_HEIGHT;
  }
}

/* Handle dots */
void demo_handle(void) {
	int i;
	for(i = 0;i < NUM_DOTS;i++) {
		/* Move */
		demo_dots[i].x += demo_dots[i].vx*demo_time_step;
		demo_dots[i].y += demo_dots[i].vy*demo_time_step;
		/* Hit walls? */
		if(demo_dots[i].x < 0.0f || demo_dots[i].x >= 320.0f) {
			/* Undo move (demo_time_step is still the same value it was before and is valid for the current frame) */
			demo_dots[i].x -= demo_dots[i].vx*demo_time_step;
			/* Reverse */
			demo_dots[i].vx = -demo_dots[i].vx;
		}
		if(demo_dots[i].y < 0.0f || demo_dots[i].y >= 240.0f) {
			/* Undo move (demo_time_step is still the same value it was before and is valid for the current frame) */
			demo_dots[i].y -= demo_dots[i].vy*demo_time_step;
			/* Reverse */
			demo_dots[i].vy = -demo_dots[i].vy;
		}
	}
}
  /* Draw dots */
void demo_draw(void) {
	int i,rank,x,y;
	Uint32 *pixel;
	/* Lock surface */
	SDL_LockSurface(demo_screen);
	rank = demo_screen->pitch / sizeof(Uint32);
	pixel = (Uint32*) demo_screen->pixels;
	/* Draw all dots */
	for(i = 0; i < NUM_DOTS; i++) {
		/* Rasterize position as integer */
		x = (int) (demo_dots[i].x);
		y = (int) (demo_dots[i].y);
		/* Set pixel */
		pixel[x+y*rank] = SDL_MapRGBA(demo_screen->format, demo_dots[i].red, demo_dots[i].green,0,255);
	}
  /* Unlock surface */
  SDL_UnlockSurface(demo_screen);
}
