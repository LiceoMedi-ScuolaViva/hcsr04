/**
 * @file hcsr04_polling.cpp
 * @brief Implementation of HCSR04_Polling (blocking, polling-based).
 * @version 1.1
 * @date 2025-09-09
 */

#include "hcsr04_polling.hpp"

/* ======== Local forward declarations (internal helpers) ================== */
/* None: helpers are inlined inside methods to keep linkage minimal. */

/* ============================== begin() ================================== */

HCSR04_Status HCSR04_Polling::begin(void)
{
  HCSR04_Status status = HCSR04_OK;

  /* Configure pin modes deterministically. */
  pinMode(getTrigPin(), OUTPUT);
  pinMode(getEchoPin(), INPUT);

  /* Ensure TRIG is low before any shot (datasheet-friendly). */
  digitalWrite(getTrigPin(), LOW);

  /* No hardware self-test here; consider adding a ping test if needed. */
  /* Single exit point. */
  return status;
}

/* =============================== read() ================================== */

HCSR04_Status HCSR04_Polling::read(float &out_cm)
{
  HCSR04_Status status = HCSR04_ERR_BAD_STATE;
  float tmp_cm = 0.0F;

  /* Enforce minimum cycle time between shots. */
  if (canStartShot_() == HCSR04_OK)
  {
    /* Mark shot start (timestamp used by base for cycle control). */
    markShotStart_();

    /* Generate TRIG pulse: LOW (≥2 us) -> HIGH (HCSR04_TRIG_PULSE_US) -> LOW. */
    digitalWrite(getTrigPin(), LOW);
    delayMicroseconds(2U);
    digitalWrite(getTrigPin(), HIGH);
    delayMicroseconds(HCSR04_TRIG_PULSE_US);
    digitalWrite(getTrigPin(), LOW);

    /* Wait for ECHO rising edge within timeout (referenced to t_start_us). */
    const unsigned long t_start_us = micros();
    const unsigned long timeout_us = getTimeoutUs();

    unsigned long t_rise_us = 0UL;
    unsigned long now_us = micros();

    while (((now_us - t_start_us) < timeout_us) && (t_rise_us == 0UL))
    {
      if (digitalRead(getEchoPin()) == HIGH)
      {
        t_rise_us = now_us;
      }
      now_us = micros();
    }

    if (t_rise_us == 0UL)
    {
      status = HCSR04_ERR_TIMEOUT_ECHO_START;
    }
    else
    {
      /* Wait for ECHO falling edge using the SAME global timeout window. */
      unsigned long t_fall_us = 0UL;
      now_us = micros();

      while (((now_us - t_start_us) < timeout_us) && (t_fall_us == 0UL))
      {
        if (digitalRead(getEchoPin()) == LOW)
        {
          t_fall_us = now_us;
        }
        now_us = micros();
      }

      if (t_fall_us == 0UL)
      {
        status = HCSR04_ERR_TIMEOUT_ECHO_END;
      }
      else
      {
        /* Compute high-time and convert to centimeters. */
        const unsigned long echo_high_us = t_fall_us - t_rise_us;
        status = timeUsToCm_(echo_high_us, tmp_cm);
        if (status == HCSR04_OK)
        {
          out_cm = tmp_cm;
        }
      }
    }
  }

  /* Single exit point. */
  return status;
}