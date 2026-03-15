/*

  Displays/Lcd3Digit/Simple
  The Simple (Unoptimised) Approach

  For info and circuit diagrams see
  https://github.com/tardate/LittleArduinoProjects/tree/master/Electronics101/Displays/Lcd3Digit/Simple

*/
/*
    Arranged for small-airchecker using 74HC595 shift-reister in order to save using GPIO
*/
//#define ENABLE_SERIAL_DEBUG

#include "airchecker.h"


const byte segmentEncoding[13] = {
  // two bits for each com line for each digit
  // {7,6}(com4{1,0}), {5,4}(com3{1,0}), {3,2}(com2{1,0}), {1,0}(com1{1,0}) -


#if !LCD_UpsideDown
  0b11011110,        // 0
  0b10001000,        // 1
  0b01111010,        // 2
  0b11101010,        // 3
  0b10101100,        // 4
  0b11100110,        // 5
  0b11110110,        // 6
  0b10001110,        // 7
  0b11111110,        // 8
  0b11101110,        // 9
  0b00110000,        // r
  0b00100000,        // -
  0b00000000         // ' '
#else
  0b11011110,        // 0
  0b00010100,        // 1
  0b01111010,        // 2
  0b01110110,        // 3
  0b10110100,        // 4
  0b11100110,        // 5
  0b11101110,        // 6
  0b01010100,        // 7
  0b11111110,        // 8
  0b11110110,        // 9
  0b00101000,        // r
  0b00100000,        // -
  0b00000000         // ' '
#endif

};

static byte comSegmentSteps[8] = {
  // defines the segment pins PORTC at each step
  0, 0, 0, 0, 0, 0, 0, 0
};


volatile boolean timer_tick_triggered = false;

byte d3Encoding;
byte d2Encoding;
byte d1Encoding;


void Set_LCDLED_GPIO() {

  pinMode(SRCLK_PIN, OUTPUT);
  pinMode(RCLK_PIN, OUTPUT);
  pinMode(SER_PIN , OUTPUT);

  digitalWrite(SRCLK_PIN, LOW);
  digitalWrite(RCLK_PIN, LOW);
  digitalWrite(SER_PIN , LOW);

  pinMode(COM1_PIN, INPUT);
  pinMode(COM2_PIN, INPUT);
  pinMode(COM3_PIN, INPUT);
  pinMode(COM4_PIN, INPUT);
}


/*
   Command: calculates and sets the comSegmentSteps for the given number
*/

