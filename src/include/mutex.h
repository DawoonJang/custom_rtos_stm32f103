#ifndef __MUTEX_H__
#define __MUTEX_H__

#include "scheduler.h"
#include "task.h"
#include <list>

class Mutex
{
  public:
    Mutex() : available(true), ownerTaskID(-1)
    {
    }

    void acquire(Task &currentTask, Scheduler &scheduler);
    void release(Task &currentTask, Scheduler &scheduler);

    void changeMutexStatus(bool available, int ownerTaskID);
    int getOwnerTaskID(void);
    bool isAvailable(void);

  private:
    bool available;
    int ownerTaskID;

    std::list<Task *> waitingTaskList;

    Task *getHighestPriorityWaitingTask(void);
};

#endif