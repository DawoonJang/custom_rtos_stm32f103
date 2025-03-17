#include "../include/device_driver.h"

void SystemInit()
{
    // Because the debugger switches PLL on, we may
    // need to switch back to the HSI oscillator without PLL

    // // Switch to HSI oscillator
    // MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_HSI);

    // // Wait until the switch is done
    // while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != RCC_CFGR_SWS_HSI)
    // {
    // }

    // // Disable the PLL, then we can configure it
    // CLEAR_BIT(RCC->CR, RCC_CR_PLLON);

    // // Flash latency 2 wait states
    // MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, FLASH_ACR_LATENCY_1);

    // // Enable HSE oscillator
    // SET_BIT(RCC->CR, RCC_CR_HSEON);

    // // Wait until HSE oscillator is ready
    // while (!READ_BIT(RCC->CR, RCC_CR_HSERDY))
    // {
    // }

    // // 72 MHz using the 8 MHz HSE oscillator with 9x PLL, lowspeed I/O runs at 36 MHz
    // WRITE_REG(RCC->CFGR, RCC_CFGR_PLLSRC + RCC_CFGR_PLLMULL9 + RCC_CFGR_PPRE1_DIV2);

    // // Enable PLL
    // SET_BIT(RCC->CR, RCC_CR_PLLON);

    // // Wait until PLL is ready
    // while (!READ_BIT(RCC->CR, RCC_CR_PLLRDY))
    // {
    // }

    // // Select PLL as clock source
    // MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_PLL);

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

    LED_All_Off();
    /* LED INIT */

    /* LCD_INIT */
    Lcd_Init();
    /* LCD_INIT */

    /* Buzzer INIT */
    TIM3_Out_Init();

    SCB->VTOR = 0x08003000;
    SCB->SHCSR = 7 << 16;
    SCB->AIRCR = 0x05FA0000;
}

void systemDelay(uint32_t msec)
{
    scopedItrLock lock;

    for (uint32_t j = 0; j < 2000UL * msec; j++)
    {
        __NOP();
    }
}

void TIM3_Out_Init(void)
{
    Macro_Set_Bit(RCC->APB1ENR, 1);
    Macro_Set_Bit(RCC->APB2ENR, 3);
    Macro_Write_Block(GPIOB->CRL, 0xf, 0xb, 0);
    Macro_Write_Block(TIM3->CCMR2, 0x7, 0x6, 4);
    TIM3->CCER = (0 << 9) | (1 << 8);
}

void TIM3_Out_Freq_Generation(unsigned short freq)
{
    TIM3->PSC = (unsigned int)(TIMXCLK / (double)TIM3_FREQ + 0.5) - 1;
    TIM3->ARR = (double)TIM3_FREQ / freq - 1;
    TIM3->CCR3 = TIM3->ARR / 2;

    Macro_Set_Bit(TIM3->EGR, 0);
    TIM3->CR1 = (1 << 4) | (0 << 3) | (0 << 1) | (1 << 0);
}

void TIM3_Out_Stop(void)
{
    Macro_Clear_Bit(TIM3->CR1, 0);
    Macro_Clear_Bit(TIM3->DIER, 0);
}