#include "sdl2_gfx.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define FPS 60
#define FRAME_TARGET_TIME (1000 / FPS) // 1 second/FPS
#define ARR_SIZE 100

int quit = 0;
int last_frame_time = 0;
TTF_Font *font = NULL;
SDL_Color textColor = {255, 255, 255, 255};

struct Circle {
  char name[100];
  float xPos;
  float yPos;
  float xSpeed;
  float ySpeed;
  int R;
  int G;
  int B;
  float radius;
  bool drawCircle;
  bool drawText;

  // for ttf
  SDL_Texture *textTex;
  SDL_FRect *textRect;
};

struct Rectangle {
  char name[100];
  float xPos;
  float yPos;
  float xSpeed;
  float ySpeed;
  int R;
  int G;
  int B;
  float w;
  float h;
  bool drawRectangle;
  bool drawText;
  SDL_Texture *textTex;
  SDL_FRect *textRect;
};

// keep track of where to insert the next struct
int circleArrIndex, rectangleArrIndex = 0;

struct Circle circleArr[ARR_SIZE];
struct Rectangle rectangleArr[ARR_SIZE];
struct Circle circle;
struct Rectangle rectangle;

void process_input() {
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_EVENT_QUIT:
      quit = 1;
      break;
    case SDL_EVENT_KEY_DOWN:
      if (event.key.key == SDLK_ESCAPE || event.key.key == SDLK_Q) {
        quit = 1;
      }
    }
  }
}

void update() {
  int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - last_frame_time);

  // caps the framerate
  if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
    SDL_Delay(time_to_wait);
  }
  last_frame_time = SDL_GetTicks();

  // update rect/circle and their corresponding text positions
  for (int i = 0; i < circleArrIndex; i++) {
    circleArr[i].xPos += circleArr[i].xSpeed;
    circleArr[i].yPos += circleArr[i].ySpeed;
    circleArr[i].textRect->x += circleArr[i].xSpeed;
    circleArr[i].textRect->y += circleArr[i].ySpeed;
  }
  for (int i = 0; i < rectangleArrIndex; i++) {
    rectangleArr[i].xPos += rectangleArr[i].xSpeed;
    rectangleArr[i].yPos += rectangleArr[i].ySpeed;
    rectangleArr[i].textRect->x += rectangleArr[i].xSpeed;
    rectangleArr[i].textRect->y += rectangleArr[i].ySpeed;
  }

  // detect rectangle collision
  for (int i = 0; i < rectangleArrIndex; i++) {
    if (rectangleArr[i].xPos < 0 ||
        rectangleArr[i].xPos + rectangleArr[i].w > WINDOW_WIDTH) {
      rectangleArr[i].xSpeed *= -1;
    } else if (rectangleArr[i].yPos < 0 ||
               rectangleArr[i].yPos + rectangleArr[i].h > WINDOW_HEIGHT) {
      rectangleArr[i].ySpeed *= -1;
    }
  }

  // detect circle collision
  for (int i = 0; i < circleArrIndex; i++) {
    if (circleArr[i].xPos - circleArr[i].radius < 0 ||
        circleArr[i].xPos + circleArr[i].radius > WINDOW_WIDTH) {
      circleArr[i].xSpeed *= -1;
    } else if (circleArr[i].yPos - circleArr[i].radius < 0 ||
               circleArr[i].yPos + circleArr[i].radius > WINDOW_HEIGHT) {
      circleArr[i].ySpeed *= -1;
    }
  }
}

void render(SDL_Renderer *renderer) {
  // sets the color for drawing operations
  // SDL_SetRenderDrawColor() is like taking the brush and dipping in black ink
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);

  // clear the screen before drawing
  // screen color set to black due to the previous SDL_SetRenderDrawColor() call
  SDL_RenderClear(renderer);

  // draw circle with sdl2_gfx library
  for (int i = 0; i < circleArrIndex; i++) {
    filledCircleRGBA(renderer, circleArr[i].xPos, circleArr[i].yPos,
                     circleArr[i].radius, circleArr[i].R, circleArr[i].G,
                     circleArr[i].B, 255);

    // render text (circle name) for circle
    SDL_RenderTexture(renderer, circleArr[i].textTex, NULL,
                      circleArr[i].textRect);
  }

  // draw rect with inbuilt SDL3 lib
  for (int i = 0; i < rectangleArrIndex; i++) {
    SDL_FRect rect = {rectangleArr[i].xPos, rectangleArr[i].yPos,
                      rectangleArr[i].w, rectangleArr[i].h};
    SDL_SetRenderDrawColor(renderer, rectangleArr[i].R, rectangleArr[i].G,
                           rectangleArr[i].B, 255);
    SDL_RenderFillRect(renderer, &rect);
    SDL_RenderRect(renderer, &rect);
    SDL_RenderTexture(renderer, rectangleArr[i].textTex, NULL,
                      rectangleArr[i].textRect);
  }
  SDL_RenderPresent(renderer);
}

