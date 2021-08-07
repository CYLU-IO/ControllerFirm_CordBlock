#ifndef DEFINITION_H
#define DEFINITION_H

#define DEBUG                   1

///// PIN SETS /////
#define RST_PIN                 2
#define BUTTON_PIN              3
#define WIFI_STATE_PIN          7
#define MODULES_STATE_PIN       9

///// INTERVALS /////
#define BUTTON_LONG_CLICK_INTERVAL 5000

///// UART MESSAGE STRUC /////
#define CMD_FAIL                0x10
#define CMD_EOF                 0x20
#define CMD_START               0xFF

///// UART RECEIVE STATE /////
typedef enum UART_MSG_RC_STATE {
  RC_NONE,
  RC_HEADER,
  RC_PAYLOAD,
  RC_CHECK
};

#endif
