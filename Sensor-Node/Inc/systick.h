/**
 * @file systick.h
 * @brief SysTick timer interface for STM32F446xx.
 *
 * This header provides declarations for initializing the SysTick timer,
 * retrieving system uptime in milliseconds, and performing millisecond delays.
 * It also defines shared variables for tracking time and triggering anomaly events.
 */

#ifndef SYSTICK_H_
#define SYSTICK_H_

#include "stm32f446xx.h"
#include <stdbool.h>

/**
 * @brief Global millisecond tick counter, incremented every 1 ms in SysTick handler.
 */
extern volatile uint32_t ms_ticks;

/**
 * @brief Flag to indicate an anomaly event triggered via SysTick.
 */
extern volatile bool anomaly_trigger;

/// Function Prototypes
void SysTick_Handler(void);
void systick_init(void);
uint32_t systickGetMillis(void);
void systickDelayMs(int delay);

#endif /* SYSTICK_H_ */
