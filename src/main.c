#include <SDL2/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#define ASSERT(_e, ...) if(!(_e)) { fprintf(stderr, __VA_ARGS__); exit(1); }

/*
 * THE BIG TODOS:
 * - actually cap fps, dont just usleep for multiple seconds
 * - move the magic numbers to definitions
 * - implement proper hit detection
 * - refactor the codebase :3
 */

#define WIN_WIDTH 1600
#define WIN_HEIGHT 900

#define MOVELEFT  0b00000001
#define MOVERIGHT 0b00000010
#define JUMP      0b00000100

#define SU        25 // pixels to define a SIZE UNIT (my answer of a measurement unit i can convert to pixels)

/* structs */
typedef struct {
  float x, y;
} Vector2;

typedef struct {
  SDL_Rect matter;
  Vector2 velocity;
  Vector2 acceleration;
} Entity;

/* bad practice but whatever */
SDL_Window *win;
SDL_Renderer *ren;
Entity player;
uint8_t moveflags;
const int GRAV = 1*SU;

void startApp(){
  win = SDL_CreateWindow(
    "test",
    SDL_WINDOWPOS_UNDEFINED,
    SDL_WINDOWPOS_UNDEFINED,
    WIN_WIDTH, WIN_HEIGHT,
    0);
  
  ASSERT(win,
	 "[main.c > main] failed to create window: %s\n",
	 SDL_GetError());

  ren = SDL_CreateRenderer(
    win,
    -1,
    SDL_RENDERER_ACCELERATED);

  ASSERT(ren,
	 "[main.c > main] failed to create renderer: %s\n",
	 SDL_GetError());
}

void exitApp(){
  if(ren) { SDL_DestroyRenderer(ren); }
  if(win) { SDL_DestroyWindow(win); }
  SDL_Quit();
}

void startPlayer(){
  memset(&player, 0, sizeof(Entity));
  player.matter.x = WIN_WIDTH/2;
  player.matter.y = WIN_HEIGHT/2;
  player.matter.w = 75;
  player.matter.h = 100;
}

void movePlayer(){
  // set gravity
  player.acceleration.y = GRAV;
  
  /* get user input */
  player.velocity.x = 0;
  if(moveflags & MOVELEFT)        { player.velocity.x = -1*SU; }
  else if (moveflags & MOVERIGHT) { player.velocity.x =  1*SU; }

  if(moveflags & JUMP)            { player.velocity.y = -5*SU; }

  printf("%3d\n", player.acceleration.y);
  
  /* apply acceleration */
  player.velocity.x += player.acceleration.x;
  player.velocity.y += player.acceleration.y;
  
  /* apply velocity */
  player.matter.x += player.velocity.x;
  player.matter.y += player.velocity.y;
}

/* until i get collision detection :) */
void setEntityWithinBounds(Entity *e){
  //e->acceleration.x = (e->matter.x+e->matter.w > WIN_WIDTH) ? e->acceleration.x-GRAV : e->acceleration.x;

  if(e->matter.y+e->matter.h > WIN_HEIGHT){
    e->matter.y=WIN_HEIGHT-e->matter.h;
    e->velocity.y = 0;
  }
}

int main(){
  /* initialise sdl, set exit condition */
  ASSERT(
    SDL_Init(SDL_INIT_VIDEO) == 0,
    "[main.c > main] failed to initialize SDL: %s\n",
    SDL_GetError()
    );
  atexit(exitApp);

  /* initialise window and renderer */
  startApp();
  
  /* initialise player */
  startPlayer();
  
  /* initialise game loop */
  SDL_Event event;
  bool run=true;
  const uint8_t *ks;
  
  while(run){
    // reset the move flags
    moveflags = 0;
    
    while(SDL_PollEvent(&event) > 0){
      if (event.type==SDL_QUIT)
	run=false;
      if (event.type==SDL_KEYUP){
	if(event.key.keysym.scancode == SDL_SCANCODE_SPACE)
	  moveflags += JUMP;
      }
    }

    /* get the keyboard state, move the player accordingly. */
    ks = SDL_GetKeyboardState(NULL);
    if(ks[SDL_SCANCODE_A])       {moveflags += MOVELEFT;}
    else if (ks[SDL_SCANCODE_D]) {moveflags += MOVERIGHT;}
    
    movePlayer();
    setEntityWithinBounds(&player);

    /* draw the scene (player is the red rectangle) */
    SDL_SetRenderDrawColor(ren, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(ren);

    SDL_SetRenderDrawColor(ren, 0xFF, 0x00, 0x00, 0xFF);
    SDL_RenderDrawRect(ren, &player.matter);
    
    SDL_RenderPresent(ren);

    /* hard-coded delay to mimic 60 fps? HELL YEAH! */
    usleep(64000);
  }
  
  exit(0);
}

