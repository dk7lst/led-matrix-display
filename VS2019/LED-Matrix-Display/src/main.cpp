#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <assert.h>
#include <windows.h>
#include "FrameBuffer.h"
#include "SerialProtocol.h"

int main() {
  puts("LED-Matrix-Display 0.01");

  SerialProtocol sport;
  int res = sport.open("COM4");
  if(res) {
    puts("ERROR opening serial port!");
    return 1;
  }

  puts("Waiting...");
  Sleep(5000); // Arduino rebootet beim Verbinden, daher warten bis der Selbsttest fertig ist.
  puts("Starting...");

  FrameBuffer img;
  int phase = 0, vu[2] = {0, 0}, vuMax[2] = {0, 2};
  while(!_kbhit()) {
    img.clear();
    if(phase % 15 == 0) {
      for(int i = 0; i < 2; ++i) vu[i] = vuMax[i] = max(vu[i], img.PIXEL_X / 2 + rand() % (img.PIXEL_X / 2));
    }
    for(int y = 0; y < img.PIXEL_Y; ++y) {
      for(int x = 0; x < img.PIXEL_X; ++x) {
        // Schräge Linien im Hintergrund:
        if((x - y + phase) % 15 == 0) img.setPixel(x, y, img.mkColor(1, 0));
        if((x + y - phase) % 15 == 0) img.setPixel(x, y, img.mkColor(0, 1));

        // VU-Meter:
        if(x < vu[0] && y >= 1 && y <= 2) img.setPixel(x, y, img.mkColor(3 - x * 4 / img.PIXEL_X, x * 4 / img.PIXEL_X));
        if(x < vu[1] && y >= 4 && y <= 5) img.setPixel(x, y, img.mkColor(3 - x * 4 / img.PIXEL_X, x * 4 / img.PIXEL_X));
      }
      if(y >= 1 && y <= 2) img.setPixel(vuMax[0], y, img.RED);
      if(y >= 4 && y <= 5) img.setPixel(vuMax[1], y, img.RED);
    }
    for(int i = 0; i < 2; ++i) if(vu[i] > 0) --vu[i];
    sport.sendFrame(&img);
    ++phase;
  }

  return 0;
}
