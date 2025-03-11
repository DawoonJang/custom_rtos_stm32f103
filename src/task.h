#ifndef __TASK_H__
#define __TASK_H__

#include <vector>

typedef enum _task_state
{
    STATE_READY = 0,
    STATE_BLOCKED
} E_TASK_STATE;

typedef enum _task_data_state
{
    DATA_STATE_NONE = 0,
    DATA_STATE_WAITING
} E_TASK_DATA_STATE;

class CustomQ
{
  private:
    std::vector<int> data;

  public:
    void push(const int val);
    int pop(void);

    bool empty(void) const;
    size_t size(void) const;
};

struct Task
{
    unsigned long *top_of_stack; // 8: pparam 14: pfunc 15:PSR
    int no_task;
    int prio;
    E_TASK_STATE state;
    Task *prev;
    Task *next;
    int currentTick;
    E_TASK_DATA_STATE d_state;
    int signalData;

    CustomQ q;

    Task()
        : top_of_stack(nullptr), no_task(-1), prio(0), state(STATE_READY), prev(nullptr), next(nullptr), currentTick(0),
          d_state(DATA_STATE_NONE), signalData(0)
    {
        ;
    }

    Task(int id, int priority)
        : top_of_stack(nullptr), no_task(id), prio(priority), state(STATE_READY), prev(nullptr), next(nullptr),
          currentTick(0), d_state(DATA_STATE_NONE), signalData(0)
    {
        ;
    }

    bool isQueueEmpty(void) const;
    bool isQueueFull(void) const;

    bool enQueue(const int val);
    bool deQueue(int *const val, const int timeout);
    void waitForReadyState(void);

    int getCurrentTick(void);
};

#endif