#include "../include/os.h"
#include "../include/device_driver.h"
#include <algorithm>

#include <string.h>

LivingRTOS rtos;
Task *currentTaskGlobal = nullptr;

#ifdef __cplusplus
extern "C"
{
#endif

    void switchingTask(void)
    {
        rtos.executeTaskSwitching();
    }

#ifdef __cplusplus
}
#endif

LivingRTOS::LivingRTOS() : stack_limit(stack), pstack(stack + STACK_SIZE)
{
    createTask(
        [](void *para) {
            while (1)
            {
                ;
            }
        },
        nullptr, PRIO_LOWEST, 128);
}

char *LivingRTOS::getStack(int size)
{
    size = (size + 7) & ~0x7;

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

    Task *ptask = sche.getTCBFromFreeList();

    if (ptask == nullptr || size_stack < 0 || prio < 0)
    {
        return FAIL;
    }

    ptask->top_of_stack = (unsigned long *)getStack(size_stack);

    if (ptask->top_of_stack == nullptr)
    {
        sche.insertTCBToFreeList(ptask->taskID);
        return FAIL;
    }

    ptask->prio = ptask->originPrio = prio;
    ptask->state = TaskState::Ready;

    ptask->top_of_stack -= 16;
    ptask->top_of_stack[8] = (unsigned long)para;
    ptask->top_of_stack[14] = (unsigned long)ptaskfunc;
    ptask->top_of_stack[15] = INIT_PSR;

    sche.insertTCBToReadyList(ptask->taskID);

    return ptask->taskID;
}

void LivingRTOS::deleteTask(int taskID)
{
    sche.deleteTask(taskID);
}

void LivingRTOS::scheduleTask(void)
{
    executeTaskSwitching();

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

void LivingRTOS::increaseTick(void)
{
    sche.increaseTick();
}

void LivingRTOS::delayByTick(unsigned int delay_time)
{
    sche.delayByTick(delay_time);
}

bool LivingRTOS::waitSignalForDataTransfer(int *pdata, int timeout)
{
    scopedItrLock lock;

    currentTaskGlobal->state = TaskState::Blocked;
    currentTaskGlobal->currentTick = sche.timeTick + timeout;

    currentTaskGlobal->blockedReason = BlockedReason::Wait;
    currentTaskGlobal->signalData = 0;

    sche.deleteTCBFromReadyList(currentTaskGlobal->taskID);
    sche.insertTCBToDelayList(currentTaskGlobal->taskID);

    trigger_context_switch();

    BlockedReason initialState = currentTaskGlobal->blockedReason;
    currentTaskGlobal->blockedReason = BlockedReason::None;
    *pdata = currentTaskGlobal->signalData;

    return (initialState == currentTaskGlobal->blockedReason);
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
    Task *receiverTask = sche.getTaskPointer(receiverTaskID);

    memcpy(queuePool[queueID].rear, pdata, queuePool[queueID].elementSize);
    moveRearPointerOfQueue(queueID);

    if (receiverTask->blockedReason == BlockedReason::Wait)
    {
        receiverTask->state = TaskState::Ready;
        receiverTask->blockedReason = BlockedReason::None;

        sche.deleteTCBFromDelayList(receiverTaskID);
        sche.insertTCBToReadyList(receiverTaskID);

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
    static char *queueMemPtr = (char *)queueStack;

    int size = (size_arr + 7) & ~(0x7);

    if (queueMemPtr + size >= (char *)queueStack + QUEUE_STACK_SIZE)
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

    currentTaskGlobal->state = TaskState::Blocked;
    currentTaskGlobal->currentTick = sche.timeTick + timeout;
    currentTaskGlobal->blockedReason = BlockedReason::Wait;

    sche.deleteTCBFromReadyList(currentTaskGlobal->taskID);
    sche.insertTCBToDelayList(currentTaskGlobal->taskID);

    trigger_context_switch();

    if (currentTaskGlobal->blockedReason != BlockedReason::None)
    {
        currentTaskGlobal->blockedReason = BlockedReason::None;
        return false;
    }

    memcpy(data, queuePool[queueID].front, queuePool[queueID].elementSize);

    moveFrontPointerOfQueue(queueID);
    return true;
}

void LivingRTOS::wakeUpTaskWithSignal(int taskID, int value)
{
    scopedItrLock lock;

    Task *curTask = sche.getTaskPointer(taskID);

    if (curTask->state == TaskState::Blocked && curTask->blockedReason == BlockedReason::Wait)
    {
        curTask->state = TaskState::Ready;
        curTask->blockedReason = BlockedReason::None;
        curTask->signalData = value;

        sche.deleteTCBFromDelayList(taskID);

        sche.insertTCBToReadyList(taskID);

        trigger_context_switch();
    }
}

void LivingRTOS::executeTaskSwitching(void)
{
    sche.executeTaskSwitching();
}

int LivingRTOS::createMutex(void)
{
    scopedItrLock lock;
    static int mutexIdx = 0;

    if (mutexIdx >= MAX_MUTEX)
    {
        return FAIL;
    }

    mutexIdx++;

    return mutexIdx;
}

void LivingRTOS::takeMutex(int mutexID)
{
    mutexPool[mutexID].acquire(*currentTaskGlobal, sche);
}

void LivingRTOS::giveMutex(int mutexID)
{
    mutexPool[mutexID].release(*currentTaskGlobal, sche);
}