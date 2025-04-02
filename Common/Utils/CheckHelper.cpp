#include "CheckHelper.h"
#include <stdexcept>

bool operator >> (HRESULT Result, const Check& ck)
{
    if(FAILED(Result))
    {
        throw std::runtime_error(ck.outMsg);
    }

    return true;
}
