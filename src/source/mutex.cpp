#include "../include/mutex.h"
#include "../include/device_driver.h"
#include "../include/task.h"

extern Task *currentTaskGlobal;

void Mutex::lock(TaskManager &taskManager)
{
    scopedItrLock lock;

    if (isAvailable())
    {
        changeMutexStatus(false, currentTaskGlobal->taskID);
        return;
    }

    insertWaitList(currentTaskGlobal->taskID);

    taskManager.setTaskBlockedStatus(currentTaskGlobal->taskID, BlockedReason::Mutex, 0);
    taskManager.deleteTCBFromReadyList(currentTaskGlobal->taskID);

    Task *ownerTask = taskManager.getTaskPointer(ownerTaskID);

    if (ownerTask->prio > currentTaskGlobal->prio)
    {
        if (ownerTask->state == TaskState::Ready)
        {
            taskManager.deleteTCBFromReadyList(ownerTaskID);
            ownerTask->prio = currentTaskGlobal->prio;
            taskManager.insertTCBToReadyList(ownerTaskID);
        }
        else
        {
            ownerTask->prio = currentTaskGlobal->prio;
        }
    }

    trigger_context_switch();
}

void Mutex::unlock(TaskManager &taskManager)
{
    scopedItrLock lock;

    bool isContextSwitchingNeed = false;

    if (ownerTaskID != currentTaskGlobal->taskID)
    {
        return;
    }

    changeMutexStatus(true, -1);

    if (currentTaskGlobal->prio != currentTaskGlobal->originPrio)
    {
        taskManager.deleteTCBFromReadyList(currentTaskGlobal->taskID);
        currentTaskGlobal->prio = currentTaskGlobal->originPrio;
        taskManager.insertTCBToReadyList(currentTaskGlobal->taskID);

        isContextSwitchingNeed = true;
    }

    Task *highestWaitingTask = getHighestPriorityWaitingTask(taskManager);

    if (highestWaitingTask)
    {
        changeMutexStatus(false, highestWaitingTask->taskID);

        deleteWaitList(highestWaitingTask->taskID);
        highestWaitingTask->state = TaskState::Ready;
        highestWaitingTask->blockedReason = BlockedReason::None;

        taskManager.insertTCBToReadyList(highestWaitingTask->taskID);

        isContextSwitchingNeed = true;
    }

    if (isContextSwitchingNeed)
    {
        trigger_context_switch();
    }
}

Task *Mutex::getHighestPriorityWaitingTask(TaskManager &taskManager)
{
    int highestPrio = PRIO_LOWEST;
    Task *highestTaskPtr = nullptr;
    Task *curTaskPtr;

    for (int tIdx = 0; tIdx < MAX_TCB; tIdx++)
    {
        if (waitTaskList[tIdx])
        {
            curTaskPtr = taskManager.getTaskPointer(tIdx);
            int curPrio = curTaskPtr->prio;

            if (curPrio < highestPrio)
            {
                highestTaskPtr = curTaskPtr;
                highestPrio = curPrio;
            }
        }
    }
    return highestTaskPtr;
}

void Mutex::changeMutexStatus(bool avail, int ownID)
{
    available = avail;
    ownerTaskID = ownID;
}

bool Mutex::isAvailable(void)
{
    return available;
}

int Mutex::getOwnerTaskID(void)
{
    return ownerTaskID;
}

void Mutex::insertWaitList(const int taskID)
{
    waitTaskList[taskID] = true;
}

void Mutex::deleteWaitList(const int taskID)
{
    waitTaskList[taskID] = false;
}