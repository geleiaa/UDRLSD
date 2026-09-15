#pragma once
#include <windows.h>
#include <beacon.h>
#include <Native.h>
#include <Macros.h>
#include <stdio.h>

// Define size of sleepmask and BOF memory
#define SZ_SLEEPMASK            PAGE_SIZE * 10
#define SZ_BOF                  PAGE_SIZE * 20

// Define all of the DLLs that we require beyond Ntdll and Kernel32
#define DLL_LIST \
    DLL_ENTRY( User32 ) \
    DLL_ENTRY( Msvcrt )

#define API_LIST \
    \
    /*Ntdll.dll*/ \
    \
    API_ENTRY( RtlAllocateHeap, Ntdll ) \
    API_ENTRY( NtProtectVirtualMemory, Ntdll ) \
    API_ENTRY( LdrLoadDll, Ntdll ) \
    API_ENTRY( LdrGetProcedureAddress, Ntdll ) \
    \
    /*Kernel32.dll*/\
    \
    API_ENTRY( LoadLibraryA, Kernel32 ) \
    API_ENTRY( AllocConsole, Kernel32 ) \
    API_ENTRY( GetConsoleWindow, Kernel32 ) \
    API_ENTRY( HeapAlloc, Kernel32 ) \
    \
    /*Msvcrt.dll*/\
    API_ENTRY( freopen, Msvcrt ) \
    API_ENTRY( __iob_func, Msvcrt ) \
    API_ENTRY( printf, Msvcrt ) \
    API_ENTRY( _snprintf, Msvcrt ) \
    API_ENTRY( vprintf, Msvcrt ) \
    API_ENTRY( getchar, Msvcrt ) \
    API_ENTRY( calloc, Msvcrt ) \
    \
    /*User32.dll*/\
    \
    API_ENTRY( MessageBoxW, User32 ) \
    API_ENTRY( ShowWindow, User32 ) \
    API_ENTRY( SetForegroundWindow, User32 ) \
    API_ENTRY( UpdateWindow, User32 )

    EXTERN_C VOID PrintMsg(const char*, BOOL, const char*, ...);

typedef struct _CUSTOM_DATA
{
    PVOID placeholder;
} CUSTOM_DATA, *PCUSTOM_DATA;

