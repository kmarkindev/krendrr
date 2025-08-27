#include <windows.h>

extern "C" {
    __declspec(dllexport) extern const UINT D3D12SDKVersion = 616; // Agility SDK version 1.616.1, this integer is taken from Agility SDK download page
    __declspec(dllexport) extern const char* D3D12SDKPath = "..\\D3D12\\x64";
}