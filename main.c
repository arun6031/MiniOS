/**
 * main.c
 */
#include "myos.h"
#include <ti/devices/msp/m0p/mspm0l130x.h>
#include "stdint.h"

uint32_t volatile vtick_count = 0;
void delay(uint32_t t);
uint32_t mut(void);

#define STACK_SIZE 128
uint32_t stack1[STACK_SIZE];
uint32_t stack2[STACK_SIZE];

void SystickConfig()
{
    NVIC_SetPriority(SysTick_IRQn, 0U);
    SysTick->LOAD = 32000 - 1; // Assuming 32MHz clock for 1ms tick (32000 cycles)
    SysTick->VAL = 0x0;        // Clear current value
    SysTick->CTRL = (1U << 2) | (1U << 1) | (1U << 0); // CLKSOURCE (CPU clock), TICKINT (enable interrupt), ENABLE (enable counter)
}

void GPIOConfig()
{


    GPIOA->GPRCM.PWREN = (GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE); // Enable power to GPIOA
    IOMUX->SECCFG.PINCM[27] = 0x81; // Assuming PINCM[27] for PA27, set to GPIO output (0x81 usually means default GPIO mode, output)
    IOMUX->SECCFG.PINCM[26] = 0x81; // Assuming PINCM[26] for PA26, set to GPIO output

    GPIOA->DOE31_0 |= (1U << 27) | (1U << 26); // Set PA27 and PA26 as output
}

OSThread LED1;
void LED1_task() {
    while (1) {
        GPIOA->DOUT31_0 |= (1U << 27);
        OS_delay(500);
        GPIOA->DOUT31_0 &= ~(1U << 27);
        OS_delay(500);
    }
}

OSThread LED2;
void LED2_task() {
    while (1) {

        GPIOA->DOUT31_0 |= (1U << 26);
        OS_delay(100);
        GPIOA->DOUT31_0 &= ~(1U << 26);
        OS_delay(100);
    }
}


int main(void)
{

    GPIOConfig();
    OS_init();
    SystickConfig();
    OSThread_start(&LED1, &LED1_task, stack1, sizeof(stack1));
    OSThread_start(&LED2, &LED2_task, stack2, sizeof(stack2));

    OS_start();

    return 0;
}


void SysTick_Handler()
{
    __asm volatile ("cpsid i");
       OS_tick();
       __asm volatile ("cpsie i");
       OS_sched();

}



