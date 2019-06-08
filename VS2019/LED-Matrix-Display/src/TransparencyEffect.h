#pragma once

#include <vector>
#include "BaseEffect.h"

// Stack multiple effects using transparent color
class TransparencyEffect : public BaseEffect, public std::vector<BaseEffect *> {
public:
  bool renderFrame(int frame) override;
};
