#include "device_driver.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    void Invalid_ISR(void)
    {
        Uart_Printf("Invalid_Exception: %d!\n", Macro_Extract_Area(SCB->ICSR, 0x1ff, 0));
        Uart_Printf("Invalid_ISR: %d!\n", Macro_Extract_Area(SCB->ICSR, 0x1ff, 0) - 16);
        for (;;)
            ;
    }

    void NMI_Handler(void)
    {
        Invalid_ISR();
    }

    void HardFault_Handler(void)
    {
        Invalid_ISR();
    }

    void MemManage_Handler(void)
    {
        Uart_Printf("MMF Fault!!\n");
        for (;;)
            ;
    }

    void BusFault_Handler(void)
    {
        Invalid_ISR();
    }

    void UsageFault_Handler(void)
    {
        Invalid_ISR();
    }

    void SVC_Handler(void)
    {
        Invalid_ISR();
    }

    void PendSV_Handler(void)
    {
        Invalid_ISR();
    }

    void SysTick_Handler(void)
    {
        Invalid_ISR();
    }

#ifdef __cplusplus
}
#endif /* __cplusplus */