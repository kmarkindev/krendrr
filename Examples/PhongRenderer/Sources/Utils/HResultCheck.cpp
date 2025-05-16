#include "HResultCheck.h"

#include <stdexcept>
#include <comdef.h>

namespace kRendrr
{
    HResultException::HResultException(HRESULT HResult)
        : std::runtime_error(_com_error {HResult}.ErrorMessage())
    {

    }

    HResultException::HResultException(std::string_view MessageOverride)
        : std::runtime_error(MessageOverride.data())
    {
    }

    bool operator >> (HRESULT HResult, const HResultCheck& HResultCheck)
    {
        if(FAILED(HResult))
        {
            if (HResultCheck.ErrorMessage.empty()) {
                throw HResultException(HResult);
            } else {
                throw HResultException(HResultCheck.ErrorMessage);
            }
        }

        return true;
    }
}
