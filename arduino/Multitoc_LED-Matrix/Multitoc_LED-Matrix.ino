// Pin-Belegung: https://www.arduino.cc/en/Hacking/PinMapping168

// Digital Pin 00 (PORTD.0): Serial Rx
// Digital Pin 01 (PORTD.1): Serial Tx
// Digital Pin 02 (PORTD.2): DATA0: Daten-Leitung zu den Schieberegistern für ROT
// Digital Pin 03 (PORTD.3): Daten-Leitung zu den Schieberegistern für GRÜN
// Digital Pin 04 (PORTD.4): SCK: Clock-Signal zum Weiterschieben des Schiebe-Registers
// Digital Pin 05 (PORTD.5): RCX: Clock-Signal um Schiebe-Register in Output-Latches zu übernehmen
// Digital Pin 06 (PORTD.6): REMO: Low-Active-Signal der Fernbedienung. Beim Drücken einer FB-Taste erscheinen hier kurze Bursts.) Eingang! (Optional)
// Digital Pin 07 (PORTD.7): O0: Zeilen-Ausgang Zeile 1 (oberste) (über Widerstand!)
// Digital Pin 08 (PORTB.0): O1: Zeilen-Ausgang Zeile 2 (über Widerstand!)
// Digital Pin 09 (PORTB.1): O2: Zeilen-Ausgang Zeile 3 (über Widerstand!)
// Digital Pin 10 (PORTB.2): O3: Zeilen-Ausgang Zeile 4 (über Widerstand!)
// Digital Pin 11 (PORTB.3): O4: Zeilen-Ausgang Zeile 5 (über Widerstand!)
// Digital Pin 12 (PORTB.4): O5: Zeilen-Ausgang Zeile 6 (über Widerstand!)
// Digital Pin 13 (PORTB.5): O6: Zeilen-Ausgang Zeile 7 (unterste) (über Widerstand!)
// GND nicht vergessen!

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

const int BLACK = 0, RED = B0011, GREEN = B1100, ORANGE = B1111; // Farben

const int CPU_CLOCK_MHz = 8; // Sollten eigentlich 16 MHz sein, Werte passen mit 16 aber nicht.
const int PIXELS_PER_IRQ = 8; // Anzahl der Pixel, die pro Timer-Aufruf verarbeitet werden damit die ISR schneller zurückkehrt damit die serielle Schnittstelle nicht überläuft. Muss ein ganzzahliger Teiler von PIXEL_X sein!
const int TIMERVALUE = 65536 - CPU_CLOCK_MHz * 1000000 / (1260 * (PIXEL_X / PIXELS_PER_IRQ)); // 1260 Hz (3*7*60 Hz) damit wir 60x pro Sekunde 7 Zeilen mit je 3 PWM-Stufen ausgeben können.

#include "font.h"

enum TransferMode {
  TM_CMD, // auf Befehl warten
  TM_BMP, // auf unkomprimierte Bitmap-Daten warten
  TM_TEXT // auf Textdaten warten
};

void clear() {
  memset(FrameBuffer, 0, sizeof FrameBuffer);
}

void clear(int color) {
  for(int i = 0; i < sizeof color * PIXEL_PER_BYTE; ++i) color = (color << PIXEL_BIT) | color;
  memset(FrameBuffer, color, sizeof FrameBuffer);
}

void setPixel(int page, int x, int y, int color) {
  byte *p = FrameBuffer + page * PAGE_SIZE + y * FRAME_X + (x >> 1);
  const int b = (x & 1) << 2;
  *p = (*p & ~(0xF << b)) | ((color & 0xF) << b);
}

