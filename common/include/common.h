#pragma once
#include <windows.h>
#include <beacon.h>
#include <Native.h>
#include <Macros.h>

// Define size of sleepmask and BOF memory
#define SZ_SLEEPMASK            PAGE_SIZE * 10
#define SZ_BOF                  PAGE_SIZE * 20

// Define all of the DLLs that we require beyond Ntdll and Kernel32
#define DLL_LIST \
    DLL_ENTRY( User32 )

#define API_LIST \
    \
    /*Ntdll.dll*/ \
    \
    API_ENTRY( RtlAllocateHeap, Ntdll ) \
    API_ENTRY( NtProtectVirtualMemory, Ntdll ) \
    \
    /*Kernel32.dll*/\
    \
    API_ENTRY( LoadLibraryA, Kernel32 ) \
    \
    /*Msvcrt.dll*/\
    \
    /*User32.dll*/\
    \
    API_ENTRY( MessageBoxW, User32 ) \


typedef struct _CUSTOM_DATA
{

    SIZE_T szStomp;
    SIZE_T szBeacon;

};

