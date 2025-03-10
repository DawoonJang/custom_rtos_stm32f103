#include "macro.h"
#include "malloc.h"
#include "option.h"
#include "os.h"
#include "stm32f103xb.h"
#include "stm32f1xx.h"

// Uart.c
#define Uart_Init Uart1_Init
#define Uart_Send_Byte Uart1_Send_Byte
#define Uart_Send_String Uart1_Send_String
#define Uart_Printf Uart1_Printf

#define DISABLE_INTERRUPTS() (__set_BASEPRI(0x10))
#define ENABLE_INTERRUPTS() (__set_BASEPRI(0))
#define RUN_CONTEXT_SWITCH() (SCB->ICSR = 1 << 28)

extern LivingRTOS rtos;

#ifdef __cplusplus
extern "C"
{
#endif
    // Led.c
    extern void LED_Init(void);
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

    // Clock.c
    extern void ClockInit(void);

    // Key.c
    extern void Key_Poll_Init(void);
    extern int Key_Get_Pressed(void);
    extern void Key_Wait_Key_Released(void);
    extern int Key_Wait_Key_Pressed(void);
    extern void Key_ISR_Enable(int);

    // Asm_Function.s
    extern uint32_t __get_IPSR(void);

    extern void _OS_Start_First_Task(void);

#ifdef __cplusplus
}
#endif