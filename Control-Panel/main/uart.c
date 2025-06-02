/**
 * @file uart.c
 * @brief UART initialization for ESP32 using ESP-IDF.
 *
 * This source file configures and initializes UART0 for serial communication.
 * It sets baud rate, pins, and buffer sizes for RX and TX.
 */

#include "soc/uart_struct.h"
#include "driver/uart.h"
#include "esp_system.h"
#include "sdkconfig.h"
#include "esp_log.h"

#define UART_0_TX              1       ///< UART TX pin number
#define UART_0_RX              3       ///< UART RX pin number
#define RX_BUF_SIZE            256     ///< UART RX buffer size
#define TX_BUF_SIZE            256     ///< UART TX buffer size
#define UART_BAUD_RATE         115200  ///< UART communication baud rate

/**
 * @brief Initialize UART0 with default configuration.
 *
 * This function configures UART0 with 8 data bits, 1 stop bit, no parity, 
 * no flow control, and a baud rate of 115200. It also sets the TX and RX pins,
 * installs the UART driver, and allocates RX and TX buffers.
 */
void uart_init(void) {
    // Configure UART Parameters
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    uart_param_config(UART_NUM_0, &uart_config);  
    uart_set_pin(UART_NUM_0, UART_0_TX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE); 
    uart_driver_install(UART_NUM_0, RX_BUF_SIZE, TX_BUF_SIZE, 0, NULL, 0);
}