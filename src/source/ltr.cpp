#include "ltr.h"
#include "device_driver.h"

scopedItrLock::scopedItrLock()
{
#ifdef DEBUG_ITR
    InterruptManager::disable_interrupts();
#else
    disable_interrupts();
#endif
}

scopedItrLock::~scopedItrLock()
{
#ifdef DEBUG_ITR
    InterruptManager::enable_interrupts();
#else
    enable_interrupts();
#endif
}
