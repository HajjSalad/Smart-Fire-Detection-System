
#include "soc/uart_struct.h"
#include "driver/uart.h"
#include "esp_system.h"
#include "sdkconfig.h"
#include "esp_log.h"

#define UART_0_TX              1       // TX pin
#define UART_0_RX              3       // RX pin
#define RX_BUF_SIZE            256
#define TX_BUF_SIZE            256
#define UART_BAUD_RATE         115200
#define UART_TASK_STACK_SIZE   3072

// Initialize UART
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