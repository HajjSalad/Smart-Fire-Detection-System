/**
 * @file  iwdg_driver.c
 * @brief Independent Watchdog peripheral Implementation.
 * 
 * Uses LSI (32kHz) internal clock - independent of system clock
 * Timeout = (prescaler * reload) / 32000
 * Prescaler = 256, Reload = 1250 → 10 second timeout
*/

#include "iwdg_driver.h"
#include "stm32f446xx.h"

#define IWDG_KEY_UNLOCK     0x5555U     // unlock PR and RLR registers
#define IWDG_KEY_KICK       0xAAAAU     // reload counter - kick 
#define IWDG_KEY_START      0xCCCCU     // start IWDG 
#define IWDG_RELOAD         1250U

/**
 * @brief Initialize and start the Independent Watchdog
 * 
 * Once started, the IWDG cannot be stopped.
 * iwdg_kick() must be called within 10 secs or MCU resets.
*/
void iwdg_init() 
{
    // 1. Start LSI oscillator
    RCC->CSR |= RCC_CSR_LSION;
    while(!(RCC->CSR & RCC_CSR_LSIRDY));       // wait until LSI stable

    // 2. Unlock PR and RLR registers
    IWDG->KR = IWDG_KEY_UNLOCK;

    // 3. Set prescaler - /256
    IWDG->PR = 6U;

    // 4. Set reload value - 1250
    IWDG->RLR = IWDG_RELOAD;

    // 5. Start IWDG
    IWDG->KR = IWDG_KEY_START;
}

/**
 * @brief Kick the watchdog - reset the countdown timer
 * 
 * Must be called within 10 seonds by the watchdog monitor task.
 * If not called - MCU resets automatically.
*/
void iwdg_kick(void) 
{
    IWDG->KR = IWDG_KEY_KICK;
}