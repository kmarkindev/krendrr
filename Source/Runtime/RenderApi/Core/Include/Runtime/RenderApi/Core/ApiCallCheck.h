#pragma once

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>

// TODO: add error log
/**
 * Checks if api call was successful. If not, logs error and returns false or default value
 */
#define CHECKED(ApiCall, ErrorMessage) \
{ \
    if(FAILED(ApiCall)) \
    { \
        __debugbreak(); \
        return {}; \
    } \
} \

/**
 * Checks if api call was successful. returns false or default value if false
 */
#define CHECKED_S(ApiCall) \
{ \
    if(FAILED(ApiCall)) \
    { \
        __debugbreak(); \
        return {}; \
    } \
} \

// TODO: add error log
/**
 * Checks if api call was successful. If not, logs error and returns
 */
#define CHECKEDV(ApiCall, ErrorMessage) \
{ \
    if(FAILED(ApiCall)) \
    { \
        __debugbreak(); \
        return; \
    } \
} \

/**
 * Checks if api call was successful. Returns if not
 */
#define CHECKEDV_S(ApiCall) \
{ \
    if(FAILED(ApiCall)) \
    { \
        __debugbreak(); \
        return; \
    } \
} \
