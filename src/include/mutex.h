#pragma once

#include "task.h"
#include <list>

struct Tnode
{
    int taskID;

    Tnode *prev;
    Tnode *next;
};

class Mutex
{
  public:
    Mutex() : available(true), ownerTaskID(-1)
    {
        // for (int i = 0; i < MAX_TCB; i++)
        // {
        //     waitTaskList[i].taskID = i;
        // }
    }

    void lock(TaskManager &taskManager);
    void unlock(TaskManager &taskManager);

    void changeMutexStatus(bool available, int ownerTaskID);
    int getOwnerTaskID(void);
    bool isAvailable(void);

  private:
    bool available;
    int ownerTaskID;

    bool waitTaskList[MAX_TCB];

    void insertWaitList(int taskID);
    void deleteWaitList(int taskID);

    Task *getHighestPriorityWaitingTask(TaskManager &taskManager);
};