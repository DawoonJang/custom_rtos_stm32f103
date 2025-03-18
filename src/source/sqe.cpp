#include "sqe.h"
#include "arm_math.h"
#include "device_driver.h"
#include "lcd.h"

extern volatile int Uart1_Rx_In;
extern volatile bool keyInputFlag;

volatile char signalMemoryMutexID;

static float pSrc[FFT_LENGTH];
static float pSrcTemp[FFT_LENGTH];
static float pSrcFiltered[FFT_LENGTH];

static float pDst_real[FFT_LENGTH];
static float pDst_imag[FFT_LENGTH];

static short magnitude[FFT_HALF_LENGTH];
static short freqs[FFT_HALF_LENGTH];

#ifdef TESTCASE2

struct Boxes
{
    unsigned short x, y, w, h;
    int color;
};

Boxes templateBoxes[FFT_HALF_LENGTH];
volatile char queueBoxes;
volatile bool queueSignal;

void init_templateBoxes()
{
    unsigned short barWidth = MAX_WIDTH / (FFT_HALF_LENGTH);
    for (int i = 0; i < FFT_HALF_LENGTH; ++i)
    {
        templateBoxes[i].x = i * barWidth;
        templateBoxes[i].w = barWidth;

        if (i < FFT_HALF_LENGTH / 8)
        {
            templateBoxes[i].color = DARK_BLUE; // 짙은 파랑
        }
        else if (i < FFT_HALF_LENGTH * 2 / 8)
        {
            templateBoxes[i].color = BLUE; // 파랑
        }
        else if (i < FFT_HALF_LENGTH * 3 / 8)
        {
            templateBoxes[i].color = CYAN; // 청록
        }
        else if (i < FFT_HALF_LENGTH * 4 / 8)
        {
            templateBoxes[i].color = GREEN; // 초록
        }
        else if (i < FFT_HALF_LENGTH * 5 / 8)
        {
            templateBoxes[i].color = BROWN; // 노랑
        }
        else if (i < FFT_HALF_LENGTH * 6 / 8)
        {
            templateBoxes[i].color = BEIGE; // 주황
        }
        else if (i < FFT_HALF_LENGTH * 7 / 8)
        {
            templateBoxes[i].color = RED; // 빨강
        }
        else
        {
            templateBoxes[i].color = MAGENTA; // 자홍
        }
    }
}

void canvasGKTask(void *para)
{
    Boxes obj;
    keyInputFlag = 1;
    init_templateBoxes();

    queueBoxes = rtos.createQueue(2 * FFT_LENGTH, sizeof(Boxes));

    while (1)
    {
        if (rtos.deQueue(queueBoxes, &obj, 100))
        {
            Lcd_Draw_Box(obj.x, obj.y, obj.w, obj.h, obj.color);
        }

        if (keyInputFlag)
        {
            keyInputFlag = 0;
            Lcd_Draw_Font(dsp.filterOption);
        }
    }
}

void signalTask(void *para)
{
    while (1)
    {
        rtos.lockMutex(signalMemoryMutexID);

        for (size_t i = 0; i < FFT_LENGTH; ++i)
        {
            pSrc[i] = 0.5 * arm_sin_f32((2 * PI * 1906 * i) / SAMPLE_RATE) +
                      0.75 * arm_sin_f32((2 * PI * (SIGNAL_FREQ / 2) * i) / SAMPLE_RATE) +
                      2 * arm_sin_f32((2 * PI * (SIGNAL_FREQ / 8) * i) / SAMPLE_RATE) +
                      1.5 * arm_sin_f32((2 * PI * (SIGNAL_FREQ / 4) * i) / SAMPLE_RATE) +
                      arm_sin_f32((2 * PI * 1500 * i) / SAMPLE_RATE);
        }

        rtos.delay(500);

        rtos.unlockMutex(signalMemoryMutexID);
    }
}

void draw_line(short *fftData, short maxMagnitude)
{
    for (int i = 0; i < FFT_HALF_LENGTH; ++i)
    {
        Boxes obj;
        obj.x = templateBoxes[i].x;
        obj.w = templateBoxes[i].w;
        obj.y = templateBoxes[i].y;
        obj.h = templateBoxes[i].h;
        obj.color = BLACK;
        rtos.enQueue(queueBoxes, &obj);

        int normalizedHeight = Y_MIN + ((Y_MAX - Y_MIN) * fftData[i]) / maxMagnitude;
        obj.x = templateBoxes[i].x;
        obj.w = templateBoxes[i].w;
        obj.y = Y_MAX - normalizedHeight;
        obj.h = normalizedHeight + 1;
        obj.color = templateBoxes[i].color;

        templateBoxes[i].y = obj.y;
        templateBoxes[i].h = obj.h;
        rtos.enQueue(queueBoxes, &obj);
    }
}

