#include "../include/device_driver.h"

void LED_Display(unsigned int num)
{
    Macro_Write_Block(GPIOB->ODR, 0x3, (~num & 3), 8);
}

void LED_All_On(void)
{
    Macro_Clear_Area(GPIOB->ODR, 0x3, 8);
}

void LED_All_Off(void)
{
    Macro_Set_Area(GPIOB->ODR, 0x3, 8);
}

void LED_0_Toggle(void)
{
    Macro_Invert_Bit(GPIOB->ODR, 8);
}

void LED_1_Toggle(void)
{
    Macro_Invert_Bit(GPIOB->ODR, 9);
}
