#pragma once

#include <windows.h>

namespace krendrr::Runtime::TaskFlowEx
{
    class TaskFailedException : public std::exception
    {
    public:
        [[nodiscard]] const char* what() const override
        {
            return "Taskflow was cancelled";
        }
    };

    inline void CancelCurrentTaskflow()
    {
        throw TaskFailedException();
    }
}

// TODO: add error log
/**
 * Checks if api call was successful. If not, logs error and cancels current taskflow
 */
#define CHECKED_TF(ApiCall, ErrorMessage) \
{ \
if(FAILED(ApiCall)) \
{ \
    __debugbreak(); \
    krendrr::Runtime::TaskFlowEx::CancelCurrentTaskflow(); \
} \
} \

/**
 * Checks if api call was successful. If not, cancels current taskflow
 */
#define CHECKED_TF_S(ApiCall) \
{ \
if(FAILED(ApiCall)) \
{ \
    __debugbreak(); \
    krendrr::Runtime::TaskFlowEx::CancelCurrentTaskflow(); \
} \
} \