/**
 * @file Esercizio3_interrupt.ino
 * @brief Demo per HC-SR04 con driver interrupt-based su Arduino UNO.
 * @version 1.0
 * @date 2025-09-17
 *
 * Wiring consigliato:
 *   - TRIG -> D9  (OUTPUT)
 *   - ECHO -> D2  (INT0, external interrupt)
 *
 * Serial: 115200 baud
 */

#include <Arduino.h>
#include "hcsr04_interrupt.hpp"

/* ========================= Configurazione ================================= */

static const uint8_t  PIN_TRIG = 9U;   /* D9  */
static const uint8_t  PIN_ECHO = 2U;   /* D2 = INT0 */
static const unsigned long TIMEOUT_US   = HCSR04_DEFAULT_TIMEOUT_US;     /* ~30 ms */
static const unsigned long MIN_CYCLE_US = HCSR04_DEFAULT_MIN_CYCLE_US;   /* ~60 ms */
static const float SOUND_CM_PER_US      = HCSR04_CM_PER_US;

/* Istanza driver (stack-allocated, nessuna allocazione dinamica). */
static HCSR04_Interrupt g_sonar(PIN_TRIG, PIN_ECHO, TIMEOUT_US, SOUND_CM_PER_US, MIN_CYCLE_US);

/* ========================= Helpers ======================================== */

static void printStatus_(HCSR04_Status st)
{
  /* Nota: mappa compatta dei codici per diagnosi veloce. */
  switch (st)
  {
    case HCSR04_OK:
      Serial.print(F("OK"));
      break;
    case HCSR04_ERR_NOT_READY:
      Serial.print(F("NOT_READY"));
      break;
    case HCSR04_ERR_TIMEOUT_ECHO_START:
      Serial.print(F("TIMEOUT_ECHO_START"));
      break;
    case HCSR04_ERR_TIMEOUT_ECHO_END:
      Serial.print(F("TIMEOUT_ECHO_END"));
      break;
    case HCSR04_ERR_BUSY:
      Serial.print(F("BUSY"));
      break;
    case HCSR04_ERR_BAD_PARAM:
      Serial.print(F("BAD_PARAM"));
      break;
    case HCSR04_ERR_BAD_STATE:
      Serial.print(F("BAD_STATE"));
      break;
    case HCSR04_ERR_TIMEOUT_TRIG:
    default:
      Serial.print(F("ERR"));
      break;
  }
}

/* ========================= Arduino setup/loop ============================= */

void setup(void)
{
  Serial.begin(9600);
  while (!Serial) { /* wait for USB CDC on some boards */ }

  Serial.println(F("\n=== HC-SR04 Interrupt Demo ==="));
  Serial.println(F("Pins: TRIG=D9, ECHO=D2(INT0)"));
  Serial.flush();

  const HCSR04_Status st = g_sonar.begin();
  if (st != HCSR04_OK)
  {
    Serial.print(F("begin(): "));
    printStatus_(st);
    Serial.println();
  }
}

void loop(void)
{
  float cm = 0.0F;

  /* Timestamp prima della chiamata */
  const unsigned long t_start = micros();
  HCSR04_Status st = g_sonar.read(cm);
  const unsigned long t_end = micros();

  /* Delta tempo */
  const unsigned long elapsed_us = t_end - t_start;

  if (st == HCSR04_OK)
  {
    Serial.print(F("Distanza: "));
    Serial.print(cm, 2);
    Serial.print(F(" cm"));

    Serial.print(F(" | Tempo read(): "));
    Serial.print(elapsed_us);
    Serial.println(F(" us"));
  }
  else if (st == HCSR04_ERR_NOT_READY)
  {
    /* Non fare nulla */
  }
  else
  {
    Serial.print(F("Status: "));
    printStatus_(st);
    Serial.print(F(" | Tempo read(): "));
    Serial.print(elapsed_us);
    Serial.println(F(" us"));
  }
}