/**
 * @file  button_flame_sensor.c
 * @brief Flame sensor driver
 * 
 * Digital GPIO input = 0 or 1
 * EXTI input interrupt
 *
*/
#include "stm32f446xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "sensor_drivers.h"
#include "shared_resources.h"

volatile uint8_t flame_detected;  /* Flame detected : 0/1 — GPIO */

/**
 * @brief EXTI15_10 interrupt handler — flame sensor detection
*/
void EXTI15_10_IRQHandler(void)
{
    if (EXTI->PR & (1U<<13))        // Check if EXTI13 triggered
    {
        EXTI->PR |= (1U<<13);       // Clear pending flag by writing 1

        // Read current pin state to determine press or release
        if (!(GPIOB->IDR & (1U<<13))) {
            flame_detected = 1U;                    // falling edge -> flame detected
        } else {
            flame_detected = 0U;                    // rising edge -> flame cleared
        }
    }
}