ISR(TIMER1_OVF_vect) {
#if PERFORMANCE_MEASURING
  PORTD |= 1 << 6;
#endif

  const int SCK = 1 << 4, RCX = 1 << 5, DATA_RED = 1 << 2, DATA_GREEN = 1 << 3;
  static int x = 0, y = 0, pwm = 0;

  // Aktuelle Zeile y mit PWM-Phase pwm aus Frame-Buffer in Schieberegister laden:
  const byte *pLine = FrameBuffer + DisplayPage * PAGE_SIZE + y * FRAME_X;
  int pixLeft = PIXELS_PER_IRQ;
  while(--pixLeft >= 0) {
    const byte pix = (pLine[x >> 1] >> ((x & 1) << 2)) & 0xF;
    const byte red = pix & 3, green = pix >> 2;
    PORTD = (PORTD & ~(DATA_RED | DATA_GREEN | RCX)) | (red > pwm ? DATA_RED : 0) | (green > pwm ? DATA_GREEN : 0) | SCK;
    PORTD &= ~SCK; // SCK off
    ++x;
  }

  if(x < PIXEL_X) {
#if PERFORMANCE_MEASURING
    PORTD &= ~(1 << 6);
#endif
    TCNT1 = TIMERVALUE;
    return; // Zeile ist noch nicht vollständig im Schieberegister - nächstes Mal gehts weiter!
  }

  x = 0;

  byte PD = PORTD, PB = PORTB;

  if(pwm == 0) {
    // Alte Treiber-Bits löschen:
    PD &= 0x7F;
    PB &= 0xC0;

    // Treiber für neue Zeile aktivieren und sie damit zur aktuellen Zeile machen.
    // y darf noch nicht inktrementiert werden, da die Zeile noch 2 weitere Male
    // "drübergemalt" wird um die Helligkeitsabstufungen per PWM zu realisieren.
    if(y == 0) PD |= 0x80;
    else PB |= 1 << (y - 1);
  }

  PORTD = PD | RCX; // Schieberegister in Ausgangs-Latches übernehmen und Zeilen-Treiber aktivieren
  PORTB = PB;

  if(++pwm >= 3) {
    // Aktuelle Zeile ist fertig, beim nächsten Aufruf mit der ersten PWM-Phase
    // der nächsten Zeile fortsetzen.
    if(++y >= 7) y = 0;
    pwm = 0;
  }

#if PERFORMANCE_MEASURING
  PORTD &= ~(1 << 6);
#endif
  TCNT1 = TIMERVALUE;
}

void setup() {
  DDRD = B10111100; // Configure PORTD 2...5 + 7 for output
  DDRB = B00111111; // Configure PORTB 0...5 for output

#if PERFORMANCE_MEASURING
  DDRD |= 1 << 6;
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
  Serial.println("READY");

  TransferMode tmode = TM_CMD;
  int Offset = -1;
  while(true) {
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
          const int SCROLL_STEP_X = FONT_X + 1; // Muss gerade sein, da Scrolling sonst Nibbles umsortieren müsste!
          const int SCROLL_BYTES = SCROLL_STEP_X / PIXEL_PER_BYTE;
          int i = 0;
          while(FontMap[i]) {
            if(FontMap[i] == b) { // Font-Daten für Zeichen gefunden?
              if(Offset >= PIXEL_X - SCROLL_STEP_X) { // Nicht genug Platz, Anzeige muss gescrollt werden
                memcpy(FrameBuffer + WorkPage * PAGE_SIZE, FrameBuffer + DisplayPage * PAGE_SIZE + SCROLL_BYTES, PAGE_SIZE - SCROLL_BYTES);
                for(int y = 1; y <= FONT_Y; ++y) memset(FrameBuffer + WorkPage * PAGE_SIZE + y * FRAME_X - SCROLL_BYTES, 0, SCROLL_BYTES);
                Offset -= SCROLL_STEP_X; // Cursor rücksetzen
              }
              else memcpy(FrameBuffer + WorkPage * PAGE_SIZE, FrameBuffer + DisplayPage * PAGE_SIZE, PAGE_SIZE); // genug Platz vorhanden, Page direkt kopieren
              for(int y = 0; y < FONT_Y; ++y)
                for(int x = 0; x < FONT_X; ++x)
                  setPixel(WorkPage, Offset + x, y, FontData[i * FONT_X + x] & (1 << y) ? RED : BLACK);
              DisplayPage = WorkPage; // Neue Page sichtbar machen
              break;
            }
            ++i;
          }
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
        for(int c = 0; c <= 15; ++c) {
          clear(c);
          delay(1000);
        }
        clear();
        Serial.println("OK");
      }
    }
  }
}
