#include "sqe.h"
#include "arm_math.h"
#include "device_driver.h"
#include "lcd.h"

extern volatile int Key_Value;
extern volatile int Uart1_Rx_In;
volatile int keyWaitTaskID;

volatile int signalQueueID;
volatile int uartQueueID;
volatile int mutexID;
volatile FilterOption filterOptions;

#ifdef TESTCASE2

struct Boxes
{
    unsigned short x, y, w, h;
    int color;
};

Boxes templateBoxes[FFT_LENGTH / 2];

volatile int queueBoxes;

void init_templateBoxes()
{
    unsigned short barWidth = MAX_WIDTH / (FFT_LENGTH / 2);
    for (int i = 0; i < FFT_LENGTH / 2; ++i)
    {
        templateBoxes[i].x = i * barWidth;
        templateBoxes[i].w = barWidth;
        templateBoxes[i].color = RED;
    }
}

void draw_line(short *fftData, short maxMagnitude)
{
    for (int i = 0; i < FFT_LENGTH / 2; ++i)
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

void canvasGKTask(void *para)
{
    Boxes obj;
    init_templateBoxes();
    queueBoxes = rtos.createQueue(2 * FFT_LENGTH, sizeof(Boxes));
    while (1)
    {
        if (rtos.deQueue(queueBoxes, &obj, 100))
        {
            Lcd_Draw_Box(obj.x, obj.y, obj.w, obj.h, obj.color);
        }
    }
}

// void Task1(void *para)
// {
//     int fflag;
//     static char cnt;

//     for (;;)
//     {
//         if (rtos.waitForSignal(&fflag, 5000) == true)
//         {
//             LED_1_Toggle();
//             cnt++;
//             Uart_Printf("%d Button_Pressed\n", cnt);
//         }
//         else
//         {
//             ;
//         }
//     }
// }

void Task2(void *para)
{
    uartQueueID = rtos.createQueue(4, sizeof(char));
    signalQueueID = rtos.createQueue(1, sizeof(char));
    unsigned short recvByte;
    unsigned short recvSignal;
    int shortFlag = 0;

    for (;;)
    {
        if (!rtos.deQueue(uartQueueID, &recvByte, 1000))
        {
            LED_0_Toggle();
        }
        else
        {
            if (!shortFlag)
            {
                recvSignal = (unsigned char)recvByte;
            }
            else
            {
                recvSignal |= ((unsigned char)recvByte << 8);
                // TIM3_Out_Freq_Generation(recvSignal);
                // Uart_Printf("Received: %d\n", recvSignal);
                // TIM3_Out_Stop();
            }
            systemDelay(50);
            shortFlag ^= 1;
        }
    }
}

float pSrc[FFT_LENGTH];
float pSrcTemp[FFT_LENGTH];
float pSrcFiltered[FFT_LENGTH];

float pDst_real[FFT_LENGTH];
float pDst_imag[FFT_LENGTH];

short magnitude[FFT_LENGTH / 2];
int freqs[FFT_LENGTH / 2];
short maxMagnitude;

void dspTask(void *para)
{

    for (size_t i = 0; i < FFT_LENGTH; ++i)
    {
        pSrc[i] = 0.5 * sin((2 * PI * SIGNAL_FREQ * i) / SAMPLE_RATE) + sin((2 * PI * 64 * i) / SAMPLE_RATE) +
                  2 * sin((2 * PI * 200 * i) / SAMPLE_RATE) + sin((2 * PI * SIGNAL_FREQ * 3 * i) / SAMPLE_RATE) +
                  1.5 * sin((2 * PI * SIGNAL_FREQ * 6 * i) / SAMPLE_RATE);
    }

    while (1)
    {
        switch (dsp.filterOption)
        {
        case FilterOption::Normal:
            dsp.FFT(pSrc, pDst_real, pDst_imag, FFT_LENGTH);
            break;

        case FilterOption::LPF:
            dsp.FIR_Filter(pSrc, pSrcFiltered, FFT_LENGTH, dsp.LPF_Coefficients_20);
            dsp.FFT(pSrcFiltered, pDst_real, pDst_imag, FFT_LENGTH);
            break;

        case FilterOption::HPF:
            dsp.FIR_Filter(pSrc, pSrcFiltered, FFT_LENGTH, dsp.HPF_Coefficients_200);
            dsp.FFT(pSrcFiltered, pDst_real, pDst_imag, FFT_LENGTH);
            break;

        case FilterOption::BPF:
            dsp.FIR_Filter(pSrc, pSrcTemp, FFT_LENGTH, dsp.LPF_Coefficients_20);
            dsp.FIR_Filter(pSrcTemp, pSrcFiltered, FFT_LENGTH, dsp.HPF_Coefficients_80);
            dsp.FFT(pSrcFiltered, pDst_real, pDst_imag, FFT_LENGTH);
            break;

        default:
            break;
        }

        maxMagnitude = 1;
        for (size_t i = 0; i < FFT_LENGTH / 2; i++)
        {

            freqs[i] = (double)i * SAMPLE_RATE / FFT_LENGTH;
            magnitude[i] = sqrt(pDst_real[i] * pDst_real[i] + pDst_imag[i] * pDst_imag[i]);

            if (magnitude[i] > maxMagnitude)
                maxMagnitude = magnitude[i];

            // Uart_Printf("%d: %d_%d\n", i, freqs[i], magnitude[i]);
        }
        // Uart_Printf("\n");
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

    // keyWaitTaskID = rtos.createTask(Task1, nullptr, 2, 2048);
    rtos.createTask(canvasGKTask, nullptr, 2, 2048);
    keyWaitTaskID = rtos.createTask(dspTask, nullptr, 1, 2048);

#elif defined(TESTCASE3)

    rtos.createTask(Task1, nullptr, 1, 1024);
    rtos.createTask(Task2, nullptr, 2, 1024);
    rtos.createTask(dspTask, nullptr, 3, 1024);

#endif
}