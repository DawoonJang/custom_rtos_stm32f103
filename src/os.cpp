#include "os.h"
#include "device_driver.h"

LivingRTOS::LivingRTOS()
{
    stack_limit = stack;
    pstack = stack + STACK_SIZE;
    Init();
}

void LivingRTOS::Init()
{
    for (int i = 0; i < NUM_PRIO; i++)
    {
        ready_list[i] = nullptr;
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

    if (ready_list[prio] == nullptr)
    {
        ready_list[prio] = task;
        task->next = task;
        task->prev = task;
    }
    else
    {
        task->prev = ready_list[prio]->prev;
        task->next = ready_list[prio];

        task->prev->next = task;
        task->next->prev = task;
    }
}

void LivingRTOS::DeleteTCBFromReadyList(Task *task)
{
    int prio = task->prio;

    task->prev->next = task->next;
    task->next->prev = task->prev;

    if (ready_list[prio] == task)
    {
        ready_list[prio] = (task->next == task) ? nullptr : ready_list[prio]->next;
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
    InsertTCBToReadyList(task);

    return task->no_task;
}

void LivingRTOS::CheckReadyList(void)
{
    Uart_Printf("HELLO\n");

    for (int prio = PRIO_HIGHEST; prio < NUM_PRIO; prio++) // NUM_PRIO
    {
        Task *task = ready_list[prio];

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
        } while (task != ready_list[prio]);

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