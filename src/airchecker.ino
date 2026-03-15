#pragma once
//======================================================================
/*
 * Project Name    : <AirChecker>
 * Creation Date   : <Apl.24,2024>
 *
 * Copyright © 2024 CLEALINK TECHNOLOGY Co., Ltd. All rights reserved.
 *
 * このファイルは Apache ライセンス 2.0 のもとでライセンスされています。
 * ライセンスの詳細については、LICENSE ファイルを参照してください。
 * https://www.apache.org/licenses/LICENSE-2.0
*/
//======================================================================

#include "airchecker.h"

int sender_or_receiver;                 //　送信側：１か、受信側：０か

CLoRa lora;
struct LoRaConfigItem_t config;         // E220-900T22S(JP)の設定値
struct RecvFrameE220900T22SJP_t data;   // ESP32受信データ

uint16_t send_target_address;           // 送信先アドレス
uint8_t  send_target_channel;           // 送信先チャンネル
uint16_t send_subpacket_size;           // 送信パケットサイズ


uint8_t LED_register;                   // LED点灯用
int rssi_no_sign;                       // RSSI値の正数値

uint16_t NoReceived_Interrupt_flag;     // 受信なしチェックタイマーフラグ
uint16_t SendInterval_Interrupt_flag ;  // 送信用送信インターバルタイマー


// LCD Display using multi-task
void LCDDispTask(void *pvParameters) {
  while (1) {
    DispLCD();
  }
}

// SetUp
void setup()
{
  sender_or_receiver = SENDER_or_RECEIVER;

  // UART0(Serial0) のグローバルコンストラクタが GPIO 20/21 を占有しているため
  // Serial1 で使う前に明示的に解放する
  Serial0.end();
  delay(50);

  Serial.begin(19200);
  delay(2000);
  Serial.println("\r\n=== AirChecker Debug Start ===");
  Serial.printf("AUX pin = %d\r\n", digitalRead(LoRa_AUXPin));
  
  Serial1.end();         // end()を実行　just in case
  delay( 100 );          // 100m秒待つ　just in case
  Serial1.begin(LoRa_BaudRate, SERIAL_8N1, LoRa_RxPin, LoRa_TxPin);
  Serial.printf("Serial1.begin done. AUX pin = %d\r\n", digitalRead(LoRa_AUXPin));

  Set_LCDLED_GPIO();
  Set_LoRa_GPIO();
  NoReceived_Interrupt_flag = SendInterval_Interrupt_flag = 0;
  data.rssi = 0;
  LED_register = 0x00;

// LCD表示用タスク起動
  xTaskCreateUniversal(LCDDispTask, "LCDDispTask", 8192, NULL, 1, NULL, CONFIG_ARDUINO_RUNNING_CORE);

  char ver_str[10] = {0};

  makeVersionMsg(VERSION_NO, ver_str);
  PrintF("\r\nStart AirChecker Ver.%s\r\n", ver_str);

  LCD_Check();                       // Check LCD 0,111,222,-999
  LED_All_Blink_4times();            // LED ALL Blink 4 times

  if (sender_or_receiver == 1) PrintF("--- RUN as Sender ---\r\n");
  else                         PrintF("--- RUN as Receiver ---\r\n");

  lora.SetDefaultConfigValue(config); // default value of e220_900t22S(JP)
  lora.InitLoRaModule(config);      // E220-900T22S(JP) initialize
  lora.Set_subpacket_size();        // set Sending subpacket size in CLoRa::SendFrame
 
  Set_SEGMENT(VERSION_NO);          // LCD Display Version No
  Set_Target_Ad_Ch();
#if STARTUP_NEGOTIATION
  negotiation();                    // Setup Sender or Receiver / ignore the value of SENDER_OR_RECEIVER
#else
  delay(1000);                       // for Recognizing Version No
#endif

  NoReceivedCheck_InterruptStart();     // Timer 0
  SendInterval_InterruptStart();        // Timer 1

  PrintF("\r\nsender_or_receiver = %d\r\n", sender_or_receiver);

  Set_SEGMENT(MACHINE_NO);        // LCD Display Machine No
  
  sender_or_receiver_LED_ON();
  delay(300);

  if ( sender_or_receiver ) SendInterval_Interrupt_flag = 1;   // Forced Send
}

