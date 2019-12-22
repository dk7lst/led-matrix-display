#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <assert.h>
#include "CrossedLinesEffect.h"
#include "VuMeterEffect.h"
#include "TransparencyEffect.h"
#include "SerialProtocol.h"
#include "NetworkProtocol.h"

#define USE_NETWORK 1 // 0: USB-Serial, 1: UDP/IP

int main() {
  puts("LED-Matrix-Display 0.10");

#if USE_NETWORK
  NetworkProtocol link;
  link.open("192.168.0.147", 8000);
#else
  SerialProtocol link;
  int res = link.open("COM4");
  if(res) {
    puts("ERROR opening serial port!");
    return 1;
  }
#endif

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
    link.sendFrame(&trans);
    Sleep(1000 / 60);
  }

  return 0;
}
