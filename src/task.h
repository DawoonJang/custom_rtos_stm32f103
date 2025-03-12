#ifndef __TASK_H__
#define __TASK_H__

#include <vector>

enum class TaskState
{
    Ready = 0,
    Blocked
};

enum class BlockedReason
{
    None = 0,
    Wait,
    Mutex
};

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
    int originPrio;

    TaskState state;
    BlockedReason blockedReason;

    Task *prev;
    Task *next;
    int currentTick;
    int signalData;

    Task()
        : top_of_stack(nullptr), taskID(-1), prio(0), state(TaskState::Ready), blockedReason(BlockedReason::None),
          prev(nullptr), next(nullptr), currentTick(0), signalData(0)
    {
        ;
    }

    Task(int id, int priority)
        : top_of_stack(nullptr), taskID(id), prio(priority), state(TaskState::Ready),
          blockedReason(BlockedReason::None), prev(nullptr), next(nullptr), currentTick(0), signalData(0)
    {
        ;
    }
};

#endif