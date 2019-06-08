#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <windows.h>
#include "FrameBuffer.h"

FrameBuffer::FrameBuffer() {
  clear();
}

FrameBuffer::~FrameBuffer() {
}

void FrameBuffer::clear() {
  memset(m_ImageBuf, 0, IMAGE_BUFFER_SIZE);
}

bool FrameBuffer::packToBuffer(BYTE *pBuf, int size) const {
  assert(size >= FRAME_BUFFER_SIZE);
  if(size < FRAME_BUFFER_SIZE) return false;
  for(int i = 0; i < FRAME_BUFFER_SIZE; ++i) pBuf[i] = (m_ImageBuf[i * 2 + 1] << 4) | m_ImageBuf[i * 2];
  return true;
}
