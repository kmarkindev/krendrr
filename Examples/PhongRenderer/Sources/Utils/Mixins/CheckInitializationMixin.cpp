#include "CheckInitializationMixin.h"
#include <stdexcept>

namespace kRendrr
{
    void CheckInitializationMixin::MarkAsInitialized() const
    {
        if(bWasInitialized)
        {
            throw std::runtime_error("Double initialization detected");
        }

        bWasInitialized = true;
    }

    void CheckInitializationMixin::CheckInitialization(bool bShouldBeInitialized) const
    {
        if(!bWasInitialized && bShouldBeInitialized)
        {
            throw std::runtime_error("Unexpected call before initialization");
        }
        else if(bWasInitialized && !bShouldBeInitialized)
        {
            throw std::runtime_error("Unexpected call after initialization");
        }
    }

}
