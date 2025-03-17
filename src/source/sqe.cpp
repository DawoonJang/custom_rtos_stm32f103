#include "sqe.h"
#include "arm_math.h"
#include "device_driver.h"

extern volatile int Key_Value;
extern volatile int Uart1_Rx_In;
extern volatile int keyWaitTaskID;

volatile int signalQueueID;
volatile int uartQueueID;
volatile int mutexID;

#ifdef TESTCASE2

void Task1(void *para)
{
    int fflag;
    static char cnt;

    for (;;)
    {
        if (rtos.waitForSignal(&fflag, 5000) == true)
        {
            LED_1_Toggle();
            cnt++;
            Uart_Printf("%d Button_Pressed\n", cnt);
        }
        else
        {
            ;
        }
    }
}

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
                Uart_Printf("Received: %d\n", recvSignal);
                // TIM3_Out_Stop();
            }
            systemDelay(50);
            shortFlag ^= 1;
        }
    }
}

#define FFT_LENGTH 64
#define SAMPLE_RATE 512
#define SIGNAL_FREQ 16

#define PI 3.14159265358979f
#define Q15_SCALE (1 << 15) // 2^15
#define Q31_SCALE (1 << 31)
#define FLOAT_TO_Q15(x) ((q15_t)((x) * Q15_SCALE))
#define FLOAT_TO_Q31(x) ((q31_t)((x) * Q31_SCALE))
#define MAKE_FREQ_IDX(i) (q15_t)(((float)(i)) / (float)FFT_LENGTH * Q15_SCALE)

void ftos(char *str, double fl)
{
    char buf[20];
    long long i = (long long)fl; // 정수 부분
    double f = fl - i;           // 소수 부분
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

void Uart1_PrintStr(double *real, double *imag, size_t length)
{
    char real_char[20];
    char imag_char[20];

    for (size_t i = 0; i < length; i++)
    {
        ftos(real_char, real[i]);
        ftos(imag_char, imag[i]);

        Uart_Printf("%s+j%s\n", real_char, imag_char);
    }
    Uart1_Printf("\n");
}

int FFT(long N, double *XR, double *XI)
{
    /* 1. check 2^n count -> if else then return -1 */
    if ((N != 0) && ((N & (N - 1)) != 0))
        return -1;

    /* 2. calculate W */
    long N2 = N >> 1;
    double WR[N2], WI[N2]; /* Real and Imaginary part */
    double T = 2 * PI / N;
    for (long i = 0; i < N2; ++i)
    {
        WR[i] = cos(T * i);
        WI[i] = -sin(T * i);
    }

    /* 3. shuffle input array index to calculate bottom-up style FFT */
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
            tmp = XR[n];
            XR[n] = XR[m];
            XR[m] = tmp;
        }
    }

    /* 4. execute fft */
    for (int loop = 0; loop < log2N; ++loop)
    {
        long regionSize = 1 << (loop + 1);    /* if N=8: 2 -> 4 -> 8 */
        long kJump = 1 << (log2N - loop - 1); /* if N=8: 4 -> 2 -> 1 */
        long half = regionSize >> 1;
        for (register long i = 0; i < N; i += regionSize)
        {
            long blockEnd = i + half - 1;
            long k = 0;
            double TR, TI;
            for (register long j = i; j <= blockEnd; ++j)
            { /* j start from i */
                TR = WR[k] * XR[j + half] - WI[k] * XI[j + half];
                TI = WI[k] * XR[j + half] + WR[k] * XI[j + half];

                XR[j + half] = XR[j] - TR;
                XI[j + half] = XI[j] - TI;

                XR[j] = XR[j] + TR;
                XI[j] = XI[j] + TI;

                k += kJump;
            }
        }
    }
    return 0;
}

void Task3(void *para)
{
    // double pSrc[FFT_LENGTH];
    double pDst_real[FFT_LENGTH];
    double pDst_imag[FFT_LENGTH];

    int magnitude[FFT_LENGTH / 2];
    int freqs[FFT_LENGTH / 2];

    for (size_t i = 0; i < FFT_LENGTH; ++i)
    {
        pDst_real[i] = sin((2 * PI * SIGNAL_FREQ * i) / SAMPLE_RATE);
    }

    while (1)
    {
        FFT(FFT_LENGTH, pDst_real, pDst_imag);

        for (size_t i = 0; i < FFT_LENGTH / 2; i++)
        {
            freqs[i] = (double)i * SAMPLE_RATE / FFT_LENGTH;
            magnitude[i] = sqrt(pDst_real[i] * pDst_real[i] + pDst_imag[i] * pDst_imag[i]);
            Uart_Printf("%d: %d_%d\n", i, freqs[i], magnitude[i]);
        }
        Uart_Printf("\n");

        rtos.delay(2000);
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

void Task3(void *para)
{
    volatile int j;
    mutexID = rtos.createMutex();

    Uart_Printf("\nTask3 Lock\n");

    rtos.lockMutex(mutexID);

    for (j = 0; j < 10; j++)
    {

        systemDelay(250);

        LED_0_Toggle();
    }

    Uart_Printf("\nTask3 unLock\n");
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

    // keyWaitTaskID = rtos.createTask(Task1, nullptr, 2, 1024);
    // rtos.createTask(Task2, nullptr, 3, 1024);
    rtos.createTask(Task3, nullptr, 1, 2048);

#elif defined(TESTCASE3)

    rtos.createTask(Task1, nullptr, 1, 1024);
    rtos.createTask(Task2, nullptr, 2, 1024);
    rtos.createTask(Task3, nullptr, 3, 1024);

#endif
}