#include "device_driver.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

    void Uart1_Init(int baud)
    {
        double div;
        unsigned int mant;
        unsigned int frac;

        Macro_Set_Bit(RCC->APB2ENR, 2);
        Macro_Set_Bit(RCC->APB2ENR, 14);
        Macro_Write_Block(GPIOA->CRH, 0xff, 0x8a, 4);
        Macro_Set_Bit(GPIOA->ODR, 10);

        div = PCLK2 / (16. * baud);
        mant = (int)div;
        frac = (int)((div - mant) * 16. + 0.5);
        mant += frac >> 4;
        frac &= 0xf;

        USART1->BRR = (mant << 4) + (frac << 0);
        USART1->CR1 = (1 << 13) | (0 << 12) | (0 << 10) | (1 << 3) | (1 << 2);
        USART1->CR2 = 0 << 12;
        USART1->CR3 = 0;
    }

    void Uart1_Send_Byte(char data)
    {
        if (data == '\n')
        {
            while (Macro_Check_Bit_Clear(USART1->SR, 7))
                ;
            USART1->DR = 0x0d;
        }

        while (Macro_Check_Bit_Clear(USART1->SR, 7))
            ;
        USART1->DR = data;
    }

    void Uart1_Send_String(char *pt)
    {
        while (*pt != 0)
        {
            Uart1_Send_Byte(*pt++);
        }
    }

    void ftos(char *str, float fl)
    {
        char buf[20];
        long long i = (long long)fl; // 정수 부분
        float f = fl - i;            // 소수 부분
        int idx = 0;

        // 음수 처리
        if (fl < 0)
        {
            str[idx++] = '-';
            i = -i; // 정수 부분 양수화
            f = -f; // 소수 부분 양수화
        }

        // 정수 부분을 문자열로 변환
        if (i == 0)
        {
            str[idx++] = '0';
        }
        else
        {
            int int_start = idx; // 숫자 시작 위치 저장
            while (i)
            {
                buf[idx - int_start] = i % 10 + '0';
                i /= 10;
                idx++;
            }
            // 숫자 반전 (거꾸로 저장되어 있기 때문)
            for (int j = 0; j < (idx - int_start) / 2; j++)
            {
                char temp = buf[j];
                buf[j] = buf[idx - int_start - 1 - j];
                buf[idx - int_start - 1 - j] = temp;
            }
            // 복사
            for (int j = 0; j < idx - int_start; j++)
            {
                str[int_start + j] = buf[j];
            }
        }

        str[idx++] = '.'; // 소수점 추가

        // 소수 부분 변환 (5자리까지)
        for (int j = 0; j < 5; j++)
        {
            f *= 10;
            int digit = (int)f;
            str[idx++] = digit + '0';
            f -= digit;
        }

        str[idx] = '\0'; // 문자열 종료
    }
    void Uart1_PrintStr2(float *real, float *imag, size_t length)
    {
        char real_char[20];
        char imag_char[20];

        for (size_t i = 0; i < length; i++)
        {
            ftos(real_char, real[i]);
            ftos(imag_char, imag[i]);

            Uart_Printf("%s\n", real_char);
            // Uart_Printf("%s+j%s\n", real_char, imag_char);
        }
        // Uart1_Printf("\n");
    }

    void Uart1_PrintFloat(float *data, size_t length)
    {
        char float_char[10];

        for (size_t i = 0; i < length; i++)
        {
            ftos(float_char, data[i]);

            Uart_Printf("%s\n", float_char);
        }
    }

    void Uart1_Printf(const char *fmt, ...)
    {
#ifdef DEBUG_PRINT
        va_list ap;
        char string[256];

        va_start(ap, fmt);
        vsprintf(string, fmt, ap);
        Uart1_Send_String(string);
        va_end(ap);
#endif
    }

    char Uart1_Get_Pressed(void)
    {
        if (Macro_Check_Bit_Set(USART1->SR, 5))
        {
            return (char)USART1->DR;
        }
        else
        {
            return (char)0;
        }
    }

    char Uart1_Get_Char(void)
    {
        char rx;

        do
        {
            rx = Uart1_Get_Pressed();
        } while (!rx);

        return rx;
    }

    void Uart1_Get_String(char *string)
    {
        char *string2 = string;
        char c;

        while ((c = Uart1_Get_Char()) != '\r')
        {
            if (c == '\b')
            {
                if ((int)string2 < (int)string)
                {
                    Uart1_Printf("\b \b");
                    string--;
                }
            }

            else
            {
                *string++ = c;
                Uart1_Send_Byte(c);
            }
        }

        *string = '\0';
        Uart1_Send_Byte('\n');
    }

    int Uart1_Get_Int_Num(void)
    {
        char str[30];
        char *string = str;
        int base = 10;
        int minus = 0;
        int result = 0;
        int lastIndex;
        int i;

        Uart1_Get_String(string);

        if (string[0] == '-')
        {
            minus = 1;
            string++;
        }

        if (string[0] == '0' && (string[1] == 'x' || string[1] == 'X'))
        {
            base = 16;
            string += 2;
        }

        lastIndex = strlen(string) - 1;

        if (lastIndex < 0)
            return -1;

        if (string[lastIndex] == 'h' || string[lastIndex] == 'H')
        {
            base = 16;
            string[lastIndex] = 0;
            lastIndex--;
        }

        if (base == 10)
        {
            result = atoi(string);
            result = minus ? (-1 * result) : result;
        }

        else
        {
            for (i = 0; i <= lastIndex; i++)
            {
                if (isalpha((int)string[i]))
                {
                    if (isupper((int)string[i]))
                        result = (result << 4) + string[i] - 'A' + 10;
                    else
                        result = (result << 4) + string[i] - 'a' + 10;
                }

                else
                {
                    result = (result << 4) + string[i] - '0';
                }
            }

            result = minus ? (-1 * result) : result;
        }

        return result;
    }

    void Uart1_RX_Interrupt_Enable(int en)
    {
        if (en)
        {
            Macro_Set_Bit(USART1->CR1, 5);
            NVIC_ClearPendingIRQ((IRQn_Type)37);
            NVIC_EnableIRQ((IRQn_Type)37);
        }

        else
        {
            Macro_Clear_Bit(USART1->CR1, 5);
            NVIC_DisableIRQ((IRQn_Type)37);
        }
    }

#ifdef __cplusplus
}
#endif