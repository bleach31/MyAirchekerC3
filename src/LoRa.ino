#include "esp32_e220900t22s_jp_lib_3.h"

#define LORA_MESSAGE 1              //  set 1 in the case of debugging

void Set_LoRa_GPIO() {
  pinMode(LoRa_ModeSettingPin_M0, OUTPUT);
  pinMode(LoRa_ModeSettingPin_M1, OUTPUT);
  pinMode(LoRa_AUXPin, INPUT);
  digitalWrite(LoRa_ModeSettingPin_M0, LOW);
  digitalWrite(LoRa_ModeSettingPin_M1, LOW);
}


int LoRaRecvTask() {

  while (1) {

    int retcode;
    uint16_t recv_add;
    uint8_t  recv_ch;
    
    retcode = Check_Timer();
    if ( retcode != 0) return retcode;
    
    retcode = lora.RecieveFrame(&data);
    
    if (retcode  >= 1) return retcode;
    else {

      // RecieveFrame完了後に送信元アドレス・チャンネルを読み取る
      recv_add = data.recv_data[9] << 8 | data.recv_data[10];
      recv_ch = data.recv_data[11];
      if ( sender_or_receiver == 0 ) {
        send_target_address = recv_add;
        send_target_channel = recv_ch;
      }

#if LORA_MESSAGE
      PrintF("\r\n... LoRa Received from  ");
      PrintF(" %04x %02x\r\n", recv_add, recv_ch);
      PrintF("hex dump:\r\n");
      for (int i = 0; i < data.recv_data_len + 1; i++) {
        PrintF("%02x ", data.recv_data[i]);
      }
      PrintF("\r\n");
      PrintF("RSSI: %d dBm\r\n", data.rssi);
      PrintF("\r\n");
#endif
      return 0;
    }

  }
}

void LoRaSendTask() {  

  char initMsg[] = {' ', ' ', ' ', ' ', ' ', '1', '2', '3', ' ', ' ', ' ', ' ', ' ', '\0'};
  char msg[200] = {0};

  for (int i = 0; i < strlen(initMsg); i++) {
    msg[i] = initMsg[i];
  }
  msg[0] = MAGIC_KEY;
  msg[1] = UNIQUE_ID>>8;
  msg[2] = UNIQUE_ID&0xff;
  msg[3] = MACHINE_NO & 0xff;
  if(sender_or_receiver)   msg[4] = 1;       // Sender
  else                     msg[4] = 2;       // Receiver
  msg[9] = config.own_address >> 8;
  msg[10] = config.own_address & 0xff;
  msg[11] = config.own_channel;

  uint8_t subpacket = config.subpacket_size;
  if( sender_or_receiver == 0 ) config.subpacket_size = 0b00;    // if receiver, set max send back size to check length in CLoRa::SendFrame  
  

#if LORA_MESSAGE
  PrintF("\r\n\nSend_subpacket_size = %d", send_subpacket_size);
  PrintF("\r\nLoRa Sending to   ");
  PrintF(" %04x %02x", send_target_address, send_target_channel);
  PrintF("\r\n");
  if (lora.SendFrame(config, (uint8_t *)msg, send_subpacket_size) == 0) {
    PrintF("send succeeded.\r\n\r\n");
  } else {
    PrintF("send failed.\r\n\r\n");
  }
  SerialLoRa.flush();

#else
  lora.SendFrame(config, (uint8_t *)msg, send_subpacket_size);
#endif
  config.subpacket_size = subpacket;

}

void LoRaSendTask(int Normal_or_Nego) {          // Normal = 0, Nego > 1

  char initMsg[] = {' ', ' ', ' ', ' ', ' ', '1', '2', '3', '4', ' ', ' ', ' ', ' ', '\0'};
  char msg[200] = {0};

  for (int i = 0; i < strlen(initMsg); i++) {
    msg[i] = initMsg[i];
  }
  msg[0] = MAGIC_KEY;
  msg[1] = UNIQUE_ID>>8;
  msg[2] = UNIQUE_ID&0xff;
  msg[3] = MACHINE_NO & 0xff;
  msg[4] = Normal_or_Nego;
  msg[9] = config.own_address >> 8;
  msg[10] = config.own_address & 0xff;
  msg[11] = config.own_channel;
//
//  uint8_t subpacket = config.subpacket_size;
//  if( sender_or_receiver == 0 ) config.subpacket_size = 0b00;    // if receiver, set max send back size to check length in CLoRa::SendFrame  
//  

#if LORA_MESSAGE
  PrintF("\r\n\nSend_subpacket_size = %d", send_subpacket_size);
  PrintF("\r\nLoRa Sending to   ");
  PrintF(" %04x %02x", send_target_address, send_target_channel);
  PrintF("\r\n");
  if (lora.SendFrame(config, (uint8_t *)msg, send_subpacket_size) == 0) {
    PrintF("send succeeded.\r\n\r\n");
  } else {
    PrintF("send failed.\r\n\r\n");
  }
  SerialLoRa.flush();

#else
  lora.SendFrame(config, (uint8_t *)msg, send_subpacket_size);
#endif

}