void setComSegmentSteps(int16_t number) {

  byte d2 = 0;
  byte d1 = 0;
  byte d0 = 0;

  if (number != 0) {
    d2 = number / 100;
    d1 = (number - number / 100 * 100) / 10;
    d0 = (number - number / 10 * 10);
  }

  int suppressF = 0;
  if (d2 == 0) {
    d3Encoding = '\0';                  // Zero suppress
    suppressF = 1;
  }  else {
    d3Encoding = segmentEncoding[d2];
  }

  d2Encoding = segmentEncoding[d1];
  
  if (d1 == 0)
    if (suppressF) d2Encoding = '\0';

  d1Encoding = segmentEncoding[d0];

#if !LCD_UpsideDown
  byte temp;
  temp = d3Encoding;
  d3Encoding = d1Encoding;
  d1Encoding = temp;
#endif

#ifdef ENABLE_SERIAL_DEBUG
  PrintLN("# setComSegmentSteps");
  //  Print("number: "); PrintLN(number);
  //  Print("d2: "); Print(d2); Print(" d3Encoding: "); PrintLN(d3Encoding, BIN);
  //  Print("d1: "); Print(d1); Print(" d2Encoding: "); PrintLN(d2Encoding, BIN);
  //  Print("d0: "); Print(d0); Print(" d1Encoding: "); PrintLN(d1Encoding, BIN);
#endif

  for (int step = 0; step < 8; step++) {
    byte value = step % 2 == 0x00 ? 0x3f : 0x00;

    switch (step) {
      case 0: // COM1 normally high phase
        if (d1Encoding & _BV(1)) value &= ~_BV(1); // if pin 2 should be asserted, flip output low
        if (d2Encoding & _BV(1)) value &= ~_BV(3); // if pin 4 should be asserted, flip output low
        if (d3Encoding & _BV(1)) value &= ~_BV(5); // if pin 6 should be asserted, flip output low
        break;
      case 1: // COM1 normally low phase
        if (d1Encoding & _BV(1)) value |= _BV(1); // if pin 2 should be asserted, flip output high
        if (d2Encoding & _BV(1)) value |= _BV(3); // if pin 4 should be asserted, flip output high
        if (d3Encoding & _BV(1)) value |= _BV(5); // if pin 6 should be asserted, flip output high
        break;
      case 2: // COM2 normally high phase
        if (d1Encoding & _BV(2)) value &= ~_BV(0); // if pin 1 should be asserted, flip output low
        if (d1Encoding & _BV(3)) value &= ~_BV(1); // if pin 2 should be asserted, flip output low
        if (d2Encoding & _BV(2)) value &= ~_BV(2); // if pin 3 should be asserted, flip output low
        if (d2Encoding & _BV(3)) value &= ~_BV(3); // if pin 4 should be asserted, flip output low
        if (d3Encoding & _BV(2)) value &= ~_BV(4); // if pin 5 should be asserted, flip output low
        if (d3Encoding & _BV(3)) value &= ~_BV(5); // if pin 6 should be asserted, flip output low
        break;
      case 3: // COM2 normally low phase
        if (d1Encoding & _BV(2)) value |= _BV(0); // if pin 1 should be asserted, flip output high
        if (d1Encoding & _BV(3)) value |= _BV(1); // if pin 2 should be asserted, flip output high
        if (d2Encoding & _BV(2)) value |= _BV(2); // if pin 3 should be asserted, flip output high
        if (d2Encoding & _BV(3)) value |= _BV(3); // if pin 4 should be asserted, flip output high
        if (d3Encoding & _BV(2)) value |= _BV(4); // if pin 5 should be asserted, flip output high
        if (d3Encoding & _BV(3)) value |= _BV(5); // if pin 6 should be asserted, flip output high
        break;
      case 4: // COM3 normally high phase
        if (d1Encoding & _BV(4)) value &= ~_BV(0); // if pin 1 should be asserted, flip output low
        if (d1Encoding & _BV(5)) value &= ~_BV(1); // if pin 2 should be asserted, flip output low
        if (d2Encoding & _BV(4)) value &= ~_BV(2); // if pin 3 should be asserted, flip output low
        if (d2Encoding & _BV(5)) value &= ~_BV(3); // if pin 4 should be asserted, flip output low
        if (d3Encoding & _BV(4)) value &= ~_BV(4); // if pin 5 should be asserted, flip output low
        if (d3Encoding & _BV(5)) value &= ~_BV(5); // if pin 6 should be asserted, flip output low
        break;
      case 5: // COM3 normally low phase
        if (d1Encoding & _BV(4)) value |= _BV(0); // if pin 1 should be asserted, flip output high
        if (d1Encoding & _BV(5)) value |= _BV(1); // if pin 2 should be asserted, flip output high
        if (d2Encoding & _BV(4)) value |= _BV(2); // if pin 3 should be asserted, flip output high
        if (d2Encoding & _BV(5)) value |= _BV(3); // if pin 4 should be asserted, flip output high
        if (d3Encoding & _BV(4)) value |= _BV(4); // if pin 5 should be asserted, flip output high
        if (d3Encoding & _BV(5)) value |= _BV(5); // if pin 6 should be asserted, flip output high
        break;
      case 6: // COM4 normally high phase
        if (d1Encoding & _BV(6)) value &= ~_BV(0); // if pin 1 should be asserted, flip output low
        if (d1Encoding & _BV(7)) value &= ~_BV(1); // if pin 2 should be asserted, flip output low
        if (d2Encoding & _BV(6)) value &= ~_BV(2); // if pin 3 should be asserted, flip output low
        if (d2Encoding & _BV(7)) value &= ~_BV(3); // if pin 4 should be asserted, flip output low
        if (d3Encoding & _BV(6)) value &= ~_BV(4); // if pin 5 should be asserted, flip output low
        if (d3Encoding & _BV(7)) value &= ~_BV(5); // if pin 6 should be asserted, flip output low
        break;
      case 7: // COM4 normally low phase
        if (d1Encoding & _BV(6)) value |= _BV(0); // if pin 1 should be asserted, flip output high
        if (d1Encoding & _BV(7)) value |= _BV(1); // if pin 2 should be asserted, flip output high
        if (d2Encoding & _BV(6)) value |= _BV(2); // if pin 3 should be asserted, flip output high
        if (d2Encoding & _BV(7)) value |= _BV(3); // if pin 4 should be asserted, flip output high
        if (d3Encoding & _BV(6)) value |= _BV(4); // if pin 5 should be asserted, flip output high
        if (d3Encoding & _BV(7)) value |= _BV(5); // if pin 6 should be asserted, flip output high
        break;
    }
    comSegmentSteps[step] = value;
#ifdef ENABLE_SERIAL_DEBUG
    Print("step: "); Print(step); Print("value: "); PrintLN(comSegmentSteps[step], BIN);
#endif
  }

}

