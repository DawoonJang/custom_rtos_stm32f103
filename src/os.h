#ifndef __OS_H__
#define __OS_H__

#include "mutex.h"
#include "option.h"
#include "scheduler.h"
#include "task.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void switchingTask(void);

#ifdef __cplusplus
}
#endif

typedef enum _fail_value
{
    FAIL = -99,
} E_FAIL_VALUE;

class LivingRTOS
{
  private:
    char *stack_limit;
    char *pstack;
    char stack[STACK_SIZE] __attribute__((__aligned__(8)));

    Queue queuePool[MAX_QUEUE];
    char queueStack[QUEUE_STACK_SIZE] __attribute__((__aligned__(8)));

    Mutex mutexPool[MAX_MUTEX];

    Scheduler sche;

  public:
    LivingRTOS();

    void executeTaskSwitching(void);

    int createTask(void (*ptask)(void *), void *para, int prio, int size_stack);
    void deleteTask(int task_no);
    void scheduleTask(void);

    int timeTick;
    int osStartFlag;

    bool waitSignalForDataTransfer(int *, int);

    void increaseTick(void);
    void delayByTick(unsigned int delay_time);

    bool deQueue(int, void *, int);
    void enQueue(int, void *);
    bool isQueueEmpty(int);
    bool isQueueFull(int);
    int createQueue(int, int);

    void moveQueuePointer(int, char *&);
    void moveFrontPointerOfQueue(int);
    void moveRearPointerOfQueue(int);

    char *allocateQueueMemory(int size_arr);

    void wakeUpTaskWithSignal(int, int);

    int createMutex(void);
    void takeMutex(int);
    void giveMutex(int);

  private:
    char *getStack(int);
};

#endif /* OS_H */