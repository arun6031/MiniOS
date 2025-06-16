/**
 * main.c
 */
#include "myos.h"
#include <ti/devices/msp/m0p/mspm0l130x.h>
#include <stdint.h> // Include for uint32_t

uint32_t volatile vtick_count = 0;
void delay(uint32_t t);
uint32_t mut(void);

#define STACK_SIZE 128 // A bit more stack for safety (adjust as needed)
uint32_t stack1[STACK_SIZE];
uint32_t stack2[STACK_SIZE];

void SystickConfig()
{
    // SysTick interrupt priority should be higher than PendSV
    // 0U is the highest priority (lowest numerical value)
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
        GPIOA->DOUT31_0 |= (1U << 27);  // Turn on LED1 (PA27)


        // No delay here, as the scheduler will preempt
        delay(500);
         // Turn off LED1 (PA27)
        GPIOA->DOUT31_0 &= ~(1U << 27);
        delay(500);
    }
}

OSThread LED2;
void LED2_task() {
    while (1) {

        GPIOA->DOUT31_0 |= (1U << 26);
        delay(100);
        // No delay here, as the scheduler will preempt
        GPIOA->DOUT31_0 &= ~(1U << 26); // Turn off LED2 (PA26)

        delay(100);
    }
}


int main(void)
{


    GPIOConfig();


    // Initialize both threads with their own stacks
    OSThread_start(&LED1, &LED1_task, stack1, sizeof(stack1));
    OSThread_start(&LED2, &LED2_task, stack2, sizeof(stack2));

    OS_start();

    return 0;
}
void OS_StartUp(){
    OS_init();
    SystickConfig();
}

void SysTick_Handler()
{
    vtick_count++;
    // Interrupts are automatically disabled on entry to ISR by hardware
    // You re-enable them later in PendSV. Disabling them here is redundant
    // and might cause issues if not carefully managed.
    // The PendSV handler itself needs to manage interrupt state.
    // For a typical RTOS, SysTick merely pends PendSV.

    // If SysTick triggers, and then you call OS_sched, OS_sched will
    // pend the PendSV. The PendSV, having lower priority, will run
    // after SysTick_Handler completes.
   // SCB->ICSR |= (1U << 28); // Pend PendSV here directly

    // __disable_irq(); // Removed - interrupts are already disabled by hardware on ISR entry
     OS_sched(); // This function calls the PendSV pend bit.
    // __enable_irq(); // Removed - re-enabled by PendSV_Handler at the end
}

uint32_t mut()
{
    __asm("cpsid i"); // Disable interrupts
    uint32_t tick_count = vtick_count;
    __asm("cpsie i"); // Enable interrupts
    return tick_count;
}

void delay(uint32_t ticks)
{
    uint32_t start = mut();
    while ((mut() - start) < ticks);
}
