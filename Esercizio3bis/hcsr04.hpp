/**
 * @file hcsr04.hpp
 * @brief HC-SR04 ultrasonic sensor driver for Arduino UNO — abstract interface (enhanced).
 * @version 1.1
 * @date 2025-09-09
 *
 * Design goals:
 * - Provide a minimal yet practical abstract base (IHCSR04) with built-in configuration.
 * - MISRA-oriented style: explicit types, no dynamic allocation, no exceptions, no 'auto'.
 * - No dynamic polymorphic creation here; derived concrete drivers should be stack-allocated.
 *
 * Notes:
 * - Suggested wiring (polling): TRIG -> D9, ECHO -> D8 (Arduino UNO).
 * - Sampling: respect HC-SR04 minimum cycle (~60 ms) to avoid echo overlap.
 */

#ifndef HCSR04_HPP_
#define HCSR04_HPP_

#include <Arduino.h>

/* ========================= Configuration constants ========================= */

/** @brief Speed of sound (cm/us) at ~20°C, no humidity compensation. */
#define HCSR04_CM_PER_US              (0.0343F)

/** @brief Overall timeout for a single transaction (microseconds, ~5 m round-trip). */
#define HCSR04_DEFAULT_TIMEOUT_US     (30000UL)

/** @brief TRIG high pulse width in microseconds. */
#define HCSR04_TRIG_PULSE_US          (10U)

/** @brief Minimum allowed idle time between shots (microseconds, datasheet ~60 ms). */
#define HCSR04_DEFAULT_MIN_CYCLE_US   (60000UL)

/* ============================== Status codes =============================== */

/**
 * @brief Driver status codes for diagnosability.
 */
typedef enum
{
  HCSR04_OK = 0,
  HCSR04_ERR_TIMEOUT_TRIG       = -1, /**< Reserved for future use. */
  HCSR04_ERR_TIMEOUT_ECHO_START = -2, /**< No rising edge detected within timeout. */
  HCSR04_ERR_TIMEOUT_ECHO_END   = -3, /**< No falling edge detected within timeout. */
  HCSR04_ERR_BUSY               = -4, /**< Operation not allowed while another is pending. */
  HCSR04_ERR_NOT_READY          = -5, /**< Non-blocking read: result not yet ready (not used in polling). */
  HCSR04_ERR_BAD_STATE          = -6, /**< API misuse or invalid configuration. */
  HCSR04_ERR_BAD_PARAM          = -7  /**< Invalid parameter passed to setter. */
} HCSR04_Status;

/* ============================= Abstract interface ========================== */

/**
 * @class IHCSR04
 * @brief Abstract base for HC-SR04 distance measurement with built-in configuration.
 *
 * The interface provides:
 * - Blocking single-shot measurement via read().
 * - Configuration of TRIG/ECHO pins, timeouts, sound speed, min cycle.
 * - Lightweight parameter validation without exceptions.
 *
 * Concrete implementations (e.g., polling-based, interrupt-based) should derive from this
 * class and implement begin() and read(), optionally reusing protected helpers.
 */
class IHCSR04
{
public:
  /**
   * @brief Construct with pin mapping and optional parameters.
   * @param trig_pin Arduino digital pin used as TRIG (OUTPUT).
   * @param echo_pin Arduino digital pin used as ECHO (INPUT).
   * @param timeout_us Round-trip timeout in microseconds (default ~30 ms).
   * @param cm_per_us Speed of sound in cm/us (default ~0.0343F @20°C).
   * @param min_cycle_us Minimum idle time between consecutive reads (default ~60 ms).
   *
   * @note No exceptions are thrown; invalid inputs can be detected later by getters
   *       or during begin() via return codes of derived classes.
   */
  explicit IHCSR04(uint8_t trig_pin,
                   uint8_t echo_pin,
                   unsigned long timeout_us = HCSR04_DEFAULT_TIMEOUT_US,
                   float cm_per_us = HCSR04_CM_PER_US,
                   unsigned long min_cycle_us = HCSR04_DEFAULT_MIN_CYCLE_US) :
      m_trig_pin(trig_pin),
      m_echo_pin(echo_pin),
      m_timeout_us(timeout_us),
      m_cm_per_us(cm_per_us),
      m_min_cycle_us(min_cycle_us),
      m_last_shot_us(0UL)
  {
    /* No dynamic work here. Derived::begin() will configure pin modes. */
  }

  /* Virtual destructor required for polymorphic base. */
  virtual ~IHCSR04() {}

  /* ----------------------------- Lifecycle -------------------------------- */

  /**
   * @brief Configure I/O directions and internal state. Call in setup().
   * @return HCSR04_Status
   */
  virtual HCSR04_Status begin(void) = 0;

  /* ------------------------ Configuration setters ------------------------- */

  /**
   * @brief Set TRIG pin.
   * @param trig_pin New TRIG pin number.
   * @return HCSR04_Status (HCSR04_ERR_BAD_PARAM if same as ECHO or out of range).
   */
  HCSR04_Status setTrigPin(uint8_t trig_pin)
  {
    HCSR04_Status status = HCSR04_ERR_BAD_PARAM;
    if (trig_pin != m_echo_pin)
    {
      m_trig_pin = trig_pin;
      status = HCSR04_OK;
    }
    return status;
  }

  /**
   * @brief Set ECHO pin.
   * @param echo_pin New ECHO pin number.
   * @return HCSR04_Status (HCSR04_ERR_BAD_PARAM if same as TRIG or out of range).
   */
  HCSR04_Status setEchoPin(uint8_t echo_pin)
  {
    HCSR04_Status status = HCSR04_ERR_BAD_PARAM;
    if (echo_pin != m_trig_pin)
    {
      m_echo_pin = echo_pin;
      status = HCSR04_OK;
    }
    return status;
  }

