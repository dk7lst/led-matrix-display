#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "FrameBuffer.h"

FrameBuffer::FrameBuffer() {
  clear();
}

FrameBuffer::~FrameBuffer() {
}

void FrameBuffer::clear() {
  memset(m_ImageBuf, 0, IMAGE_BUFFER_SIZE);
}

bool FrameBuffer::packToBuffer(byte *pBuf, int size) const {
  assert(size >= FRAME_BUFFER_SIZE);
  if(size < FRAME_BUFFER_SIZE) return false;
  for(int i = 0; i < FRAME_BUFFER_SIZE; ++i) pBuf[i] = (m_ImageBuf[i * 2 + 1] << 4) | m_ImageBuf[i * 2];
  return true;
}

void FrameBuffer::copyFrom(const FrameBuffer *pOther) {
  memcpy(m_ImageBuf, pOther->m_ImageBuf, IMAGE_BUFFER_SIZE);
}

void FrameBuffer::mergeFrom(const FrameBuffer *pOther, byte transcolor) {
  for(int i = 0; i < IMAGE_BUFFER_SIZE; ++i)
    if(pOther->m_ImageBuf[i] != transcolor) m_ImageBuf[i] = pOther->m_ImageBuf[i];
}
