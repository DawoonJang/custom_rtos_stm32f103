#include "device_driver.h"
#include "os.h"
#include "sqe.h"
#include "task.h"

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
