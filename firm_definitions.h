#ifndef FIRM_DEFINITION_H
#define FIRM_DEFINITION_H

#define MAX_MODULES             20
#define MAX_CURRENT             500
#define LONG_PRESS_TIME         10000
#define DEBUG                   1

#define RST_PIN                 2
#define BUTTON_PIN              3
#define WIFI_STATE_PIN 7
#define MODULES_CONNC_STATE_PIN 9

/*** SERIAL ***/
#define CMD_FAIL                0x11
#define CMD_EOF                 0x20
#define CMD_REQ_ADR             0x41 //'A'
#define CMD_LOAD_MODULE         0x42 //'B'
#define CMD_CONFIRM_RECEIVE     0x43 //'C'
#define CMD_DO_MODULE           0x44 //'D'
#define CMD_HI                  0x48 //'H'
#define CMD_INIT_MODULE         0x49 //'I'
#define CMD_LINK_MODULE         0x4C //'L'
#define CMD_UPDATE_MASTER       0x55 //'U'
#define CMD_START               0xFF

/*** Module Actions ***/
#define DO_TURN_ON              0x6E //'n'
#define DO_TURN_OFF             0x66 //'f'

/*** Characteristic Type ***/
#define MODULE_SWITCH_STATE     0x61 //'a' 
#define MODULE_CURRENT          0x62 //'b'

#endif
