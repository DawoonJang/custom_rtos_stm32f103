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

#ifdef TESTCASE2

#define FFT_LENGTH 64
#define SAMPLE_RATE 512
#define SIGNAL_FREQ 16

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

void draw_line(int *fftData, int maxMagnitude)
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
            Uart_Printf("TP: %d_%d_%d_%d_%d\n", obj.x, obj.y, obj.w, obj.h, obj.color);

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

#define PI 3.14159265358979f
#define Q15_SCALE (1 << 15) // 2^15
#define FLOAT_TO_Q15(x) ((q15_t)((x) * Q15_SCALE))

int FFT(long N, const double *pSrc, double *pDstReal, double *pDstImag)
{
    /* 1. check 2^n count -> if else then return -1 */
    if ((N != 0) && ((N & (N - 1)) != 0))
        return -1;

    /* 2. initialize destination arrays */
    for (long i = 0; i < N; ++i)
    {
        pDstReal[i] = pSrc[i];
        pDstImag[i] = 0.0;
    }

    /* 3. calculate W */
    long N2 = N >> 1;
    double WR[N2], WI[N2]; /* Real and Imaginary part */
    double T = 2 * PI / N;
    for (long i = 0; i < N2; ++i)
    {
        WR[i] = cos(T * i);
        WI[i] = -sin(T * i);
    }

    /* 4. shuffle input array index to calculate bottom-up style FFT */
    int log2N = (int)log2(N);
    double tmp;
    for (long n = 1; n < N - 1; ++n)
    {
        long m = 0; /* reversed num of n*/
        for (int i = 0; i < log2N; ++i)
        {
            m |= ((n >> i) & 1) << (log2N - i - 1); /* reverse the bits */
        }
        if (n < m)
        { /* exchange */
            tmp = pDstReal[n];
            pDstReal[n] = pDstReal[m];
            pDstReal[m] = tmp;
        }
    }

    /* 5. execute fft */
    for (int loop = 0; loop < log2N; ++loop)
    {
        long regionSize = 1 << (loop + 1);    /* if N=8: 2 -> 4 -> 8 */
        long kJump = 1 << (log2N - loop - 1); /* if N=8: 4 -> 2 -> 1 */
        long half = regionSize >> 1;
        for (long i = 0; i < N; i += regionSize)
        {
            long blockEnd = i + half - 1;
            long k = 0;
            double TR, TI;
            for (long j = i; j <= blockEnd; ++j)
            { /* j start from i */
                TR = WR[k] * pDstReal[j + half] - WI[k] * pDstImag[j + half];
                TI = WI[k] * pDstReal[j + half] + WR[k] * pDstImag[j + half];

                pDstReal[j + half] = pDstReal[j] - TR;
                pDstImag[j + half] = pDstImag[j] - TI;

                pDstReal[j] = pDstReal[j] + TR;
                pDstImag[j] = pDstImag[j] + TI;

                k += kJump;
            }
        }
    }
    return 0;
}

#define FILTER_ORDER 32 // FIR 필터 계수 개수

float LPF_Coefficients[FILTER_ORDER] = {0.0022,  0.0042,  0.0061,  0.0076,  0.0087,  0.0091,  0.0087,  0.0076,
                                        0.0061,  0.0042,  0.0022,  0.0001,  -0.0019, -0.0037, -0.0053, -0.0067,
                                        -0.0076, -0.0081, -0.0081, -0.0076, -0.0067, -0.0053, -0.0037, -0.0019,
                                        0.0001,  0.0022,  0.0042,  0.0061,  0.0076,  0.0087,  0.0091,  0.0087}; // 20Hz

float HPF_Coefficients[FILTER_ORDER] = {0.0006,  0.0013,  0.0024,  0.0038,  0.0054,  0.0065,  0.0060,  0.0027,
                                        -0.0067, -0.0042, -0.0153, -0.0305, -0.0488, -0.0685, -0.0874, -0.1031,
                                        0.8825,  -0.1135, -0.1031, -0.0874, -0.0685, -0.0488, -0.0305, -0.0153,
                                        -0.0042, 0.0027,  0.0060,  0.0065,  0.0054,  0.0038,  0.0024,  0.0013}; // 30Hz

void LPF(double *input, double *output, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        output[i] = 0.0;
        for (size_t j = 0; j < FILTER_ORDER; j++)
        {
            if (i >= j)
            {
                output[i] += LPF_Coefficients[j] * input[i - j];
            }
        }
    }
}

void HPF(double *input, double *output, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        output[i] = 0.0;
        for (size_t j = 0; j < FILTER_ORDER; j++)
        {
            if (i >= j)
            {
                output[i] += HPF_Coefficients[j] * input[i - j];
            }
        }
    }
}

volatile char filterOptions;

void dspTask(void *para)
{
    static short prevfilterOptions;

    double pSrc[FFT_LENGTH];
    double pSrcFiltered[FFT_LENGTH];

    double pDst_real[FFT_LENGTH];
    double pDst_imag[FFT_LENGTH];

    int magnitude[FFT_LENGTH / 2];
    int freqs[FFT_LENGTH / 2];
    int maxMagnitude = 1;

    for (size_t i = 0; i < FFT_LENGTH; ++i)
    {
        pSrc[i] = sin((2 * PI * SIGNAL_FREQ * i) / SAMPLE_RATE) + 0.5 * sin((2 * PI * 64 * i) / SAMPLE_RATE) +
                  2 * sin((2 * PI * 200 * i) / SAMPLE_RATE);
    }

    while (1)
    {
        // memset(pDst_real, 0, sizeof(pDst_real));
        // memset(pDst_imag, 0, sizeof(pDst_imag));

        // Uart_Printf("TP: %d\n", filterOptions);

        switch (filterOptions)
        {
        case 0:
            FFT(FFT_LENGTH, pSrc, pDst_real, pDst_imag);
            break;

        case 1:
            LPF(pSrc, pSrcFiltered, FFT_LENGTH);
            FFT(FFT_LENGTH, pSrcFiltered, pDst_real, pDst_imag);
            break;

        case 2:
            HPF(pSrc, pSrcFiltered, FFT_LENGTH);
            FFT(FFT_LENGTH, pSrcFiltered, pDst_real, pDst_imag);
            break;

        case 3:
            FFT(FFT_LENGTH, pSrc, pDst_real, pDst_imag);
            break;

        default:
            break;
        }

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

        rtos.delay(1000);
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