/*
   Command: set COM port state for given step
*/
void setComState(byte step) {
  switch (step) {
    case 0: // COM1 normally high phase
#ifdef ENABLE_SERIAL_DEBUG
      PrintLN("COM1_PIN: HIGH ");
#endif
      pinMode(COM4_PIN, INPUT);
      digitalWrite(COM1_PIN, HIGH);
      pinMode(COM1_PIN, OUTPUT);
      digitalWrite(COM1_PIN, HIGH);
      break;
    case 1: // COM1 normally low phase
#ifdef ENABLE_SERIAL_DEBUG
      PrintLN("COM1_PIN: LOW ");
#endif
      digitalWrite(COM1_PIN, LOW);
      break;
    case 2: // COM2 normally high phase
#ifdef ENABLE_SERIAL_DEBUG
      PrintLN("COM2_PIN: HIGH ");
#endif
      pinMode(COM1_PIN, INPUT);
      delayMicroseconds(100);
      digitalWrite(COM2_PIN, HIGH);
      pinMode(COM2_PIN, OUTPUT);
      digitalWrite(COM2_PIN, HIGH);
      break;
    case 3: // COM2 normally low phase
#ifdef ENABLE_SERIAL_DEBUG
      PrintLN("COM2_PIN: LOW ");
#endif
      digitalWrite(COM2_PIN, LOW);
      break;
    case 4: // COM3 normally high phase
#ifdef ENABLE_SERIAL_DEBUG
      PrintLN("COM3_PIN: HIGH ");
#endif
      pinMode(COM2_PIN, INPUT);
      digitalWrite(COM3_PIN, HIGH);
      pinMode(COM3_PIN, OUTPUT);
      digitalWrite(COM3_PIN, HIGH);
      break;
    case 5: // COM3 normally low phase
#ifdef ENABLE_SERIAL_DEBUG
      PrintLN("COM3_PIN: LOW ");
#endif
      digitalWrite(COM3_PIN, LOW);
      break;
    case 6: // COM4 normally high phase
#ifdef ENABLE_SERIAL_DEBUG
      PrintLN("COM4_PIN: HIGH ");
#endif
      pinMode(COM3_PIN, INPUT);
      digitalWrite(COM4_PIN, HIGH);
      pinMode(COM4_PIN, OUTPUT);
      digitalWrite(COM4_PIN, HIGH);
      break;
    case 7: // COM4 normally low phase
#ifdef ENABLE_SERIAL_DEBUG
      PrintLN("COM4_PIN: LOW ");
#endif
      digitalWrite(COM4_PIN, LOW);
      break;
  }
  delayMicroseconds(8);
}





/*
   Command: set COM port state for given step
*/
void updateSegments(byte step) {
  byte value = comSegmentSteps[step];
#ifdef ENABLE_SERIAL_DEBUG
  Print("updateSegments step: "); Print(step); Print(" value: "); PrintLN(value, BIN);
  Print("           SEG1_PIN: "); PrintLN(value & _BV(0), BIN);
  Print("           SEG2_PIN: "); PrintLN(value & _BV(1), BIN);
  Print("           SEG3_PIN: "); PrintLN(value & _BV(2), BIN);
  Print("           SEG4_PIN: "); PrintLN(value & _BV(3), BIN);
  Print("           SEG5_PIN: "); PrintLN(value & _BV(4), BIN);
  Print("           SEG6_PIN: "); PrintLN(value & _BV(5), BIN);
  PrintF("           Value   : %02x\n", value);
#endif

  write_Segment_and_LED(value);


}


void Set_SEGMENT(int value) {
  setComSegmentSteps(value);
}


void DispLCD() {
  
//  Command: run a single full duty cycle
  for (int comStep = 0; comStep < 8; comStep++) {

    setComState(comStep);
    updateSegments(comStep);

    delay(4);
  }
}



