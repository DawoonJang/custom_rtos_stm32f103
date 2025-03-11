#ifndef __OS_H__
#define __OS_H__

#include "option.h"
#include "task.h"
#include <array>
#include <stack>

#ifdef __cplusplus
extern "C"
{
#endif

    void SwitchingTask(void);

#ifdef __cplusplus
}
#endif

typedef enum _fail_value
{
    FAIL = -99,
    FAIL_TCB_ALLOCATION,
    FAIL_TIME_OUT,
    FAIL_QUEUE_EMPTY

} E_FAIL_VALUE;

typedef enum _task_blocked_state
{
    BLOCKED_STATE_NONE = 0,
    BLOCKED_STATE_WAIT

} E_TASK_BLOCKED_STATE;

class LivingRTOS
{
  private:
    std::array<Task, MAX_TCB> tcbPool{};
    std::array<Task *, NUM_PRIO> readyTaskPool{};
    std::stack<Task *> freeTaskPool;
    Task *delayList;

    char stack[STACK_SIZE] __attribute__((__aligned__(8))); // Stack aligned to 8 bytes
    Queue queuePool[MAX_QUEUE];
    char queue_arr[QUEUE_ARR_SIZE] __attribute__((__aligned__(8)));
    char *stack_limit;
    char *pstack;

  public:
    LivingRTOS();

    int createTask(void (*ptask)(void *), void *para, int prio, int size_stack);
    void deleteTask(int task_no);
    void scheduleTask(void);

    std::array<Task *, NUM_PRIO> &getReadyList(void);

    Task *getTCBFromFreeList(void);

    void insertTCBToFreeList(Task *const ptask);

    Task *currentTask;

    int timeTick;

    bool waitSignalForDataTransfer(int *, int);

    void insertTCBToReadyList(Task *const);
    void insertTCBToDelayList(Task *const);

    void deleteTCBFromReadyList(Task *const);
    void deleteTCBFromDelayList(Task *const);

    void increaseTick(void);
    void delayByTick(unsigned int delay_time);

    int deQueue(int, void *, int);
    void enQueue(int, void *);
    bool isQueueEmpty(int);
    bool isQueueFull(int);
    int createQueue(int, int);

    void moveQueuePointer(int, char *&);
    void moveFrontPointerOfQueue(int);
    void moveRearPointerOfQueue(int);

    char *allocateQueueMemory(int size_arr);

    void wakeUpTaskWithSignal(int, int);

  private:
    char *getStack(int);
};

#endif /* OS_H */