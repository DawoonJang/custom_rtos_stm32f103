#include "device_driver.h"
#include "os.h"
#include "sqe.h"
#include "task.h"

int main(void)
{
    developmentVerify();

    rtos.scheduleTask();

    while (1)
    {
        ;
    }

    return 0;
}
