#include <SDL3/SDL.h>

extern int filledCircleRGBA(SDL_Renderer *renderer, Sint16 x, Sint16 y,
                            Sint16 rad, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

extern int ellipseColor(SDL_Renderer *renderer, Sint16 x, Sint16 y, Sint16 rx,
                        Sint16 ry, Uint32 color);

extern int ellipseRGBA(SDL_Renderer *renderer, Sint16 x, Sint16 y, Sint16 rx,
                       Sint16 ry, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
