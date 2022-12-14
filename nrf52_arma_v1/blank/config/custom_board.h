#ifndef CUSTOM_BOARD_H__
#define CUSTOM_BOARD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nrf_gpio.h"

#define LEDS_NUMBER    3

#define LED_R          NRF_GPIO_PIN_MAP(1,9)
#define LED_G          NRF_GPIO_PIN_MAP(0,12)//回路図と逆
#define LED_B          NRF_GPIO_PIN_MAP(0,11)//回路図と逆

#define LED_START      LED_1
#define LED_STOP       LED_4

#define LEDS_ACTIVE_STATE 0

#define LEDS_LIST { LED_R, LED_G, LED_B }

#define LEDS_INV_MASK  LEDS_MASK

#define BSP_LED_0      LED_R
#define BSP_LED_1      LED_G
#define BSP_LED_2      LED_B

#define BUTTONS_NUMBER 6

#define BUTTON_R       NRF_GPIO_PIN_MAP(0,0)   //0.00
#define BUTTON_L       NRF_GPIO_PIN_MAP(0,1)   //0.01
#define BUTTON_M       NRF_GPIO_PIN_MAP(0,3)   //0.03
#define BUTTON_FOW     NRF_GPIO_PIN_MAP(0,2)   //0.02
#define BUTTON_PREV    NRF_GPIO_PIN_MAP(0,28)  //0.28
#define BUTTON_DPI     NRF_GPIO_PIN_MAP(0,29)  //0.29
#define BUTTON_LED     NRF_GPIO_PIN_MAP(0,30)  //0.30
#define BUTTON_AIM     NRF_GPIO_PIN_MAP(0,31)  //0.31
#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP

#define BUTTONS_ACTIVE_STATE 0

#define BUTTONS_LIST { BUTTON_R, BUTTON_L, BUTTON_M, BUTTON_FOW, BUTTON_PREV, BUTTON_AIM }

#define BSP_BUTTON_0   BUTTON_R
#define BSP_BUTTON_1   BUTTON_L
#define BSP_BUTTON_2   BUTTON_M
#define BSP_BUTTON_3   BUTTON_FOW
#define BSP_BUTTON_4   BUTTON_PREV
#define BSP_BUTTON_5   BUTTON_AIM

// #define RX_PIN_NUMBER  8
// #define TX_PIN_NUMBER  6
// #define CTS_PIN_NUMBER 7
// #define RTS_PIN_NUMBER 5
// #define HWFC           true

// #define BSP_QSPI_SCK_PIN   19
// #define BSP_QSPI_CSN_PIN   17
// #define BSP_QSPI_IO0_PIN   20
// #define BSP_QSPI_IO1_PIN   21
// #define BSP_QSPI_IO2_PIN   22
// #define BSP_QSPI_IO3_PIN   23


// serialization APPLICATION board - temp. setup for running serialized MEMU tests
#define SER_APP_RX_PIN              NRF_GPIO_PIN_MAP(1,12)    // UART RX pin number.
#define SER_APP_TX_PIN              NRF_GPIO_PIN_MAP(1,13)    // UART TX pin number.
// #define SER_APP_CTS_PIN             NRF_GPIO_PIN_MAP(0,2)     // UART Clear To Send pin number.
// #define SER_APP_RTS_PIN             NRF_GPIO_PIN_MAP(1,15)    // UART Request To Send pin number.

// #define SER_CON_SPIS_SCK_PIN        NRF_GPIO_PIN_MAP(0,07)    // SPI SCK signal.
// #define SER_CON_SPIS_MOSI_PIN       NRF_GPIO_PIN_MAP(0,05)    // SPI MOSI signal.
// #define SER_CON_SPIS_MISO_PIN       NRF_GPIO_PIN_MAP(0,06)    // SPI MISO signal.
// #define SER_CON_SPIS_CSN_PIN        NRF_GPIO_PIN_MAP(0,26)    // SPI CSN signal.
// #define SER_CON_SPIS_RDY_PIN        NRF_GPIO_PIN_MAP(1,15)    // SPI READY GPIO pin number.
// #define SER_CON_SPIS_REQ_PIN        NRF_GPIO_PIN_MAP(1,14)    // SPI REQUEST GPIO pin number.

