#pragma once

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>

struct Check
{
    const char* outMsg = "HRESULT failed";
};

bool operator >> (HRESULT Result, const Check& ck);