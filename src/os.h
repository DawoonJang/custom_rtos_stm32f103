#ifndef __OS_H__
#define __OS_H__

#include <vector>

#define MAX_TCB (5)

#define PRIO_HIGHEST (0)
#define PRIO_LOWEST (10)
#define NUM_PRIO (PRIO_LOWEST - PRIO_HIGHEST + 1)

#define STACK_SIZE (8 * 1024)
#define INIT_PSR (0x01000000)
#define TICK_MS (1)

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

typedef enum _task_state
{
    STATE_READY = 0,
    STATE_BLOCKED
} E_TASK_STATE;

typedef enum _task_blocked_state
{
    BLOCKED_STATE_NONE = 0,
    BLOCKED_STATE_WAIT

} E_TASK_BLOCKED_STATE;

struct VectQueue
{
    std::vector<int> data;

    void push(const int val)
    {
        data.push_back(val);
    }

    int pop()
    {
        if (data.empty())
        {
            return FAIL_QUEUE_EMPTY;
        }

        int val = data.front();
        data.erase(data.begin());
        return val;
    }

    bool empty() const
    {
        return data.empty();
    }

    size_t size() const
    {
        return data.size();
    }
};

class Task
{
  public:
    unsigned long *top_of_stack;
    int no_task;
    int prio;
    E_TASK_STATE state;
    Task *prev;
    Task *next;
    int tick_ready;

    VectQueue q;

    Task() : top_of_stack(nullptr), no_task(-1), prio(0), state(STATE_READY), prev(nullptr), next(nullptr)
    {
    }
};

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