int changeSign(int num) {
  int shft = 0;
  
  if (num == -256) return 0;                //  -256 --> 0

  for (; (num & 1) == 0; shft++) {
    num >>= 1;
  }
  int rssiNum = ~num << shft | 1 << shft;
  return rssiNum;
}

void LCD_disp_RSSI() {
  if ( data.rssi != 0)  rssi_no_sign = changeSign(data.rssi);
  else rssi_no_sign = 0;
  Set_SEGMENT(rssi_no_sign);
}


// Basic Function of Setting Shift register 

void relock(const uint8_t flag)
{
  int targetPin = 0;

  if (flag == 0x1) targetPin = SRCLK_PIN;
  else             targetPin = RCLK_PIN;

  digitalWrite(targetPin, HIGH);
  digitalWrite(targetPin, LOW);
#ifdef COMMENT
  if (flag == 0x01) PrintF("SRCLK ");
  else              PrintF("RCLK" );
#endif
}

/* 
 *  Two Types of setting 8 bits 
 */

void shiftreg_Write(unsigned char ref)
{
  for (int i = 8; i >= 1; i--)
  {
#ifdef COMMENT
    PrintF(" #%d", i - 1);
#endif
    if (ref & 0x80)
      digitalWrite(SER_PIN, HIGH);
    else
      digitalWrite(SER_PIN, LOW);
    ref <<= 1;
    relock(0x1);
  }
}

/*
 * --------------------------------------
 */

void write_Segment_and_LED(unsigned char X)
{
  
  LED_write();              // at first, setting LED on shift-register

  shiftreg_Write(X);             // Next Setting LCD 

  relock(0x0);

}


void LED_write() {
  uint8_t ref = LED_register;

  for (int i = 8; i >= 1; i--)
  {
    if (ref & 0x80) {
      digitalWrite(SER_PIN, HIGH);

    } else {
      digitalWrite(SER_PIN, LOW);

    }
    ref <<= 1;
    relock(0x1);
  }
}


void LCD_Check() {
  for(int i = 0; i < 2 ; i++) {
    for(int count = 0 ; count < 1000 ; count+=111) {
      Set_SEGMENT(count);
      delay(200);
    }
  }
}



// If USING Bloetooth or Wi-Fi, display the certification code like below 
#define USING_WIFI_BLUETOOTH false

#if  USING_WIFI_BLUETOOTH
void Set_SEGMENT_r() {
  char r_mark[] = {0xc,0xc,0xc,0xa,'\2','\0','\1',0xb,'\2','\2','\0','\5','\5','\5',0xc,0xc,0xc};
  byte seg1;
  byte seg2;
  byte seg3;

  for(int i=0; i<15; i++){
    seg1 = r_mark[i];
    seg2 = r_mark[i+1];
    seg3 = r_mark[i+2];
    setComSegmentSteps_r(seg1,seg2,seg3);
    delay(500);
  }
}


