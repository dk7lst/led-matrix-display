#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include "CrossedLinesEffect.h"
#include "TextEffect.h"
#include "VuMeterEffect.h"
#include "TransparencyEffect.h"
#include "NetworkProtocol.h"

int main() {
  puts("LED-Matrix-Display 0.10");

  NetworkProtocol link;
  link.open("192.168.4.189", 8000);

  puts("Starting...");

  CrossedLinesEffect crossedlines;
  VuMeterEffect vumeters;
  TextEffect text;
  TransparencyEffect trans;
  trans.push_back(&crossedlines);
  //trans.push_back(&vumeters);
  trans.push_back(&text);

  struct tm tm;
  memset(&tm, 0, sizeof tm);
  tm.tm_year = 2020 - 1900;
  tm.tm_mon = 0;
  tm.tm_mday = 1;
  tm.tm_hour = 0;
  tm.tm_min = 0;
  tm.tm_sec = 0;
  time_t targetTime = mktime(&tm);
  //targetTime = time(NULL) + 20;

  char buf[16];
  int frame = 0, blink = 0;
  time_t last;
  while(true) {
    const time_t now = time(NULL);
    if(now != last) {
      blink = 5;
      last = now;
    }
    int t = (int)difftime(targetTime, now);
    if(t >= 0) {
      char cSep = --blink >= 0 ? ':' : ' ';
      sprintf(buf, "%02d%c%02d%c%02d", t / 3600, cSep, (t / 60) % 60, cSep, t % 60);
    }
    else *buf = 0;
    text.setText(3 * 6, buf, FrameBuffer::ORANGE);

    trans.renderFrame(frame++);
    link.sendFrame(&trans);
    usleep(1000000 / 10);
  }

  return 0;
}
