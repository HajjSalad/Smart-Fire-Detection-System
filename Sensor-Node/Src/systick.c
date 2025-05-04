/*
 * systick.c
 *
*/

#include "stm32f446xx.h"
#include "systick.h"
#include "spi.h"
#include <stdbool.h>

#define SYSTICK_LOAD_VAL		16000
#define CTRL_ENABLE				(1U<<0)
#define CTRL_CLKSRC				(1U<<2)
#define CTRL_COUNTFLAG			(1U<<16)

volatile uint32_t systickMillis = 0;		// Global variable to store milliseconds
volatile uint32_t ms_ticks = 0;
volatile bool anomaly_trigger = false;

void SysTick_Handler(void) {
	systickMillis++;						// Increment milliseconds counter
	Systick_Runner();				// Use this to trigger anomaly every 10sec for testing
}

// Configure SysTick
void systick_init(void) {

	SysTick->LOAD = SYSTICK_LOAD_VAL;		// Reload with number of clocks per ms
	SysTick->VAL = 0;						// Clear Systick current value

	// Enable, set clock source, and enable interrupt
	SysTick->CTRL = CTRL_ENABLE | CTRL_CLKSRC | (1U << 1);
}

uint32_t systickGetMillis(void) {
	return systickMillis;					// Return the current milliseconds count
}

void systickDelayMs(int delay) {
	uint32_t start = systickGetMillis();
	while (systickGetMillis() - start < delay) {}	// Busy-wait for the specified delay
}
