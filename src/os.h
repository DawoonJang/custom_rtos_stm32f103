#ifndef __OS_H__
#define __OS_H__

#include "option.h"
#include "task.h"

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
    FAIL_STACK_ALLOCATION = -99,
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
    Task tcb[MAX_TCB];
    Task *free_list;
    Task *readyList[NUM_PRIO];
    Task *delayList;

    char stack[STACK_SIZE] __attribute__((__aligned__(8))); // Stack aligned to 8 bytes
    char *stack_limit;
    char *pstack;

  public:
    LivingRTOS();

    int createTask(void (*ptask)(void *), void *para, int prio, int size_stack);
    void checkReadyList(void);
    void deleteTask(int task_no);
    // void SwitchingTask(void);
    void Scheduling(void);

    Task **getReadyList(void);
    Task *getCurrentTask(void);
    Task *getDelayList(void);
    Task *getTCBInfo(int);
    Task *getTCBFromFreeList(void);

    void insertTCBToFreeList(Task *task);

    Task *currentTask;

    int timeTick;

    bool lookAroundForDataTransfer(int *pdata, int timeout);

    void insertTCBToReadyList(Task *task);
    void insertTCBToDelayList(Task *ptask);

    void deleteTCBFromReadyList(Task *task);
    void deleteTCBFromDelayList(Task *ptask);

    void increaseTick(void);
    void delayByTick(unsigned int delay_time);

  private:
    char *getStack(int size);
};

#endif /* OS_H */