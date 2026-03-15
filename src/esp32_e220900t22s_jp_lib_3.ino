//======================================================================
/*
 * Module Name     : e220_900t22s_jp_3 lib <in AirChecker>
 * Creation Date   : <Apl.24,2024> * Creation Date   : <Apl.24,2024>
 *
 * Copyright © 2024 CLEALINK TECHNOLOGY Co., Ltd. All rights reserved.
 *
 * このファイルは Apache ライセンス 2.0 のもとでライセンスされています。
 * ライセンスの詳細については、LICENSE ファイルを参照してください。
 * https://www.apache.org/licenses/LICENSE-2.0
*/
//======================================================================


#include "airchecker.h"
#include "CONFIG.h"

template <typename T> bool ConfRange(T target, T min, T max);

void CLoRa::Set_subpacket_size() {
  switch ( config.subpacket_size & 0b00011 ) {
    case 0b11: send_subpacket_size = 32; break;
    case 0b10: send_subpacket_size = 64; break;
    case 0b01: send_subpacket_size = 128; break;
    case 0b00: send_subpacket_size = 200; break;
  }
}

int CLoRa::InitLoRaModule(struct LoRaConfigItem_t &config) {
  int ret = 0;
  //  while(( digitalRead(LoRa_AUXPin) &0x1) == 0){ delay(1);}

  Set_UART_BaudRate();

  // コンフィグモード(M0=1,M1=1)へ移行する
  SwitchToConfigurationMode();

  SerialLoRa.begin(9600, SERIAL_8N1, LoRa_RxPin, LoRa_TxPin);

  // Configuration
  std::vector<uint8_t> command = {0xc0, 0x00, 0x08};
  std::vector<uint8_t> response = {};

  // Register Address 00H, 01H
  uint8_t ADDH = config.own_address >> 8;
  uint8_t ADDL = config.own_address & 0xff;
  command.push_back(ADDH);
  command.push_back(ADDL);

  // Register Address 02H
  uint8_t REG0 = 0;
  REG0 = REG0 | (config.baud_rate << 5);
  REG0 = REG0 | (config.air_data_rate);
  command.push_back(REG0);

  // Register Address 03H
  uint8_t REG1 = 0;
  REG1 = REG1 | (config.subpacket_size << 6);
  REG1 = REG1 | (config.rssi_ambient_noise_flag << 5);
  REG1 = REG1 | (config.transmission_pause_flag << 4);
  REG1 = REG1 | (config.transmitting_power);
  command.push_back(REG1);

  // Register Address 04H
  uint8_t REG2 = config.own_channel;
  command.push_back(REG2);

  // Register Address 05H
  uint8_t REG3 = 0;
  REG3 = REG3 | (config.rssi_byte_flag << 7);
  REG3 = REG3 | (config.transmission_method_type << 6);
  REG3 = REG3 | (config.lbt_flag << 4);
  REG3 = REG3 | (config.wor_cycle);
  command.push_back(REG3);

  // Register Address 06H, 07H
  uint8_t CRYPT_H = config.encryption_key >> 8;
  uint8_t CRYPT_L = config.encryption_key & 0xff;
  command.push_back(CRYPT_H);
  command.push_back(CRYPT_L);

  PrintF("# Command Request\r\n");
  for (auto i : command) {
    PrintF("0x%02x ", i);
  }
  PrintF("\r\n");

  for (auto i : command) {
    SerialLoRa.write(i);
  }
  SerialLoRa.flush();


  while (!SerialLoRa.available()) delayMicroseconds(10);

  while (SerialLoRa.available()) {
    uint8_t data = SerialLoRa.read();
    response.push_back(data);
  }

  PrintF("# Command Response\r\n");
  for (auto i : response) {
    PrintF("0x%02x ", i);
  }
  PrintF("\r\n");

  if (response.size() != command.size()) {
    ret = 1;
  }

  SwitchToNormalMode();

  SerialLoRa.begin(LoRa_BaudRate, SERIAL_8N1, LoRa_RxPin, LoRa_TxPin);

  return ret;
}

int CLoRa::RecieveFrame(struct RecvFrameE220900T22SJP_t *recv_frame) {
  int len = 0;
  uint8_t *start_p = recv_frame->recv_data;

  LED_LORA_AUX_DISP();

  while (1) {

    int retcode = Check_Timer();
    if ( retcode != 0) return retcode;

    while(  ( digitalRead(LoRa_AUXPin) == 0) || (SerialLoRa.available() !=0 )){
      LED_LORA_AUX_DISP();
TWICE:
      while (SerialLoRa.available()) {
TWICE2:
        uint8_t ch = SerialLoRa.read();
        *(start_p + len) = ch;
        len++;
        if (len > 400) {             // 200 -->400  バッファサイズも要変更　　
          return 1;
        }
      }
      if (len > 0) {
        recv_frame->recv_data_len = len - 1;
        recv_frame->rssi = recv_frame->recv_data[len - 1] - 256;
      }

      for ( int i = 0 ; i < 5000 ; i++) {              // 180msec for example
        delayMicroseconds(60);                         // 60
        if ( digitalRead(LoRa_AUXPin) == 0) goto TWICE;
        if ( SerialLoRa.available() ) goto TWICE2;
      }
      LED_LORA_AUX_DISP();

      return 0;
    }
  }
}