  /**
   * @brief Set the round-trip timeout (in microseconds).
   * @param timeout_us Microseconds before giving up (e.g., 30000UL ~5 m RT).
   * @return HCSR04_Status (HCSR04_ERR_BAD_PARAM if too small).
   */
  HCSR04_Status setTimeoutUs(unsigned long timeout_us)
  {
    HCSR04_Status status = HCSR04_ERR_BAD_PARAM;
    if (timeout_us >= (HCSR04_TRIG_PULSE_US + 100UL)) /* simple sanity margin */
    {
      m_timeout_us = timeout_us;
      status = HCSR04_OK;
    }
    return status;
  }

  /**
   * @brief Set the minimum cycle time between shots (in microseconds).
   * @param min_cycle_us Typical >= 60000 us per datasheet guidance.
   * @return HCSR04_Status (HCSR04_ERR_BAD_PARAM if zero).
   */
  HCSR04_Status setMinCycleUs(unsigned long min_cycle_us)
  {
    HCSR04_Status status = HCSR04_ERR_BAD_PARAM;
    if (min_cycle_us != 0UL)
    {
      m_min_cycle_us = min_cycle_us;
      status = HCSR04_OK;
    }
    return status;
  }

  /**
   * @brief Set the speed of sound in cm/us (e.g., temperature compensation).
   * @param cm_per_us Centimeters per microsecond (default ~0.0343F @20°C).
   * @return HCSR04_Status (HCSR04_ERR_BAD_PARAM if non-positive or absurd).
   */
  HCSR04_Status setSoundSpeed(float cm_per_us)
  {
    HCSR04_Status status = HCSR04_ERR_BAD_PARAM;
    /* Basic plausibility window: 0.02..0.06 cm/us (approx. 200..600 m/s). */
    if ((cm_per_us > 0.02F) && (cm_per_us < 0.06F))
    {
      m_cm_per_us = cm_per_us;
      status = HCSR04_OK;
    }

    return status;
  }

  /* -------------------------- Configuration getters ----------------------- */

  /** @brief Get TRIG pin. */
  uint8_t getTrigPin(void) const noexcept { return m_trig_pin; }

  /** @brief Get ECHO pin. */
  uint8_t getEchoPin(void) const noexcept { return m_echo_pin; }

  /** @brief Get current timeout (us). */
  unsigned long getTimeoutUs(void) const noexcept { return m_timeout_us; }

  /** @brief Get current min cycle (us). */
  unsigned long getMinCycleUs(void) const noexcept { return m_min_cycle_us; }

  /** @brief Get current speed of sound (cm/us). */
  float getSoundSpeed(void) const noexcept { return m_cm_per_us; }

  /** @brief Returns micros() timestamp of last shot start mark. */
  unsigned long getLastShotTimestampUs(void) const noexcept { return m_last_shot_us; }

  /* ---------------------------- Measurement API --------------------------- */

  /**
   * @brief Perform a single-shot distance measurement (blocking).
   * @param[out] out_cm Distance in centimeters when HCSR04_OK is returned.
   * @return HCSR04_Status
   *
   * @note Derived classes should:
   *  - Enforce the min cycle via canStartShot_().
   *  - Produce the TRIG pulse (>= HCSR04_TRIG_PULSE_US).
   *  - Capture ECHO high-time or return a timeout error.
   *  - Convert echo time to cm via timeUsToCm_().
   */
  virtual HCSR04_Status read(float &out_cm) = 0;

  /* -------------------------- Rule-of-5 limitations ----------------------- */

  IHCSR04(const IHCSR04&) = delete;
  IHCSR04& operator=(const IHCSR04&) = delete;

protected:
  /* --------- Helpers for derived implementations (no exceptions) ---------- */

  /**
   * @brief Check if min cycle time has elapsed since last shot.
   * @return HCSR04_Status (HCSR04_OK if shot can start, HCSR04_ERR_BUSY otherwise).
   */
  HCSR04_Status canStartShot_(void) const
  {
    HCSR04_Status status = HCSR04_ERR_BUSY;
    const unsigned long now_us = micros();
    const unsigned long elapsed = now_us - m_last_shot_us; /* wraps fine on unsigned long */
    if (elapsed >= m_min_cycle_us)
    {
      status = HCSR04_OK;
    }
    return status;
  }

  /**
   * @brief Mark the start time of the current shot (called by derived before TRIG).
   */
  void markShotStart_(void)
  {
    m_last_shot_us = micros();
  }

  /**
   * @brief Convert echo round-trip time (us) to distance (cm).
   * @param echo_high_us Time ECHO stayed HIGH (round-trip).
   * @param[out] out_cm Resulting distance in centimeters.
   * @return HCSR04_Status (HCSR04_ERR_BAD_PARAM if echo_high_us==0).
   *
   * @note Distance(cm) = (echo_time_us * speed_cm_per_us) / 2
   */
  HCSR04_Status timeUsToCm_(unsigned long echo_high_us, float &out_cm) const
  {
    HCSR04_Status status = HCSR04_ERR_BAD_PARAM;
    if (echo_high_us != 0UL)
    {
      const float half = 0.5F;
      out_cm = (static_cast<float>(echo_high_us) * m_cm_per_us) * half;
      status = HCSR04_OK;
    }
    return status;
  }

private:
  /* ------------------------------ Data members ---------------------------- */
  uint8_t       m_trig_pin;
  uint8_t       m_echo_pin;
  unsigned long m_timeout_us;
  float         m_cm_per_us;
  unsigned long m_min_cycle_us;
  unsigned long m_last_shot_us;
};

#endif /* HCSR04_HPP_ */