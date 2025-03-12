#pragma once

#include "option.h"
#include "task.h"
#include <array>
#include <stack>

#include "task.h"
#include <array>
#include <stack>

class Scheduler
{
  public:
    std::array<Task, MAX_TCB> tcbPool{};
    std::array<Task *, NUM_PRIO> readyTaskPool{};
    std::stack<Task *> freeTaskPool;
    Task *delayList;
    int timeTick;

    Scheduler();

    void insertTCBToDelayList(const int taskID);
    void deleteTCBFromDelayList(const int taskID);
    void insertTCBToFreeList(const int taskID);
    Task *getTCBFromFreeList(void);
    void insertTCBToReadyList(const int taskID);
    void deleteTCBFromReadyList(const int taskID);

    void deleteTask(int taskID);

    Task *getTaskPointer(int taskID);

    void executeTaskSwitching(void);
    void increaseTick(void);
    void delayByTick(unsigned int delay_time);
};