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
        ;
    }

    void lock(TaskManager &taskManager);
    void unlock(TaskManager &taskManager);

    void changeMutexStatus(bool available, const unsigned char ownID);
    int getOwnerTaskID(void);
    bool isAvailable(void);

  private:
    bool available;
    int ownerTaskID;

    bool waitTaskList[MAX_TCB];

    void insertWaitList(const unsigned char taskID);
    void deleteWaitList(const unsigned char taskID);

    Task *getHighestPriorityWaitingTask(TaskManager &taskManager);
};