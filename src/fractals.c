#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <SDL2/SDL.h>

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 600

typedef struct {
  double real;
  double imaginary;
} ComplexDouble;

int is_in_set(double x, double y, ComplexDouble c)
{
  const unsigned int iterations = 50;
  ComplexDouble z = {.real = x, .imaginary = y};
  double real, imaginary;
  for (unsigned int i = 0; i < iterations; i++) {
    if ( sqrt((z.real * z.real) + (z.imaginary * z.imaginary)) >= 2.0)
      return i;
    /* z = z^2 + c */
    /* z^2 = (x + yi) * (x + yi) = x^2 + 2*x*yi  - y^2 */
    real = z.real;
    imaginary = z.imaginary;
    z.real = (real * real) - (imaginary * imaginary);
    z.imaginary = 2 * real * imaginary;
    z.real += c.real;
    z.imaginary += c.imaginary;

    //fprintf(stdout, "ComplexDouble(real: %f, imaginary: %f)\n", z.real, z.imaginary);
  }
  return -1;
}

void print_sdl_err()
{
  fprintf(stderr, "[error]: SDL Error: %s\n", SDL_GetError());
  SDL_ClearError();
}

bool sdl_init(SDL_Window **w, SDL_Renderer **r)
{
  if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
    print_sdl_err();
    return false;
  }
  
  *w = SDL_CreateWindow("fractals", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_BORDERLESS | SDL_WINDOW_OPENGL);
  if (w == NULL) {
    print_sdl_err();
    return false;
  }
  
  *r = SDL_CreateRenderer(*w, -1, SDL_RENDERER_ACCELERATED);
  if (r == NULL) {
    print_sdl_err();
    return false;
  }
  
  SDL_SetRenderDrawBlendMode(*r, SDL_BLENDMODE_BLEND);

  srand(time(NULL));
  
  return true;
}

bool sdl_free(SDL_Window *w, SDL_Renderer *r)
{
  SDL_DestroyRenderer(r);
  SDL_DestroyWindow(w);
  SDL_Quit();

  return true;
}

int main(int argc, char *argv[])
{
  SDL_Window *window = NULL;
  SDL_Renderer *renderer = NULL;

  SDL_Event e;
  SDL_Keycode sym;
  
  bool should_quit = false;
  bool should_redraw = true;

  Uint64 loop_start_ms, delta_time_ms, wait_time_ms;
  
  ComplexDouble c = {.real = 1.1, .imaginary = -0.01};
  
  if (argc != 1) {
    for (int i = 1; i < argc; i++)
      fprintf(stdout, "arg[%i]: %s\n", i, argv[i]);
  }
  
  if (!sdl_init(&window, &renderer))
    return EXIT_FAILURE;

  while (should_quit == false) {
    loop_start_ms = SDL_GetTicks64();
    
    while (SDL_PollEvent(&e)) {
      switch (e.type) {
      case SDL_QUIT:
	should_quit = true;
	break;
	/* handle KEYDOWN events */
      case SDL_KEYDOWN:
	sym = e.key.keysym.sym;
	if (sym == SDLK_q || sym == SDLK_ESCAPE)
	  should_quit = true;
	if (sym == SDLK_SPACE)
	  should_redraw = true;
	break;
      default:
	break;
      } /* END switch(e.type) */
    } /* END while (SDL_PollEvent(&e) */

    if (should_redraw) {
      SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
      SDL_RenderClear(renderer);
      SDL_RenderPresent(renderer);
      
      /* do the drawing */
      for (double x = 0.0; x < 1.0; x += 0.001) {
	for (double y = 0.0; y < 1.0; y += 0.001) {
	  int iters = is_in_set(x, y, c);
	  if (iters > -1) {
	    SDL_SetRenderDrawColor(renderer, (iters * 1000) % 255, (iters * 1055) % 255, (iters * 1111) % 255, 0xff);
	    SDL_RenderDrawPoint(renderer, (int) (x * WINDOW_WIDTH), (int) (y * WINDOW_HEIGHT));
	  } else {
	    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
	    SDL_RenderDrawPoint(renderer, (int) (x * WINDOW_WIDTH), (int) (y * WINDOW_HEIGHT));
	    //fprintf(stdout, "Drawing (%i, %i)\n", (int) (x * WINDOW_WIDTH), (int) (y * WINDOW_HEIGHT));
	  }
	 
	  //fprintf(stdout, "Is (%f, %f) in set? Iters: %i\n", x, y, is_in_set(x, y));
	}
      }
      
      SDL_RenderPresent(renderer);

      c.real = -1.0 + (rand() / (RAND_MAX / 2));
      c.imaginary = -1.0 + (rand() / (RAND_MAX / 2));

      should_redraw = false;
    } /* end if(should_redraw) */
    
    delta_time_ms = SDL_GetTicks64() - loop_start_ms;
    wait_time_ms = (( 1000 / 30 ) - (int) delta_time_ms) > 0 ? ((1000 / 30) - delta_time_ms) : 0;
    if (wait_time_ms > 0) {
      SDL_Delay(wait_time_ms);
    }
      
  } /* END while (should_quit == false) */
  
  if (!sdl_free(window, renderer))
      return EXIT_FAILURE;
  
  return EXIT_SUCCESS;
}
