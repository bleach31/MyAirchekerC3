#ifndef __airchecker_H__
#define __airchecker_H__

#include <Arduino.h>
#include <vector>
#include "esp32_e220900t22s_jp_lib_3.h"


#define VERSION_NO 100          // means 1.0.0
#define MAGIC_KEY 0x54          // 本 Aird伝送確認のためのマジックキー.
#define UNIQUE_ID 123          // 本 Aird伝送確認のための固有ID.
#define MACHINE_NO 1            // Machine No

#define STARTUP_NEGOTIATION 1

#define SENDER_or_RECEIVER 0    // 1 : Sender / 0 : Receiver

#define LCD_UpsideDown 0        // 液晶表示を上下逆にするか否かの設定。PCなどのUSBポートの差し込み位置に対応。

#define USE_SerialPrint 0       // Use or not Serial.print() to display Monitor or TerminalAp on PC

#define RSSI_MEASUREMENT_ONLY false // Air受信時のRSSI表示のみを行う場合　true




// E220-900T22S(JP)へのピンアサイン
#define LoRa_ModeSettingPin_M0 2
#define LoRa_ModeSettingPin_M1 8
#define LoRa_RxPin 20
#define LoRa_TxPin 21
#define LoRa_AUXPin 10

// 7SEG LCD : for making wave
#define COM1_PIN 4
#define COM2_PIN 5
#define COM3_PIN 6
#define COM4_PIN 7

// 7SEG LCD & 6 LEDs  using 2pieces of 74HC595
#define SRCLK_PIN 0
#define RCLK_PIN 1
#define SER_PIN 3

// 6LEDs
#define LED_LoRa_ITVL 0b00000010    // 00000001
#define LED_LoRa_AUX  0b00000001    // 00000011
#define LED_RED       0b00000100
#define LED_YELLOW    0b00001000
#define LED_GREEN     0b00010000
#define LED_BLUE      0b00100000


#if USE_SerialPrint
// MACROS

#define Print \
  Serial.print
#define PrintF \
  Serial.printf
#define PrintLN \
  Serial.println
  
#else

#define Print \
  // No Operation
#define PrintF \
  // No Operation
#define PrintLN \
  // No Operation

#endif




extern uint16_t NoReceived_Interrupt_flag;
extern uint16_t SendInterval_Interrupt_flag;
extern int  RSSI_present;      // present  value of ambient RSSI
extern int  RSSI_previous;     // previous value of RSSI

#endif