// // serialization CONNECTIVITY board
// #define SER_CON_RX_PIN              NRF_GPIO_PIN_MAP(1,14)    // UART RX pin number.
// #define SER_CON_TX_PIN              NRF_GPIO_PIN_MAP(1,13)    // UART TX pin number.
// #define SER_CON_CTS_PIN             NRF_GPIO_PIN_MAP(1,15)    // UART Clear To Send pin number. Not used if HWFC is set to false.
// #define SER_CON_RTS_PIN             NRF_GPIO_PIN_MAP(0,2)     // UART Request To Send pin number. Not used if HWFC is set to false.


// #define SER_CON_SPIS_SCK_PIN        NRF_GPIO_PIN_MAP(0,27)    // SPI SCK signal.
// #define SER_CON_SPIS_MOSI_PIN       NRF_GPIO_PIN_MAP(0,2)     // SPI MOSI signal.
// #define SER_CON_SPIS_MISO_PIN       NRF_GPIO_PIN_MAP(0,26)    // SPI MISO signal.
// #define SER_CON_SPIS_CSN_PIN        NRF_GPIO_PIN_MAP(1,13)    // SPI CSN signal.
// #define SER_CON_SPIS_RDY_PIN        NRF_GPIO_PIN_MAP(1,15)    // SPI READY GPIO pin number.
// #define SER_CON_SPIS_REQ_PIN        NRF_GPIO_PIN_MAP(1,14)    // SPI REQUEST GPIO pin number.

// #define SER_CONN_CHIP_RESET_PIN     NRF_GPIO_PIN_MAP(1,1)    // Pin used to reset connectivity chip

// // Arduino board mappings
// #define ARDUINO_SCL_PIN             27    // SCL signal pin
// #define ARDUINO_SDA_PIN             26    // SDA signal pin
// #define ARDUINO_AREF_PIN            2     // Aref pin

// #define ARDUINO_13_PIN              NRF_GPIO_PIN_MAP(1, 15)  // Digital pin 13
// #define ARDUINO_12_PIN              NRF_GPIO_PIN_MAP(1, 14)  // Digital pin 12
// #define ARDUINO_11_PIN              NRF_GPIO_PIN_MAP(1, 13)  // Digital pin 11
// #define ARDUINO_10_PIN              NRF_GPIO_PIN_MAP(1, 12)  // Digital pin 10
// #define ARDUINO_9_PIN               NRF_GPIO_PIN_MAP(1, 11)  // Digital pin 9
// #define ARDUINO_8_PIN               NRF_GPIO_PIN_MAP(1, 10)  // Digital pin 8

// #define ARDUINO_7_PIN               NRF_GPIO_PIN_MAP(1, 8) // Digital pin 7
// #define ARDUINO_6_PIN               NRF_GPIO_PIN_MAP(1, 7) // Digital pin 6
// #define ARDUINO_5_PIN               NRF_GPIO_PIN_MAP(1, 6) // Digital pin 5
// #define ARDUINO_4_PIN               NRF_GPIO_PIN_MAP(1, 5) // Digital pin 4
// #define ARDUINO_3_PIN               NRF_GPIO_PIN_MAP(1, 4) // Digital pin 3
// #define ARDUINO_2_PIN               NRF_GPIO_PIN_MAP(1, 3) // Digital pin 2
// #define ARDUINO_1_PIN               NRF_GPIO_PIN_MAP(1, 2) // Digital pin 1
// #define ARDUINO_0_PIN               NRF_GPIO_PIN_MAP(1, 1) // Digital pin 0

// #define ARDUINO_A0_PIN              3     // Analog channel 0
// #define ARDUINO_A1_PIN              4     // Analog channel 1
// #define ARDUINO_A2_PIN              28    // Analog channel 2
// #define ARDUINO_A3_PIN              29    // Analog channel 3
// #define ARDUINO_A4_PIN              30    // Analog channel 4
// #define ARDUINO_A5_PIN              31    // Analog channel 5


#define LED_POWER_OFF   NRF_GPIO_PIN_MAP(1,15)  //0.08

#define ENCODER_A       NRF_GPIO_PIN_MAP(0,8)  //0.08
#define ENCODER_B       NRF_GPIO_PIN_MAP(1,8)  //1.08
#define SNS_MOTION      NRF_GPIO_PIN_MAP(0,27)  //0.27

#ifdef __cplusplus
}
#endif

#endif