#include "../include/sqe.h"
#include "../include/device_driver.h"

extern volatile int Key_Value;
extern volatile int Uart1_Rx_In;
extern volatile int keyWaitTaskID;

extern volatile int signalQueueID;
extern volatile int uartQueueID;
extern volatile int mutexID;

#ifdef TESTCASE1

void Task1(void *para)
{

    for (;;)
    {
        Uart_Printf("Task1:\n");
        LED_0_Toggle();
        rtos.delay(1000);
    }
}

void Task2(void *para)
{
    for (;;)
    {
        Uart_Printf("Task2:\n");
        LED_1_Toggle();
        rtos.delay(500);
    }
}
#elif defined(TESTCASE2)

void Task1(void *para)
{
    int fflag;
    static int cnt;

    for (;;)
    {
        if (rtos.waitForSignal(&fflag, 5000) == true)
        {
            rtos.enQueue(signalQueueID, &cnt);
            LED_1_Toggle();
            cnt++;
        }
        else
        {
            ;
        }
    }
}

void Task2(void *para)
{
    signalQueueID = rtos.createQueue(1, sizeof(int));
    uartQueueID = rtos.createQueue(1, sizeof(int));
    int recvSignal;

    for (;;)
    {
        if (!rtos.deQueue(signalQueueID, &recvSignal, 1000))
        {
            LED_0_Toggle();
        }
        else
        {
            Uart_Printf("T2: %d\n", recvSignal);
        }
    }
}

#elif defined(TESTCASE3)

void Task1(void *para)
{
    volatile int j;
    rtos.delay(500);

    Uart_Printf("\nTask1 : Semaphore Take!\n");

    rtos.lockMutex(mutexID);

    for (j = 0; j < 10; j++)
    {
        systemDelay(250);

        LED_1_Toggle();
    }

    rtos.giveMutex(mutexID);

    Uart_Printf("Task1 : Semaphore Give!\n");
    for (;;)
    {
        rtos.delay(500);
    }
}

void Task2(void *para)
{
    rtos.delay(7000);
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

    Uart_Printf("\nTask3 : Semaphore Take!\n");

    rtos.lockMutex(mutexID);

    for (j = 0; j < 10; j++)
    {
        systemDelay(250);

        LED_0_Toggle();
    }

    rtos.giveMutex(mutexID);
    Uart_Printf("Task3 : Semaphore Give!\n");

    for (;;)
    {
        rtos.delay(1000);
        Uart_Printf("Task3: Still Running\n");
    }
}

#endif

void developmentVerify(void)
{
#ifdef TESTCASE1

    rtos.createTask(Task1, nullptr, 1, 1024);
    rtos.createTask(Task2, nullptr, 2, 1024);

#elif defined(TESTCASE2)

    keyWaitTaskID = rtos.createTask(Task1, nullptr, 1, 1024);
    rtos.createTask(Task2, nullptr, 2, 1024); // Equal Prio is not working

#elif defined(TESTCASE3)

    rtos.createTask(Task1, nullptr, 1, 1024);
    rtos.createTask(Task2, nullptr, 2, 1024);
    rtos.createTask(Task3, nullptr, 3, 1024);

#endif
}