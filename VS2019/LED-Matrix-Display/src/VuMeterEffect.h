#pragma once

#include "BaseEffect.h"

// (For the moment random) VU-Meters
class VuMeterEffect : public BaseEffect {
public:
  bool renderFrame(int frame) override;

protected:
  int m_vuMax[2] = {0, 0};
  int m_vu[2] = {0, 0};
};
