#include "ltr.h"
#include "device_driver.h"

scopedItrLock::scopedItrLock()
{
#ifdef DEBUGDEBUG
    InterruptManager::disable_interrupts();
#else
    disable_interrupts();
#endif
}

scopedItrLock::~scopedItrLock()
{
#ifdef DEBUGDEBUG
    InterruptManager::enable_interrupts();
#else
    enable_interrupts();
#endif
}
