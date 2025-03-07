#include "macro.h"
#include "malloc.h"
#include "option.h"
#include "stm32f103xb.h"
#include "stm32f1xx.h"

// Led.c
extern void LED_Init(void);
extern void LED_Display(unsigned int num);
extern void LED_All_On(void);
extern void LED_All_Off(void);

// Uart.c
#define Uart_Init Uart1_Init
#define Uart_Send_Byte Uart1_Send_Byte
#define Uart_Send_String Uart1_Send_String
#define Uart_Printf Uart1_Printf

extern void Uart1_Init(int baud);
extern void Uart1_Send_Byte(char data);
extern void Uart1_Send_String(char *pt);
extern void Uart1_Printf(char *fmt, ...);

// Clock.c
extern void Clock_Init(void);

// Key.c
extern void Key_Poll_Init(void);
extern int Key_Get_Pressed(void);
extern void Key_Wait_Key_Released(void);
extern int Key_Wait_Key_Pressed(void);

// Asm_Function.s
extern uint32_t __get_IPSR(void);
