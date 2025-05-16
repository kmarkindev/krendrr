#pragma once

#define WINDOWS_LEAN_AND_MEAN
#include <stdexcept>
#include <string_view>
#include <windows.h>

namespace kRendrr
{
    class HResultException : public std::runtime_error
    {
    public:

        explicit HResultException(HRESULT HResult);

        explicit HResultException(std::string_view MessageOverride);

    };

    struct HResultCheck
    {
        std::string_view ErrorMessage {};

        explicit HResultCheck(std::string_view ErrorMessage = {})
            : ErrorMessage(ErrorMessage)
        {

        }
    };

    bool operator >> (HRESULT Result, const HResultCheck& HResultCheck);
}
