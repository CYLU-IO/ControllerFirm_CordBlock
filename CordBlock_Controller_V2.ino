#include <CRC.h>
#include <CRC8.h>
#include <CoreBridge.h>

#include "definitions.h"
#include "wiring_private.h"

Uart Serial3 (&sercom0, 5, 6, SERCOM_RX_PAD_1, UART_TX_PAD_0);

void setup() {
  pinPeripheral(5, PIO_SERCOM_ALT);
  pinPeripheral(6, PIO_SERCOM_ALT);

#if DEBUG
  Serial.begin(9600);
#endif

  Serial1.begin(9600);
  Serial3.begin(9600); //RX: 5, TX: 6

  CoreBridge.begin();

  //TEST ONLY- Remove
  while (!Serial);
  SerialNina.begin(115200);
  //
}

void loop() {
  static UART_MSG_RC_STATE state = RC_NONE;
  static int length;
  static char buffer[128];
  static int buffer_pos;

  if (uartReceive(Serial1, state, length, buffer, buffer_pos))
    CoreBridge.uartReceive(length, (uint8_t *)buffer);

  static unsigned long t = millis();

  static uint8_t port;
  static uint16_t tlength;
  static char *payload;
  if (millis() - t > 100) {
    if ((payload = CoreBridge.uartTransmit(port, tlength)) && payload) {
      Stream *serial = &Serial;

      switch (port) {
        case 1: {
            *serial = Serial1;
            break;
          }
          
        case 3: {
            *serial = Serial3;
            break;
          }
      }

      uartTransmit(*serial, tlength, payload);
      free(payload);
    }

    t = millis();
  }


  //if (SerialNina.available())
  //  Serial.write(SerialNina.read());
}

uint8_t calcCRC(char* str, int length) {
  static CRC8 crc;

  crc.reset();
  crc.setPolynome(0x05);
  crc.add((uint8_t*)str, length);

  return crc.getCRC();
}




#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}