int CLoRa::SendFrame(struct LoRaConfigItem_t &config, uint8_t *send_data,
                     int size) {
  uint8_t subpacket_size = 0;

  LED_LORA_AUX_DISP();


  switch (config.subpacket_size) {
    case 0b00:
      subpacket_size = 200;
      break;
    case 0b01:
      subpacket_size = 128;
      break;
    case 0b10:
      subpacket_size = 64;
      break;
    case 0b11:
      subpacket_size = 32;
      break;
    default:
      subpacket_size = 200;
      break;
  }
  if (size > subpacket_size) {
    PrintF("send data length too long\r\n");
    return 1;
  }
  uint8_t target_address_H = send_target_address >> 8;
  uint8_t target_address_L = send_target_address & 0xff;
  uint8_t target_channel = send_target_channel;


  uint8_t frame[3 + size] = {target_address_H, target_address_L,
                             target_channel
                            };

  if (config.transmission_method_type == 0) {
    memmove(frame , send_data, size);          // transparent mode : "size" is payload size
  } else {
    memmove(frame + 3, send_data, size - 3);   // fix mode : "size-3" is payload size
  }
  for (auto i : frame) {
    SerialLoRa.write(i);
  }
  SerialLoRa.flush();
  delay(100);
  LED_LORA_AUX_DISP();

  while (SerialLoRa.available()) {
    while (SerialLoRa.available()) {
      SerialLoRa.read();
    }
    delay(10);           // 100
  }

  LED_LORA_AUX_DISP();
  if ( digitalRead(LoRa_AUXPin) == 0 ){
    while (digitalRead(LoRa_AUXPin) == 0) {delay(1);}
  }
  return 0;
}

void CLoRa::SwitchToNormalMode(void) {
  pinMode(LoRa_ModeSettingPin_M0, OUTPUT);
  pinMode(LoRa_ModeSettingPin_M1, OUTPUT);

  digitalWrite(LoRa_ModeSettingPin_M0, 0);
  digitalWrite(LoRa_ModeSettingPin_M1, 0);
  delay(100);
}

void CLoRa::SwitchToWORSendingMode(void) {
  pinMode(LoRa_ModeSettingPin_M0, OUTPUT);
  pinMode(LoRa_ModeSettingPin_M1, OUTPUT);

  digitalWrite(LoRa_ModeSettingPin_M0, 1);
  digitalWrite(LoRa_ModeSettingPin_M1, 0);
  delay(100);
}

void CLoRa::SwitchToWORReceivingMode(void) {
  pinMode(LoRa_ModeSettingPin_M0, OUTPUT);
  pinMode(LoRa_ModeSettingPin_M1, OUTPUT);

  digitalWrite(LoRa_ModeSettingPin_M0, 0);
  digitalWrite(LoRa_ModeSettingPin_M1, 1);
  delay(100);
}

void CLoRa::SwitchToConfigurationMode(void) {
  pinMode(LoRa_ModeSettingPin_M0, OUTPUT);
  pinMode(LoRa_ModeSettingPin_M1, OUTPUT);

  digitalWrite(LoRa_ModeSettingPin_M0, 1);
  digitalWrite(LoRa_ModeSettingPin_M1, 1);
  delay(100);
}

void CLoRa::SetDefaultConfigValue(struct LoRaConfigItem_t &config) {
  const LoRaConfigItem_t default_config = {
    OWN_ADDRESS,         // own_address;              00H,01H  ADDH, ADDL
    UART_BAUD_RATE,      // baud_rate;                02H      REG0      b7-b5  no meaning
    AIR_DATA_RATE,       // air_data_rate;            02H      REG0      b4-b0
    SUBPACKET_SIZE,      // subpacket_size;           03H      REG1      b7,b6
    AMBIENT_NOISE,       // rssi_ambient_noise_flag;  03H      REG1      b5
    RESERVE1,          // reserve                  ----  always 0
    TRANSMITTING_PWR,    // transmitting_power;       03H      REG1      b1,b0
    OWN_CHANNEL,         // own_channel;              04H      REG2
    RSSI_BYTE,           // rssi_byte_flag;           05H      REG3      b7
    TRANSMITTION_METHOD, // transmission_method_type; 05H      REG3      B6
    RESERVE2,          // lbt_flag;                ----  always 0 for using carrier detect
    WOR_CYCLE,           // wor_cycle;                05H      REG3      b2-b0
    ENCRYPTION_KEY,      // encryption_key;           06H,07H  CRYPT_H, CRYPT_L

    TARGET_ADDRESS,      // target_address;     固定送信モードで使用　0xffff = bloadcast
    TARGET_CHANNEL,      // target_channel;     固定送信モードで使用

    0x00                 // wor mode                    M1,M0のモードに反映
  };
  config = default_config;
  Set_UART_BaudRate();   // Setting   uint8_t baud_rate;
}


void CLoRa::Set_UART_BaudRate() {
  uint8_t ratebit;
  switch ( LoRa_BaudRate ) {
    case   1200 : ratebit = 0b000; break;
    case   2400 : ratebit = 0b001; break;
    case   4800 : ratebit = 0b010; break;
    case   9600 : ratebit = 0b011; break;
    case  19200 : ratebit = 0b100; break;
    case  38400 : ratebit = 0b101; break;
    case  57600 : ratebit = 0b110; break;
    case 115200 : ratebit = 0b111; break;
  }
  config.baud_rate = ratebit;
}
