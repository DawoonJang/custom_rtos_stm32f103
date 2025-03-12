#include "../include/device_driver.h"

// void ReceiveAndPlayTask(void *para)
// {
//     uint8_t buffer[1024];
//     int received = 0;
//     int total_received = 0;

//     Uart_Printf("Waiting for WAV file...\n");

//     while (total_received < WAV_HEADER_SIZE)
//     {
//         received = Uart_Receive(buffer, WAV_HEADER_SIZE - total_received, 5000);
//         if (received > 0)
//         {
//             total_received += received;
//         }
//     }

//     Uart_Printf("WAV Header received. Starting playback...\n");

//     for (;;)
//     {
//         received = Uart_Receive(buffer, sizeof(buffer), 5000);
//         if (received > 0)
//         {
//             for (int i = 0; i < received; i += 2)
//             {
//                 uint16_t sample = buffer[i] | (buffer[i + 1] << 8);
//                 DAC_Output(sample);
//             }
//         }
//         rtos.delay(1);
//     }
// }

extern Task *currentTaskGlobal;

TaskManager::TaskManager() : delayList(nullptr)
{
    for (size_t i = 0; i < MAX_TCB; i++)
    {
        tcbPool[i].taskID = i;
        insertTCBToFreeList(tcbPool[i].taskID);
    }
}

void TaskManager::insertTCBToDelayList(const int taskID)
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

void TaskManager::deleteTCBFromDelayList(const int taskID)
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

void TaskManager::insertTCBToFreeList(const int taskID)
{
    Task *ptask = &(tcbPool[taskID]);

    if (ptask)
    {
        freeTaskPool.push(ptask);
    }
}

Task *TaskManager::getTCBFromFreeList(void)
{
    if (freeTaskPool.empty())
    {
        return nullptr;
    }

    Task *ret = freeTaskPool.top();
    freeTaskPool.pop();
    return ret;
}

void TaskManager::insertTCBToReadyList(const int taskID)
{
    Task *ptask = &(tcbPool[taskID]);
    int prio = ptask->prio;
    Task *&head = readyTaskPool[prio];

    if (!ptask)
        return;

    if (!head)
    {
        head = ptask;

        ptask->next = ptask;
        ptask->prev = ptask;
    }
    else
    {
        ptask->prev = head->prev;
        ptask->next = head;

        head->prev->next = ptask;
        head->prev = ptask;
    }
}

void TaskManager::deleteTCBFromReadyList(const int taskID)
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

void TaskManager::deleteTask(int taskID)
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

Task *TaskManager::getTaskPointer(int taskID)
{
    return &(tcbPool[taskID]);
}

void TaskManager::executeTaskSwitching(void)
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

void TaskManager::increaseTick(void)
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

void TaskManager::delayTask(unsigned int ticks)
{
    scopedItrLock lock;

    currentTaskGlobal->state = TaskState::Blocked;
    currentTaskGlobal->currentTick = timeTick + ticks;

    deleteTCBFromReadyList(currentTaskGlobal->taskID);
    insertTCBToDelayList(currentTaskGlobal->taskID);

    trigger_context_switch();
}
