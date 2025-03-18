#pragma once

#include "macro.h"
#include "malloc.h"
#include "option.h"
#include "os.h"
#include "stm32f103xb.h"
#include "stm32f1xx.h"
#include <queue>

// Uart.c
#define Uart_Init Uart1_Init
#define Uart_Send_Byte Uart1_Send_Byte
#define Uart_Send_String Uart1_Send_String
#define Uart_Printf Uart1_Printf
#define Uart_PrintFloat Uart1_PrintFloat

static inline void disable_interrupts(void)
{
    __set_BASEPRI(0x10);
}

static inline void enable_interrupts(void)
{
    __set_BASEPRI(0);
}

static inline void trigger_context_switch(void)
{
    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
}

extern LivingRTOS rtos;

#ifdef __cplusplus
extern "C"
{
#endif
    // Led.c
    extern void LED_Display(unsigned int num);
    extern void LED_All_On(void);
    extern void LED_All_Off(void);
    extern void LED_0_Toggle(void);
    extern void LED_1_Toggle(void);

    extern void Uart1_Init(int baud);
    extern void Uart1_Send_Byte(char data);
    extern void Uart1_Send_String(char *pt);
    extern void Uart1_Printf(const char *fmt, ...);
    extern char Uart1_Get_Char(void);
    extern char Uart1_Get_Pressed(void);
    extern void Uart1_Get_String(char *string);
    extern int Uart1_Get_Int_Num(void);
    extern void Uart1_RX_Interrupt_Enable(int en);
    extern void Uart1_PrintStr2(float *real, float *imag, size_t length);
    extern void Uart1_PrintFloat(float *data, size_t length);

    // Clock.c
    extern void ClockInit(void);

    // Key.c
    extern void Key_Poll_Init(void);
    extern int Key_Get_Pressed(void);
    extern void Key_Wait_Key_unlockd(void);
    extern int Key_Wait_Key_Pressed(void);
    extern void Key_ISR_Enable(int);

    // Asm_Function.s
    extern uint32_t __get_IPSR(void);

    void SystemInit(void);

    void systemDelay(uint32_t msec);

    void TIM3_Out_Init(void);
    void TIM3_Out_Freq_Generation(unsigned short freq);
    void TIM3_Out_Stop(void);

    // lcd.c
    extern void Lcd_Init(void);
    
#ifdef __cplusplus
}
#endif

struct scopedItrLock
{
    scopedItrLock()
    {
        disable_interrupts();
    }
    ~scopedItrLock()
    {
        enable_interrupts();
    }
};