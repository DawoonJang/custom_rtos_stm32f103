#ifndef __TASK_H__
#define __TASK_H__

#include <vector>

typedef enum _task_state
{
    STATE_READY = 0,
    STATE_BLOCKED
} E_TASK_STATE;

typedef enum _task_num
{
    TASK_1 = 1,
    TASK_2,
    TASK_3,
    TASK_4,
} E_TASK_NUM;

typedef enum _task_data_state
{
    DATA_STATE_NONE = 0,
    DATA_STATE_WAITING
} E_TASK_DATA_STATE;

struct Queue
{
    int receiverTaskID;
    char *front;
    char *rear;
    char *buffer;
    char *bufferEnd;

    int elementSize;
    int capacity;
};

struct Task
{
    unsigned long *top_of_stack; // 8: pparam 14: pfunc 15:PSR
    int taskID;
    int prio;
    E_TASK_STATE state;
    Task *prev;
    Task *next;
    int currentTick;
    E_TASK_DATA_STATE dataWaitState;
    int signalData;

    Task()
        : top_of_stack(nullptr), taskID(-1), prio(0), state(STATE_READY), prev(nullptr), next(nullptr), currentTick(0),
          dataWaitState(DATA_STATE_NONE), signalData(0)
    {
        ;
    }

    Task(int id, int priority)
        : top_of_stack(nullptr), taskID(id), prio(priority), state(STATE_READY), prev(nullptr), next(nullptr),
          currentTick(0), dataWaitState(DATA_STATE_NONE), signalData(0)
    {
        ;
    }

    int getCurrentTick(void);
};

#endif