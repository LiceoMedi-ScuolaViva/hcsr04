/**
 * @file hcsr04_interrupt.cpp
 * @brief Implementation of HCSR04_Interrupt (non-blocking, interrupt-based).
 * @version 1.0
 * @date 2025-09-17
 */

#include "hcsr04_interrupt.hpp"

/* ======== Static member definitions ====================================== */
volatile bool HCSR04_Interrupt::s_waiting_rise = true;
volatile unsigned long HCSR04_Interrupt::s_rise_us = 0UL;
volatile unsigned long HCSR04_Interrupt::s_fall_us = 0UL;
HCSR04_Interrupt* HCSR04_Interrupt::s_instance = 0;

/* ============================= Constructor =============================== */

HCSR04_Interrupt::HCSR04_Interrupt(uint8_t trig_pin,
                                   uint8_t echo_pin,
                                   unsigned long timeout_us,
                                   float cm_per_us,
                                   unsigned long min_cycle_us) :
  IHCSR04(trig_pin, echo_pin, timeout_us, cm_per_us, min_cycle_us)
{
  /* No work: deferred to begin(). */
}

/* ============================= Destructor ================================ */

HCSR04_Interrupt::~HCSR04_Interrupt()
{
  detachInterrupt(digitalPinToInterrupt(getEchoPin()));
  s_instance = 0;
}

/* ============================== begin() ================================== */

HCSR04_Status HCSR04_Interrupt::begin(void)
{
  HCSR04_Status status = HCSR04_OK;

  /* Configure pins. */
  pinMode(getTrigPin(), OUTPUT);
  pinMode(getEchoPin(), INPUT);
  digitalWrite(getTrigPin(), LOW);

  /* Register singleton instance for ISR. */
  s_instance = this;
  s_waiting_rise = true;
  s_rise_us = 0UL;
  s_fall_us = 0UL;

  attachInterrupt(digitalPinToInterrupt(getEchoPin()), echoChangeISR_, CHANGE);

  return status;
}

/* =============================== read() ================================== */

HCSR04_Status HCSR04_Interrupt::read(float &out_cm)
{
  HCSR04_Status status = canStartShot_();

  if (status == HCSR04_OK)
  {
    /* Mark new shot. */
    markShotStart_();

    /* Reset ISR state. */
    s_waiting_rise = true;
    s_rise_us = 0UL;
    s_fall_us = 0UL;

    /* Generate TRIG pulse. */
    digitalWrite(getTrigPin(), LOW);
    delayMicroseconds(2U);
    digitalWrite(getTrigPin(), HIGH);
    delayMicroseconds(HCSR04_TRIG_PULSE_US);
    digitalWrite(getTrigPin(), LOW);
  }

  /* If rising and falling edges are captured, compute result. */
  if ((s_rise_us != 0UL) && (s_fall_us != 0UL))
  {
    unsigned long echo_high_us = s_fall_us - s_rise_us;
    float tmp_cm = 0.0F;
    status = timeUsToCm_(echo_high_us, tmp_cm);
    if (status == HCSR04_OK)
    {
      out_cm = tmp_cm;
    }

    /* Reset for next shot. */
    s_rise_us = 0UL;
    s_fall_us = 0UL;
  }
  else
  {
    status = HCSR04_ERR_NOT_READY;
  }

  return status;
}

/* =============================== ISR ===================================== */

void HCSR04_Interrupt::echoChangeISR_(void)
{
  const int level = digitalRead(s_instance->getEchoPin());
  const unsigned long now_us = micros();

  if (s_waiting_rise == true)
  {
    if (level == HIGH)
    {
      s_rise_us = now_us;
      s_waiting_rise = false;
    }
  }
  else
  {
    if (level == LOW)
    {
      s_fall_us = now_us;
      s_waiting_rise = true;
    }
  }
}