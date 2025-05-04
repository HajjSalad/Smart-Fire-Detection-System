/*
 * systick.h
 *
 *  Created on: Feb 22, 2025
 *      Author: Hajj
 */

#ifndef SYSTICK_H_
#define SYSTICK_H_

#include "stm32f446xx.h"
#include <stdbool.h>

extern volatile uint32_t ms_ticks;
extern volatile bool anomaly_trigger;

void SysTick_Handler(void);
void systick_init(void);
uint32_t systickGetMillis(void);
void systickDelayMs(int delay);

#endif /* SYSTICK_H_ */
