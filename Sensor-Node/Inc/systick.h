/*
 * systick.h
 *
 *  Created on: Feb 22, 2025
 *      Author: Hajj
 */

#ifndef SYSTICK_H_
#define SYSTICK_H_

#include "stm32f446xx.h"

void SysTick_Handler(void);
void systick_init(void);
uint32_t systickGetMillis(void);
void systickDelayMs(int delay);

#endif /* SYSTICK_H_ */
