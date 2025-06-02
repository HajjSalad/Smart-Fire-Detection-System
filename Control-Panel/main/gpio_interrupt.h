/**
 * @file gpio_interrupt.h
 * @brief Header for GPIO interrupt initialization.
 */

#ifndef GPIO_INTERRUPT_H
#define GPIO_INTERRUPT_H

/**
 * @brief Initialize GPIO interrupt line and ISR handler.
 *
 * Configures the GPIO pin for interrupt on positive edge and sets up the ISR.
 */
void interrupt_line_init();

#endif // GPIO_INTERRUPT_H