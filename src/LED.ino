#include "airchecker.h"


void LED_ON_LoRa_ITVL(){LED_register |= LED_LoRa_ITVL;}
void LED_ON_LoRa_AUX(){LED_register |=  LED_LoRa_AUX;}
void LED_ON_RED(){LED_register |= LED_RED;}
void LED_ON_YELLOW(){LED_register |= LED_YELLOW;}
void LED_ON_GREEN(){LED_register |= LED_GREEN;}
void LED_ON_BLUE(){LED_register |= LED_BLUE;}

void LED_OFF_LoRa_ITVL(){LED_register &= (uint8_t)~LED_LoRa_ITVL;}
void LED_OFF_LoRa_AUX(){LED_register &= (uint8_t)~LED_LoRa_AUX;}
void LED_OFF_RED(){LED_register &= (uint8_t)~LED_RED;}
void LED_OFF_YELLOW(){LED_register &= (uint8_t)~LED_YELLOW;}
void LED_OFF_GREEN(){LED_register &= (uint8_t)~LED_GREEN;}
void LED_OFF_BLUE(){LED_register &= (uint8_t)~LED_BLUE;}



void LED_ON_ABCD(){LED_register = 0b00111100;}
void LED_OFF_ABCD(){LED_register = 0b00000000;}

void LED_ON_ALL(){LED_register = 0b00111111;}
void LED_OFF_ALL(){LED_register = 0b00000000;}

void LED_LORA_AUX_DISP() {
  if (digitalRead(LoRa_AUXPin))LED_OFF_LoRa_AUX();
  else                         LED_ON_LoRa_AUX();
}

void  LED_All_Blink_4times() {
  for ( int i = 0; i < 4 ; i++) {
    LED_ON_ALL();
    delay(300);
    LED_OFF_ALL();
    delay(300);
  }
}

void sender_or_receiver_LED_ON() {
  if (!sender_or_receiver) {
    LED_OFF_GREEN();                    // Receiver
    LED_ON_BLUE();
  }
  else {
    LED_OFF_BLUE();                     // Sender
    LED_ON_GREEN();
  }
  Set_Target_Ad_Ch();
}


void printb(unsigned int v) {
  unsigned int mask = (int)1 << (sizeof(v) * CHAR_BIT - 1);
  do PrintF("%c",mask & v ? '1' : '0');
  while (mask >>= 1);
}

void putb(unsigned int v) {
  PrintF("0");
  PrintF("b");
  printb(v);
  PrintF("\n");
}
