#include <SDL3/SDL.h>

int pixel(SDL_Renderer *renderer, Sint16 x, Sint16 y) {
  return SDL_RenderPoint(renderer, x, y);
}

int vline(SDL_Renderer *renderer, Sint16 x, Sint16 y1, Sint16 y2) {
  return SDL_RenderLine(renderer, x, y1, x, y2);
  ;
}

int hline(SDL_Renderer *renderer, Sint16 x1, Sint16 x2, Sint16 y) {
  return SDL_RenderLine(renderer, x1, y, x2, y);
  ;
}

int _drawQuadrants(SDL_Renderer *renderer, Sint16 x, Sint16 y, Sint16 dx,
                   Sint16 dy, Sint32 f) {
  int result = 0;
  Sint16 xpdx, xmdx;
  Sint16 ypdy, ymdy;

  if (dx == 0) {
    if (dy == 0) {
      result |= pixel(renderer, x, y);
    } else {
      ypdy = y + dy;
      ymdy = y - dy;
      if (f) {
        result |= vline(renderer, x, ymdy, ypdy);
      } else {
        result |= pixel(renderer, x, ypdy);
        result |= pixel(renderer, x, ymdy);
      }
    }
  } else {
    xpdx = x + dx;
    xmdx = x - dx;
    ypdy = y + dy;
    ymdy = y - dy;
    if (f) {
      result |= vline(renderer, xpdx, ymdy, ypdy);
      result |= vline(renderer, xmdx, ymdy, ypdy);
    } else {
      result |= pixel(renderer, xpdx, ypdy);
      result |= pixel(renderer, xmdx, ypdy);
      result |= pixel(renderer, xpdx, ymdy);
      result |= pixel(renderer, xmdx, ymdy);
    }
  }

  return result;
}

#define ELLIPSE_OVERSCAN 4
int _ellipseRGBA(SDL_Renderer *renderer, Sint16 x, Sint16 y, Sint16 rx,
                 Sint16 ry, Uint8 r, Uint8 g, Uint8 b, Uint8 a, Sint32 f) {
  int result;
  Sint32 rx2, ry2, rx22, ry22;
  Sint32 error;
  Sint32 curX, curY, curXp1, curYm1;
  Sint32 scrX, scrY, oldX, oldY;
  Sint32 deltaX, deltaY;

  /*
   * Sanity check radii
   */
  if ((rx < 0) || (ry < 0)) {
    return (-1);
  }

  /*
   * Set color
   */
  result = 0;
  result |= SDL_SetRenderDrawBlendMode(
      renderer, (a == 255) ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);
  result |= SDL_SetRenderDrawColor(renderer, r, g, b, a);

  /*
   * Special cases for rx=0 and/or ry=0: draw a hline/vline/pixel
   */
  if (rx == 0) {
    if (ry == 0) {
      return (pixel(renderer, x, y));
    } else {
      return (vline(renderer, x, y - ry, y + ry));
    }
  } else {
    if (ry == 0) {
      return (hline(renderer, x - rx, x + rx, y));
    }
  }

  /*
   * Top/bottom center points.
   */
  oldX = scrX = 0;
  oldY = scrY = ry;
  result |= _drawQuadrants(renderer, x, y, 0, ry, f);

  /* Midpoint ellipse algorithm with overdraw */
  rx *= ELLIPSE_OVERSCAN;
  ry *= ELLIPSE_OVERSCAN;
  rx2 = rx * rx;
  rx22 = rx2 + rx2;
  ry2 = ry * ry;
  ry22 = ry2 + ry2;
  curX = 0;
  curY = ry;
  deltaX = 0;
  deltaY = rx22 * curY;

  /* Points in segment 1 */
  error = ry2 - rx2 * ry + rx2 / 4;
  while (deltaX <= deltaY) {
    curX++;
    deltaX += ry22;

    error += deltaX + ry2;
    if (error >= 0) {
      curY--;
      deltaY -= rx22;
      error -= deltaY;
    }

    scrX = curX / ELLIPSE_OVERSCAN;
    scrY = curY / ELLIPSE_OVERSCAN;
    if ((scrX != oldX && scrY == oldY) || (scrX != oldX && scrY != oldY)) {
      result |= _drawQuadrants(renderer, x, y, scrX, scrY, f);
      oldX = scrX;
      oldY = scrY;
    }
  }

  /* Points in segment 2 */
  if (curY > 0) {
    curXp1 = curX + 1;
    curYm1 = curY - 1;
    error = ry2 * curX * curXp1 + ((ry2 + 3) / 4) + rx2 * curYm1 * curYm1 -
            rx2 * ry2;
    while (curY > 0) {
      curY--;
      deltaY -= rx22;

      error += rx2;
      error -= deltaY;

      if (error <= 0) {
        curX++;
        deltaX += ry22;
        error += deltaX;
      }

      scrX = curX / ELLIPSE_OVERSCAN;
      scrY = curY / ELLIPSE_OVERSCAN;
      if ((scrX != oldX && scrY == oldY) || (scrX != oldX && scrY != oldY)) {
        oldY--;
        for (; oldY >= scrY; oldY--) {
          result |= _drawQuadrants(renderer, x, y, scrX, oldY, f);
          /* prevent overdraw */
          if (f) {
            oldY = scrY - 1;
          }
        }
        oldX = scrX;
        oldY = scrY;
      }
    }

    /* Remaining points in vertical */
    if (!f) {
      oldY--;
      for (; oldY >= 0; oldY--) {
        result |= _drawQuadrants(renderer, x, y, scrX, oldY, f);
      }
    }
  }

  return (result);
}

int filledCircleRGBA(SDL_Renderer *renderer, Sint16 x, Sint16 y, Sint16 rad,
                     Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
  return _ellipseRGBA(renderer, x, y, rad, rad, r, g, b, a, 1);
}
