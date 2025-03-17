#include "../include/lcd.h"
#include "../include/device_driver.h"

#define Lcd_W 240
#define Lcd_H 320

// Pin Map(A Port)

#define Lcd_CS 4
#define Lcd_LED 11

#define Lcd_CS_DIS() Macro_Set_Bit(GPIOA->ODR, Lcd_CS);
#define Lcd_CS_EN() Macro_Clear_Bit(GPIOA->ODR, Lcd_CS);

#define Lcd_LED_ON() Macro_Set_Bit(GPIOA->ODR, Lcd_LED);
#define Lcd_LED_OFF() Macro_Clear_Bit(GPIOA->ODR, Lcd_LED);

// Pin Map(B Port)

#define Lcd_RST 4
#define Lcd_RS 9

#define Lcd_RST_DIS() Macro_Set_Bit(GPIOB->ODR, Lcd_RST);
#define Lcd_RST_EN() Macro_Clear_Bit(GPIOB->ODR, Lcd_RST);

#define Lcd_RS_REG() Macro_Set_Bit(GPIOB->ODR, Lcd_RS);
#define Lcd_RS_DATA() Macro_Clear_Bit(GPIOB->ODR, Lcd_RS);

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    unsigned short color_set[] = {RED, GREEN, BLUE};
    unsigned int font_set[BAND_CHAR_NUM] = {YELLOW, GREEN, BLUE, VIOLET};
    unsigned char band_char[BAND_CHAR_NUM] = {'N', 'L', 'H', 'G'};

    void Lcd_Write_Reg(unsigned char Lcd_Reg, unsigned short Lcd_RegValue);
    void Lcd_Write_Data_16Bit(unsigned short Data);
    void Lcd_WR_REG(unsigned char data);
    void Lcd_WR_DATA(unsigned char data);
    void Lcd_Set_Display_Mode(int mode);

    typedef struct
    {
        unsigned short width;
        unsigned short height;
        unsigned char dir;
        unsigned short wramcmd;
        unsigned short setxcmd;
        unsigned short setycmd;
    } LCD_CONFIG;

    LCD_CONFIG lcddev;

    unsigned short POINT_COLOR = 0x0000;
    unsigned short BACK_COLOR = 0xFFFF;

    static void _SPI1_Init(void)
    {
        Macro_Set_Bit(RCC->APB2ENR, 2);
        Macro_Write_Block(GPIOA->CRL, 0xffff, 0xb4b3, 16);
        Macro_Set_Bit(GPIOA->ODR, 4);

        Macro_Set_Bit(RCC->APB2ENR, 12);
        SPI1->CR1 =
            (0 << 11) | (0 << 10) | (1 << 9) | (1 << 8) | (0 << 7) | (0 << 3) | (1 << 2) | (0 << 1) | (0 << 0); // 36MHz
        Macro_Set_Bit(SPI1->CR1, 6);
    }

    static void _SPI1_Write_Byte(unsigned char data)
    {
        SPI1->DR = (data & 0xff);
        while (Macro_Check_Bit_Clear(SPI1->SR, 1))
            ;
        while (Macro_Check_Bit_Set(SPI1->SR, 7))
            ;
    }

#if 0

static unsigned char _SPI1_Read_Byte(void)
{
	unsigned char data = 0xff;

	for(;;)
	{
		SPI1->DR = 0xff;

		while(Macro_Check_Bit_Clear(SPI1->SR,1));

		if(Macro_Check_Bit_Set(SPI1->SR,0))
		{
			data = (unsigned char)SPI1->DR;
			return data;
		 }
    }

	return 0;
}

#endif

    static void _Delay(int ms)
    {
        volatile int i;
        for (i = 0; i < (0x1000 * ms); i++)
            ;
    }

    // Pin Map(A Port)

#define Lcd_CS 4
#define Lcd_LED 11

    // Pin Map(B Port)

