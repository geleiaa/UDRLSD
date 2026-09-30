#ifndef STARDUST_COMMON_H
#define STARDUST_COMMON_H

//
// system headers
//
#include <windows.h>

//
// stardust headers
//
#include <common.h>
#include <Ldr.h>
#include <Defs.h>
#include <Utils.h>

// Added define and include to be able to use D_API ( DllMain )
#define _DECL_DLLMAIN
#include <process.h>

//
// stardust instances
//
EXTERN_C ULONG __Instance_offset;
EXTERN_C PVOID __Instance;

typedef struct _INSTANCE {

    //
    // base address and size
    // of the implant
    //
    BUFFER Base;

    struct {

        // Populate struct using macro
        #define API_ENTRY(x, y) D_API(x)
        API_LIST
        #undef API_ENTRY

        // Beacon entry point
        D_API( DllMain )

    } Win32;

    struct {

        PVOID Ntdll;
        PVOID Kernel32;

        // Populate other libraries via macro
        #define DLL_ENTRY(x) PVOID x;
        //DLL_ENTRY( User32 )
        DLL_LIST
        #undef DLL_ENTRY
        
    } Modules;

    PCUSTOM_DATA cData;

} INSTANCE, *PINSTANCE;

EXTERN_C PVOID StRipStart();
EXTERN_C PVOID StRipEnd();

VOID Main(
    _In_ PVOID Param
);

#endif //STARDUST_COMMON_H
