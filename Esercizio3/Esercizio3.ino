/**
 * @file Esercizio3.ino
 * @brief Minimal example using HCSR04_Polling (blocking).
 * Wiring (UNO): TRIG=D9, ECHO=D8
 */

#include <Arduino.h>
#include "hcsr04_polling.hpp"

/* Pin mapping */
static const uint8_t PIN_TRIG = 9U;
static const uint8_t PIN_ECHO = 8U;

/* Serial and timing */
static const unsigned long SERIAL_BAUD = 9600UL;
static const unsigned long TIMEOUT_US  = HCSR04_DEFAULT_TIMEOUT_US;   /* ~30 ms */
static const unsigned long MIN_CYCLE_US = HCSR04_DEFAULT_MIN_CYCLE_US; /* ~60 ms */

/* Driver instance (stack) */
static HCSR04_Polling g_sensor(PIN_TRIG, PIN_ECHO, TIMEOUT_US, HCSR04_CM_PER_US, MIN_CYCLE_US);

void setup(void)
{
  Serial.begin(SERIAL_BAUD);
  while (!Serial) { /* wait for serial if needed */ }

  (void)g_sensor.setTimeoutUs(TIMEOUT_US);
  (void)g_sensor.setMinCycleUs(MIN_CYCLE_US);

  const HCSR04_Status st = g_sensor.begin();
  Serial.print(F("begin="));
  Serial.println(static_cast<int>(st));
}

void loop(void)
{
  float dist_cm = 0.0F;
  const HCSR04_Status st = g_sensor.read(dist_cm);

  if (st == HCSR04_OK)
  {
    Serial.print(F("d="));
    Serial.print(dist_cm, 2);
    Serial.println(F("cm"));
  }
  else if (st == HCSR04_ERR_BUSY)
  {
    /* respect min cycle; no print to reduce noise */
  }
  else
  {
    Serial.print(F("err="));
    Serial.println(static_cast<int>(st));
  }
}