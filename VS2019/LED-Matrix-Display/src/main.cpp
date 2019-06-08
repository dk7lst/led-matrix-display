#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <assert.h>
#include <windows.h>

const int PIXEL_X = 80, PIXEL_Y = 7; // Display-Auflösung
const int PIXEL_BIT = 4, PIXEL_PER_BYTE = 8 / PIXEL_BIT; // Jeder Pixel hat 4 Bit, so dass 2 Pixel in ein Byte passen.
const int FRAME_X = PIXEL_X / PIXEL_PER_BYTE, FRAME_Y = PIXEL_Y;
const int PAGES = 2, PAGE_SIZE = FRAME_X * FRAME_Y;

const int BLACK = 0, RED = 0x3, GREEN = 0xC, ORANGE = 0xF; // Farben
#define COLOR(r, g) ((g << 2) | r)

class Image {
public:
  Image();
  void clear();
  void setPixel(int x, int y, int color);
  void send(HANDLE hPort);
  int receiveByte(HANDLE hPort);
  void dump();

  void setCmd(int cmd) {
    m_Packet.m_Cmd = cmd;
  }

#pragma pack(1)
  struct Packet {
    byte m_Cmd;
    byte m_FrameBuffer[FRAME_X * FRAME_Y]; // Frame-Buffer mit Prefix-Byte
  } m_Packet;
#pragma pack()
};

Image::Image() {
  clear();
}

void Image::clear() {
  memset(&m_Packet, 0, sizeof m_Packet);
}

void Image::setPixel(int x, int y, int color) {
  assert(x >= 0 && x < PIXEL_X && y >= 0 && y < PIXEL_Y);
  byte *p = m_Packet.m_FrameBuffer + y * FRAME_X + (x >> 1);
  const int b = (x & 1) << 2;
  *p = (*p & ~(0xF << b)) | ((color & 0xF) << b);
}

void Image::send(HANDLE hPort) {
#if 1
  DWORD bytesWritten;
  BOOL bRes = WriteFile(hPort, &m_Packet, sizeof m_Packet, &bytesWritten, NULL);
#else
  const int MaxChunckSize = 8; // Arduino hat 64 Byte Rx-Puffer
  int bytesLeft = sizeof m_Packet;
  byte *pData = (byte *)&m_Packet;
  while (bytesLeft > 0) {
    DWORD bytesToWrite = min(bytesLeft, MaxChunckSize), bytesWritten = 0;
    BOOL bRes = WriteFile(hPort, pData, bytesToWrite, &bytesWritten, NULL);
    if(bytesWritten < bytesToWrite) printf("ERROR: Write failed: bytesToWrite=%d bytesWritten=%d\n", bytesToWrite, bytesWritten);
    pData += bytesWritten;
    bytesLeft -= bytesWritten;
    Sleep(10);
  }
#endif
}

int Image::receiveByte(HANDLE hPort) {
  byte buf;
  DWORD bytesRead;
  BOOL bRes = ReadFile(hPort, &buf, 1, &bytesRead, NULL);
  if(bytesRead != 1) return -1;
  return buf;
}

void Image::dump() {
  printf("FB:");
  for(int i = 0; i < sizeof m_Packet.m_FrameBuffer; ++i) printf(" %02X", m_Packet.m_FrameBuffer[i]);
  putchar('\n');
}

int main() {
  puts("LED-Matrix-Display 0.01");

  HANDLE hPort = CreateFile("COM4", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
  DCB dcb;
  if (!GetCommState(hPort, &dcb)) {
    puts("ERROR opening serial port!");
    return 1;
  }

  dcb.BaudRate = CBR_115200;
  dcb.ByteSize = 8;
  dcb.Parity = NOPARITY;
  dcb.StopBits = ONESTOPBIT;
  if (!SetCommState(hPort, &dcb)) {
    puts("ERROR configuring serial port parameters!");
    return 2;
  }

  puts("Waiting...");
  Sleep(5000); // Arduino rebootet beim Verbinden, daher warten bis der Selbsttest fertig ist.
  puts("Starting...");

  Image img;
  int phase = 0, vu[2] = {0, 0}, vuMax[2] = {0, 2};
  while(!_kbhit()) {
    img.clear();
    img.setCmd('+');
    if(phase % 15 == 0) {
      for(int i = 0; i < 2; ++i) vu[i] = vuMax[i] = max(vu[i], PIXEL_X / 2 + rand() % (PIXEL_X / 2));
    }
    for(int y = 0; y < PIXEL_Y; ++y) {
      for(int x = 0; x < PIXEL_X; ++x) {
#if 1
        // Schräge Linien im Hintergrund:
        if((x - y + phase) % 15 == 0) img.setPixel(x, y, COLOR(1, 0));
        if((x + y - phase) % 15 == 0) img.setPixel(x, y, COLOR(0, 1));

        // VU-Meter:
        if(x < vu[0] && y >= 1 && y <= 2) img.setPixel(x, y, COLOR(3 - x * 4 / PIXEL_X, x * 4 / PIXEL_X));
        if(x < vu[1] && y >= 4 && y <= 5) img.setPixel(x, y, COLOR(3 - x * 4 / PIXEL_X, x * 4 / PIXEL_X));
#else
        img.setPixel(x, y, (x + y + phase) % 0xF);
#endif
      }
      if(y >= 1 && y <= 2) img.setPixel(vuMax[0], y, RED);
      if(y >= 4 && y <= 5) img.setPixel(vuMax[1], y, RED);
    }
    for(int i = 0; i < 2; ++i) if(vu[i] > 0) --vu[i];
    img.send(hPort);
    ++phase;
  }

#if 0
  img.setPixel(0, 0, GREEN);
  img.setPixel(1, 0, ORANGE);
  img.setPixel(2, 0, RED);
  img.setPixel(79, 0, RED);
  img.setPixel(79, 6, RED);

  img.send(hPort);
  printf("Received: %d\n", img.receiveByte(hPort));
#endif
  //img.dump();

  CloseHandle(hPort);
	return 0;
}
