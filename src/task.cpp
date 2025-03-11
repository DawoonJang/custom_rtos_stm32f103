

#include "device_driver.h"

void CustomQ::push(const int val)
{
    data.push_back(val);
}

int CustomQ::pop(void)
{
    if (data.empty())
    {
        return -1;
    }

    int val = data.front();
    data.erase(data.begin());
    return val;
}

bool CustomQ::empty(void) const
{
    return data.empty();
}

size_t CustomQ::size(void) const
{
    return data.size();
}

// TODO
bool Task::isQueueFull(void) const
{
    return false;
}

bool Task::isQueueEmpty(void) const
{
    return q.empty();
}

bool Task::enQueue(const int val)
{
    scopedItrLock lock;

    if (isQueueFull())
    {
        return false;
    }
    q.push(val);

    return true;
}

bool Task::deQueue(int *const val, const int timeout)
{
    // scopedItrLock lock;
    enable_interrupts();

    if (!isQueueEmpty())
    {
        *val = q.pop();

        return true;
    }

    state = STATE_BLOCKED;
    currentTick = getCurrentTick() + timeout;

    disable_interrupts();

    waitForReadyState();

    return false;
}

void Task::waitForReadyState(void)
{
    // Task가 block 상태일 경우 Ready로 전환될 때까지 대기
    while (state == STATE_BLOCKED && getCurrentTick() < currentTick)
    {
        // OS Context Switching 필요
    }
    state = STATE_READY;
}

int Task::getCurrentTick(void)
{
    return currentTick;
}
