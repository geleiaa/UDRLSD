#include <Common.h>
#include <Constexpr.h>


FUNC BOOL ResolveApis()
{
    STARDUST_INSTANCE

    // Resolve primary DLLs
    MOD( Ntdll ) = LdrModulePeb( H_MODULE_NTDLL );
    MOD( Kernel32 ) = LdrModulePeb( H_MODULE_KERNEL32 );

    // Resolve Load Library
    RESOLVE( LoadLibraryA, Kernel32 );

    // Validate pointers
    if ( !MOD( Ntdll ) || !MOD( Kernel32 ) || !API(LoadLibraryA) )
        return FALSE;

    // Resolve other DLLs
    #define DLL_ENTRY(mod) if ( !( MOD( mod ) = API( LoadLibraryA )( #mod ) ) ) return FALSE;
    DLL_LIST
    #undef DLL_ENTRY

    // Now resolve all other APIs 
    #define API_ENTRY(api, mod) if ( !( RESOLVE( api, mod ) ) ) return FALSE;
    API_LIST
    #undef API_ENTRY

    return TRUE;
}


FUNC VOID Main(
    _In_ PVOID Param
) {
    STARDUST_INSTANCE
    //PCUSTOM_DATA            cData               = { 0 };
    PIMAGE_DOS_HEADER       DOS_Beacon          = { 0 };
    PIMAGE_NT_HEADERS       NT_Beacon           = { 0 };
    PVOID                   Message             = { 0 };

     // Resolve APIs
    if (!ResolveApis())
    {
        return;
    }

    Message = NtCurrentPeb()->ProcessParameters->ImagePathName.Buffer;

    //
    // pop da message
    //
    API( MessageBoxW )( NULL, Message, L"Stardust MessageBox", MB_OK );
    
    //cData = API( Instance()->Base.Buffer, Instance()->Base.Length);
    //Instance()->cData = cData;

    //DOS_Beacon = PADD( Instance()->Base.Buffer, Instance()->Base.Length );
    //NT_Beacon = PADD( DOS_Beacon, DOS_Beacon->e_lfanew );

    //// Store size of Beacon and calculate total stomp size
	//cData->szBeacon = NT_Beacon->OptionalHeader.SizeOfImage;
    //cData->szStomp = cData->szBeacon + SZ_SLEEPMASK + SZ_BOF;
}

