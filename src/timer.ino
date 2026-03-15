#include "airchecker.h"

//refer to this site
//https://github.com/khoih-prog/ESP32_C3_TimerInterrupt
// Can be included as many times as necessary, without `Multiple Definitions` Linker Error
#include "ESP32_C3_TimerInterrupt.h"

// These define's must be placed at the beginning before #include "TimerInterrupt_Generic.h"
// _TIMERINTERRUPT_LOGLEVEL_ from 0 to 4
// Don't define _TIMERINTERRUPT_LOGLEVEL_ > 0. Only for special ISR debugging only. Can hang the system.
#define TIMER_INTERRUPT_DEBUG         0
#define _TIMERINTERRUPT_LOGLEVEL_     4


// With core v2.0.0+, you can't use Print/println in ISR or crash.
// and you can't use float calculation inside ISR
// Only OK in core v1.0.6-
bool IRAM_ATTR TimerHandler0(void * timerNo)
{
  NoReceived_Interrupt_flag++;
  return true;
}

bool IRAM_ATTR TimerHandler1(void * timerNo)
{
  SendInterval_Interrupt_flag++;
  return true;
}

#define TIMER0_INTERVAL_MS        8000
#define TIMER1_INTERVAL_MS        6000

// Init ESP32 timer 0 and 1
ESP32Timer ITimer0(0);
ESP32Timer ITimer1(1);


// Using ESP32  => 80 / 160 / 240MHz CPU clock ,
// For 64-bit timer counter
// For 16-bit timer prescaler up to 1024

void Disp_Timer_interrupt() {
  Print(F("\nStarting TimerInterruptTest on "));
  PrintLN(ARDUINO_BOARD);
  PrintLN(ESP32_C3_TIMER_INTERRUPT_VERSION);
  Print(F("CPU Frequency = "));
  Print(F_CPU / 1000000);
  PrintLN(F(" MHz"));
}


/*
 * Timer0InterruptStart
 */
void NoReceivedCheck_InterruptStart() {
  // Interval in microsecs
#if TIMER_INTERRUPT_DEBUG  
  if (ITimer0.attachInterruptInterval(TIMER0_INTERVAL_MS * 1000, TimerHandler0))
  {
    Print(F("Starting  ITimer0 OK, millis() = "));
    PrintLN(millis());
  }
  else
    PrintLN(F("Can't set ITimer0. Select another freq. or timer"));
#else
  ITimer0.attachInterruptInterval(TIMER0_INTERVAL_MS * 1000, TimerHandler0);
#endif

  NoReceived_Interrupt_flag = 0;

}

/*
 * Timer1InterruptStart
 */
void SendInterval_InterruptStart() {
  // Interval in microsecs
#if TIMER_INTERRUPT_DEBUG  

  if (ITimer1.attachInterruptInterval(TIMER1_INTERVAL_MS * 1000, TimerHandler1))
  {
    Print(F("Starting  ITimer1 OK, millis() = "));
    PrintLN(millis());
  }
  else
    PrintLN(F("Can't set ITimer1. Select another freq. or timer"));
#else
  ITimer1.attachInterruptInterval(TIMER1_INTERVAL_MS * 1000, TimerHandler1);
#endif

    SendInterval_Interrupt_flag = 0;

}