#define Lcd_RST 4
#define Lcd_RS 9

    void Lcd_GPIO_Init(void)
    {
        Macro_Write_Block(GPIOA->CRL, 0xf, 0x3, Lcd_CS * 4);
        Macro_Write_Block(GPIOA->CRH, 0xf, 0x3, (Lcd_LED - 8) * 4);

        Macro_Write_Block(GPIOB->CRL, 0xf, 0x3, Lcd_RST * 4);
        Macro_Write_Block(GPIOB->CRH, 0xf, 0x3, (Lcd_RS - 8) * 4);
    }

    void Lcd_Write_Reg(unsigned char Lcd_Reg, unsigned short Lcd_RegValue)
    {
        Lcd_WR_REG(Lcd_Reg);
        Lcd_WR_DATA(Lcd_RegValue);
    }

    void Lcd_Write_Data_16Bit(unsigned short Data)
    {
        Lcd_CS_EN();
        Lcd_RS_REG();
        _SPI1_Write_Byte(Data >> 8);
        _SPI1_Write_Byte(Data);
        Lcd_CS_DIS();
    }

    void Lcd_Fill(unsigned short sx, unsigned short sy, unsigned short ex, unsigned short ey, unsigned short color)
    {
        unsigned short i, j;
        unsigned short width = ex - sx + 1;
        unsigned short height = ey - sy + 1;

        Lcd_Set_Windows(sx, sy, ex, ey);

        for (i = 0; i < height; i++)
        {
            for (j = 0; j < width; j++)
            {
                Lcd_Write_Data_16Bit(color);
            }
        }

        Lcd_Set_Windows(0, 0, lcddev.width - 1, lcddev.height - 1);
    }

    void Lcd_Reset(void)
    {
        Lcd_RST_EN();
        _Delay(100);
        Lcd_RST_DIS();
        _Delay(50);
    }

    void Lcd_WR_REG(unsigned char data)
    {
        Lcd_CS_EN();
        Lcd_RS_DATA();
        _SPI1_Write_Byte(data);
        Lcd_CS_DIS();
    }

    void Lcd_WR_DATA(unsigned char data)
    {
        Lcd_CS_EN();
        Lcd_RS_REG();
        _SPI1_Write_Byte(data);
        Lcd_CS_DIS();
    }

    void Lcd_Set_Display_Mode(int mode)
    {
        lcddev.dir = mode;
        lcddev.setxcmd = 0x2A;
        lcddev.setycmd = 0x2B;
        lcddev.wramcmd = 0x2C;

        switch (mode)
        {
        case 0:
            lcddev.width = Lcd_W;
            lcddev.height = Lcd_H;
            Lcd_Write_Reg(0x36, (1 << 3) | (0 << 7) | (0 << 6) | (0 << 5)); // BGR==1,MX==0,MY==0,MV==0
            break;
        case 1:
            lcddev.width = Lcd_H;
            lcddev.height = Lcd_W;
            Lcd_Write_Reg(0x36, (1 << 3) | (0 << 7) | (1 << 6) | (1 << 5)); // BGR==1,MX==0,MY==1,MV==1
            break;
        case 2:
            lcddev.width = Lcd_W;
            lcddev.height = Lcd_H;
            Lcd_Write_Reg(0x36, (1 << 3) | (1 << 7) | (1 << 6) | (0 << 5)); // BGR==1,MX==1,MY==1,MV==0
            break;
        case 3:
            lcddev.width = Lcd_H;
            lcddev.height = Lcd_W;
            Lcd_Write_Reg(0x36, (1 << 3) | (1 << 7) | (0 << 6) | (1 << 5)); // BGR==1,MX==1,MY==0,MV==1
            break;
        case 4:
            lcddev.width = Lcd_H;
            lcddev.height = Lcd_W;
            Lcd_Write_Reg(0x36, (1 << 3) | (1 << 7) | (1 << 6) | (1 << 5)); // BGR==1,MX==0,MY==1,MV==1
            break;
        default:
            break;
        }
    }

    void Lcd_Write_RAM_Prepare(void)
    {
        Lcd_WR_REG(lcddev.wramcmd);
    }

    void Lcd_Set_Windows(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2)
    {
        Lcd_WR_REG(lcddev.setxcmd);
        Lcd_WR_DATA(x1 >> 8);
        Lcd_WR_DATA(0xFF & x1);
        Lcd_WR_DATA(x2 >> 8);
        Lcd_WR_DATA(0xFF & x2);

        Lcd_WR_REG(lcddev.setycmd);
        Lcd_WR_DATA(y1 >> 8);
        Lcd_WR_DATA(0xFF & y1);
        Lcd_WR_DATA(y2 >> 8);
        Lcd_WR_DATA(0xFF & y2);

        Lcd_Write_RAM_Prepare();
    }

    void Lcd_Set_Cursor(unsigned short x, unsigned short y)
    {
        Lcd_Set_Windows(x, y, x, y);
    }

    void Lcd_Draw_Back_Color(unsigned short Color)
    {
        unsigned int i;
        unsigned char c0 = Color >> 8;
        unsigned char c1 = Color & 0xFF;

        Lcd_Set_Windows(0, 0, lcddev.width - 1, lcddev.height - 1);

        Lcd_CS_EN();
        Lcd_RS_REG();

        for (i = 0; i < lcddev.height * lcddev.width; i++)
        {
            _SPI1_Write_Byte(c0);
            _SPI1_Write_Byte(c1);
        }

        Lcd_CS_DIS();
    }

    void Lcd_Clr_Screen(void)
    {
        Lcd_Draw_Back_Color(BLACK);
    }

    void Lcd_Draw_Box(int xs, int ys, int w, int h, unsigned short Color)
    {
        int i;
        unsigned char c0 = Color >> 8;
        unsigned char c1 = Color & 0xFF;
        int xe, ye;
        xe = xs + w - 1;
        ye = ys + h - 1;
        if (xe >= 320)
            xe = 319;
        if (ye >= 240)
            xe = 239;

        Lcd_Set_Windows(xs, ys, xe, ye);

        Lcd_CS_EN();
        Lcd_RS_REG();

        for (i = 0; i < (xe - xs + 1) * (ye - ys + 1); i++)
        {
            _SPI1_Write_Byte(c0);
            _SPI1_Write_Byte(c1);
        }

        Lcd_CS_DIS();
    }

