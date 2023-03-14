// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            if (lpReserved != nullptr)
            {
                break; // do not do cleanup if process termination scenario
            }
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

