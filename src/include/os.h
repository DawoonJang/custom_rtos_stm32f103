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
    Mutex mutexPool[MAX_MUTEX];

    char queueStack[QUEUE_STACK_SIZE] __attribute__((__aligned__(8)));

  public:
    LivingRTOS();
    TaskManager taskManager;

    void executeTaskSwitching(void);

    int createTask(void (*ptaskfunc)(void *), void *const para, const short prio, const unsigned short stackSize);
    void deleteTask(const unsigned char taskID);
    void scheduleTask(void);

    int timeTick;
    int osStartFlag;

    bool waitForSignal(int *, int);

    void increaseTick(void);
    void delay(const unsigned short ticks);

    bool deQueue(const unsigned char queueID, void *const data, const unsigned short timeout);
    void enQueue(const unsigned char queueID, const void *const pdata);
    bool isQueueEmpty(const unsigned char queueID);
    bool isQueueFull(const unsigned char queueID);
    int createQueue(const unsigned short capacity, const unsigned short elementSize);

    void moveQueuePointer(int, char *&);
    void moveFrontPointerOfQueue(int);
    void moveRearPointerOfQueue(int);

    char *allocateQueueMemory(int size_arr);

    void sendSignal(const unsigned char destID, const char signal);

    int createMutex(void);
    void lockMutex(const unsigned char mutexId);
    void unlockMutex(const unsigned char mutexId);
};