#include "../include/task.h"
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

TaskManager::TaskManager() : stack_limit(stack), stackPointer(stack + STACK_SIZE), delayList(nullptr)
{
    for (size_t i = 0; i < MAX_TCB; i++)
    {
        tcbPool[i].taskID = i;
        insertTCBToFreeList(tcbPool[i].taskID);
    }
}

char *TaskManager::allocateStack(unsigned short stackSize)
{
    stackSize = (stackSize + 7) & ~0x7;

    char *new_stack = stackPointer - stackSize;

    if (new_stack < stack_limit)
    {
        return nullptr;
    }

    stackPointer = new_stack;
    return stackPointer;
}

int TaskManager::createTask(void (*ptaskfunc)(void *), void *const para, const short prio,
                            const unsigned short stackSize)
{
    scopedItrLock lock;

    Task *ptask = getTCBFromFreeList();

    if (ptask == nullptr || stackSize < 0 || prio < 0)
    {
        return FAIL;
    }

    ptask->top_of_stack = (unsigned long *)allocateStack(stackSize);

    if (ptask->top_of_stack == nullptr)
    {
        insertTCBToFreeList(ptask->taskID);
        return FAIL;
    }

    ptask->prio = ptask->originPrio = prio;
    ptask->state = TaskState::Ready;

    ptask->top_of_stack -= 16;
    ptask->top_of_stack[8] = (unsigned long)para;
    ptask->top_of_stack[14] = (unsigned long)ptaskfunc;
    ptask->top_of_stack[15] = INIT_PSR;

    insertTCBToReadyList(ptask->taskID);

    return ptask->taskID;
}

void TaskManager::insertTCBToDelayList(const unsigned char taskID)
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

void TaskManager::deleteTCBFromDelayList(const unsigned char taskID)
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

void TaskManager::insertTCBToFreeList(const unsigned char taskID)
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

void TaskManager::insertTCBToReadyList(const unsigned char taskID)
{
    Task *ptask = &(tcbPool[taskID]);
    int prio = ptask->prio;
    Task *&head = readyTaskPool[prio];

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

void TaskManager::deleteTCBFromReadyList(const unsigned char taskID)
{
    Task *ptask = &(tcbPool[taskID]);

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

void TaskManager::deleteTask(const unsigned char taskID)
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

Task *TaskManager::getTaskPointer(const unsigned char taskID)
{
    return &(tcbPool[taskID]);
}

void TaskManager::executeTaskSwitching(void)
{
    if (currentTaskGlobal && currentTaskGlobal->state == TaskState::Ready)
    {
        readyTaskPool[currentTaskGlobal->prio] = currentTaskGlobal->next;
    }

    for (size_t prio = PRIO_HIGHEST; prio <= PRIO_LOWEST; prio++)
    {
        if (readyTaskPool[prio])
        {
            currentTaskGlobal = readyTaskPool[prio];
            return;
        }
    }
}

void TaskManager::increaseTick(void)
{
    scopedItrLock lock;

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

void TaskManager::setTaskBlockedStatus(const unsigned char taskID, const BlockedReason whyBlocked,
                                       const unsigned short timeout)
{
    tcbPool[taskID].state = TaskState::Blocked;
    tcbPool[taskID].blockedReason = whyBlocked;
    tcbPool[taskID].currentTick = timeTick + timeout;

    switch (tcbPool[taskID].blockedReason)
    {
    case BlockedReason::Wait:
    case BlockedReason::Sleep:
        deleteTCBFromReadyList(currentTaskGlobal->taskID);
        insertTCBToDelayList(currentTaskGlobal->taskID);
        break;

    default:
        break;
    }
}

void TaskManager::setTaskReadyFromDelay(const unsigned char taskID)
{
    tcbPool[taskID].state = TaskState::Ready;
    tcbPool[taskID].blockedReason = BlockedReason::None;
    deleteTCBFromDelayList(taskID);
    insertTCBToReadyList(taskID);
}

void TaskManager::delayTask(const unsigned short ticks)
{
    scopedItrLock lock;

    setTaskBlockedStatus(currentTaskGlobal->taskID, BlockedReason::Sleep, ticks);

    trigger_context_switch();
}
