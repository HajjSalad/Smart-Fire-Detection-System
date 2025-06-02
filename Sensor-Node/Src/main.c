
#include "uart.h"
#include "demo.h"
#include "spi.h"
#include "systick.h"
#include "simulate.h"
#include "wrapper.h"

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "stm32f446xx.h"

// Get started:
// 1. Indicate desired # of sensor groups (NUM_GROUPS in simulate.h)
// 2. Add to create desired sensor group to group[] in main.c->generate_sensors()
// Done!

void generate_sensors() 
{
    // Create sensor groups
    group[0] = create_sensor_group("Fire");
    group[1] = create_sensor_group("Environment");
    group[2] = create_sensor_group("Smart");
   
    bool group_creation_failed = false;
    for (int i = 0; i < NUM_GROUPS; i++) {
        if (!group[i]) {
            group_creation_failed = true;
            break;
        }
    }
    if (group_creation_failed) {
        printf("Failed to create sensor group\r\n");
        return;
    }

    // Check the number of sensors for each group
    for (int j = 0; j < NUM_GROUPS; j++){
        groupSensorCount[j] = get_sensor_count(group[j]);
        printf("Sensors for group[%d]: %d sensors\r\n", j, groupSensorCount[j]);
    }
}

int main() {
    uart2_rxtx_init();
    demo_init();   
    spi1_slave_init();
    interrupt_line_init();
    systick_init();

    printf("\nSTM32 Sensor Node Start\r\n");                                    
    printf("Demo Message: %s\r\n", demo_get_message());    // Make sure C++ is working
    printf("\nTesting\r\n");

    srand(time(NULL));
    generate_sensors();
    process_sensor_values();

    while(1) 
    {
        __WFI();
    }
    return 0;
}

















// void trigger_anomaly_interrupt(void) {
//     GPIOB->ODR |= (1 << 6);
//     for (volatile int d = 0; d < 1000; d++);
//     GPIOB->ODR &= ~(1 << 6);
// }

// void test_anomaly_trigger(void) {
//     printf("Set interrupt line high\r\n");
//     GPIOB->ODR |= (1<<6);       // Set interrupt line high
//
//     // Wait until ESP32 triggers a SPI read
//     while (!transfer_complete);
//
//     // Reset flag for next round
//     transfer_complete = 0;
//
//     printf("Received from Master: %s\r\n", rx_buffer);
//     if (strcmp((char*)rx_buffer, "Data Request") == 0) {
//         __attribute__((aligned(4))) float sensor_data[10] = {1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f, 7.7f, 8.8f, 9.9f, 10.10f};
//         prepare_spi_response(RESPONSE_BUFFER, sensor_data, 10, true);
//     } else {
//         prepare_spi_response(RESPONSE_TEXT, "Unknown command", 0, true);
//     }
//
//     //systickDelayMs(50);
//     GPIOB->ODR &= ~(1<<6);      // Set interrupt line low
//    
//     printf("Test completed. Check ESP32 output.\r\n");
// }