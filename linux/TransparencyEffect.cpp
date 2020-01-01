#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "TransparencyEffect.h"

bool TransparencyEffect::renderFrame(int frame) {
  if(size() == 0) {
    FrameBuffer::clear();
    return true;
  }
  front()->renderFrame(frame);
  copyFrom(front());
  for(int i = 1; i < size(); ++i) {
    BaseEffect *pEffect = operator[](i);
    pEffect->renderFrame(frame);
    mergeFrom(pEffect);
  }
  return true;
}