#define DIPLAY_MODE 3

    void Lcd_Init(void)
    {
        _SPI1_Init();
        Lcd_GPIO_Init();
        Lcd_LED_OFF();
        Lcd_Reset();

        Lcd_WR_REG(0xCF);
        Lcd_WR_DATA(0x00);
        Lcd_WR_DATA(0xD9); // 0xC1
        Lcd_WR_DATA(0X30);
        Lcd_WR_REG(0xED);
        Lcd_WR_DATA(0x64);
        Lcd_WR_DATA(0x03);
        Lcd_WR_DATA(0X12);
        Lcd_WR_DATA(0X81);
        Lcd_WR_REG(0xE8);
        Lcd_WR_DATA(0x85);
        Lcd_WR_DATA(0x10);
        Lcd_WR_DATA(0x7A);
        Lcd_WR_REG(0xCB);
        Lcd_WR_DATA(0x39);
        Lcd_WR_DATA(0x2C);
        Lcd_WR_DATA(0x00);
        Lcd_WR_DATA(0x34);
        Lcd_WR_DATA(0x02);
        Lcd_WR_REG(0xF7);
        Lcd_WR_DATA(0x20);
        Lcd_WR_REG(0xEA);
        Lcd_WR_DATA(0x00);
        Lcd_WR_DATA(0x00);
        Lcd_WR_REG(0xC0);  // Power control
        Lcd_WR_DATA(0x1B); // VRH[5:0]
        Lcd_WR_REG(0xC1);  // Power control
        Lcd_WR_DATA(0x12); // SAP[2:0];BT[3:0] 0x01
        Lcd_WR_REG(0xC5);  // VCM control
        Lcd_WR_DATA(0x08); // 30
        Lcd_WR_DATA(0x26); // 30
        Lcd_WR_REG(0xC7);  // VCM control2
        Lcd_WR_DATA(0XB7);
        Lcd_WR_REG(0x36); // Memory Access Control
        Lcd_WR_DATA(0x08);
        Lcd_WR_REG(0x3A);
        Lcd_WR_DATA(0x55);
        Lcd_WR_REG(0xB1);
        Lcd_WR_DATA(0x00);
        Lcd_WR_DATA(0x1A);
        Lcd_WR_REG(0xB6); // Display Function Control
        Lcd_WR_DATA(0x0A);
        Lcd_WR_DATA(0xA2);
        Lcd_WR_REG(0xF2); // 3Gamma Function Disable
        Lcd_WR_DATA(0x00);
        Lcd_WR_REG(0x26); // Gamma curve selected
        Lcd_WR_DATA(0x01);
        Lcd_WR_REG(0xE0); // Set Gamma
        Lcd_WR_DATA(0x0F);
        Lcd_WR_DATA(0x1D);
        Lcd_WR_DATA(0x1A);
        Lcd_WR_DATA(0x0A);
        Lcd_WR_DATA(0x0D);
        Lcd_WR_DATA(0x07);
        Lcd_WR_DATA(0x49);
        Lcd_WR_DATA(0X66);
        Lcd_WR_DATA(0x3B);
        Lcd_WR_DATA(0x07);
        Lcd_WR_DATA(0x11);
        Lcd_WR_DATA(0x01);
        Lcd_WR_DATA(0x09);
        Lcd_WR_DATA(0x05);
        Lcd_WR_DATA(0x04);
        Lcd_WR_REG(0XE1); // Set Gamma
        Lcd_WR_DATA(0x00);
        Lcd_WR_DATA(0x18);
        Lcd_WR_DATA(0x1D);
        Lcd_WR_DATA(0x02);
        Lcd_WR_DATA(0x0F);
        Lcd_WR_DATA(0x04);
        Lcd_WR_DATA(0x36);
        Lcd_WR_DATA(0x13);
        Lcd_WR_DATA(0x4C);
        Lcd_WR_DATA(0x07);
        Lcd_WR_DATA(0x13);
        Lcd_WR_DATA(0x0F);
        Lcd_WR_DATA(0x2E);
        Lcd_WR_DATA(0x2F);
        Lcd_WR_DATA(0x05);
        Lcd_WR_REG(0x2B);
        Lcd_WR_DATA(0x00);
        Lcd_WR_DATA(0x00);
        Lcd_WR_DATA(0x01);
        Lcd_WR_DATA(0x3f);
        Lcd_WR_REG(0x2A);
        Lcd_WR_DATA(0x00);
        Lcd_WR_DATA(0x00);
        Lcd_WR_DATA(0x00);
        Lcd_WR_DATA(0xef);
        Lcd_WR_REG(0x11); // Exit Sleep
        _Delay(120);

        Lcd_WR_REG(0x29); // display on

        Lcd_Set_Display_Mode(DIPLAY_MODE);

        Lcd_Clr_Screen();
        _Delay(300);

        Lcd_LED_ON();
    }

    void Lcd_Put_Pixel(unsigned short x, unsigned short y, unsigned short color)
    {
        Lcd_Set_Cursor(x, y);
        Lcd_Write_Data_16Bit(color);
    }

    unsigned char font_5x7[26][5] = {
        // 'A'
        {0x1F, 0x05, 0x05, 0x1F, 0x01},
        // 'B'
        {0x1F, 0x15, 0x15, 0x1A, 0x00},
        // 'C'
        {0x0F, 0x10, 0x10, 0x10, 0x0E},
        // 'D'
        {0x1F, 0x11, 0x11, 0x1E, 0x00},
        // 'E'
        {0x1F, 0x15, 0x15, 0x10, 0x00},
        // 'F'
        {0x1F, 0x05, 0x05, 0x00, 0x00},
        // 'G'
        {0x0F, 0x10, 0x15, 0x1D, 0x00},
        // 'H'
        {0x1F, 0x04, 0x04, 0x1F, 0x00},
        // 'I'
        {0x11, 0x1F, 0x11, 0x00, 0x00},
        // 'J'
        {0x10, 0x10, 0x10, 0x1F, 0x00},
        // 'K'
        {0x1F, 0x04, 0x0A, 0x11, 0x00},
        // 'L'
        {0x1F, 0x10, 0x10, 0x10, 0x00},
        // 'M'
        {0x1F, 0x02, 0x04, 0x02, 0x1F},
        // 'N'
        {0x1F, 0x02, 0x04, 0x08, 0x1F},
        // 'O'
        {0x0F, 0x11, 0x11, 0x0F, 0x00},
        // 'P'
        {0x1F, 0x05, 0x05, 0x02, 0x00},
        // 'Q'
        {0x0F, 0x11, 0x11, 0x0F, 0x01},
        // 'R'
        {0x1F, 0x05, 0x15, 0x1A, 0x00},
        // 'S'
        {0x0E, 0x15, 0x15, 0x12, 0x00},
        // 'T'
        {0x10, 0x10, 0x1F, 0x10, 0x10},
        // 'U'
        {0x1E, 0x01, 0x01, 0x1E, 0x00},
        // 'V'
        {0x1C, 0x02, 0x01, 0x02, 0x1C},
        // 'W'
        {0x1F, 0x01, 0x06, 0x01, 0x1F},
        // 'X'
        {0x11, 0x0A, 0x04, 0x0A, 0x11},
        // 'Y'
        {0x10, 0x08, 0x07, 0x08, 0x10},
        // 'Z'
        {0x11, 0x13, 0x15, 0x19, 0x11}};

    // 문자 그리기 함수
    void Lcd_Draw_Char(unsigned short x, unsigned short y, char c, unsigned short color)
    {
        if (c < 'A' || c > 'Z')
            return; // 문자가 'A' ~ 'Z'가 아니면 그리지 않음

        // 문자에 해당하는 비트맵을 가져옴
        unsigned char *char_bitmap = font_5x7[c - 'A']; // 'A'에서 'A'를 빼면 0이 됨

        // 5x7 비트맵 글자 그리기
        for (unsigned char i = 0; i < 5; i++) // 5개의 열
        {
            for (unsigned char j = 0; j < 7; j++) // 7개의 행
            {
                if (char_bitmap[i] & (1 << j)) // 해당 비트가 1이면
                {
                    Lcd_Put_Pixel(x + i * 2, y + j * 2, color);     // 세로로 두 배 확대
                    Lcd_Put_Pixel(x + i * 2 + 1, y + j * 2, color); // 가로로 두 배 확대
                    Lcd_Put_Pixel(x + i * 2, y + j * 2 + 1, color);
                    Lcd_Put_Pixel(x + i * 2 + 1, y + j * 2 + 1, color);
                }
            }
        }
    }

    void Lcd_Draw_Bar(unsigned short x, unsigned short y, unsigned short height, unsigned short color)
    {
        // draw x(start (x ~ (x+BAR_WIDTH)), (y ~ (y+height)))
        Lcd_Draw_Box(x, y, BAR_WIDTH, height, color);
    }

    void Lcd_Draw_Bar_Graph(const int row[], const int data[], unsigned short data_size)
    {
        // unsigned short x_pos = 0;
        unsigned short color_idx = 0;

        for (unsigned short i = 0; i < data_size; i++)
        {
            short bar_height = (data[i] > MAX_HEIGHT) ? MAX_HEIGHT : data[i];
            if (row[i] < FONT_POS && bar_height >= MAX_HEIGHT - FONT_POS)
            {
                bar_height -= FONT_POS;
            }

            short remaining_height = MAX_HEIGHT - bar_height;

            Lcd_Draw_Bar(row[i], MAX_HEIGHT - bar_height, bar_height, color_set[color_idx]);

            if (remaining_height > 0)
            {
                if (row[i] < FONT_POS)
                {
                    Lcd_Draw_Bar(row[i], FONT_POS, (remaining_height - FONT_POS > 0 ? remaining_height - FONT_POS : 0),
                                 BLACK);
                }
                else
                {
                    Lcd_Draw_Bar(row[i], 0, remaining_height, BLACK);
                }
            }

            if ((i + 1) % (data_size / 3) == 0)
            {
                color_idx = (color_idx + 1) % (sizeof(color_set) / sizeof(color_set[0]));
                // x_pos += 25;
            }
            else
            {
                // x_pos += BAR_WIDTH + BAR_SPACING;
            }
        }
    }

    void Lcd_Draw_Font(unsigned short filterOptions)
    {
        Lcd_Draw_Box(0, 0, FONT_POS, FONT_POS, BLACK);
        Lcd_Draw_Char(FONT_POS / 2, FONT_POS / 2, band_char[filterOptions], font_set[filterOptions]); 
    }
#ifdef __cplusplus
}
#endif /* __cplusplus */