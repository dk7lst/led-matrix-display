#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <windows.h>
#include "FrameBuffer.h"
#include "SerialProtocol.h"

SerialProtocol::SerialProtocol() {
}

SerialProtocol::~SerialProtocol() {
  close();
}

int SerialProtocol::open(const char *szSerialPort) {
  m_hPort = CreateFile(szSerialPort, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
  if(!m_hPort) return -1;

  DCB dcb;
  if(!GetCommState(m_hPort, &dcb)) return -2;

  dcb.BaudRate = CBR_115200;
  dcb.ByteSize = 8;
  dcb.Parity = NOPARITY;
  dcb.StopBits = ONESTOPBIT;
  if(!SetCommState(m_hPort, &dcb)) return -3;

  return 0;
}

void SerialProtocol::close() {
  if(!m_hPort) return;
  CloseHandle(m_hPort);
  m_hPort = nullptr;
}

bool SerialProtocol::sendFrame(const FrameBuffer *fb) {
  if(!m_hPort) return false;
  DWORD bytesWritten;
  byte packed[1 + fb->FRAME_BUFFER_SIZE] = "+";
  fb->packToBuffer(packed + 1, sizeof packed - 1);
  BOOL bRes = WriteFile(m_hPort, packed, sizeof packed, &bytesWritten, NULL);
  return bRes == TRUE && bytesWritten == sizeof packed;
}

int SerialProtocol::receiveByte(HANDLE hPort) {
  if(!m_hPort) return -1;
  byte buf;
  DWORD bytesRead;
  BOOL bRes = ReadFile(hPort, &buf, 1, &bytesRead, NULL);
  if(!bRes || bytesRead != 1) return -1;
  return buf;
}
