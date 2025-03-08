#ifndef __OS_H__
#define __OS_H__

#define MAX_TCB (5)

#define PRIO_HIGHEST (0)
#define PRIO_LOWEST (10)
#define NUM_PRIO (PRIO_LOWEST - PRIO_HIGHEST + 1)

#define OS_SUCCESS (0)
#define OS_FAIL_ALLOCATE_TCB (-1)
#define OS_FAIL_ALLOCATE_STACK (-2)

#define STACK_SIZE (8 * 1024)

typedef enum _task_state
{
    STATE_READY = 0,
    STATE_BLOCKED
} E_TASK_STATE;

class Task
{
  public:
    unsigned long *top_of_stack;
    int no_task;
    int prio;
    E_TASK_STATE state;
    Task *prev;
    Task *next;

    Task() : top_of_stack(nullptr), no_task(-1), prio(0), state(STATE_READY), prev(nullptr), next(nullptr)
    {
    }
};

class LivingRTOS
{
  private:
    Task tcb[MAX_TCB];
    Task *free_list;
    Task *ready_list[NUM_PRIO];
    char stack[STACK_SIZE] __attribute__((__aligned__(8))); // Stack aligned to 8 bytes
    char *stack_limit;
    char *pstack;

  public:
    LivingRTOS();
    void Init();
    int CreateTask(void (*ptask)(void *), void *para, int prio, int size_stack);
    void CheckReadyList(void);
    void DeleteTask(int task_no);

  private:
    void InsertTCBToFreeList(Task *task);
    Task *GetTCBFromFreeList(void);
    void InsertTCBToReadyList(Task *task);
    void DeleteTCBFromReadyList(Task *task);
    char *GetStack(int size);
};

#endif /* OS_H */