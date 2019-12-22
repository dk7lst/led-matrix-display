#pragma once

#include <windows.h>

class FrameBuffer;

// Send frame via UDP/IP to arduino.
class NetworkProtocol {
public:
  NetworkProtocol();
  virtual ~NetworkProtocol();

  int open(const char *szHost, int iPort);
  void close();

  bool sendFrame(const FrameBuffer *fb);

protected:
  SOCKET m_Socket = INVALID_SOCKET;
  sockaddr_in m_DstAddr;
};
