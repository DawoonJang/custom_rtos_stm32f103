#include "os.h"
#include "device_driver.h"
#include "ltr.h"

#include <cstring>

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

LivingRTOS::LivingRTOS()
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

int LivingRTOS::createTask(void (*ptaskfunc)(void *), void *const para, const short prio,
                           const unsigned short stackSize)
{
    return (taskManager.createTask(ptaskfunc, para, prio, stackSize));
}

void LivingRTOS::deleteTask(const unsigned char taskID)
{
    taskManager.deleteTask(taskID);
}

void LivingRTOS::scheduleTask(void)
{
    scopedItrLock lock;

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
    taskManager.increaseTick();
}

void LivingRTOS::delay(const unsigned short ticks)
{
    taskManager.delayTask(ticks);
}

bool LivingRTOS::waitForSignal(int *pdata, int timeout)
{
    scopedItrLock lock;

    taskManager.setTaskBlockedStatus(currentTaskGlobal->taskID, BlockedReason::Wait, timeout);
    trigger_context_switch();
    enable_interrupts();

    BlockedReason stateBackup = currentTaskGlobal->blockedReason;
    currentTaskGlobal->blockedReason = BlockedReason::None;
    *pdata = currentTaskGlobal->recvSignal;

    return (stateBackup == BlockedReason::None);
}

void LivingRTOS::enQueue(const unsigned char queueID, const void *const pdata)
{
    scopedItrLock lock;

    if (queuePool[queueID].front == nullptr)
    {
        return;
    }

    if (isQueueFull(queueID))
    {
        return;
    }

    int receiverTaskID = queuePool[queueID].receiverTaskID;
    Task *receiverTask = taskManager.getTaskPointer(receiverTaskID);

    memcpy(queuePool[queueID].rear, pdata, queuePool[queueID].elementSize);
    moveRearPointerOfQueue(queueID);

    if (receiverTask->blockedReason == BlockedReason::Wait)
    {
        taskManager.setTaskReadyFromDelay(receiverTaskID);
        trigger_context_switch();
    }
}

bool LivingRTOS::isQueueEmpty(const unsigned char queueID)
{
    if (queueID < 0 || queueID >= MAX_TCB)
    {
        return true;
    }

    return queuePool[queueID].rear == queuePool[queueID].front;
}

bool LivingRTOS::isQueueFull(const unsigned char queueID)
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

int LivingRTOS::createQueue(const unsigned short capacity, const unsigned short elementSize)
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

bool LivingRTOS::deQueue(const unsigned char queueID, void *const data, const unsigned short timeout)
{
    disable_interrupts();

    if (queuePool[queueID].buffer == nullptr)
    {
        enable_interrupts();

        return false;
    }

    if (queuePool[queueID].receiverTaskID != currentTaskGlobal->taskID)
    {
        enable_interrupts();

        return false;
    }

    if (!isQueueEmpty(queueID))
    {
        memcpy(data, queuePool[queueID].front, queuePool[queueID].elementSize);
        moveFrontPointerOfQueue(queueID);
        return true;
    }

    taskManager.setTaskBlockedStatus(currentTaskGlobal->taskID, BlockedReason::Wait, timeout);

    enable_interrupts();

    trigger_context_switch();

    disable_interrupts();

    if (currentTaskGlobal->blockedReason != BlockedReason::None)
    {
        currentTaskGlobal->blockedReason = BlockedReason::None;

        enable_interrupts();
        return false;
    }

    memcpy(data, queuePool[queueID].front, queuePool[queueID].elementSize);
    moveFrontPointerOfQueue(queueID);

    enable_interrupts();
    return true;
}

void LivingRTOS::sendSignal(const unsigned char destTaskID, const char signal)
{
    scopedItrLock lock;

    Task *destTask = taskManager.getTaskPointer(destTaskID);

    if (destTask->state == TaskState::Blocked && destTask->blockedReason == BlockedReason::Wait)
    {
        destTask->recvSignal = signal;
        taskManager.setTaskReadyFromDelay(destTaskID);

        trigger_context_switch();
    }
}

void LivingRTOS::executeTaskSwitching(void)
{
    taskManager.executeTaskSwitching();
}

int LivingRTOS::createMutex(void)
{
    scopedItrLock lock;
    static char mutexIdx = 0;

    if (mutexIdx >= MAX_MUTEX)
    {
        return FAIL;
    }

    mutexIdx++;

    return mutexIdx;
}

void LivingRTOS::lockMutex(const unsigned char mutexID)
{
    mutexPool[mutexID].lock(taskManager);
}

void LivingRTOS::unlockMutex(const unsigned char mutexID)
{
    mutexPool[mutexID].unlock(taskManager);
}