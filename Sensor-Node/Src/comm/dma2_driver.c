/**
 * @file  dma2_driver.c
 * @brief DMA2 driver for SPI1 RX/TX transfers
 * 
 * DMA2 Stream 0 Channel 3 - SPI1 RX (peripheral-to-memory)
 * DMA2 Stream 3 Channel 3 - SPI1 TX (memory-to-peripheral)
*/

#include "dma2_driver.h"
#include "stm32f446xx.h"

/**
 * @brief Initialize DMA2 for SPI1 RX/TX transfers
 */
void dma2_init() 
{
    // 1. Enable DMA2 clock
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;

    /* --- Stream 0 - SPI1 RX (peripheral-to-memory) --- */

    // 2. Disable stream before configuring
    DMA2_Stream0->CR &=~ DMA_SxCR_EN;
    while (DMA2_Stream0->CR & DMA_SxCR_EN);     // wait until disabled

    // 3. Clear all interrupt flags for Stream 0
    DMA2->LIFCR |= DMA_LIFCR_CTCIF0                 // transfer complete
                |  DMA_LIFCR_CHTIF0                 // half transfer
                |  DMA_LIFCR_CTEIF0                 // transfer error
                |  DMA_LIFCR_CDMEIF0                // direct mode error
                |  DMA_LIFCR_CFEIF0;                // FIFO error

    // 4. Set peripheral addess - SPI1 data register
    DMA2_Stream0->PAR = (uint32_t)&SPI1->DR;

    // 5. Configure Stream 0
    DMA2_Stream0->CR  = 0U;
    DMA2_Stream0->CR |= (3U << DMA_SxCR_CHSEL_Pos)      // channel 3
                     |  (0U << DMA_SxCR_DIR_Pos)        // periph-to-memory
                     |  DMA_SxCR_MINC                   // memory increment
                     |  (0U << DMA_SxCR_MSIZE_Pos)      // 8-bit memory
                     |  (0U << DMA_SxCR_PSIZE_Pos);     // 8-bit peripheral

    /* --- Stream 3 - SPI1 TX (memory-to-peripheral) --- */

    // 2. Disable stream before configuring
    DMA2_Stream3->CR &=~ DMA_SxCR_EN;
    while (DMA2_Stream3->CR & DMA_SxCR_EN);         // wait until disabled

    // 3. Clear all interrupt flags for Stream 3
    DMA2->LIFCR |= DMA_LIFCR_CTCIF3                 // transfer complete
                |  DMA_LIFCR_CHTIF3                 // half transfer
                |  DMA_LIFCR_CTEIF3                 // transfer error
                |  DMA_LIFCR_CDMEIF3                // direct mode error
                |  DMA_LIFCR_CFEIF3;                // FIFO error

    // 4. Set peripheral address - SPI1 data register
    DMA2_Stream3->PAR = (uint32_t)&SPI1->DR;

    // 5. Configure Stream 3
    DMA2_Stream0->CR  = 0U;
    DMA2_Stream3->CR |= (3U << DMA_SxCR_CHSEL_Pos)      // channel 3
                     |  (1U << DMA_SxCR_DIR_Pos)        // memory-to-peripheral
                     |  DMA_SxCR_MINC                   // memory increment
                     |  (0U << DMA_SxCR_MSIZE_Pos)      // 8-bit memory
                     |  (0U << DMA_SxCR_PSIZE_Pos);     // 8-bit peripheral
}