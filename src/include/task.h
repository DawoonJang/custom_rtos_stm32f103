#pragma once

#include "option.h"
#include "task.h"
#include <array>
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
    std::array<Task, MAX_TCB> tcbPool{};
    std::array<Task *, NUM_PRIO> readyTaskPool{};
    std::stack<Task *> freeTaskPool;
    Task *delayList;
    int timeTick;

    TaskManager();

    void insertTCBToDelayList(const int taskID);
    void deleteTCBFromDelayList(const int taskID);
    void insertTCBToFreeList(const int taskID);
    Task *getTCBFromFreeList(void);
    void insertTCBToReadyList(const int taskID);
    void deleteTCBFromReadyList(const int taskID);

    void deleteTask(int taskID);

    Task *getTaskPointer(int taskID);
    char *allocateStack(int size);
    void executeTaskSwitching(void);
    int createTask(void (*ptaskfunc)(void *), void *para, int prio, int size_stack);

    void increaseTick(void);
    void delayTask(unsigned int ticks);
};

void ReceiveAndPlayTask(void *para);
