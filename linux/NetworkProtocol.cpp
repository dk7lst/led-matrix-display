#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include "FrameBuffer.h"
#include "NetworkProtocol.h"

NetworkProtocol::NetworkProtocol() {
  m_Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

NetworkProtocol::~NetworkProtocol() {
  close();
}

int NetworkProtocol::open(const char *szHost, int iPort) {
  if(m_Socket == INVALID_SOCKET) return -1;
  m_DstAddr.sin_family = AF_INET;
  m_DstAddr.sin_addr.s_addr = inet_addr(szHost);
  m_DstAddr.sin_port = htons(iPort);
  return 0;
}

void NetworkProtocol::close() {
  if(m_Socket == INVALID_SOCKET) return;
  ::close(m_Socket);
}

bool NetworkProtocol::sendFrame(const FrameBuffer *fb) {
  if(m_Socket == INVALID_SOCKET) return false;
  const int ciHeaderBytes = 4;
  byte packed[ciHeaderBytes + fb->FRAME_BUFFER_SIZE];
  memcpy(packed, "LED0", ciHeaderBytes); // Paket-Signatur
  fb->packToBuffer(packed + ciHeaderBytes, sizeof packed - ciHeaderBytes);
  int iResult = sendto(m_Socket, (const char *)packed, sizeof packed, 0, (const sockaddr *)&m_DstAddr, sizeof m_DstAddr);
  return iResult == sizeof packed;
}
