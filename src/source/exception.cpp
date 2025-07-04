#include "device_driver.h"
#include "ltr.h"
#include "os.h"
#include "sqe.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    void Invalid_ISR(void)
    {
        Uart1_Printf("Invalid_Exception: %d!\n", Macro_Extract_Area(SCB->ICSR, 0x1ff, 0));
        Uart1_Printf("Invalid_ISR: %d!\n", Macro_Extract_Area(SCB->ICSR, 0x1ff, 0) - 16);
        for (;;)
            ;
    }

    static const char *const Stack_reg[] = {"R0", "R1", "R2", "R3", "R12", "LR", "RA", "xPSR"};

    static void Stack_Dump(const char *stack, unsigned int *sp)
    {
        unsigned int i;

        for (i = 0; i < (sizeof(Stack_reg) / sizeof(Stack_reg[0])); i++)
        {
            Uart1_Printf("%s[%d],%s=0x%.8X\n", stack, i, Stack_reg[i], sp[i]);
        }
    }

    static void Fault_Report(unsigned int *msp, unsigned int lr, unsigned int *psp)
    {
        Uart1_Printf("SCB->ICSR=0x%.8X\n", SCB->ICSR);
        Uart1_Printf("LR(EXC_RETURN)=0x%.8X\n", lr);
        Uart1_Printf("MSP=0x%.8X\n", msp);
        Uart1_Printf("PSP=0x%.8X\n", psp);

        switch ((lr & (0xF << 28)) | (lr & 0xF))
        {
        case 0xF0000001:
            Uart1_Printf("Exception occurs from handler mode\n");
            Stack_Dump("MSP", msp);
            break;

        case 0xF0000009:
            Uart1_Printf("Exception occurs from thread mode with MSP\n");
            Stack_Dump("MSP", msp);
            break;

        case 0xF000000d:
            Uart1_Printf("Exception occurs from thread mode with PSP\n");
            Stack_Dump("PSP", psp);
            break;

        default:
            Uart1_Printf("Invalid exception return value => %#.8X\n", lr & 0xf);
            break;
        }

        Uart1_Printf("SHCSR => %#.8X\n", SCB->SHCSR);
        Uart1_Printf("CFSR(Fault Reason) => %#.8X\n", SCB->CFSR);
    }

    void NMI_Handler(void)
    {
        Invalid_ISR();
    }

    void HardFault_Handler(unsigned int *msp, unsigned int lr, unsigned int *psp)
    {
        Uart1_Printf("Hard Fault!\n");
        Uart1_Printf("HFSR(Hard Fault Reason) => %#.8X\n", SCB->HFSR);

        Fault_Report(msp, lr, psp);

        Uart1_Printf("MMFAR => %#.8X\n", SCB->MMFAR);
        Uart1_Printf("BFAR => %#.8X\n", SCB->BFAR);

        for (;;)
            ;
    }

    void MemManage_Handler(unsigned int *msp, unsigned int lr, unsigned int *psp)
    {
        Uart1_Printf("Memory Management Fault!\n");

        Fault_Report(msp, lr, psp);

        Uart1_Printf("MMFAR Valid => %d\n", Macro_Check_Bit_Set(SCB->CFSR, 7));
        Uart1_Printf("MMFAR => %#.8X\n", SCB->MMFAR);

        for (;;)
            ;
    }

    void BusFault_Handler(unsigned int *msp, unsigned int lr, unsigned int *psp)
    {
        Uart1_Printf("Bus Fault!\n");

        Fault_Report(msp, lr, psp);

        Uart1_Printf("BFAR Valid => %d\n", Macro_Check_Bit_Set(SCB->CFSR, 15));
        Uart1_Printf("BFAR => %#.8X\n", SCB->BFAR);

        for (;;)
            ;
    }

    void UsageFault_Handler(unsigned int *msp, unsigned int lr, unsigned int *psp)
    {
        Uart1_Printf("Usage Fault!\n");

        Fault_Report(msp, lr, psp);

        for (;;)
            ;
    }

    void SysTick_Handler(void)
    {
        scopedItrLock lock;

        rtos.increaseTick();
        trigger_context_switch();
    }

    volatile int keyValue;
    extern volatile int keyWaitTaskID;
    volatile bool keyInputFlag;

    void EXTI9_5_IRQHandler(void)
    {
        scopedItrLock lock;
        keyValue = Macro_Extract_Area(EXTI->PR, 0x3, 6);
        EXTI->PR = 0x3 << 6;
        NVIC_ClearPendingIRQ((IRQn_Type)23);

        keyInputFlag = 1;
        dsp.changeFilterOption();
        // rtos.sendSignal(keyWaitTaskID, keyValue);
    }

    volatile char Uart1_Rx_Data;
    extern volatile char queueSignal;
    extern volatile char targetSignal;
    void USART1_IRQHandler(void)
    {
        scopedItrLock lock;

        Uart1_Rx_Data = (char)USART1->DR;
        NVIC_ClearPendingIRQ((IRQn_Type)37);
        // Uart_Printf("tp:1 %d\n", Uart1_Rx_Data);
        targetSignal = (char)(Uart1_Rx_Data & 0x0F);
        dsp.filterType = (char)((Uart1_Rx_Data >> 4) & 0x0F);
        dsp.prevfilterOption = FilterOption::None;
        dsp.filterOption = FilterOption::Normal;
        keyInputFlag = 1;

    }

#ifdef __cplusplus
}
#endif /* __cplusplus */