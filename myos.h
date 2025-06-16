#ifndef MYOS_H
#define MYOS_H

#include <stdint.h>
#include <ti/devices/msp/m0p/mspm0l130x.h>

typedef struct {
    void *sp;
} OSThread;

typedef void (*Handler)();

void OS_init(void);

void OS_sched(void);

void OSThread_start(
    OSThread *th,
    Handler threadHandler,
    void *stkSto, uint32_t stkSize);
void OS_start(void);
void OS_StartUp(void);

#endif // MYOS_H
