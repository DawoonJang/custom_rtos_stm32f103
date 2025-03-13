#pragma once

#include "mutex.h"
#include "option.h"
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
    Queue queuePool[MAX_QUEUE];
    char queueStack[QUEUE_STACK_SIZE] __attribute__((__aligned__(8)));

    Mutex mutexPool[MAX_MUTEX];

  public:
    LivingRTOS();
    TaskManager taskManager;

    void executeTaskSwitching(void);

    int createTask(void (*ptask)(void *), void *para, int prio, int size_stack);
    void deleteTask(int task_no);
    void scheduleTask(void);

    int timeTick;
    int osStartFlag;

    bool waitForSignal(int *, int);

    void increaseTick(void);
    void delay(unsigned int);

    bool deQueue(int, void *, int);
    void enQueue(int, void *);
    bool isQueueEmpty(int);
    bool isQueueFull(int);
    int createQueue(int, int);

    void moveQueuePointer(int, char *&);
    void moveFrontPointerOfQueue(int);
    void moveRearPointerOfQueue(int);

    char *allocateQueueMemory(int size_arr);

    void sendSignal(const int, const int);

    int createMutex(void);
    void lockMutex(int);
    void unlockMutex(int);
};