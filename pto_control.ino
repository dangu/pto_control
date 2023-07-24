
#define USE_TIMER_1 true

// These define's must be placed at the beginning before #include "AVR_Slow_PWM.h"
// _PWM_LOGLEVEL_ from 0 to 4
// Don't define _PWM_LOGLEVEL_ > 0. Only for special ISR debugging only. Can hang the system.
#define _PWM_LOGLEVEL_ 3

#define USING_MICROS_RESOLUTION true

// To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
#include "AVR_Slow_PWM.h"

#define LED_OFF HIGH
#define LED_ON LOW

#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif

// Don't change these numbers to make higher Timer freq. System can hang
#define HW_TIMER_INTERVAL_MS 0.1f
#define HW_TIMER_INTERVAL_FREQ 10000L

volatile uint32_t startMicros = 0;

// Init AVR_Slow_PWM, each can service 16 different ISR-based PWM channels
AVR_Slow_PWM ISR_PWM;

//////////////////////////////////////////////////////

void TimerHandler() {
  ISR_PWM.run();
}

//////////////////////////////////////////////////////

#define USING_PWM_FREQUENCY true

//////////////////////////////////////////////////////

// You can assign pins here. Be careful to select good pin to use or crash
uint32_t PWM_Pin = LED_BUILTIN;

// You can assign any interval for any timer here, in Hz
float PWM_Freq = 50.0f;

// You can assign any duty_cycle for any PWM here, from 0-100
float PWM_DutyCycle1 = 20.0;
// You can assign any duty_cycle for any PWM here, from 0-100
float PWM_DutyCycle2 = 90.0;

/**@brief Change duty cycle of output

@param duty_cycle [float] duty cycle
*/
static void changeDutyCycle(float dutyCycle) {
  static int channelNum;
  static bool firstRun;
  if (!firstRun) {
    ISR_PWM.deleteChannel((unsigned)channelNum);

  } else {
    firstRun = false;
  }
  channelNum = ISR_PWM.setPWM(PWM_Pin, PWM_Freq, dutyCycle);
}

////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

  delay(2000);

  Serial.print(F("\nStarting ISR_Changing_PWM on "));
  Serial.println(BOARD_NAME);
  Serial.println(AVR_SLOW_PWM_VERSION);
  Serial.print(F("CPU Frequency = "));
  Serial.print(F_CPU / 1000000);
  Serial.println(F(" MHz"));

  // Timer0 is used for micros(), millis(), delay(), etc and can't be used
  // Select Timer 1-2 for UNO, 1-5 for MEGA, 1,3,4 for 16u4/32u4
  // Timer 2 is 8-bit timer, only for higher frequency
  // Timer 4 of 16u4 and 32u4 is 8/10-bit timer, only for higher frequency

  ITimer1.init();

  // Using ATmega328 used in UNO => 16MHz CPU clock ,

  if (ITimer1.attachInterrupt(HW_TIMER_INTERVAL_FREQ, TimerHandler)) {
    Serial.print(F("Starting  ITimer1 OK, micros() = "));
    Serial.println(micros());
  } else
    Serial.println(F("Can't set ITimer1. Select another freq. or timer"));

  //   ISR_PWM.setPWM(PWM_Pin, PWM_Freq1, 0);
}



void loop() {
  static float dutyCycle;
  char receivedChar;

  if (Serial.available() > 0) {
    receivedChar = Serial.read();
    if ((receivedChar >= '0') && (receivedChar <= '9')) {
      dutyCycle = (receivedChar - '0') * 10.0;
      changeDutyCycle(dutyCycle);
    } else if (receivedChar == 'a') {
      dutyCycle = 100.0;
      changeDutyCycle(dutyCycle);
    } else if (receivedChar == '+') {
      changeDutyCycle(++dutyCycle);
    } else if (receivedChar == '-') {
      changeDutyCycle(--dutyCycle);
    }
  }
}
