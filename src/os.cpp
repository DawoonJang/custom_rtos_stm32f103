#include "os.h"
#include "device_driver.h"

LivingRTOS rtos;
Task *currentTaskGlobal = rtos.currentTask;
// void (LivingRTOS::*SwitchingTask)(void) = &LivingRTOS::SwitchingTask;

#ifdef __cplusplus
extern "C"
{
#endif

    void SwitchingTask(void)
    {
        Task **readyList = rtos.getReadyList();

        if (currentTaskGlobal->state == STATE_READY)
        {
            readyList[currentTaskGlobal->prio] = readyList[currentTaskGlobal->prio]->next;
        }

        for (int prio = PRIO_HIGHEST; prio <= PRIO_LOWEST; prio++)
        {
            if (readyList[prio])
            {
                currentTaskGlobal = readyList[prio];
                break;
            }
        }
    }

#ifdef __cplusplus
}
#endif

LivingRTOS::LivingRTOS()
{
    stack_limit = stack;
    pstack = stack + STACK_SIZE;

    // init
    for (int i = 0; i < NUM_PRIO; i++)
    {
        readyList[i] = nullptr;
    }

    free_list = nullptr;
    for (int i = 0; i < MAX_TCB; i++)
    {
        tcb[i].no_task = i;
        insertTCBToFreeList(&tcb[i]);
    }

    createTask(
        [](void *para) {
            Uart_Printf("IDLE\n");
            for (;;)
                ;
        },
        nullptr, PRIO_LOWEST, 128);
}

void LivingRTOS::insertTCBToFreeList(Task *task)
{
    task->next = free_list;
    free_list = task;
}

Task *LivingRTOS::getTCBFromFreeList(void)
{
    if (free_list == nullptr)
        return nullptr;

    Task *ret = free_list;
    free_list = free_list->next;

    return ret;
}

void LivingRTOS::insertTCBToReadyList(Task *task)
{
    int prio = task->prio;

    if (readyList[prio] == nullptr)
    {
        readyList[prio] = task;
        task->next = task;
        task->prev = task;
    }
    else
    {
        task->prev = readyList[prio]->prev;
        task->next = readyList[prio];

        task->prev->next = task;
        task->next->prev = task;
    }
}

void LivingRTOS::deleteTCBFromReadyList(Task *task)
{
    int prio = task->prio;

    task->prev->next = task->next;
    task->next->prev = task->prev;

    if (readyList[prio] == task)
    {
        readyList[prio] = (task->next == task) ? nullptr : readyList[prio]->next;
    }
    task->prev = task->next = nullptr;
}

char *LivingRTOS::getStack(int size)
{
    size = (size + 7) & ~0x7; // 8-byte alignment

    if (pstack - size < stack_limit)
    {
        return nullptr;
    }
    pstack -= size;

    if (pstack < stack_limit)
    {
        return nullptr;
    }

    return pstack;
}

int LivingRTOS::createTask(void (*ptaskfunc)(void *), void *para, int prio, int size_stack)
{
    Task *task = getTCBFromFreeList();
    scopedItrLock lock;

    if (task == nullptr)
    {
        return FAIL_TCB_ALLOCATION;
    }

    task->top_of_stack = (unsigned long *)getStack(size_stack);

    if (task->top_of_stack == nullptr)
    {
        insertTCBToFreeList(task);
        return FAIL_STACK_ALLOCATION;
    }

    task->prio = prio;
    task->state = STATE_READY;

    task->top_of_stack -= 16;
    task->top_of_stack[8] = (unsigned long)para;
    task->top_of_stack[14] = (unsigned long)ptaskfunc;
    task->top_of_stack[15] = INIT_PSR;

    insertTCBToReadyList(task);

    return task->no_task;
}

void LivingRTOS::deleteTask(int task_no)
{
    if (task_no >= 0 && task_no < MAX_TCB)
    {
        Task *task = &tcb[task_no];
        if (task->state == STATE_READY)
        {
            deleteTCBFromReadyList(task);
        }

        task->state = STATE_BLOCKED;
        insertTCBToFreeList(task);
    }
}

// void LivingRTOS::SwitchingTask(void)
// {
//     readyList[currentTask->prio] = readyList[currentTask->prio]->next;

//     for (int prio = PRIO_HIGHEST; prio <= PRIO_LOWEST; prio++)
//     {
//         if (!readyList[prio])
//         {
//             continue;
//         }

//         currentTask = readyList[prio];
//     }
// }

void LivingRTOS::Scheduling(void)
{
    SwitchingTask();

    // Priority
    SCB->SHP[15 - 4] = 0xf << 4;
    SCB->SHP[14 - 4] = 0xf << 4;

    // Interrupt Exception Priority
    for (int i = 0; i <= 42; i++)
    {
        NVIC_SetPriority((IRQn_Type)i, 0xe);
    }

    // SysTick_OS_Tick(TICK_MS);
    SysTick->CTRL = (0 << 2) + (1 << 1) + (0 << 0);
    SysTick->LOAD = (unsigned int)((HCLK / (8. * 1000.)) * TICK_MS + 0.5);
    SysTick->VAL = 0;
    Macro_Set_Bit(SysTick->CTRL, 0);
    // SysTick_OS_Tick(TICK_MS);

    __asm__ volatile("svc #0");
}

void LivingRTOS::insertTCBToDelayList(Task *ptask)
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

void LivingRTOS::deleteTCBFromDelayList(Task *ptask)
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

Task *LivingRTOS::getCurrentTask(void)
{
    return currentTask;
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

bool LivingRTOS::lookAroundForDataTransfer(int *pdata, int timeout)
{
    int flag;

    scopedItrLock lock;

    currentTaskGlobal->state = STATE_BLOCKED;
    currentTaskGlobal->currentTick = timeTick + timeout;

    currentTaskGlobal->d_state = DATA_STATE_WAITING;
    currentTaskGlobal->signalData = 0;

    deleteTCBFromReadyList(currentTaskGlobal);
    insertTCBToDelayList(currentTaskGlobal);

    trigger_context_switch();

    flag = currentTaskGlobal->d_state;
    currentTaskGlobal->d_state = DATA_STATE_NONE;
    *pdata = currentTaskGlobal->signalData;
    return (flag == currentTaskGlobal->d_state) ? true : false;
}

Task *LivingRTOS::getTCBInfo(int taskNum)
{
    return &(tcb[taskNum]);
}

Task *LivingRTOS::getDelayList(void)
{
    return rtos.delayList;
}

Task **LivingRTOS::getReadyList(void)
{
    return rtos.readyList;
}