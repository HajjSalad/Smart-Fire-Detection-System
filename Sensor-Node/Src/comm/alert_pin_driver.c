/**
 * @file  alert_pin_driver.c
 * @brief GPIO output driver for anomaly alert pin
 * 
 * PB10 - GPIO output, active HIGH
 * HIGH -> anomaly detected, notify ESP32
 * LOW  -> normal operation
*/

#include "alert_pin_driver.h"
#include "stm32f446xx.h"

#define ALERT_GPIO_PIN    (1U << 10)

void alert_pin_init(void)
{ 
    // 1. Enable GPIOB clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    // 2. Configure PB10 as output mode (01)
    GPIOB->MODER &= ~(1U<<26);
	GPIOB->MODER &= ~(1U<<27);

    // 3. Idle LOW
    GPIOB->ODR &=~ ALERT_GPIO_PIN;
}

void alert_pin_set(void)
{
    GPIOB->ODR |= ALERT_GPIO_PIN;           // HIGH — anomaly
}

void alert_pin_clear(void)
{
    GPIOB->ODR &=~ ALERT_GPIO_PIN;          // LOW — normal
}
