#include "os.h"
#include "device_driver.h"
#include <algorithm>

#include <string.h>

LivingRTOS rtos;
Task *currentTaskGlobal = rtos.currentTask;
// void (LivingRTOS::*SwitchingTask)(void) = &LivingRTOS::SwitchingTask;

#ifdef __cplusplus
extern "C"
{
#endif

    void SwitchingTask(void)
    {
        auto &readyTaskPool = rtos.getReadyList();

        if (currentTaskGlobal->state == STATE_READY)
        {
            readyTaskPool.at(currentTaskGlobal->prio) = readyTaskPool[currentTaskGlobal->prio]->next;
        }

        for (int prio = PRIO_HIGHEST; prio <= PRIO_LOWEST; prio++)
        {
            if (readyTaskPool[prio] != nullptr)
            {
                currentTaskGlobal = readyTaskPool[prio];
                return;
            }
        }
    }

#ifdef __cplusplus
}
#endif

LivingRTOS::LivingRTOS() : stack_limit(stack), pstack(stack + STACK_SIZE)
{
    readyTaskPool.fill(nullptr);

    for (size_t i = 0; i < MAX_TCB; i++)
    {
        tcbPool[i].taskID = i;
        insertTCBToFreeList(&tcbPool[i]);
    }

    createTask(
        [](void *para) {
            while (1)
            {
                ;
            }
        },
        nullptr, PRIO_LOWEST, 128);
}

void LivingRTOS::insertTCBToFreeList(Task *const ptask)
{
    if (ptask != nullptr)
    {
        freeTaskPool.push(ptask);
    }
}

Task *LivingRTOS::getTCBFromFreeList(void)
{
    if (freeTaskPool.empty())
    {
        return nullptr;
    }

    Task *ret = freeTaskPool.top();
    freeTaskPool.pop();
    return ret;
}

