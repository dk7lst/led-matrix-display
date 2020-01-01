#pragma once

#include "BaseEffect.h"

class TextEffect : public BaseEffect {
public:
  bool renderFrame(int frame) override;
  void setText(int x, const char *szStr, byte textColor = RED, byte bgColor = BLACK);

protected:
  void renderChar(int Offset, char ch, byte textColor = RED, byte bgColor = BLACK);
};
