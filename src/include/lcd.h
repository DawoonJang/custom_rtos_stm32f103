#pragma once
#include "dsp.h"

#define BLACK 0x0000
#define WHITE 0xFFFF
#define GRAY 0x8410       // 중간 회색
#define DARK_GRAY 0x4208  // 어두운 회색
#define LIGHT_GRAY 0xC618 // 밝은 회색

#define RED 0xF800
#define DARK_RED 0x8000  // 어두운 빨강
#define LIGHT_RED 0xF810 // 밝은 빨강

#define GREEN 0x07E0
#define DARK_GREEN 0x03E0  // 어두운 초록
#define LIGHT_GREEN 0x87F0 // 밝은 초록

#define BLUE 0x001F
#define DARK_BLUE 0x000F  // 어두운 파랑
#define LIGHT_BLUE 0x051F // 밝은 파랑

#define YELLOW 0xFFE0
#define GOLD 0xFEA0   // 황금색
#define ORANGE 0xFD20 // 주황

#define CYAN 0x07FF       // 청록색
#define DARK_CYAN 0x03EF  // 어두운 청록
#define LIGHT_CYAN 0x67FF // 밝은 청록

#define MAGENTA 0xF81F // 자홍색
#define PINK 0xF97F    // 연한 분홍
#define PURPLE 0x801F  // 보라
#define VIOLET 0x915F  // 연보라

#define BROWN 0xA145 // 갈색
#define BEIGE 0xF7BB // 베이지

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