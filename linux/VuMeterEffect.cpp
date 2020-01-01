#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include <assert.h>
#include "VuMeterEffect.h"

bool VuMeterEffect::renderFrame(int frame) {
  clear();
  if(frame % 15 == 0) {
    for(int i = 0; i < 2; ++i) m_vu[i] = m_vuMax[i] = std::max(m_vu[i], PIXEL_X / 2 + rand() % (PIXEL_X / 2));
  }

  for(int y = 1; y <= 2; ++y) {
    for(int x = 0; x < m_vu[0]; ++x) setPixel(x, y, mkColor(3 - x * 4 / PIXEL_X, x * 4 / PIXEL_X));
    setPixel(m_vuMax[0], y, RED);
  }

  for(int y = 4; y <= 5; ++y) {
    for(int x = 0; x < m_vu[1]; ++x) setPixel(x, y, mkColor(3 - x * 4 / PIXEL_X, x * 4 / PIXEL_X));
    setPixel(m_vuMax[1], y, RED);
  }

  for(int i = 0; i < 2; ++i) if(m_vu[i] > 0) --m_vu[i];
  return true;
}
