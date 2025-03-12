#include "../include/mutex.h"
#include "../include/device_driver.h"
#include "../include/task.h"

void Mutex::lock(Task &currentTask, TaskManager &taskManager)
{
    scopedItrLock lock;

    if (available)
    {
        changeMutexStatus(false, currentTask.taskID);
    }
    else
    {
        waitingTaskList.push_back(&currentTask);

        currentTask.state = TaskState::Blocked;
        currentTask.blockedReason = BlockedReason::Mutex;
        taskManager.deleteTCBFromReadyList(currentTask.taskID);

        Task *ownerTask = taskManager.getTaskPointer(ownerTaskID);
        if (ownerTask->prio > currentTask.prio)
        {
            if (ownerTask->state == TaskState::Ready)
            {
                taskManager.deleteTCBFromReadyList(ownerTaskID);
                ownerTask->prio = currentTask.prio;
                taskManager.insertTCBToReadyList(ownerTaskID);
            }
            else
            {
                ownerTask->prio = currentTask.prio;
            }
        }

        trigger_context_switch();
    }
}

void Mutex::unlock(Task &currentTask, TaskManager &taskManager)
{
    scopedItrLock lock;

    if (ownerTaskID != currentTask.taskID)
    {
        return;
    }

    available = true;
    ownerTaskID = -1;

    if (currentTask.prio != currentTask.originPrio)
    {
        taskManager.deleteTCBFromReadyList(currentTask.taskID);
        currentTask.prio = currentTask.originPrio;
        taskManager.insertTCBToReadyList(currentTask.taskID);
    }

    Task *highestWaitingTask = getHighestPriorityWaitingTask();
    if (highestWaitingTask)
    {
        available = false;
        ownerTaskID = highestWaitingTask->taskID;

        waitingTaskList.remove(highestWaitingTask);
        highestWaitingTask->state = TaskState::Ready;
        highestWaitingTask->blockedReason = BlockedReason::None;

        taskManager.insertTCBToReadyList(highestWaitingTask->taskID);
        trigger_context_switch();
    }
}

Task *Mutex::getHighestPriorityWaitingTask()
{
    Task *highestTask = nullptr;
    int highestPriority = PRIO_LOWEST;

    for (Task *task : waitingTaskList)
    {
        if (task->prio < highestPriority)
        {
            highestPriority = task->prio;
            highestTask = task;
        }
    }
    return highestTask;
}

void Mutex::changeMutexStatus(bool available, int ownerTaskID)
{
    available = available;
    ownerTaskID = ownerTaskID;
}

bool Mutex::isAvailable(void)
{
    return available;
}

int Mutex::getOwnerTaskID(void)
{
    return ownerTaskID;
}