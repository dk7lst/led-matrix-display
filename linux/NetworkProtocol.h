#pragma once

//#include <sys/types.h>
//#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

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
  const int INVALID_SOCKET = -1;
  int m_Socket = INVALID_SOCKET;
  sockaddr_in m_DstAddr;
};
