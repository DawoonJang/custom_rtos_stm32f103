#pragma once
#include "device_driver.h"

static char interrupt_disable_count;

struct scopedItrLock
{
    scopedItrLock();
    ~scopedItrLock();
};

class InterruptManager
{
  public:
    static void disable_interrupts()
    {
        uintptr_t sp, lr;

        __asm volatile("mov %0, sp" : "=r"(sp));
        __asm volatile("mov %0, lr" : "=r"(lr));

        __set_BASEPRI(0x10);
        ++interrupt_disable_count;

        if (interrupt_disable_count > 1)
        {
            Uart_Printf("Warning: Nested interrupt locks detected! SP: 0x%08X, LR: 0x%08X CNT: %d\n", sp, lr,
                        interrupt_disable_count);
        }
    }

    static void enable_interrupts()
    {
        uintptr_t sp, lr;

        __asm volatile("mov %0, sp" : "=r"(sp));
        __asm volatile("mov %0, lr" : "=r"(lr));

        __set_BASEPRI(0);
        --interrupt_disable_count;

        if (interrupt_disable_count < 0)
        {
            Uart_Printf("Warning: Interrupt unlock called without a matching lock! SP: 0x%08X, LR: 0x%08X, CNT: %d\n",
                        sp, lr, interrupt_disable_count);
        }
    }
};