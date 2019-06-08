#pragma once

// Class for an image using the complete display size.
class FrameBuffer {
public:
  static const int PIXEL_X = 80, PIXEL_Y = 7; // Display resolution
  static const int IMAGE_BUFFER_SIZE = PIXEL_X * PIXEL_Y; // Size of unpacked image

  static const int PIXEL_BIT = 4, PIXEL_PER_BYTE = 8 / PIXEL_BIT; // Jeder Pixel hat 4 Bit, so dass 2 Pixel in ein Byte passen.
  static const int FRAME_X = PIXEL_X / PIXEL_PER_BYTE, FRAME_Y = PIXEL_Y;
  static const int FRAME_BUFFER_SIZE = FRAME_X * FRAME_Y;

  static const int BLACK = 0, RED = 0x3, GREEN = 0xC, ORANGE = 0xF; // Colors

  static inline byte mkColor(byte red, byte green) {
    return (green << 2) | red;
  }

  FrameBuffer();
  virtual ~FrameBuffer();

  virtual void clear();
  virtual bool packToBuffer(BYTE *pBuf, int size) const;

  virtual inline void setPixel(int x, int y, byte color) {
    assert(x >= 0 && x < PIXEL_X && y >= 0 && y < PIXEL_Y && color >= 0 && color <= 0xF);
    m_ImageBuf[y * PIXEL_X + x] = color;
  }

protected:
  byte m_ImageBuf[IMAGE_BUFFER_SIZE];
};
