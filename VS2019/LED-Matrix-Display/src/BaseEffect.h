#pragma once

#include "FrameBuffer.h"

// Base-Class for effects like animations or transitions.
class BaseEffect : public FrameBuffer {
public:
  BaseEffect() {
  }

  virtual ~BaseEffect() {
  }

  virtual bool init() {
    return true;
  }

  virtual bool start() {
    return true;
  }

  virtual bool renderFrame(int frame) {
    return true;
  }

  virtual bool stop() {
    return true;
  }
};
