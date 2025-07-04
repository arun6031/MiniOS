#include "myos.h"
#include "stddef.h"
#include <stdint.h> // Include for uint32_t
#define MAX_THREAD 32
#define IDLE_STACK_SIZE 64
 OSThread * volatile OS_curr; /* pointer to the current thread */
 OSThread * volatile OS_next; /* pointer to the next thread to run */
 OSThread* OS_thread[32+1];
 uint32_t OS_threadNum;
 uint32_t current_id;
 uint32_t idle_stack[IDLE_STACK_SIZE];
 uint32_t OS_readySet;
 OSThread idle;

 void idle_thread(){
     while(1){

        __asm volatile("WFI"); //Waiting for interrupt
     }
 }

void OS_init(void) {


   *(uint32_t volatile *)0xE000ED20 |= (0xFFU << 16); // SHPR3 register, bits[23:16] for PendSV
    OSThread_start(&idle,&idle_thread,idle_stack,sizeof(idle_stack));
}


void OS_start(void){
    __disable_irq();
    OS_sched();
    __enable_irq();
}


void OS_sched(void) {
    if (OS_readySet == 0U)
    {
        current_id = 0U;
    }
      else {
            do {
                ++current_id;
                if (current_id == OS_threadNum)
                {
                   current_id = 1U;
                }
            } while ((OS_readySet & (1U << (current_id - 1U))) == 0U);
        }
        OS_next = OS_thread[current_id];
        if (OS_next != OS_curr) { *(uint32_t volatile *)0xE000ED04 = (1U << 28); }
}

void OSThread_start(
    OSThread *th,
    Handler threadHandler,
    void *stkSto, uint32_t stkSize)
{

    uint32_t *sp = (uint32_t *)((((uint32_t)stkSto + stkSize) / 8) * 8);
    uint32_t *stk_limit;

    // Simulate the exception stack frame pushed by hardware
    *(--sp) = (1U << 24);             /* xPSR (T-bit set for Thumb state) */
    *(--sp) = (uint32_t)threadHandler|1U; /* PC (start address of the thread function) */
    *(--sp) = 0x00010001U;            /* LR (EXC_RETURN value: return to Thread mode, use PSP) */
    *(--sp) = 0x0000000CU;            /* R12 */
    *(--sp) = 0x00000003U;            /* R3  */
    *(--sp) = 0x00000002U;            /* R2  */
    *(--sp) = 0x00000001U;            /* R1  */
    *(--sp) = 0x00000000U;            /* R0  */

    /* Additionally, fake registers R4-R11 */
    *(--sp) = 0x0000000BU; /* R11 */
    *(--sp) = 0x0000000AU; /* R10 */
    *(--sp) = 0x00000009U; /* R9 */
    *(--sp) = 0x00000008U; /* R8 */
    *(--sp) = 0x00000007U; /* R7 */
    *(--sp) = 0x00000006U; /* R6 */
    *(--sp) = 0x00000005U; /* R5 */
    *(--sp) = 0x00000004U; /* R4 */


    th->sp = sp;


    stk_limit = (uint32_t *)(((((uint32_t)stkSto - 1U) / 8) + 1U) * 8);

    for (sp = sp - 1U; sp >= stk_limit; --sp) {
        *sp = 0xDEADBEEFU;

    }
    if (OS_threadNum >= (MAX_THREAD + 1))
    {
        return;
    }
      th->timeout = 0U;
      OS_thread[OS_threadNum] = th;

       if (OS_threadNum > 0U)
       {
           OS_readySet |= (1U << (OS_threadNum - 1U));
       }
          ++OS_threadNum;

}
void OS_tick(void) {
    uint8_t i;
    for (i = 1U; i < OS_threadNum; ++i) {
        OSThread *p = OS_thread[i];
        if (p->timeout != 0U) {
            --p->timeout;
            if (p->timeout == 0U ) {
                OS_readySet |= (1U << (i - 1U));
            }
        }
    }
}

void OS_delay(uint32_t ticks)
{
    __asm volatile ("cpsid i");
    OS_curr->timeout = ticks;
    OS_readySet &= ~(1U << (current_id - 1U));
    OS_sched();
    __asm volatile ("cpsie i");
}


__attribute__ ((naked))
void PendSV_Handler(void) {
__asm volatile (

    "  CPSID           I                  \n"

    /* if (OS_curr != (OSThread *)0) { */
    "  LDR             r1,=OS_curr        \n"
    "  LDR             r1,[r1,#0x00]      \n"
    "  CMP             r1,#0              \n"
    "  BEQ             PendSV_restore     \n"

    /* push registers r4-r11 on the stack */

    "  SUB             sp,sp,#(8*4)      \n"
    "  MOV             r0,sp             \n"
    "  STMIA           r0!,{r4-r7}       \n"
    "  MOV             r4,r8             \n"
    "  MOV             r5,r9             \n"
    "  MOV             r6,r10            \n"
    "  MOV             r7,r11            \n"
    "  STMIA           r0!,{r4-r7}       \n" // save the high registers


    /* OS_curr->sp = sp; */
    "  LDR             r1,=OS_curr        \n"
    "  LDR             r1,[r1,#0x00]      \n"
    "  MOV             r0,sp              \n"
    "  STR             r0,[r1,#0x00]      \n"
    /* } */

    "PendSV_restore:                    \n"
    /* sp = OS_next->sp; */
    "  LDR             r1,=OS_next        \n"
    "  LDR             r1,[r1,#0x00]      \n"
    "  LDR             r0,[r1,#0x00]      \n"
    "  MOV             sp,r0              \n"

    /* OS_curr = OS_next; */
    "  LDR             r1,=OS_next        \n"
    "  LDR             r1,[r1,#0x00]      \n"
    "  LDR             r2,=OS_curr        \n"
    "  STR             r1,[r2,#0x00]      \n"

    /* pop registers r4-r11 */
    "  MOV             r0,sp              \n" // r0 := top of stack
    "  MOV             r2,r0              \n"
    "  ADDS            r2,r2,#(4*4)      \n" // point r2 to the 4 high registers r7-r11 (assuming r4-r7 were pushed first by STMIA)
    "  LDMIA           r2!,{r4-r7}       \n" // pop the 4 high registers into low registers
    "  MOV             r8,r4              \n" // move low registers into high registers
    "  MOV             r9,r5              \n"
    "  MOV             r10,r6             \n"
    "  MOV             r11,r7             \n"
    "  LDMIA           r0!,{r4-r7}       \n" // pop the low registers
    "  ADD             sp,sp,#(8*4)      \n" // remove 8 registers from the stack

    "  CPSIE           I                  \n"

    "  BX              lr                  \n"
    );
}
