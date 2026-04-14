/**
 * @file  spi1_driver.c
 * @brief Bare-metal SPI1 peripheral driver for STM32F446RE
*/

#include "stm32f446xx.h"
#include <stdio.h>

/**
 * @brief Bare-metal SPI1 driver - Master mode, 8-bit, Mode 0, 1MHz
 * 
 * SPI1 SCK  - PB3  (AF5)
 * SPI1 MOSI - PA7  (AF5)
 * SPI1 MISO - PA6  (AF5)
 * SPI1 CS   - PC7  (GPIO output, active low)
*/
void spi1_init(void) 
{
    // 1. Enable peripheral clocks used by SPI1
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;         // Enable SPI1 clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;        // Enable GPIOA clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;        // Enable GPIOB clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;        // Enable GPIOC clock

    // 2. PB3, PA7, PA6 for Alternate Function mode
    GPIOB->MODER &=~ (1U<<6);           // PB3 to alternate function mode
    GPIOB->MODER |=  (1U<<7);
    GPIOA->MODER &=~ (1U<<14);          // PA7 to alternate function mode
    GPIOA->MODER |=  (1U<<15);
    GPIOA->MODER &=~ (1U<<12);          // PA6 to alternate function mode
    GPIOA->MODER |=  (1U<<13);

    // 3. PC7 -> GPIO output, idle HIGH
    GPIOC->MODER |=  (1U<<14);          
    GPIOC->MODER &=~ (1U<<15);
    GPIOC->ODR   |=  (1U<<7);           // output data register

    // 3. Set AF5 (SPI1) for SCK, MOSI, MISO
    GPIOB->AFR[0] |= (5<<12);           // PB3
    GPIOA->AFR[0] |= (5<<28);           // PA7
    GPIOA->AFR[0] |= (5<<24);           // PA6

    // 4. Disable SPI1 before configuration
    SPI1->CR1 &=~ SPI_CR1_SPE;

    // 5. Master mode
    SPI1->CR1 |= SPI_CR1_MSTR;

    // 6. Mode 0 - Clock Polarity (CPOL) and Phase (CPHA)
    SPI1->CR1 &=~ SPI_CR1_CPOL;          // CPOL = 0 (clock idle LOW)
    SPI1->CR1 &=~ SPI_CR1_CPHA;          // CPHA = 0 (sample on first edge)

    // 7. Baud rate — APB2/16 = 16MHz/16 = 1MHz
    SPI1->CR1 |= SPI_CR1_BR_1 | SPI_CR1_BR_0;               

    // 8. 8-bit data frame
    SPI1->CR1 &=~ SPI_CR1_DFF;
    
    // 9. Bit order - MSB first
    SPI1->CR1 &=~ SPI_CR1_LSBFIRST;

    // 10. Software CS management — CS controlled manually via PC7
    SPI1->CR1 |= SPI_CR1_SSM;           // software slave management
    SPI1->CR1 |= SPI_CR1_SSI;           // internal slave select high

    // 11. Enable SPI
    SPI1->CR1 |= SPI_CR1_SPE;
}

/** @brief Pull CS low - Select */
void spi1_cs_low(void)
{
    GPIOC->ODR &=~ (1U<<7);     // PC7 low
}

/** @brief Pull CS high - deselect */
void spi1_cs_high(void)
{
    GPIOC->ODR |= (1U<<7);     // PC7 high
}

/**
 * @brief Transmit and receive one byte over SPI1
 * 
 * Full duplex SPI - every transmit is simultaneously 
 * a receive
*/
uint8_t spi1_transmit_receive(uint8_t data)
{
    // Wait until TX buffer is empty
    while(!(SPI1->SR & SPI_SR_TXE));

    // Send byte
    SPI1->DR = data;

    // Wait until RX buffer has data
    while(!(SPI1->SR & SPI_SR_RXNE));

    return (uint8_t)SPI1->DR;
}

/**
 * @brief Write one byte to a BME680 register
 * 
 * Used for config - set oversampling, mode, filter etc
 * MSB = 0 indicates write operation to BME680
*/
void spi1_write_reg(uint8_t reg, uint8_t data)
{
    spi1_cs_low();
    spi1_transmit_receive(reg & 0x7F);      // MSB=0 -> write
    spi1_transmit_receive(data);
    spi1_cs_high();
}

/**
 * @brief Read one byte from a BME680 register
 * 
 * MSB = 1 indicate read operation to BME680
 * First byte selects register, second byte clocks out the data
*/
uint8_t spi1_read_reg(uint8_t reg)
{
    uint8_t data;

    spi1_cs_low();
    spi1_transmit_receive(reg & 0x80);      // MSB=1 -> write
    data = spi1_transmit_receive(0x00);     // dummy byte -> clock out data
    spi1_cs_high();

    return data;
}

/**
 * @brief Read multiple consecutive registers
 * 
 * Keep CS low for the entire burst read
 * Sends register start address once, then clocks out len bytes
 * BME680 auto-increments register pointer after each byte clocked out
*/
void spi1_read_regs(uint8_t reg, uint8_t *buf, uint8_t len)
{
    spi1_cs_low();
    spi1_transmit_receive(reg | 0x80);  // MSB=1 -> read, set start address

    for (uint8_t i = 0; i < len; i++){
        buf[i] = spi1_transmit_receive(0x00);   // clock out each byte
    }

    spi1_cs_high();
}