void dspTask(void *para)
{
    short maxMagnitude;

    rtos.delay(10);

    while (1)
    {

        rtos.lockMutex(signalMemoryMutexID);

        switch (dsp.filterOption)
        {
        case FilterOption::Normal:
            dsp.FFT(pSrc, pDst_real, pDst_imag, FFT_LENGTH);
            break;

        case FilterOption::LPF:
            // dsp.FIR_Filter(pSrc, pSrcFiltered, FFT_LENGTH, dsp.FIR_LPF_Coefficients_575);
            dsp.IIR_Filter(pSrc, pSrcFiltered, FFT_LENGTH, dsp.IIR_LPF_B_Coef_575, dsp.IIR_LPF_A_Coef_575);
            dsp.FFT(pSrcFiltered, pDst_real, pDst_imag, FFT_LENGTH);
            break;

        case FilterOption::HPF:
            // dsp.FIR_Filter(pSrc, pSrcFiltered, FFT_LENGTH, dsp.FIR_HPF_Coefficients_1200);
            dsp.IIR_Filter(pSrc, pSrcFiltered, FFT_LENGTH, dsp.IIR_HPF_B_Coef_1200, dsp.IIR_HPF_A_Coef_1200);
            dsp.FFT(pSrcFiltered, pDst_real, pDst_imag, FFT_LENGTH);
            break;

        case FilterOption::BPF:
            // dsp.FIR_Filter(pSrc, pSrcTemp, FFT_LENGTH, dsp.FIR_LPF_Coefficients_1200);
            // dsp.FIR_Filter(pSrcTemp, pSrcFiltered, FFT_LENGTH, dsp.FIR_HPF_Coefficients_575);

            dsp.IIR_Filter(pSrc, pSrcTemp, FFT_LENGTH, dsp.IIR_HPF_B_Coef_575, dsp.IIR_HPF_A_Coef_575);
            dsp.IIR_Filter(pSrcTemp, pSrcFiltered, FFT_LENGTH, dsp.IIR_LPF_B_Coef_1200, dsp.IIR_LPF_A_Coef_1200);
            dsp.FFT(pSrcFiltered, pDst_real, pDst_imag, FFT_LENGTH);
            break;

        default:
            break;
        }

        rtos.unlockMutex(signalMemoryMutexID);

        maxMagnitude = 0;

        for (size_t i = 0; i < FFT_HALF_LENGTH; i++)
        {
            freqs[i] = (double)i * SAMPLE_RATE / FFT_LENGTH;
            magnitude[i] = sqrt(pDst_real[i] * pDst_real[i] + pDst_imag[i] * pDst_imag[i]);

            if (magnitude[i] > maxMagnitude)
                maxMagnitude = magnitude[i];

            // Uart_Printf("%d: %d_%d\n", i, freqs[i], magnitude[i]);
        }
        draw_line(magnitude, maxMagnitude);

        rtos.delay(300);
    }
}

#elif defined(TESTCASE3)

void Task1(void *para)
{
    volatile int j;
    rtos.delay(500);

    Uart_Printf("\nTask1 Lock\n");

    rtos.lockMutex(mutexID);

    for (j = 0; j < 10; j++)
    {
        systemDelay(250);

        LED_1_Toggle();
    }

    Uart_Printf("\nTask1 unLock\n");
    rtos.unlockMutex(mutexID);

    for (;;)
    {
        rtos.delay(500);
    }
}

void Task2(void *para)
{
    rtos.delay(1000);
    Uart_Printf("\nTask2 : Run!\n");

    for (;;)
    {
        systemDelay(1000);

        Uart_Printf(".");
    }
}

void dspTask(void *para)
{
    volatile int j;
    mutexID = rtos.createMutex();

    Uart_Printf("\ndspTask Lock\n");

    rtos.lockMutex(mutexID);

    for (j = 0; j < 10; j++)
    {

        systemDelay(250);

        LED_0_Toggle();
    }

    Uart_Printf("\ndspTask unLock\n");
    rtos.unlockMutex(mutexID);

    for (;;)
    {
        rtos.delay(1000);
        Uart_Printf("3\n");
    }
}

#endif

void developmentVerify(void)
{
#ifdef TESTCASE2

    rtos.createTask(signalTask, nullptr, 1, 2048);
    rtos.createTask(canvasGKTask, nullptr, 3, 2048);
    rtos.createTask(dspTask, nullptr, 2, 2048);

#elif defined(TESTCASE3)

    rtos.createTask(Task1, nullptr, 1, 1024);
    rtos.createTask(Task2, nullptr, 2, 1024);
    rtos.createTask(dspTask, nullptr, 3, 1024);

#endif
}