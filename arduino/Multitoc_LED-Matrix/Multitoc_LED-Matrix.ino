#include <SPI.h>
#include <Ethernet.h>

#define VERSIONSTR "0.90"

// Pin-Belegung: https://www.arduino.cc/en/Hacking/PinMapping168

// Digital Pin 00 (PORTD.0): Serial Rx (serieller Port zum USB-Host)
// Digital Pin 01 (PORTD.1): Serial Tx (serieller Port zum USB-Host)
// Digital Pin 02 (PORTD.2): DATA0: Daten-Leitung zu den Schieberegistern für ROT + ROW_DATA: Daten-Leitung zu Zeilen-Treiber-Schieberegister
// Digital Pin 03 (PORTD.3): DATA1: Daten-Leitung zu den Schieberegistern für GRÜN
// Digital Pin 04 (PORTD.4): SPI-Bus SD_CS (SD-Card)
// Digital Pin 05 (PORTD.5): SCK: Clock-Signal zum Weiterschieben des Schiebe-Registers
// Digital Pin 06 (PORTD.6): RCX: Clock-Signal um Schiebe-Register in Output-Latches zu übernehmen
// Digital Pin 07 (PORTD.7): ROW_CLK: CLK für Zeilen-Treiber-Schieberegister (74HC164)
// Digital Pin 08 (PORTB.0): REMO: Low-Active-Signal der Fernbedienung. Beim Drücken einer FB-Taste erscheinen hier kurze Bursts.) Eingang! (Optional) Alternativ Ausgang für Performance-Messung.
// Digital Pin 09 (PORTB.1): not connected
// Digital Pin 10 (PORTB.2): SPI-Bus SS (Ethernet)
// Digital Pin 11 (PORTB.3): SPI-Bus MOSI
// Digital Pin 12 (PORTB.4): SPI-Bus MISO
// Digital Pin 13 (PORTB.5): SPI-Bus SCK
// GND nicht vergessen!
const byte COL_SCK = 1 << 5, COL_RCX = 1 << 6, COL_DATA_RED = 1 << 2, COL_DATA_GREEN = 1 << 3, ROW_CLK = 1 << 7, ROW_DATA = COL_DATA_RED; // Bitmasken für PORTD
const byte REMOTE = 1 << 0; // Bitmasken für PORTB

byte mac[] = {0xA8, 0x61, 0x0A, 0xAE, 0x14, 0x2B};
const unsigned int localPort = 8000;
volatile bool spisync = false; // SPI-Zugriffe auf Ethernet mit Timer synchronisieren um Darstellungsfehler zu minimieren

// Für Performance-Messung Digital Pin 06 (PORTD.6) setzen solange CPU in Timer-ISR ist.
// Achtung: Nicht benutzen wenn Fernbedienungs-Rx angeschlossen ist!
#define PERFORMANCE_MEASURING 0

const int PIXEL_X = 80, PIXEL_Y = 7; // Display-Auflösung
const int PIXEL_BIT = 4, PIXEL_PER_BYTE = 8 / PIXEL_BIT; // Jeder Pixel hat 4 Bit, so dass 2 Pixel in ein Byte passen.
const int FRAME_X = PIXEL_X / PIXEL_PER_BYTE, FRAME_Y = PIXEL_Y;
const int PAGES = 2, PAGE_SIZE = FRAME_X * FRAME_Y;
byte FrameBuffer[PAGES * PAGE_SIZE]; // Frame-Buffer
byte DisplayPage = 0; // Aktuell sichtbare Bildschirmseite
#define WorkPage (1 - DisplayPage) // Unsichtbare Arbeits-Bildschirmseite

const int HEADER_SIZE = 4; // Header im UDP-Paket mit Magic-Number
const int PACKET_SIZE = HEADER_SIZE + PAGE_SIZE; // Paket-Größe (UDP-Payload)

const byte BLACK = 0, RED = B0011, GREEN = B1100, ORANGE = B1111; // Farben

const int CPU_CLOCK_MHz = 8; // Sollten eigentlich 16 MHz sein, Werte passen mit 16 aber nicht.
const int PIXELS_PER_IRQ = 8; // Anzahl der Pixel, die pro Timer-Aufruf verarbeitet werden damit die ISR schneller zurückkehrt damit die serielle Schnittstelle nicht überläuft. Muss ein ganzzahliger Teiler von PIXEL_X sein!
const int TIMERVALUE = 65536 - CPU_CLOCK_MHz * 1000000 / (1260 * (PIXEL_X / PIXELS_PER_IRQ)); // 1260 Hz (3*7*60 Hz) damit wir 60x pro Sekunde 7 Zeilen mit je 3 PWM-Stufen ausgeben können.

#include "font.h"

enum TransferMode {
  TM_CMD, // auf Befehl warten
  TM_BMP, // auf unkomprimierte Bitmap-Daten warten
  TM_TEXT // auf Textdaten warten
};