int main() {
  SDL_Window *window = NULL;
  SDL_Renderer *renderer = NULL;
  int width = 1280, height = 720; // default values

  int result = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
  if (result < 0) {
    SDL_Log("SDL_Init error: %s\n", SDL_GetError());
    SDL_Quit();
  }

  // title, w, h, 0 for window render flags
  window = SDL_CreateWindow("SDL3 window", width, height, 0);
  if (window == NULL) {
    SDL_Log("SDL_CreateWindow error: %s\n", SDL_GetError());
    SDL_Quit();
  }

  // 2nd parameter - name of rendering driver to init
  // NULL to init the first one supporting the requested flag
  renderer = SDL_CreateRenderer(window, NULL);
  if (renderer == NULL) {
    SDL_Log("SDL_CreateRenderer error: %s\n", SDL_GetError());
    SDL_Quit();
  }

  if (!TTF_Init()) {
    SDL_Log("SDL_GetError: %s\n", SDL_GetError());
    SDL_Quit();
  }
  TTF_Font *font = TTF_OpenFont("tech.ttf", 24); // load font of size 16

  if (!font) {
    SDL_Log("SDL_GetError: %s\n", SDL_GetError());
    SDL_Quit();
    TTF_Quit();
  }

  /*
   * Window W H
   * Font filepath size R G B
   * Circle
   * Rectangle name xPos yPos xSpeed ySpeed R G B W H
   * Circle name xPos yPos xSpeed ySpeed R G B radius
   *
   * e.g:
   * Window 1280 720
   * Font fonts/tech.ttf 18 255 255 255
   * Circle CGreen 100 100 -3 2 0 255 0 50
   * Rectangle RTeal 25 100 -2 -2 0 255 255 100 100
   */

  /* fgets doesn't strip the terminating \n, checking its
   * presence would allow to handle lines longer that sizeof(buffer)
   */

  SDL_Surface *textSurface = NULL;
  SDL_Texture *textTexture = NULL;

  FILE *file = fopen("config.txt", "r");
  char buffer[255];

  while (fgets(buffer, sizeof(buffer), file)) {
    // malloc needed to get unique address of each SDL_FRect
    SDL_FRect *tempRect = NULL;

    // temporary int as TTF_GetStringSize() requires int instead of float
    int tempW = 0;
    int tempH = 0;

    char *token = strtok(buffer, " ");

    // able to use switch because first characters are unique
    // W for Windows, F for Font etc...
    switch (token[0]) {
    case 'W':
      // atoi - ASCII to int
      // NULL to continue using the prior given string
      width = atoi(strtok(NULL, " "));  // 1st param
      height = atoi(strtok(NULL, " ")); // 2nd param
      break;

    case 'F':
      break;

    case 'C':
      // create and initialize circle struct, add to circle array
      strcpy(circle.name, strtok(NULL, " "));
      circle.xPos = atof(strtok(NULL, " "));
      circle.yPos = atof(strtok(NULL, " "));
      circle.xSpeed = atof(strtok(NULL, " "));
      circle.ySpeed = atof(strtok(NULL, " "));
      circle.R = atoi(strtok(NULL, " "));
      circle.G = atoi(strtok(NULL, " "));
      circle.B = atoi(strtok(NULL, " "));
      circle.radius = atof(strtok(NULL, " "));
      circle.drawText = true;
      circle.drawCircle = true;

      // 3rd param - length of the text in bytes, 0 for null terminated text
      textSurface = TTF_RenderText_Solid(font, circle.name, 0, textColor);
      if (textSurface == NULL) {
        SDL_Log("Unable to render text surface! SDL_ttf Error: %s\n",
                SDL_GetError());
        SDL_Quit();
        TTF_Quit();
      }

      textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
      if (textTexture == NULL) {
        SDL_Log("Unable to create texture from rendered text! SDL Error: %s\n",
                SDL_GetError());
        SDL_Quit();
        TTF_Quit();
      }
      // SDL_CreateTextureFromSurface uploads the data from CPU to the GPU,
      // after running it, surface can be destroyed as it is no longer needed
      SDL_DestroySurface(textSurface);
      circle.textTex = textTexture;

      tempRect = malloc(sizeof(SDL_FRect));
      // TTF_GetStringSize() alters tempW and tempH
      // equivalent of TTF_SizeText() in SDl2
      if (!TTF_GetStringSize(font, circle.name, strlen(circle.name), &tempW,
                             &tempH)) {
        SDL_Log("Unable to get string size: %s\n", SDL_GetError());
        SDL_Quit();
        TTF_Quit();
      };

      // subtract ((float)tempW / 2) from the position to center the text,
      // circle draws text from midpoint hence, just subtracting will do
      tempRect->x = circle.xPos - ((float)tempW / 2);
      tempRect->y = circle.yPos - ((float)tempH / 2);
      tempRect->w = tempW;
      tempRect->h = tempH;
      circle.textRect = tempRect;

      circleArr[circleArrIndex] = circle;
      circleArrIndex++;
      break;

    case 'R':
      strcpy(rectangle.name, strtok(NULL, " "));
      rectangle.xPos = atof(strtok(NULL, " "));
      rectangle.yPos = atof(strtok(NULL, " "));
      rectangle.xSpeed = atof(strtok(NULL, " "));
      rectangle.ySpeed = atof(strtok(NULL, " "));
      rectangle.R = atoi(strtok(NULL, " "));
      rectangle.G = atoi(strtok(NULL, " "));
      rectangle.B = atoi(strtok(NULL, " "));
      rectangle.w = atof(strtok(NULL, " "));
      rectangle.h = atof(strtok(NULL, " "));
      rectangle.drawText = true;
      rectangle.drawRectangle = true;

      SDL_Surface *textSurface =
          TTF_RenderText_Solid(font, rectangle.name, 0, textColor);
      if (textSurface == NULL) {
        SDL_Log("Unable to render text surface! SDL_ttf Error: %s\n",
                SDL_GetError());
        SDL_Quit();
        TTF_Quit();
      }
      SDL_Texture *textTexture =
          SDL_CreateTextureFromSurface(renderer, textSurface);
      if (textTexture == NULL) {
        SDL_Log("Unable to create texture from rendered text! SDL Error: %s\n",
                SDL_GetError());
        SDL_Quit();
        TTF_Quit();
      }
      SDL_DestroySurface(textSurface);
      rectangle.textTex = textTexture;

      tempRect = malloc(sizeof(SDL_FRect));
      if (!TTF_GetStringSize(font, rectangle.name, strlen(rectangle.name),
                             &tempW, &tempH)) {
        SDL_Log("Unable to get string size: %s\n", SDL_GetError());
        SDL_Quit();
        TTF_Quit();
      };

      // rect draws text from top left corner
      tempRect->x = rectangle.xPos + ((rectangle.w / 2) - ((float)tempW / 2));
      tempRect->y = rectangle.yPos + ((rectangle.h / 2) - ((float)tempH / 2));
      tempRect->w = tempW;
      tempRect->h = tempH;
      rectangle.textRect = tempRect;
      rectangleArr[rectangleArrIndex] = rectangle;
      rectangleArrIndex++;
      break;

    default:
      SDL_Log("Error reading config.txt\n");
      SDL_Quit();
      TTF_Quit();
      break;
    }
  }
  fclose(file);

  /* for (int i = 0; i < circleArrIndex; i++) { */
  /*   SDL_Log("name: %s \n", circleArr[i].name); */
  /*   SDL_Log("xpos: %f \n", circleArr[i].xPos); */
  /*   SDL_Log("ypos: %f \n", circleArr[i].yPos); */
  /*   SDL_Log("xspeed: %f \n", circleArr[i].xSpeed); */
  /*   SDL_Log("yspeed: %f \n", circleArr[i].ySpeed); */
  /*   SDL_Log("R: %d \n", circleArr[i].R); */
  /*   SDL_Log("G: %d \n", circleArr[i].G); */
  /*   SDL_Log("B: %d \n", circleArr[i].B); */
  /*   SDL_Log("radius: %f \n", circleArr[i].radius); */
  /*   SDL_Log("text Placeholder Rect (x,y,w,h): %f %f %f %f\n", */
  /*           circleArr[i].textRect->x, circleArr[i].textRect->y, */
  /*           circleArr[i].textRect->w, circleArr[i].textRect->h); */
  /* } */

  while (!quit) {
    process_input();
    update();
    render(renderer);
  }

  for (int i = 0; i < circleArrIndex; i++) {
    SDL_DestroyTexture(circleArr[i].textTex);
    free(circleArr[i].textRect);
  }
  for (int i = 0; i < rectangleArrIndex; i++) {
    SDL_DestroyTexture(rectangleArr[i].textTex);
    free(rectangleArr[i].textRect);
  }
  TTF_CloseFont(font);
  TTF_Quit();
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
