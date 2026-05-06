/**
 * @file  exti_driver.c
 * @brief External Interrupt (EXTI) configuration for flame sensor input.
*/

#include "exti_driver.h"
#include "stm32f446xx.h"

/**
 * @brief Initialize external interrupt inputs for flame detection.
 * 
 * This function configures GPIO pin PB13 as input signal with internal 
 * pull-up resistor and maps it to EXTI line 13.
 * Falling edge trigger is enabled to detect press events.
 * 
 * The EXTI line is unmasked and routed through the NVIC using the 
 * EXTI15_10 interrupt channel.
*/
void exti_init(void)
{ 
    // 1. Disable global interrupts
    __disable_irq();

    // 2. Enable GPIOB and SYSCFG clocks
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    // 3. Configure PB13 as input mode (00)
    GPIOB->MODER &= ~(1U<<26);
	GPIOB->MODER &= ~(1U<<27);

    // 4. Enable pull-up resistor on PB13 (01)
    GPIOB->PUPDR &= ~(1U<<27);
	GPIOB->PUPDR |=  (1U<<26);

    // 5. Map EXTI13 to PORTB - EXTICR[3] bits [7:4] = 0001
    SYSCFG->EXTICR[3] |= (1U<<4);

    // 6. Unmask EXTI13
    EXTI->IMR |= (1u<<13);

    // 7. Select edge trigger
    EXTI->FTSR |= (1U<<13);                 // Trigger on falling edge
    EXTI->RTSR |= (1U<<13);                 // Release on rising edge

    // 8. Enable EXTI 10-15 lines in NVIC
    NVIC_SetPriority(EXTI15_10_IRQn, 6);
    NVIC_EnableIRQ(EXTI15_10_IRQn);

    // 9. Enable global interrupts
	__enable_irq();
}