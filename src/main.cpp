#include "device_driver.h"
#include "os.h"
#include <stdint.h>

extern volatile int Key_Value;
extern volatile int Uart1_Rx_In;
extern volatile int Uart1_Rx_Data;

void delay(uint32_t msec)
{
    for (uint32_t j = 0; j < 2000UL * msec; j++)
    {
        __NOP();
    }
}

void Task1(void *para)
{
    for (;;)
    {
        Uart_Printf("Task1:\n");
        LED_0_Toggle();
        rtos.TickDelay(1000);
    }
}

void Task2(void *para)
{
    for (;;)
    {
        Uart_Printf("Task2:\n");
        LED_1_Toggle();
        rtos.TickDelay(500);
    }
}

void Task3(void *para)
{
    int cnt = 0;
    for (;;)
    {
        Uart_Printf("Task3 : %d\n", cnt++);
        rtos.TickDelay(2000);
    }
}

// volatile int a = 100;

int main(void)
{
    rtos.CreateTask(Task1, nullptr, 1, 2048);
    rtos.CreateTask(Task2, nullptr, 1, 2048);
    rtos.CreateTask(Task3, nullptr, 1, 2048);

    rtos.Scheduling();

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
    LED_Init();
    Key_ISR_Enable(1);
    Uart1_RX_Interrupt_Enable(1);

    SCB->VTOR = 0x08003000;
    SCB->SHCSR = 7 << 16;
    SCB->AIRCR = 0x05FA0000;
}