void setComSegmentSteps_r(byte seg1, byte seg2, byte seg3) {

  d3Encoding = segmentEncoding[seg1];
  d2Encoding = segmentEncoding[seg2];
  d1Encoding = segmentEncoding[seg3];

#if !LCD_UpsideDown
  byte temp;
  temp = d3Encoding;
  d3Encoding = d1Encoding;
  d1Encoding = temp;
#endif


  for (int step = 0; step < 8; step++) {
    byte value = step % 2 == 0x00 ? 0x3f : 0x00;

    switch (step) {
      case 0: // COM1 normally high phase
        if (d1Encoding & _BV(1)) value &= ~_BV(1); // if pin 2 should be asserted, flip output low
        if (d2Encoding & _BV(1)) value &= ~_BV(3); // if pin 4 should be asserted, flip output low
        if (d3Encoding & _BV(1)) value &= ~_BV(5); // if pin 6 should be asserted, flip output low
        break;
      case 1: // COM1 normally low phase
        if (d1Encoding & _BV(1)) value |= _BV(1); // if pin 2 should be asserted, flip output high
        if (d2Encoding & _BV(1)) value |= _BV(3); // if pin 4 should be asserted, flip output high
        if (d3Encoding & _BV(1)) value |= _BV(5); // if pin 6 should be asserted, flip output high
        break;
      case 2: // COM2 normally high phase
        if (d1Encoding & _BV(2)) value &= ~_BV(0); // if pin 1 should be asserted, flip output low
        if (d1Encoding & _BV(3)) value &= ~_BV(1); // if pin 2 should be asserted, flip output low
        if (d2Encoding & _BV(2)) value &= ~_BV(2); // if pin 3 should be asserted, flip output low
        if (d2Encoding & _BV(3)) value &= ~_BV(3); // if pin 4 should be asserted, flip output low
        if (d3Encoding & _BV(2)) value &= ~_BV(4); // if pin 5 should be asserted, flip output low
        if (d3Encoding & _BV(3)) value &= ~_BV(5); // if pin 6 should be asserted, flip output low
        break;
      case 3: // COM2 normally low phase
        if (d1Encoding & _BV(2)) value |= _BV(0); // if pin 1 should be asserted, flip output high
        if (d1Encoding & _BV(3)) value |= _BV(1); // if pin 2 should be asserted, flip output high
        if (d2Encoding & _BV(2)) value |= _BV(2); // if pin 3 should be asserted, flip output high
        if (d2Encoding & _BV(3)) value |= _BV(3); // if pin 4 should be asserted, flip output high
        if (d3Encoding & _BV(2)) value |= _BV(4); // if pin 5 should be asserted, flip output high
        if (d3Encoding & _BV(3)) value |= _BV(5); // if pin 6 should be asserted, flip output high
        break;
      case 4: // COM3 normally high phase
        if (d1Encoding & _BV(4)) value &= ~_BV(0); // if pin 1 should be asserted, flip output low
        if (d1Encoding & _BV(5)) value &= ~_BV(1); // if pin 2 should be asserted, flip output low
        if (d2Encoding & _BV(4)) value &= ~_BV(2); // if pin 3 should be asserted, flip output low
        if (d2Encoding & _BV(5)) value &= ~_BV(3); // if pin 4 should be asserted, flip output low
        if (d3Encoding & _BV(4)) value &= ~_BV(4); // if pin 5 should be asserted, flip output low
        if (d3Encoding & _BV(5)) value &= ~_BV(5); // if pin 6 should be asserted, flip output low
        break;
      case 5: // COM3 normally low phase
        if (d1Encoding & _BV(4)) value |= _BV(0); // if pin 1 should be asserted, flip output high
        if (d1Encoding & _BV(5)) value |= _BV(1); // if pin 2 should be asserted, flip output high
        if (d2Encoding & _BV(4)) value |= _BV(2); // if pin 3 should be asserted, flip output high
        if (d2Encoding & _BV(5)) value |= _BV(3); // if pin 4 should be asserted, flip output high
        if (d3Encoding & _BV(4)) value |= _BV(4); // if pin 5 should be asserted, flip output high
        if (d3Encoding & _BV(5)) value |= _BV(5); // if pin 6 should be asserted, flip output high
        break;
      case 6: // COM4 normally high phase
        if (d1Encoding & _BV(6)) value &= ~_BV(0); // if pin 1 should be asserted, flip output low
        if (d1Encoding & _BV(7)) value &= ~_BV(1); // if pin 2 should be asserted, flip output low
        if (d2Encoding & _BV(6)) value &= ~_BV(2); // if pin 3 should be asserted, flip output low
        if (d2Encoding & _BV(7)) value &= ~_BV(3); // if pin 4 should be asserted, flip output low
        if (d3Encoding & _BV(6)) value &= ~_BV(4); // if pin 5 should be asserted, flip output low
        if (d3Encoding & _BV(7)) value &= ~_BV(5); // if pin 6 should be asserted, flip output low
        break;
      case 7: // COM4 normally low phase
        if (d1Encoding & _BV(6)) value |= _BV(0); // if pin 1 should be asserted, flip output high
        if (d1Encoding & _BV(7)) value |= _BV(1); // if pin 2 should be asserted, flip output high
        if (d2Encoding & _BV(6)) value |= _BV(2); // if pin 3 should be asserted, flip output high
        if (d2Encoding & _BV(7)) value |= _BV(3); // if pin 4 should be asserted, flip output high
        if (d3Encoding & _BV(6)) value |= _BV(4); // if pin 5 should be asserted, flip output high
        if (d3Encoding & _BV(7)) value |= _BV(5); // if pin 6 should be asserted, flip output high
        break;
    }
    comSegmentSteps[step] = value;
  }
}
#endif
