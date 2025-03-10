#include "os.h"
#include "device_driver.h"

LivingRTOS rtos;
Task *currentTask = rtos.currentTask;
// void (LivingRTOS::*SwitchingTask)(void) = &LivingRTOS::SwitchingTask;

#ifdef __cplusplus
extern "C"
{
#endif

    void OSIncrementTick(void)
    {
        Task *ptask = rtos.getDelayList();

        rtos.timeTick += TICK_MS;
        while (ptask)
        {
            Task *pnext_task = ptask->next;

            if (ptask->tick_ready <= rtos.timeTick)
            {
                ptask->state = STATE_READY;

                rtos.DeleteTCBFromDelayList(ptask);
                rtos.InsertTCBToReadyList(ptask);
            }
            else
            {
                ;
            }

            ptask = pnext_task;
        }
    }

    void SwitchingTask(void)
    {
        Task **readyList = rtos.getReadyList();

        readyList[currentTask->prio] = readyList[currentTask->prio]->next;

        for (int prio = PRIO_HIGHEST; prio <= PRIO_LOWEST; prio++)
        {
            if (readyList[prio])
            {
                currentTask = readyList[prio];
                break;
            }
        }
    }

    void SysTick_OS_Tick(unsigned int msec)
    {
        SysTick->CTRL = (0 << 2) + (1 << 1) + (0 << 0);
        SysTick->LOAD = (unsigned int)((HCLK / (8. * 1000.)) * msec + 0.5);
        SysTick->VAL = 0;
        Macro_Set_Bit(SysTick->CTRL, 0);
    }

    void OSTickDelay(int delay_time)
    {
        DISABLE_INTERRUPTS();

        Task *currentTask = rtos.getCurrentTask();

        currentTask->state = STATE_BLOCKED;
        currentTask->tick_ready = rtos.timeTick + delay_time;

        rtos.DeleteTCBFromReadyList(currentTask);
        rtos.InsertTCBToDelayList(currentTask);

        RUN_CONTEXT_SWITCH();

        ENABLE_INTERRUPTS();
    }

    static void _IdleTask(void *para)
    {
        Uart_Printf("IDLE\n");
        for (;;)
            ;
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
        InsertTCBToFreeList(&tcb[i]);
    }
}

void LivingRTOS::InsertTCBToFreeList(Task *task)
{
    task->next = free_list;
    free_list = task;
}

Task *LivingRTOS::GetTCBFromFreeList(void)
{
    if (free_list == nullptr)
        return nullptr;

    Task *ret = free_list;
    free_list = free_list->next;

    return ret;
}

void LivingRTOS::InsertTCBToReadyList(Task *task)
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

void LivingRTOS::DeleteTCBFromReadyList(Task *task)
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

char *LivingRTOS::GetStack(int size)
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

int LivingRTOS::CreateTask(void (*ptask)(void *), void *para, int prio, int size_stack)
{
    Task *task = GetTCBFromFreeList();

    if (task == nullptr)
        return OS_FAIL_ALLOCATE_TCB;

    task->top_of_stack = (unsigned long *)GetStack(size_stack);

    if (task->top_of_stack == nullptr)
    {
        InsertTCBToFreeList(task);
        return OS_FAIL_ALLOCATE_STACK;
    }

    task->prio = prio;
    task->state = STATE_READY;

    task->top_of_stack -= 16;
    task->top_of_stack[8] = (unsigned long)para;
    task->top_of_stack[14] = (unsigned long)ptask;
    task->top_of_stack[15] = INIT_PSR;

    InsertTCBToReadyList(task);

    return task->no_task;
}

void LivingRTOS::CheckReadyList(void)
{
    Uart_Printf("HELLO\n");

    for (int prio = PRIO_HIGHEST; prio < NUM_PRIO; prio++) // NUM_PRIO
    {
        Task *task = readyList[prio];

        Uart_Printf("%d_READY_TASK: ", prio);

        if (task == nullptr)
        {
            Uart_Printf("\n");
            continue;
        }

        do
        {
            Uart_Printf("%d ", task->no_task);
            task = task->next;
        } while (task != readyList[prio]);

        Uart_Printf("\n");
    }
}

void LivingRTOS::DeleteTask(int task_no)
{
    if (task_no >= 0 && task_no < MAX_TCB)
    {
        Task *task = &tcb[task_no];
        if (task->state == STATE_READY)
        {
            DeleteTCBFromReadyList(task);
        }

        task->state = STATE_BLOCKED;
        InsertTCBToFreeList(task);
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
    CreateTask(_IdleTask, nullptr, PRIO_LOWEST, 128);

    SwitchingTask();

    // Priority
    SCB->SHP[15 - 4] = 0xf << 4;
    SCB->SHP[14 - 4] = 0xf << 4;

    // Interrupt Exception Priority
    for (int i = 0; i <= 42; i++)
    {
        NVIC_SetPriority((IRQn_Type)i, 0xe);
    }

    SysTick_OS_Tick(TICK_MS);
    _OS_Start_First_Task();
}

void LivingRTOS::InsertTCBToDelayList(Task *ptask)
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

void LivingRTOS::DeleteTCBFromDelayList(Task *ptask)
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

Task *LivingRTOS::getDelayList(void)
{
    return rtos.delayList;
}

Task **LivingRTOS::getReadyList(void)
{
    return rtos.readyList;
}