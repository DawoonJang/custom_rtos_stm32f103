#include "include/device_driver.h"
#include "include/os.h"
#include "include/sqe.h"
#include "include/task.h"

volatile int signalQueueID;
volatile int uartQueueID;
volatile int mutexID;

int main(void)
{
    developmentVerify();

    // rtos.createTask(ReceiveAndPlayTask, nullptr, 1, 1024);
    rtos.scheduleTask();

    while (1)
    {
        ;
    }

    return 0;
}
