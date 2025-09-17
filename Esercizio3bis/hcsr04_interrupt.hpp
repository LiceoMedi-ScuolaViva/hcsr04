/**
 * @file hcsr04_interrupt.hpp
 * @brief HC-SR04 ultrasonic sensor driver (interrupt-based) for Arduino UNO.
 * @version 1.0
 * @date 2025-09-17
 *
 * This concrete class derives from IHCSR04 and provides a non-blocking,
 * interrupt-driven implementation. It relies on external interrupts on
 * Arduino UNO (pins D2/D3).
 *
 * Design constraints:
 * - MISRA-oriented: explicit types, no dynamic allocation, no exceptions, no 'auto'.
 * - Single exit point for non-noexcept functions.
 * - ISR is minimal and only captures timestamps.
 */

#ifndef HCSR04_INTERRUPT_HPP_
#define HCSR04_INTERRUPT_HPP_

#include "hcsr04.hpp"

/**
 * @class HCSR04_Interrupt
 * @brief Concrete interrupt driver for HC-SR04 distance measurement.
 */
class HCSR04_Interrupt : public IHCSR04
{
public:
  explicit HCSR04_Interrupt(uint8_t trig_pin,
                            uint8_t echo_pin,
                            unsigned long timeout_us = HCSR04_DEFAULT_TIMEOUT_US,
                            float cm_per_us = HCSR04_CM_PER_US,
                            unsigned long min_cycle_us = HCSR04_DEFAULT_MIN_CYCLE_US);

  virtual ~HCSR04_Interrupt();

  /**
   * @brief Configure I/O directions and internal state. Call in setup().
   * @return HCSR04_Status
   */
  virtual HCSR04_Status begin(void);

  /**
   * @brief Start a new measurement and return result if ready.
   * @param[out] out_cm Distance in centimeters when HCSR04_OK is returned.
   * @return HCSR04_Status
   *
   * @note This API is non-blocking: it can return HCSR04_ERR_NOT_READY if
   *       the echo pulse has not completed yet.
   */
  virtual HCSR04_Status read(float &out_cm);

private:
  /* Static ISR trampoline (one instance only supported). */
  static void echoChangeISR_(void);

  /* Internal state machine. */
  static volatile bool s_waiting_rise;
  static volatile unsigned long s_rise_us;
  static volatile unsigned long s_fall_us;

  /* Enforce single instance to bind ISR. */
  static HCSR04_Interrupt* s_instance;
};

#endif /* HCSR04_INTERRUPT_HPP_ */