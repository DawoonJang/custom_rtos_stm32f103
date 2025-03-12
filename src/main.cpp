#include "device_driver.h"
#include "os.h"
#include "task.h"

#include <stdint.h>

extern volatile int Key_Value;
extern volatile int Uart1_Rx_In;
extern volatile int Uart1_Rx_Data;

// void Task1(void *para)
// {
//     Task *tcbPool = rtos.getTCBInfo(0);

//     for (;;)
//     {
//         Uart_Printf("Task1:\n");
//         LED_0_Toggle();
//         rtos.delayByTick(1000);
//     }
// }

// void Task2(void *para)
// {
//     Task *tcbPool = rtos.getTCBInfo(1);

//     for (;;)
//     {
//         Uart_Printf("Task2:\n");
//         LED_1_Toggle();
//         rtos.delayByTick(500);
//     }
// }

int queueID;
extern volatile int keyWaitTaskID;

void Task1(void *para)
{
    int fflag;
    static int cnt;

    for (;;)
    {
        if (rtos.waitSignalForDataTransfer(&fflag, 5000) == true)
        {
            rtos.enQueue(queueID, &cnt);
            LED_1_Toggle();
            cnt++;
        }
        else
        {
            ;
        }
    }
}

void Task2(void *para)
{
    queueID = rtos.createQueue(1, sizeof(int));
    int recvData;

    for (;;)
    {
        if (!rtos.deQueue(queueID, &recvData, 1000))
        {
            LED_0_Toggle();
        }
        else
        {
            Uart_Printf("T2: %d\n", recvData);
        }
    }
}

void Task3(void *para)
{
    int cnt = 0;

    for (;;)
    {
        Uart_Printf("Task3 : %d\n", cnt++);
        rtos.delayByTick(2000);
    }
}

int main(void)
{
    keyWaitTaskID = rtos.createTask(Task1, nullptr, 1, 1024);
    rtos.createTask(Task2, nullptr, 2, 1024);
    // rtos.createTask(Task3, nullptr, 3, 1024, 3);

    rtos.scheduleTask();

    while (1)
    {
        ;
    }

    return 0;
}

void SystemInit()
{
    // Because the debugger switches PLL on, we may
    // need to switch back to the HSI oscillator without PLL

    // Switch to HSI oscillator
    MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_HSI);

    // Wait until the switch is done
    while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != RCC_CFGR_SWS_HSI)
    {
    }

    // Disable the PLL, then we can configure it
    CLEAR_BIT(RCC->CR, RCC_CR_PLLON);

    // Flash latency 2 wait states
    MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, FLASH_ACR_LATENCY_1);

    // Enable HSE oscillator
    SET_BIT(RCC->CR, RCC_CR_HSEON);

    // Wait until HSE oscillator is ready
    while (!READ_BIT(RCC->CR, RCC_CR_HSERDY))
    {
    }

    // 72 MHz using the 8 MHz HSE oscillator with 9x PLL, lowspeed I/O runs at 36 MHz
    WRITE_REG(RCC->CFGR, RCC_CFGR_PLLSRC + RCC_CFGR_PLLMULL9 + RCC_CFGR_PPRE1_DIV2);

    // Enable PLL
    SET_BIT(RCC->CR, RCC_CR_PLLON);

    // Wait until PLL is ready
    while (!READ_BIT(RCC->CR, RCC_CR_PLLRDY))
    {
    }

    // Select PLL as clock source
    MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_PLL);

    // Update variable
    // SystemCoreClock = SYSCLK;
    ClockInit();

    Uart1_Init(115200);

    Key_ISR_Enable(1);
    Uart1_RX_Interrupt_Enable(1);

    /* LED INIT */
    Macro_Set_Bit(RCC->APB2ENR, 3);
    Macro_Write_Block(GPIOB->CRH, 0xff, 0x66, 0);
    Macro_Set_Area(GPIOB->ODR, 0x3, 8);
    /* LED INIT */

    SCB->VTOR = 0x08003000;
    SCB->SHCSR = 7 << 16;
    SCB->AIRCR = 0x05FA0000;
}