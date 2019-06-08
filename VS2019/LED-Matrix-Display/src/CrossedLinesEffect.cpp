#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "CrossedLinesEffect.h"

bool CrossedLinesEffect::renderFrame(int frame) {
  clear();
  for(int y = 0; y < PIXEL_Y; ++y) {
    for(int x = 0; x < PIXEL_X; ++x) {
      // Schräge Linien im Hintergrund:
      if((x - y + frame) % 15 == 0) setPixel(x, y, mkColor(1, 0));
      if((x + y - frame) % 15 == 0) setPixel(x, y, mkColor(0, 1));
    }
  }
  return true;
}
