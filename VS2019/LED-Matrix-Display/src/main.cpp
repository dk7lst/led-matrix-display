#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <assert.h>
#include "CrossedLinesEffect.h"
#include "VuMeterEffect.h"
#include "TransparencyEffect.h"
#include "SerialProtocol.h"

int main() {
  puts("LED-Matrix-Display 0.01");

  SerialProtocol sport;
  int res = sport.open("COM4");
  if(res) {
    puts("ERROR opening serial port!");
    return 1;
  }

#if 0
  puts("Waiting...");
  Sleep(5000); // Wait for Arduino to rebootet on connect
#endif

  puts("Starting...");

  CrossedLinesEffect crossedlines;
  VuMeterEffect vumeters;
  TransparencyEffect trans;
  trans.push_back(&crossedlines);
  trans.push_back(&vumeters);

  int frame = 0;
  while(!_kbhit()) {
    trans.renderFrame(frame++);
    sport.sendFrame(&trans);
  }

  return 0;
}
