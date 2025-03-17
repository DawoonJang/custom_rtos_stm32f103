#pragma once

#include "option.h"
#include <stack>

enum class TaskState
{
    Ready = 0,
    Blocked
};

enum class BlockedReason
{
    None = 0,
    Wait,
    Mutex,
    Sleep
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
    int recvSignal;

    Task()
        : top_of_stack(nullptr), taskID(-1), prio(0), state(TaskState::Ready), blockedReason(BlockedReason::None),
          prev(nullptr), next(nullptr), currentTick(0), recvSignal(0)
    {
        ;
    }

    Task(int id, int priority)
        : top_of_stack(nullptr), taskID(id), prio(priority), state(TaskState::Ready),
          blockedReason(BlockedReason::None), prev(nullptr), next(nullptr), currentTick(0), recvSignal(0)
    {
        ;
    }
};

class TaskManager
{
  private:
    char *stack_limit;
    char *stackPointer;
    char stack[STACK_SIZE] __attribute__((__aligned__(8)));

  public:
    Task tcbPool[MAX_TCB];
    Task *readyTaskPool[NUM_PRIO];

    std::stack<Task *> freeTaskPool;
    Task *delayList;
    int timeTick;

    TaskManager();

    void insertTCBToDelayList(const unsigned char taskID);
    void deleteTCBFromDelayList(const unsigned char taskID);
    void insertTCBToFreeList(const unsigned char taskID);
    Task *getTCBFromFreeList(void);
    void insertTCBToReadyList(const unsigned char taskID);
    void deleteTCBFromReadyList(const unsigned char taskID);

    void deleteTask(const unsigned char taskID);

    Task *getTaskPointer(const unsigned char taskID);
    char *allocateStack(unsigned short size);
    void executeTaskSwitching(void);
    int createTask(void (*ptaskfunc)(void *), void *const para, const short prio, const unsigned short stackSize);
    void setTaskBlockedStatus(const unsigned char taskID, BlockedReason whyBlocked, const unsigned short timeout);
    void setTaskReadyFromDelay(const unsigned char taskID);

    void increaseTick(void);
    void delayTask(const unsigned short ticks);
};

void ReceiveAndPlayTask(void *para);