// LOOP
void loop()
{

  Check_Timer();

  if (!sender_or_receiver) {
    /*
       Receiver
    */
    if ( digitalRead(LoRa_AUXPin) == 0) {
      NoReceivedCheck_InterruptStart();              // Timer 0

      if (LoRaRecvTask() == 0) {

        if ( checkAlignment() == true ) LED_OFF_YELLOW();
        else LED_ON_YELLOW();

        LCD_disp_RSSI();
        LED_OFF_RED();
        PrintF("\r\nReceive length .. = %d", data.recv_data_len);
        //send_subpacket_size = data.recv_data_len;    // same size of ReceivedData
        send_subpacket_size = 32;
        if ( RSSI_MEASUREMENT_ONLY == false ) {
          LoRaSendTask();
        }
      }
    }
  } else {
    /*
       Sender
    */
    if ( digitalRead(LoRa_AUXPin) == 0) goto RECEIVE;           // Exist Received Data ?

    if (SendInterval_Interrupt_flag) {              // SendInterval
      PrintF("\r\nTimerinterrupt for sending!!!!!\r\n");
      SendInterval_Interrupt_flag = 0;
      LoRaSendTask();

      LED_LORA_AUX_DISP();

      delay(300 );

RECEIVE:                                               // Forced Receive if AUX is LOW
      if (LoRaRecvTask() == 0) {
        NoReceivedCheck_InterruptStart();              // Timer 0
        delay(100 );

        LCD_disp_RSSI();
        LED_OFF_RED();
      }
    }
  }
  LED_LORA_AUX_DISP();
}


// check timer countup 
int Check_Timer() {
  int retcode = 0;
  if ( NoReceived_Interrupt_flag ) {                  // No Receive timer
    NoReceivedCheck_InterruptStart();                 // Timer 0
    PrintF("\r\nTimerinterrupt!!!!! No Data : RED LED ON\r\n");
    LED_ON_RED();
    retcode = 1;
  }
  if ( sender_or_receiver == 1 ) {
    if ( SendInterval_Interrupt_flag ) retcode += 1;
  }
  return retcode;
}



void Set_Target_Ad_Ch() {
  send_target_address = config.target_address;
  send_target_channel = config.target_channel;
}


void makeVersionMsg(int version_no, char *ver) {
  char charstr[10] = "";

  sprintf(charstr, "%3d", version_no);
  ver[0] = charstr[0];
  ver[1] = ver[3] = '.';
  ver[2] = charstr[1];
  ver[4] = charstr[2];
}


// For StartUp Negotiation

#if STARTUP_NEGOTIATION


void negotiation() {
  int tryCount = 0;
  int negocompleted = 0;
  int s_or_r = 0;
  int RedLED_counter = 0;
  int DownTimer;

  DownTimer = SetNegoItvlTimer();

  while (1) {
    if (  (digitalRead(LoRa_AUXPin) == 0 )|| (SerialLoRa.available()!=0) ) {
      if (LoRaRecvTask() == 0) {
        if ( checkAlignment() == true ) {
          uint8_t negocode = data.recv_data[4];

                PrintF("\r\nnegocode %02x\r\n",negocode);


          switch (negocode) {
            case 0x01 :
              sender_or_receiver = s_or_r = 0;
              delay(200);
              LoRaSendTask(2);
              negocompleted = 1;
              break;
            case 0x02 :
              sender_or_receiver = s_or_r = 1;
              negocompleted = 1;
              break;
          }
        }
      }
    }
    if (negocompleted) {
      LED_OFF_RED();
      return;                    //
    }
    if ( --DownTimer == 0 ) {
      PrintF("\r\nDownTimer Count Up!!!!!\r\n");
      SendInterval_Interrupt_flag = 0;
              DownTimer = SetNegoItvlTimer();

      LoRaSendTask(1);
      if ( ++tryCount > 5 ) {
        tryCount = 0;
      }
//      delay(300);
      RedLED_counter = 0;
    }
    delay(100);
    if (++RedLED_counter < 8 ) LED_ON_RED();
    else if ( RedLED_counter > 16 ) RedLED_counter = 0;
    else  LED_OFF_RED();
  }
}


int SetNegoItvlTimer() {
  long RandomNumber;//「RandomNumber」をlongで変数宣言
  long NegoItvlTimer;
  uint16_t seed = 5;

  seed = (uint16_t)millis() & 0x01f;
  PrintF("\r\nSeed = %d", seed);
  randomSeed(seed);
  RandomNumber = random(100);  //乱数の最大値を100に指定
  //  NegoItvlTimer = (long)((RandomNumber & 0x01f) + 30) * 100 + (millis()&0x1f*10);
  NegoItvlTimer = (long)(((uint16_t)RandomNumber & 0x1f) + 20) + (((uint16_t)millis() & 0x1e)/2 );
  PrintF("\r\nTimer = %d\r\n", NegoItvlTimer);

  
  return NegoItvlTimer;
}

#endif

bool checkAlignment() {
  if ( data.recv_data[0] != MAGIC_KEY ) return false;
  if ( data.recv_data[1] != (UNIQUE_ID >> 8)) return false;
  if ( data.recv_data[2] != (UNIQUE_ID & 0xff)) return false;
  return true;
}
