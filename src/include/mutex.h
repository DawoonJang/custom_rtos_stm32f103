#pragma once

#include "task.h"
#include <list>

class Mutex
{
  public:
    Mutex() : available(true), ownerTaskID(-1)
    {
    }

    void lock(Task &currentTask, TaskManager &taskManager);
    void unlock(Task &currentTask, TaskManager &taskManager);

    void changeMutexStatus(bool available, int ownerTaskID);
    int getOwnerTaskID(void);
    bool isAvailable(void);

  private:
    bool available;
    int ownerTaskID;

    std::list<Task *> waitingTaskList;

    Task *getHighestPriorityWaitingTask(void);
};