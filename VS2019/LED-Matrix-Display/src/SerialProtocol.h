#pragma once

class FrameBuffer;

// Send frame via serial line to arduino.
class SerialProtocol {
public:
  SerialProtocol();
  virtual ~SerialProtocol();

  int open(const char *szSerialPort);
  void close();
  
  bool sendFrame(const FrameBuffer *fb);
  int receiveByte(HANDLE hPort);

protected:
  HANDLE m_hPort = nullptr;
};
