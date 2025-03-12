#include "../include/scheduler.h"
#include "../include/device_driver.h"

extern Task *currentTaskGlobal;

Scheduler::Scheduler() : delayList(nullptr)
{
    for (size_t i = 0; i < MAX_TCB; i++)
    {
        tcbPool[i].taskID = i;
        insertTCBToFreeList(tcbPool[i].taskID);
    }
}

void Scheduler::insertTCBToDelayList(const int taskID)
{
    Task *ptask = &(tcbPool[taskID]);

    if (delayList == nullptr)
    {
        delayList = ptask;
        ptask->prev = ptask->next = nullptr;
        return;
    }

    ptask->prev = nullptr;
    ptask->next = delayList;

    delayList = ptask;
    ptask->next->prev = ptask;
}

void Scheduler::deleteTCBFromDelayList(const int taskID)
{
    Task *ptask = &(tcbPool[taskID]);

    if (ptask->prev != nullptr)
    {
        ptask->prev->next = ptask->next;
    }
    else
    {
        delayList = ptask->next;
    }

    if (ptask->next != nullptr)
    {
        ptask->next->prev = ptask->prev;
    }
}

void Scheduler::insertTCBToFreeList(const int taskID)
{
    Task *ptask = &(tcbPool[taskID]);

    if (ptask)
    {
        freeTaskPool.push(ptask);
    }
}

Task *Scheduler::getTCBFromFreeList(void)
{
    if (freeTaskPool.empty())
    {
        return nullptr;
    }

    Task *ret = freeTaskPool.top();
    freeTaskPool.pop();
    return ret;
}

void Scheduler::insertTCBToReadyList(const int taskID)
{
    Task *ptask = &(tcbPool[taskID]);

    if (!ptask)
        return;

    int prio = ptask->prio;
    Task *&head = readyTaskPool[prio];

    if (!head)
    {
        head = ptask;
    }
    else
    {
        ptask->prev = head->prev;
        ptask->next = head;

        head->prev->next = ptask;
        head->prev = ptask;
    }

    ptask->next = ptask;
    ptask->prev = ptask;
}

void Scheduler::deleteTCBFromReadyList(const int taskID)
{
    Task *ptask = &(tcbPool[taskID]);

    if (!ptask)
        return;

    int prio = ptask->prio;
    Task *&head = readyTaskPool[prio];

    if (!head)
        return;

    ptask->prev->next = ptask->next;
    ptask->next->prev = ptask->prev;

    if (head == ptask)
    {
        head = (ptask->next == ptask) ? nullptr : ptask->next;
    }

    ptask->prev = ptask->next = nullptr;
}

void Scheduler::deleteTask(int taskID)
{
    if (taskID >= MAX_TCB)
        return;

    Task &task = tcbPool[taskID];

    if (task.state == TaskState::Ready)
    {
        deleteTCBFromReadyList(taskID);
    }

    insertTCBToFreeList(taskID);
}

Task *Scheduler::getTaskPointer(int taskID)
{
    return &(tcbPool[taskID]);
}

void Scheduler::executeTaskSwitching(void)
{
    if (currentTaskGlobal->state == TaskState::Ready)
    {
        readyTaskPool.at(currentTaskGlobal->prio) = readyTaskPool.at(currentTaskGlobal->prio)->next;
    }

    for (size_t prio = PRIO_HIGHEST; prio <= PRIO_LOWEST; prio++)
    {
        if (readyTaskPool.at(prio) != nullptr)
        {
            currentTaskGlobal = readyTaskPool[prio];
            return;
        }
    }
}

void Scheduler::increaseTick(void)
{
    Task *ptask = delayList;

    timeTick += TICK_MS;

    while (ptask)
    {
        Task *pnext_task = ptask->next;

        if (ptask->currentTick <= timeTick)
        {
            ptask->state = TaskState::Ready;

            deleteTCBFromDelayList(ptask->taskID);
            insertTCBToReadyList(ptask->taskID);
        }

        ptask = pnext_task;
    }
}

void Scheduler::delayByTick(unsigned int delay_time)
{
    scopedItrLock lock;

    currentTaskGlobal->state = TaskState::Blocked;
    currentTaskGlobal->currentTick = timeTick + delay_time;

    deleteTCBFromReadyList(currentTaskGlobal->taskID);
    insertTCBToDelayList(currentTaskGlobal->taskID);

    trigger_context_switch();
}