EthernetUDP udp;

void clear() {
  memset(FrameBuffer, 0, sizeof FrameBuffer);
}

void clear(byte color) {
  for(byte i = 0; i < sizeof color * PIXEL_PER_BYTE; ++i) color = (color << PIXEL_BIT) | color;
  memset(FrameBuffer, color, sizeof FrameBuffer);
}

void setPixel(byte page, byte x, byte y, byte color) {
  byte *p = FrameBuffer + page * PAGE_SIZE + y * FRAME_X + (x >> 1);
  const byte b = (x & 1) << 2;
  *p = (*p & ~(0xF << b)) | ((color & 0xF) << b);
}

void renderChar(byte Offset, char ch, byte textColor = RED, byte bgColor = BLACK) {
  byte i = 0;
  while(FontMap[i]) {
    if(FontMap[i] == ch) { // Font-Daten für Zeichen gefunden?
      for(byte y = 0; y < FONT_Y; ++y)
        for(byte x = 0; x < FONT_X; ++x)
          setPixel(WorkPage, Offset + x, y, FontData[i * FONT_X + x] & (1 << y) ? textColor : bgColor);
      break;
    }
    ++i;
  }
}

void displayMessage(const char *szStr, byte textColor = RED, byte bgColor = BLACK) {
  clear(bgColor);
  byte x = 0;
  while(szStr && *szStr) {
    renderChar(x, *szStr++, textColor, bgColor);
    x += FONT_X + 1;
  }
  DisplayPage = WorkPage;
}

void message(const char *szStr, byte textColor = RED, byte bgColor = BLACK) {
  displayMessage(szStr, textColor, bgColor);
  Serial.println(szStr);
}

void requestIP() {
  while(true) {
    message("v" VERSIONSTR " DHCP...", GREEN);
    if(Ethernet.begin(mac)) break;
    message("DHCP NAK", RED);
    delay(5000);
  }

  char buf[16];
  sprintf(buf, "OK IP: .%d", Ethernet.localIP()[3]);
  displayMessage(buf, GREEN);

  Serial.print("DHCP ACK. IP=");
  for (byte b = 0; b < 4; ++b) {
    Serial.print(Ethernet.localIP()[b], DEC);
    Serial.print(".");
  }
  Serial.print(" UDP-port: ");
  Serial.println(localPort, DEC);
  delay(1000);
}

ISR(TIMER1_OVF_vect) {
#if PERFORMANCE_MEASURING
  PORTB |= REMOTE;
#endif

  static char x = 0, y = 0, pwm = 0; // Aktuelle Spalte, Zeile und PWM-Phase

  // Aktuelle Zeile y mit PWM-Phase pwm aus Frame-Buffer in Schieberegister laden:
  const byte *pLine = FrameBuffer + DisplayPage * PAGE_SIZE + y * FRAME_X;
  char pixLeft = PIXELS_PER_IRQ;
  while(--pixLeft >= 0) {
    const byte pix = (pLine[x >> 1] >> ((x & 1) << 2)) & 0xF;
    const byte red = pix & 3, green = pix >> 2;
    PORTD = (PORTD & ~(COL_DATA_RED | COL_DATA_GREEN | COL_RCX)) | (red > pwm ? COL_DATA_RED : 0) | (green > pwm ? COL_DATA_GREEN : 0) | COL_SCK;
    PORTD &= ~COL_SCK; // SCK off
    ++x;
  }

  if(x < PIXEL_X) {
#if PERFORMANCE_MEASURING
    PORTB &= ~REMOTE;
#endif
    TCNT1 = TIMERVALUE;
    return; // Zeile ist noch nicht vollständig im Schieberegister - nächstes Mal gehts weiter!
  }

  // Zeile ist vollständig übertragen. Sichtbar machen und mit erster Spalte der nächsten Zeile weitermachen:
  x = 0;

  byte PD = PORTD & ~(ROW_CLK | ROW_DATA);
  if(pwm == 0) {
    if(!y) PD |= ROW_DATA; // Bei Zeile 0 eine neue 1 ins Zeilen-Schieberegister schreiben
    PORTD = PD; // Data muss schon kurz vor CLK anliegen, sonst wird das Timing instabil.
    PD |= ROW_CLK; // 1 übernehmen/weiterschieben so dass der nächste Zeilen-Treiber aktiv wird.
  }
  PORTD = PD | COL_RCX; // Schieberegister in Ausgangs-Latches übernehmen

  if(++pwm >= 3) {
    // Aktuelle Zeile ist fertig, beim nächsten Aufruf mit der ersten PWM-Phase
    // der nächsten Zeile fortsetzen.
    if(++y >= 7) y = 0;
    pwm = 0;
    spisync = true;
  }

#if PERFORMANCE_MEASURING
  PORTB &= ~REMOTE;
#endif
  TCNT1 = TIMERVALUE;
}

