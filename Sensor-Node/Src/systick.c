/**
 * @file systick.c
 * @brief SysTick timer configuration and delay utility functions for STM32F446xx.
 *
 * This file sets up the SysTick timer to generate millisecond ticks using
 * a 16 MHz system clock. It also provides a basic delay function and a
 * SysTick interrupt handler used for timing and anomaly simulation.
 */

#include "stm32f446xx.h"
#include "systick.h"
#include "spi.h"
#include "simulate.h"
#include <stdbool.h>

#define SYSTICK_LOAD_VAL		16000
#define CTRL_ENABLE				(1U<<0)
#define CTRL_CLKSRC				(1U<<2)
#define CTRL_COUNTFLAG			(1U<<16)

volatile uint32_t systickMillis = 0;    /**< Global millisecond counter incremented in SysTick handler */
volatile uint32_t ms_ticks = 0;        /**< Millisecond ticks (may be used for scheduling) */
volatile bool anomaly_trigger = false; /**< Flag to indicate anomaly event */

/**
 * @brief SysTick interrupt handler.
 *
 * Increments the millisecond counter and calls `systick_simulation()`
 * for periodic event simulation or testing.
 */
void SysTick_Handler(void) {
	systickMillis++;						// Increment milliseconds counter
	systick_simulation();				    // Generates sensor values at defined simulation intervals
}

/**
 * @brief Initialize the SysTick timer.
 *
 * Configures SysTick to generate an interrupt every 1 ms
 * using the core clock. Enables the SysTick interrupt.
 */
void systick_init(void) {

	SysTick->LOAD = SYSTICK_LOAD_VAL;	
	SysTick->VAL = 0;						

	// Enable, set clock source, and enable interrupt
	SysTick->CTRL = CTRL_ENABLE | CTRL_CLKSRC | (1U << 1);
}

/**
 * @brief Get the current millisecond tick count.
 *
 * Returns the value of the millisecond counter incremented
 * by the SysTick interrupt handler.
 *
 * @return uint32_t Milliseconds since system start
 */
uint32_t systickGetMillis(void) {
	return systickMillis;					
}

/**
 * @brief Blocking delay in milliseconds.
 *
 * Busy-wait loop that delays execution for the specified number
 * of milliseconds.
 *
 * @param delay Duration in milliseconds
 */
void systickDelayMs(int delay) {
	uint32_t start = systickGetMillis();
	while (systickGetMillis() - start < delay) {}
}
