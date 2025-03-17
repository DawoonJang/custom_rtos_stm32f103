#pragma once
#include "dsp.h"

#define BLACK 0x0000
#define WHITE 0xffff
#define BLUE 0x001f
#define GREEN 0x07e0
#define RED 0xf800
#define YELLOW 0xffe0
#define VIOLET 0xf81f

#define BAR_WIDTH 3
#define BAR_SPACING 0
#define MAX_HEIGHT 240
#define MAX_WIDTH 320

#define FONT_POS 20
#define BAND_CHAR_NUM 4

#define X_MIN (0)
#define X_MAX (MAX_WIDTH - 1)
#define Y_MIN (0)
#define Y_MAX (MAX_HEIGHT - 1)
#define COLOR_BACK (0x0)

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    extern unsigned short color_set[];
    extern unsigned char band_char[];

    extern void Lcd_Init(void);
    extern void Lcd_Set_Display_Mode(int mode);
    extern void Lcd_Set_Cursor(unsigned short x, unsigned short y);
    extern void Lcd_Set_Windows(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2);
    extern void Lcd_Put_Pixel(unsigned short x, unsigned short y, unsigned short color);
    extern void Lcd_Clr_Screen(void);
    extern void Lcd_Draw_Back_Color(unsigned short color);
    extern void Lcd_Write_Data_16Bit(unsigned short color);
    extern void Lcd_Draw_Box(int xs, int ys, int w, int h, unsigned short Color);
    void Lcd_Draw_Bar_Graph(const int row[], const int data[], unsigned short data_size);
    void Lcd_Draw_Char(unsigned short x, unsigned short y, char c, unsigned short color);
    void Lcd_Draw_Bar(unsigned short x, unsigned short y, unsigned short height, unsigned short color);
    void Lcd_Draw_Font(FilterOption filterOptions);

#ifdef __cplusplus
}
#endif /* __cplusplus */