void setup() {
  DDRD |= COL_SCK | COL_RCX | COL_DATA_RED | COL_DATA_GREEN | ROW_CLK | ROW_DATA; // Configure PORTD outputs
#if PERFORMANCE_MEASURING
  DDRB |= REMOTE;
#endif

  PORTD = 0; // Display aus
  PORTB = 0;

  //Timer:
  //- https://www.mikrocontroller.net/articles/AVR-Tutorial:_Timer
  //- https://www.heise.de/developer/artikel/Timer-Counter-und-Interrupts-3273309.html
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = TIMERVALUE;
  TCCR1B |= (1 << CS10); // Vorteiler 1
  TIMSK1 |= (1 << TOIE1); // Timer Overflow Interrupt aktivieren
  interrupts();

  Serial.begin(115200);
}

void loop() {
  Serial.println("SELFTEST");
  clear(RED);
  delay(1000);
  clear(ORANGE);
  delay(1000);
  clear(GREEN);
  delay(1000);
  clear();

  requestIP();
  udp.begin(localPort);

  Serial.println("READY");
  clear();

  TransferMode tmode = TM_CMD;
  char Offset = -1;
  while(true) {
    if(spisync) {
      const int packetSize = udp.parsePacket();
      if(packetSize == PACKET_SIZE) {
        byte *const pBuf = FrameBuffer + WorkPage * PAGE_SIZE;
        *pBuf = 0; // Evtl. noch vorhandene alte Magic-Number löschen
        udp.read(pBuf, HEADER_SIZE);
        if(!memcmp(pBuf, "LED0", HEADER_SIZE)) {
          udp.read(pBuf, PAGE_SIZE);
          DisplayPage = WorkPage; // Neue Page sichtbar machen
        }
      }
      else Ethernet.maintain();
      spisync = false;
    }
    int b = Serial.read();
    if(b >= 0) {
      if(tmode == TM_BMP) {
        FrameBuffer[WorkPage * PAGE_SIZE + Offset++] = b; // Byte in Frame einfügen
        if(Offset >= PAGE_SIZE) { // Frame vollständig empfangen
          DisplayPage = WorkPage; // Neue Page sichtbar machen
          tmode = TM_CMD;
          Serial.print("#"); // ACK senden
        }
      }
      else if(tmode == TM_TEXT) {
        if(b == 10 || b == 13) tmode = TM_CMD; // Mit CR/LF zurück in den CMD-Modus
        else {
          const byte SCROLL_STEP_X = FONT_X + 1; // Muss gerade sein, da Scrolling sonst Nibbles umsortieren müsste!
          const byte SCROLL_BYTES = SCROLL_STEP_X / PIXEL_PER_BYTE;
          if(Offset >= PIXEL_X - SCROLL_STEP_X) { // Nicht genug Platz, Anzeige muss gescrollt werden
            memcpy(FrameBuffer + WorkPage * PAGE_SIZE, FrameBuffer + DisplayPage * PAGE_SIZE + SCROLL_BYTES, PAGE_SIZE - SCROLL_BYTES);
            for(byte y = 1; y <= FONT_Y; ++y) memset(FrameBuffer + WorkPage * PAGE_SIZE + y * FRAME_X - SCROLL_BYTES, 0, SCROLL_BYTES);
            Offset -= SCROLL_STEP_X; // Cursor rücksetzen
          }
          else memcpy(FrameBuffer + WorkPage * PAGE_SIZE, FrameBuffer + DisplayPage * PAGE_SIZE, PAGE_SIZE); // genug Platz vorhanden, Page direkt kopieren
          renderChar(Offset, b);
          DisplayPage = WorkPage; // Neue Page sichtbar machen
          Offset += SCROLL_STEP_X; // Cursor weitersetzen
        }
      }
      else if(b == '+') { // Neuer Bitmap-Frame startet
        tmode = TM_BMP;
        Offset = 0; // Offset im Frame-Buffer
      }
      else if(b == 't') { // Text-Modus
        tmode = TM_TEXT;
        Offset = 1; // Cursor-Position, bei 1 Anfangen damit Text etwas zentrierter ist.
      }
      else if(b == 'c') {
        clear(); // frame clear
        Serial.println("OK");
      }
      else if(b == 'r') {
        clear(RED); // display test
        Serial.println("OK");
      }
      else if(b == 'g') {
        clear(GREEN); // display test
        Serial.println("OK");
      }
      else if(b == 'o') {
        clear(ORANGE); // display test
        Serial.println("OK");
      }
      else if(b == 'p') { // PWM-Test
        for(byte c = 0; c <= 15; ++c) {
          clear(c);
          delay(1000);
        }
        clear();
        Serial.println("OK");
      }
    }
  }
}
