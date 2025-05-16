#pragma once

#include <memory>

namespace kRendrr
{
    /**
     * Sometimes we may have a case, when we share resources between different parts of our code using shared_ptr,
     * but we don't need to make dynamic allocation for this resource.
     * This is where this function comes in play: it creates a shared pointer, that doesn't manage any memory, but
     * still allows us to use all of our code and switch to any type of allocation when we need to.
     *
     * Note: ofc it means that memory should outlive all it's users.
     */
    template<typename T>
    std::shared_ptr<T> GetSharedPtrToStack(T* Pointer)
    {
        return std::shared_ptr<T>(Pointer, [](T*){});
    }

}