void LivingRTOS::insertTCBToReadyList(Task *const ptask)
{
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

void LivingRTOS::deleteTCBFromReadyList(Task *const ptask)
{
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

char *LivingRTOS::getStack(int size)
{
    size = (size + 7) & ~0x7; // 8-byte alignment

    char *new_stack = pstack - size;

    if (new_stack < stack_limit)
    {
        return nullptr;
    }

    pstack = new_stack;
    return pstack;
}

int LivingRTOS::createTask(void (*ptaskfunc)(void *), void *para, int prio, int size_stack)
{
    scopedItrLock lock;

    Task *task = getTCBFromFreeList();

    if (task == nullptr || size_stack < 0 || prio < 0)
    {
        return FAIL;
    }

    task->top_of_stack = (unsigned long *)getStack(size_stack);

    if (task->top_of_stack == nullptr)
    {
        insertTCBToFreeList(task);
        return FAIL;
    }

    task->prio = prio;
    task->state = STATE_READY;

    task->top_of_stack -= 16;
    task->top_of_stack[8] = (unsigned long)para;
    task->top_of_stack[14] = (unsigned long)ptaskfunc;
    task->top_of_stack[15] = INIT_PSR;

    insertTCBToReadyList(task);

    return task->taskID;
}

void LivingRTOS::deleteTask(int taskID)
{
    if (taskID >= MAX_TCB)
        return;

    Task &task = tcbPool[taskID];

    if (task.state == STATE_READY)
    {
        deleteTCBFromReadyList(&task);
    }

    insertTCBToFreeList(&task);
}

void LivingRTOS::scheduleTask(void)
{
    SwitchingTask();

    constexpr uint8_t HIGHEST_PRIORITY = 0xF << 4;
    SCB->SHP[static_cast<uint8_t>(SVCall_IRQn) - 4] = HIGHEST_PRIORITY;
    SCB->SHP[static_cast<uint8_t>(PendSV_IRQn) - 4] = HIGHEST_PRIORITY;

    // Set IRQ priority
    constexpr int NUM_IRQS = 43; // 0 ~ 42
    constexpr uint8_t IRQ_PRIORITY = 0xE;
    for (int i = 0; i < NUM_IRQS; ++i)
    {
        NVIC_SetPriority(static_cast<IRQn_Type>(i), IRQ_PRIORITY);
    }

    // SysTick_OS_Tick;
    constexpr uint32_t SYSTICK_CTRL_ENABLE = (1 << 1); // Enable SysTick Interrupt
    SysTick->CTRL = SYSTICK_CTRL_ENABLE;
    SysTick->LOAD = static_cast<uint32_t>((HCLK / (8.0 * 1000.0)) * TICK_MS + 0.5);
    SysTick->VAL = 0;
    // SysTick_OS_Tick;

    Macro_Set_Bit(SysTick->CTRL, 0); // Enable SysTick Timer

    __asm__ volatile("svc #0");
}

void LivingRTOS::insertTCBToDelayList(Task *const ptask)
{
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

void LivingRTOS::deleteTCBFromDelayList(Task *const ptask)
{
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

void LivingRTOS::increaseTick(void)
{
    Task *ptask = delayList;

    timeTick += TICK_MS;

    while (ptask)
    {
        Task *pnext_task = ptask->next;

        if (ptask->currentTick <= timeTick)
        {
            ptask->state = STATE_READY;

            deleteTCBFromDelayList(ptask);
            insertTCBToReadyList(ptask);
        }
        else
        {
            ;
        }

        ptask = pnext_task;
    }
}

void LivingRTOS::delayByTick(unsigned int delay_time)
{
    scopedItrLock lock;

    currentTaskGlobal->state = STATE_BLOCKED;
    currentTaskGlobal->currentTick = timeTick + delay_time;

    deleteTCBFromReadyList(currentTaskGlobal);
    insertTCBToDelayList(currentTaskGlobal);

    trigger_context_switch();
}

bool LivingRTOS::waitSignalForDataTransfer(int *pdata, int timeout)
{
    disable_interrupts();

    currentTaskGlobal->state = STATE_BLOCKED;
    currentTaskGlobal->currentTick = timeTick + timeout;

    currentTaskGlobal->dataWaitState = DATA_STATE_WAITING;
    currentTaskGlobal->signalData = 0;

    deleteTCBFromReadyList(currentTaskGlobal);
    insertTCBToDelayList(currentTaskGlobal);

    trigger_context_switch();
    enable_interrupts();

    E_TASK_DATA_STATE initialState = currentTaskGlobal->dataWaitState;
    currentTaskGlobal->dataWaitState = DATA_STATE_NONE;
    *pdata = currentTaskGlobal->signalData;

    return (initialState == currentTaskGlobal->dataWaitState);
}

void LivingRTOS::enQueue(int queueID, void *pdata)
{
    scopedItrLock lock;

    if (queuePool[queueID].buffer == nullptr)
    {
        return;
    }

    if (isQueueFull(queueID))
    {
        return;
    }

    int receiverTaskID = queuePool[queueID].receiverTaskID;
    Task &receiverTask = tcbPool[receiverTaskID];

    memcpy(queuePool[queueID].rear, pdata, queuePool[queueID].elementSize);
    moveRearPointerOfQueue(queueID);

    if (receiverTask.dataWaitState == DATA_STATE_WAITING)
    {
        receiverTask.state = STATE_READY;
        receiverTask.dataWaitState = DATA_STATE_NONE;

        deleteTCBFromDelayList(&receiverTask);
        insertTCBToReadyList(&receiverTask);

        trigger_context_switch();
    }
}

bool LivingRTOS::isQueueEmpty(int queueID)
{
    if (queueID < 0 || queueID >= MAX_TCB)
    {
        return true;
    }

    return queuePool[queueID].rear == queuePool[queueID].front;
}

bool LivingRTOS::isQueueFull(int queueID)
{
    if (queueID < 0 || queueID >= MAX_TCB)
    {
        return false;
    }

    char *next_rear = queuePool[queueID].rear + queuePool[queueID].elementSize;

    if (next_rear == queuePool[queueID].bufferEnd)
    {
        next_rear = queuePool[queueID].buffer;
    }

    return next_rear == queuePool[queueID].front;
}

void LivingRTOS::moveQueuePointer(int queueID, char *&pointer)
{
    if (queueID < 0 || queueID >= MAX_TCB)
    {
        return;
    }

    pointer += queuePool[queueID].elementSize;

    if (pointer >= queuePool[queueID].bufferEnd)
    {
        pointer = queuePool[queueID].buffer;
    }
}

void LivingRTOS::moveFrontPointerOfQueue(int queueID)
{
    moveQueuePointer(queueID, queuePool[queueID].front);
}

void LivingRTOS::moveRearPointerOfQueue(int queueID)
{
    moveQueuePointer(queueID, queuePool[queueID].rear);
}

char *LivingRTOS::allocateQueueMemory(int size_arr)
{
    static char *queueMemPtr = (char *)queue_arr;

    int size = (size_arr + 7) & ~(0x7);

    if (queueMemPtr + size >= (char *)queue_arr + QUEUE_ARR_SIZE)
    {
        return nullptr;
    }

    char *ret = queueMemPtr;
    queueMemPtr += size;

    return ret;
}

int LivingRTOS::createQueue(int capacity, int elementSize)
{
    scopedItrLock lock;
    static int queueIdx;

    if (queueIdx >= MAX_QUEUE || capacity <= 0 || elementSize <= 0)
    {
        return FAIL;
    }

    int qID = queueIdx++;

    queuePool[qID].buffer = allocateQueueMemory((capacity + 1) * elementSize);
    if (queuePool[qID].buffer == nullptr)
    {
        return FAIL;
    }

    queuePool[qID].elementSize = elementSize;
    queuePool[qID].capacity = capacity;
    queuePool[qID].front = queuePool[qID].rear = queuePool[qID].buffer;
    queuePool[qID].bufferEnd = queuePool[qID].buffer + ((capacity + 1) * elementSize);
    queuePool[qID].receiverTaskID = currentTaskGlobal->taskID;

    return qID;
}

bool LivingRTOS::deQueue(int queueID, void *data, int timeout)
{
    scopedItrLock lock;

    if (queuePool[queueID].buffer == nullptr)
    {
        return false;
    }

    if (queuePool[queueID].receiverTaskID != currentTaskGlobal->taskID)
    {
        return false;
    }

    if (!isQueueEmpty(queueID))
    {
        memcpy(data, queuePool[queueID].front, queuePool[queueID].elementSize);

        moveFrontPointerOfQueue(queueID);
        return true;
    }

    currentTaskGlobal->state = STATE_BLOCKED;
    currentTaskGlobal->currentTick = timeTick + timeout;
    currentTaskGlobal->dataWaitState = DATA_STATE_WAITING;

    deleteTCBFromReadyList(currentTaskGlobal);
    insertTCBToDelayList(currentTaskGlobal);

    trigger_context_switch();

    if (currentTaskGlobal->dataWaitState != DATA_STATE_NONE)
    {
        currentTaskGlobal->dataWaitState = DATA_STATE_NONE;
        return false;
    }

    memcpy(data, queuePool[queueID].front, queuePool[queueID].elementSize);

    moveFrontPointerOfQueue(queueID);
    return true;
}

void LivingRTOS::wakeUpTaskWithSignal(int taskID, int value)
{
    scopedItrLock lock;

    if (tcbPool[taskID].state == STATE_BLOCKED && tcbPool[taskID].dataWaitState == DATA_STATE_WAITING)
    {
        tcbPool[taskID].state = STATE_READY;
        tcbPool[taskID].dataWaitState = DATA_STATE_NONE;
        tcbPool[taskID].signalData = value;

        deleteTCBFromDelayList(&tcbPool[taskID]);
        insertTCBToReadyList(&tcbPool[taskID]);

        trigger_context_switch();
    }
}

std::array<Task *, NUM_PRIO> &LivingRTOS::getReadyList(void)
{
    return readyTaskPool;
}