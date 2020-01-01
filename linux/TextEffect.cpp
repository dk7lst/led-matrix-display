#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "TextEffect.h"
#include "font.h"

bool TextEffect::renderFrame(int frame) {
  return true;
}

void TextEffect::setText(int x, const char *szStr, byte textColor, byte bgColor) {
  clear(); //bgColor?
  while(szStr && *szStr) {
    renderChar(x, *szStr++, textColor, bgColor);
    x += FONT_X + 1;
  }
}

void TextEffect::renderChar(int Offset, char ch, byte textColor, byte bgColor) {
  int i = 0;
  while(FontMap[i]) {
    if(FontMap[i] == ch) { // Font-Daten für Zeichen gefunden?
      for(int y = 0; y < FONT_Y; ++y)
        for(int x = 0; x < FONT_X; ++x)
          setPixel(Offset + x, y, FontData[i * FONT_X + x] & (1 << y) ? textColor : bgColor);
      break;
    }
    ++i;
  }
}
