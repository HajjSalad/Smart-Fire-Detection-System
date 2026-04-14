/**
 * @file  i2c1_driver.c
 * @brief Bare-metal I2C1 peripheral driver for STM32F446RE
 *
 * Provides low-level I2C1 initialization and master transmit/receive
 * functionality for communication with I2C slave devices.
*/

#include "stm32f446xx.h"
#include <stdio.h>

/**
 * @brief Initialize I2C1 peripheral in standard mode at 100kHz.
 * 
 * I2C1 SDA - PB7
 * I2C1 SCL - PB6 
 * 
 * Initialize I2C1 at 100kHz with 16MHz APB1 clock
*/
void i2c1_init(void) 
{
    // 1. Enable required clocks for I2C1
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;         // Enable I2C1 clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;        // Enable GPIOB clock

    // 2. Configure PB7, PB6 for Alternate Function
    GPIOB->MODER &=~ (1U<<14);          // PB7 to alternate function mode
    GPIOB->MODER |=  (1U<<15);
    GPIOB->MODER &=~ (1U<<12);          // PB6 to alternate function mode
    GPIOB->MODER |=  (1U<<13);

    // 3. Set output type to open drain configuration
    GPIOB->OTYPER |= (1U<<7);
    GPIOB->OTYPER |= (1U<<6);

    // 4. Set output to high speed
    GPIOB->OSPEEDR |= (3U<<14);
    GPIOB->OSPEEDR |= (3U<<12);

    // 5. Enable Pull-up resistors
    GPIOB->PUPDR |= (1U<<14);
    GPIOB->PUPDR |= (1U<<12);

    // 6. Set I2C1 SDA & SCL to AF04
    GPIOB->AFR[0] |= (4<<28);
    GPIOB->AFR[0] |= (4<<24);

    // 7. Disable I2C before configuration
    I2C1->CR1 &=~ I2C_CR1_PE;

    // 8. Set APB1 clock frequency
    I2C1->CR2 = 16U;                    // APB1 = 16MHz

    // 9. Set clock control register for 100kHz in standard mode
    // CCR = APB1_CLK / (2 x I2C_speed) = 16,000,000 / (2 x 100,000) = 80
    I2C1->CCR = 80U;

    // 10. Set maximum rise time
    // TRISE = (max_rise_time x APB1_CLK) + 1 = (1000ns x 16MHz) + 1 = 17
    I2C1->TRISE = 17U;

    // 11. Enable I2C
    I2C1->CR1 |= I2C_CR1_PE;
}

/**
 * @brief Generate I2C start condition and send slave address.
 *
 * Initiates communication by generating a START condition, then
 * transmits the 7-bit slave address with the read/write direction bit.
*/
void i2c1_start(uint8_t address, uint8_t direction)
{
    printf("In I2C_Start Driver\n");
    // 1. Generate start condition
    I2C1->CR1 |= I2C_CR1_START;
    while(!(I2C1->SR1 & I2C_SR1_SB));       // Wait until start bit is set

    printf("I2C Start condition okay\n");

    // 2. Send slave address and direction bit
    I2C1->DR = (address << 1) | direction;
    while(!(I2C1->SR1 & I2C_SR1_ADDR));     // Wait until address flag is set
    (void)I2C1->SR2;                        // Clear the address flag
}

/** @brief Generate I2C stop condition to release the bus. */
void i2c1_stop(void)
{
    // Generate stop condition
    I2C1->CR1 |= I2C_CR1_STOP;
}

/** @brief Transmit a single byte to the slave. */
void i2c1_write(uint8_t data)
{
    while(!(I2C1->SR1 & I2C_SR1_TXE));      // Wait until data register is empty
    I2C1->DR = data;                        // Write data to the data register
    while(!(I2C1->SR1 & I2C_SR1_BTF));      // Wait until byte transfer is finished
}

/** @brief Transmit multiple bytes to the slave. */
void i2c1_write_multiple(uint8_t *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        i2c1_write(data[i]);                // Write one byte at a time
    }
}

/** @brief Receive one byte — ack=1 for more bytes, ack=0 for last byte */
uint8_t i2c1_read(uint8_t ack)
{
    // Set ACK or NACK for this byte
    if (ack) {
        I2C1->CR1 |=  I2C_CR1_ACK;           // ACK - more bytes expected
    } else {
        I2C1->CR1 &=~ I2C_CR1_ACK;           // NACK - this is the last byte
    }

    // Wait until DR is full
    while(!(I2C1->SR1 & I2C_SR1_RXNE));

    // Read and return received byte
    return (uint8_t)I2C1->DR;
}

