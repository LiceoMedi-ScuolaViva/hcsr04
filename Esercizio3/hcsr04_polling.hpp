/**
 * @file hcsr04_polling.hpp
 * @brief HC-SR04 ultrasonic sensor driver (polling implementation) for Arduino UNO.
 * @version 1.1
 * @date 2025-09-09
 *
 * This concrete class derives from IHCSR04 and provides a blocking, polling-based
 * implementation using Arduino's digital I/O and busy-wait loops on ECHO.
 *
 * Design constraints:
 * - MISRA-oriented: explicit types, no dynamic allocation, no exceptions, no 'auto'.
 * - Single exit point for non-noexcept functions.
 * - No background tasks/ISRs; timing relies on micros()/delayMicroseconds().
 */

#ifndef HCSR04_POLLING_HPP_
#define HCSR04_POLLING_HPP_

#include "hcsr04.hpp"

/**
 * @class HCSR04_Polling
 * @brief Concrete polling driver for HC-SR04 distance measurement.
 */
class HCSR04_Polling : public IHCSR04
{
public:
  /**
   * @brief Construct the driver with pin mapping and parameters.
   * @param trig_pin TRIG pin (OUTPUT).
   * @param echo_pin ECHO pin (INPUT).
   * @param timeout_us Round-trip timeout in microseconds.
   * @param cm_per_us Speed of sound in cm/us.
   * @param min_cycle_us Minimum idle time between consecutive reads (us).
   */
  explicit HCSR04_Polling(uint8_t trig_pin,
                          uint8_t echo_pin,
                          unsigned long timeout_us = HCSR04_DEFAULT_TIMEOUT_US,
                          float cm_per_us = HCSR04_CM_PER_US,
                          unsigned long min_cycle_us = HCSR04_DEFAULT_MIN_CYCLE_US) :
    IHCSR04(trig_pin, echo_pin, timeout_us, cm_per_us, min_cycle_us)
  {
    /* No work. Configuration is finalized in begin(). */
  }

  /**
   * @brief Configure I/O directions and internal state. Call in setup().
   * @return HCSR04_Status
   */
  virtual HCSR04_Status begin(void);

  /**
   * @brief Perform a single-shot distance measurement (blocking).
   * @param[out] out_cm Distance in centimeters when HCSR04_OK is returned.
   * @return HCSR04_Status
   */
  virtual HCSR04_Status read(float &out_cm);

  /* Rule-of-5: copy disabled in base. No extra resources here. */
  virtual ~HCSR04_Polling() {}

private:
  /* No data members. Implementation uses base getters and protected helpers. */
};

#endif /* HCSR04_POLLING_HPP_ */