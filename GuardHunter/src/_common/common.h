#pragma once
#include <../../ext/ia32.h>
#include <ntddk.h>
#include <intrin.h>

#pragma warning(push)
#pragma warning(disable: 4201)

//
// Debug definitions.
//

#define DBG  1

#define DBG_SUCCESS_PREFIX "GuardHunter [+] "
#define DBG_ABORTED_PREFIX "GuardHunter [-] "
#define DBG_WARNING_PREFIX "GuardHunter [!] "

#define DbgLog(fmt, ...)    \
do {                        \
    if (DBG) {              \
        DbgPrintEx(         \
        DPFLTR_SYSTEM_ID,   \
        DPFLTR_ERROR_LEVEL, \
        fmt,                \
        ##__VA_ARGS__);     \
    }                       \
} while (FALSE)

#define DBG_BREAK                          \
do {                                       \
    if (DBG) {                             \
       DbgLog(DBG_WARNING_PREFIX           \
              "DebugBreak.\n");            \
                                           \
       DbgLog("FilePath: %s\n", __FILE__); \
                                           \
       DbgLog("FileLine: %d\n", __LINE__); \
                                           \
        __debugbreak();                    \
    }                                      \
} while (FALSE)
          
//
// Internal status definitions.
//

typedef UINT32 HR_STATUS;
#define HR_SUCCESS 0x02FC9E0AUI32
#define HR_ABORTED 0xA9F34231UI32
#define HR_ERROR(status) ((status) != HR_SUCCESS)

//
// Quick obfuscation definitions.
//

#define QUICK_XOR64(Value) (((UINT64)(Value)) ^ 0xA38E4B46EF9F8246UI64)
#define QUICK_XOR32(Value) (((UINT32)(Value)) ^ 0x6B849270UI32)

//
// Memory management definitions.
//

#define REQUIRED_NUMBER_OF_PAGES(Size) \
((((PAGE_SIZE - 1) + (Size)) & ~(PAGE_SIZE - 1)) >> PAGE_SHIFT)

#define IS_RWX_TABLE_ENTRY(pPte) \
((*((UINT64*)(pPte)) & (PTE_64_WRITE_FLAG | PTE_64_EXECUTE_DISABLE_FLAG)) == \
PTE_64_WRITE_FLAG)

#define IS_CANONICAL_SYSTEM_VA(Va) \
(((((UINT64)(Va)) >> 47) & 0x1FFFF) == 0x1FFFF)

#define GET_PML4_ENTRY_ADDRESS(PxeBase, Va) \
(((UINT64)(PxeBase)) + ((((UINT64)(Va)) >> 36) & 0xFF8))

#define GET_PDPT_ENTRY_ADDRESS(PpeBase, Va) \
(((UINT64)(PpeBase)) + ((((UINT64)(Va)) >> 27) & 0x1FFFF8))

#define GET_PD_ENTRY_ADDRESS(PdeBase, Va) \
(((UINT64)(PdeBase)) + ((((UINT64)(Va)) >> 18) & 0x3FFFFFF8))

#define GET_PT_ENTRY_ADDRESS(PteBase, Va) \
(((UINT64)(PteBase)) + ((((UINT64)(Va)) >> 9) & 0x7FFFFFFFF8))

//
// CPU definitions.
//

#define IS_INTERRUPTS_ENABLED \
((BOOLEAN)(EFLAGS_INTERRUPT_ENABLE_FLAG(__readeflags())))

#define IS_WRITE_PROTECTION_ENABLED \
((BOOLEAN)(CR0_WRITE_PROTECT(__readcr0())))

//
// PE32+ definitions.
//

#include <pshpack2.h> // For 16-bit headers. 

#define IMAGE_DOS_SIGNATURE 0x5A4D // 'MZ'.

typedef struct _IMAGE_DOS_HEADER {
    UINT16 e_magic;
    UINT16 e_cblp;
    UINT16 e_cp;
    UINT16 e_crlc;
    UINT16 e_cparhdr;
    UINT16 e_minalloc;
    UINT16 e_maxalloc;
    UINT16 e_ss;
    UINT16 e_sp;
    UINT16 e_csum;
    UINT16 e_ip;
    UINT16 e_cs;
    UINT16 e_lfarlc;
    UINT16 e_ovno;
    UINT16 e_res[4];
    UINT16 e_oemid;
    UINT16 e_oeminfo;
    UINT16 e_res2[10];
    INT32 e_lfanew;
} IMAGE_DOS_HEADER;

#include <poppack.h>

#include <pshpack4.h> // For 32/64-bit headers.

#define IMAGE_FILE_MACHINE_UNKNOWN           0
#define IMAGE_FILE_MACHINE_TARGET_HOST       0x0001  // Useful for indicating we want to interact with the host and not a WoW guest.
#define IMAGE_FILE_MACHINE_I386              0x014C  // Intel 386.
#define IMAGE_FILE_MACHINE_R3000             0x0162  // MIPS little-endian, 0x160 big-endian.
#define IMAGE_FILE_MACHINE_R4000             0x0166  // MIPS little-endian.
#define IMAGE_FILE_MACHINE_R10000            0x0168  // MIPS little-endian.
#define IMAGE_FILE_MACHINE_WCEMIPSV2         0x0169  // MIPS little-endian WCE v2.
#define IMAGE_FILE_MACHINE_ALPHA             0x0184  // Alpha_AXP.
#define IMAGE_FILE_MACHINE_SH3               0x01A2  // SH3 little-endian.
#define IMAGE_FILE_MACHINE_SH3DSP            0x01A3
#define IMAGE_FILE_MACHINE_SH3E              0x01A4  // SH3E little-endian.
#define IMAGE_FILE_MACHINE_SH4               0x01A6  // SH4 little-endian.
#define IMAGE_FILE_MACHINE_SH5               0x01A8  // SH5.
#define IMAGE_FILE_MACHINE_ARM               0x01C0  // ARM Little-Endian.
#define IMAGE_FILE_MACHINE_THUMB             0x01C2  // ARM Thumb/Thumb-2 Little-Endian.
#define IMAGE_FILE_MACHINE_ARMNT             0x01C4  // ARM Thumb-2 Little-Endian.
#define IMAGE_FILE_MACHINE_AM33              0x01D3
#define IMAGE_FILE_MACHINE_POWERPC           0x01F0  // IBM PowerPC Little-Endian.
#define IMAGE_FILE_MACHINE_POWERPCFP         0x01F1
#define IMAGE_FILE_MACHINE_IA64              0x0200  // Intel 64.
#define IMAGE_FILE_MACHINE_MIPS16            0x0266  // MIPS.
#define IMAGE_FILE_MACHINE_ALPHA64           0x0284  // ALPHA64.
#define IMAGE_FILE_MACHINE_MIPSFPU           0x0366  // MIPS.
#define IMAGE_FILE_MACHINE_MIPSFPU16         0x0466  // MIPS.
#define IMAGE_FILE_MACHINE_AXP64             IMAGE_FILE_MACHINE_ALPHA64
#define IMAGE_FILE_MACHINE_TRICORE           0x0520  // Infineon.
#define IMAGE_FILE_MACHINE_CEF               0x0CEF
#define IMAGE_FILE_MACHINE_EBC               0x0EBC  // EFI Byte Code.
#define IMAGE_FILE_MACHINE_AMD64             0x8664  // AMD64 (K8).
#define IMAGE_FILE_MACHINE_M32R              0x9041  // M32R little-endian.
#define IMAGE_FILE_MACHINE_ARM64             0xAA64  // ARM64 Little-Endian.
#define IMAGE_FILE_MACHINE_CEE               0xC0EE

#define IMAGE_FILE_RELOCS_STRIPPED           0x0001  // Relocation info stripped from file.
#define IMAGE_FILE_EXECUTABLE_IMAGE          0x0002  // File is executable  (i.e. no unresolved external references).
#define IMAGE_FILE_LINE_NUMS_STRIPPED        0x0004  // Line nunbers stripped from file.
#define IMAGE_FILE_LOCAL_SYMS_STRIPPED       0x0008  // Local symbols stripped from file.
#define IMAGE_FILE_AGGRESIVE_WS_TRIM         0x0010  // Aggressively trim working set.
#define IMAGE_FILE_LARGE_ADDRESS_AWARE       0x0020  // App can handle >2gb addresses.
#define IMAGE_FILE_BYTES_REVERSED_LO         0x0080  // Bytes of machine word are reversed.
#define IMAGE_FILE_32BIT_MACHINE             0x0100  // 32 bit word machine.
#define IMAGE_FILE_DEBUG_STRIPPED            0x0200  // Debugging info stripped from file in *.DBG* file.
#define IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP   0x0400  // If Image is on removable media, copy and run from the swap file.
#define IMAGE_FILE_NET_RUN_FROM_SWAP         0x0800  // If Image is on Net, copy and run from the swap file.
#define IMAGE_FILE_SYSTEM                    0x1000  // System File.
#define IMAGE_FILE_DLL                       0x2000  // File is a DLL.
#define IMAGE_FILE_UP_SYSTEM_ONLY            0x4000  // File should only be run on a UP machine.
#define IMAGE_FILE_BYTES_REVERSED_HI         0x8000  // Bytes of machine word are reversed.

typedef struct _IMAGE_FILE_HEADER {
    UINT16 Machine;
    UINT16 NumberOfSections;
    UINT32 TimeDateStamp;
    UINT32 PointerToSymbolTable;
    UINT32 NumberOfSymbols;
    UINT16 SizeOfOptionalHeader;
    UINT16 Characteristics;
} IMAGE_FILE_HEADER;

typedef struct _IMAGE_DATA_DIRECTORY {
    UINT32 VirtualAddress;
    UINT32 Size;
} IMAGE_DATA_DIRECTORY;

#define IMAGE_DIRECTORY_ENTRY_EXPORT          0  // Export Directory.
#define IMAGE_DIRECTORY_ENTRY_IMPORT          1  // Import Directory.
#define IMAGE_DIRECTORY_ENTRY_RESOURCE        2  // Resource Directory.
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION       3  // Exception Directory.
#define IMAGE_DIRECTORY_ENTRY_SECURITY        4  // Security Directory.
#define IMAGE_DIRECTORY_ENTRY_BASERELOC       5  // Base Relocation Table.
#define IMAGE_DIRECTORY_ENTRY_DEBUG           6  // Debug Directory.
//      IMAGE_DIRECTORY_ENTRY_COPYRIGHT       7  // (X86 usage).
#define IMAGE_DIRECTORY_ENTRY_ARCHITECTURE    7  // Architecture Specific Data.
#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR       8  // RVA of GP.
#define IMAGE_DIRECTORY_ENTRY_TLS             9  // TLS Directory.
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG    10  // Load Configuration Directory.
#define IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT   11  // Bound Import Directory in headers.
#define IMAGE_DIRECTORY_ENTRY_IAT            12  // Import Address Table.
#define IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT   13  // Delay Load Import Descriptors.
#define IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR 14  // COM Runtime descriptor.

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES 16

#define IMAGE_NT_OPTIONAL_HDR32_MAGIC 0x10B
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC 0x20B
#define IMAGE_ROM_OPTIONAL_HDR_MAGIC  0x107

typedef struct _IMAGE_OPTIONAL_HEADER64 {
    UINT16 Magic;
    UINT8 MajorLinkerVersion;
    UINT8 MinorLinkerVersion;
    UINT32 SizeOfCode;
    UINT32 SizeOfInitializedData;
    UINT32 SizeOfUninitializedData;
    UINT32 AddressOfEntryPoint;
    UINT32 BaseOfCode;
    UINT64 ImageBase;
    UINT32 SectionAlignment;
    UINT32 FileAlignment;
    UINT16 MajorOperatingSystemVersion;
    UINT16 MinorOperatingSystemVersion;
    UINT16 MajorImageVersion;
    UINT16 MinorImageVersion;
    UINT16 MajorSubsystemVersion;
    UINT16 MinorSubsystemVersion;
    UINT32 Win32VersionValue;
    UINT32 SizeOfImage;
    UINT32 SizeOfHeaders;
    UINT32 CheckSum;
    UINT16 Subsystem;
    UINT16 DllCharacteristics;
    UINT64 SizeOfStackReserve;
    UINT64 SizeOfStackCommit;
    UINT64 SizeOfHeapReserve;
    UINT64 SizeOfHeapCommit;
    UINT32 LoaderFlags;
    UINT32 NumberOfRvaAndSizes;
    IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER64;

typedef struct _IMAGE_EXPORT_DIRECTORY {
    UINT32 Characteristics;
    UINT32 TimeDateStamp;
    UINT16 MajorVersion;
    UINT16 MinorVersion;
    UINT32 Name;
    UINT32 Base;
    UINT32 NumberOfFunctions;
    UINT32 NumberOfNames;
    UINT32 AddressOfFunctions;    // RVA from base of image.
    UINT32 AddressOfNames;        // RVA from base of image.
    UINT32 AddressOfNameOrdinals; // RVA from base of image.
} IMAGE_EXPORT_DIRECTORY;

#define IMAGE_NT_SIGNATURE 0x00004550 // 'PE00'.

typedef struct _IMAGE_NT_HEADERS64 {
    UINT32 Signature;
    IMAGE_FILE_HEADER FileHeader;
    IMAGE_OPTIONAL_HEADER64 OptionalHeader;
} IMAGE_NT_HEADERS64;

#define IMAGE_SIZEOF_SHORT_NAME 8

typedef struct _IMAGE_SECTION_HEADER {
    UINT8 Name[IMAGE_SIZEOF_SHORT_NAME];
    union {
        UINT32 PhysicalAddress;
        UINT32 VirtualSize;
    } Misc;
    UINT32 VirtualAddress;
    UINT32 SizeOfRawData;
    UINT32 PointerToRawData;
    UINT32 PointerToRelocations;
    UINT32 PointerToLinenumbers;
    UINT16 NumberOfRelocations;
    UINT16 NumberOfLinenumbers;
    UINT32 Characteristics;
} IMAGE_SECTION_HEADER;

#include <poppack.h> // Back to default 8-byte packing ( /Zp8 ).

typedef struct _IMAGE_RUNTIME_FUNCTION_ENTRY {
    UINT32 BeginAddress;
    UINT32 EndAddress;
    union {
        UINT32 UnwindInfoAddress;
        UINT32 UnwindData;
    } DUMMYUNIONNAME;
} IMAGE_RUNTIME_FUNCTION_ENTRY, RUNTIME_FUNCTION;

// 
// NTOS-structures definitions.
//

typedef struct _KDPC_LIST {
    SINGLE_LIST_ENTRY ListHead;
    SINGLE_LIST_ENTRY *pLastEntry;
} KDPC_LIST;

typedef struct _KDPC_DATA {
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    KDPC_LIST DpcList;
#else
    LIST_ENTRY DpcListHead;
#endif
    UINT64 DpcLock;
#if defined(_M_AMD64) || defined(_M_ARM)
    volatile INT32 DpcQueueDepth;
#else
    volatile UINT32 DpcQueueDepth;
#endif
    UINT32 DpcCount;
#if (NTDDI_VERSION >= NTDDI_LONGHORN) || defined(_M_ARM)
    KDPC *pActiveDpc;
#endif
} KDPC_DATA;

typedef struct _KTIMER2 {
    DISPATCHER_HEADER Header;
    union {
        RTL_BALANCED_NODE RbNodes[2];
        LIST_ENTRY ListEntry;
    };
    UINT64 DueTime[2];
    INT64 Period;
    VOID (FASTCALL *pCallback) (VOID *pTimer2, VOID *pCallbackContext);
    VOID *pCallbackContext;
    VOID (FASTCALL *pDisableCallback) (VOID *pDisableContext);
    VOID *pDisableContext;
    UINT8 AbsoluteSystemTime;
    union {
        UINT8 TypeFlags;
        struct {
            UINT8 Unused : 1;
            UINT8 IdleResilient : 1;
            UINT8 HighResolution : 1;
            UINT8 NoWake : 1;
            UINT8 PseudoHighRes : 1;
            UINT8 AusterityResilient : 1;
            UINT8 Unused1 : 2;
        };
    };
    UINT8 CollectionIndex[2];
} KTIMER2;

typedef struct KAPC_STATE {
    LIST_ENTRY ApcListHead[2];
    PKPROCESS pProcess;
    union {
        UINT8 InProgressFlags;
        struct {
            UINT8 KernelApcInProgress : 1;
            UINT8 SpecialApcInProgress : 1;
        };
    };
    UINT8 KernelApcPending;
    union {
        UINT8 UserApcPendingAll;
        struct {
            UINT8 SpecialUserApcPending : 1;
            UINT8 UserApcPending : 1;
        };
    };
} KAPC_STATE;

typedef struct _KAPC2 {
    UINT8 Type;
    union {
        UINT8 AllFlags;
        struct {
            UINT8 CallbackDataContext : 1;
            UINT8 Unused : 7;
        };
    };
    UINT8 Size;
    UINT8 SpareByte1;
    UINT32 SpareLong0;
    VOID *pThread;
    LIST_ENTRY ApcListEntry;
    union {
        struct {
            VOID(FASTCALL *pKernelRoutine)(
                VOID *pApc,
                VOID *pNormalRoutine,
                VOID *pNormalContext,
                VOID *pSystemArgument1,
                VOID *pSystemArgument2);
            VOID(FASTCALL *pRundownRoutine) (
                VOID *pApc);
            VOID(FASTCALL *pNormalRoutine)(
                VOID *pNormalContext,
                VOID *pSystemArgument1,
                VOID *pSystemArgument2);
        };
        VOID* pReserved[3];
    };
    VOID *pNormalContext;
    VOID *pSystemArgument1;
    VOID *pSystemArgument2;
    INT8 ApcStateIndex;
    INT8 ApcMode;
    UINT8 Inserted;
} KAPC2;

typedef enum _KAPC_ENVIRONMENT {
    OriginalApcEnvironment,
    AttachedApcEnvironment,
    CurrentApcEnvironment,
    InsertApcEnvironment
} KAPC_ENVIRONMENT;

typedef struct _MMPTE_HARDWARE {
    UINT64 Valid : 1;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    UINT64 Dirty1 : 1;
#else
#ifdef CONFIG_SMP
    UINT64 Writable : 1;
#else
    UINT64 Write : 1;
#endif
#endif
    UINT64 Owner : 1;
    UINT64 WriteThrough : 1;
    UINT64 CacheDisable : 1;
    UINT64 Accessed : 1;
    UINT64 Dirty : 1;
    UINT64 LargePage : 1;
    UINT64 Global : 1;
    UINT64 CopyOnWrite : 1;
    UINT64 Prototype : 1;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    UINT64 Write : 1;
    UINT64 PageFrameNumber : 36;
    UINT64 reserved1 : 4;
#else
#ifdef CONFIG_SMP
    UINT64 Write : 1;
#else
    UINT64 reserved0 : 1;
#endif
    UINT64 PageFrameNumber : 28;
    UINT64 reserved1 : 12;
#endif
    UINT64 SoftwareWsIndex : 11;
    UINT64 NoExecute : 1;
} MMPTE_HARDWARE;

//
// This is a reconstructed type definition,
// as the structure used in the
// RtlIcParseInstruction routine could not be found in *ntoskrnl.pdb*. 
// 
// Notes:
// 
// To quickly check for changes in the
// RTL_IC_CONTEXT structure following potential future NTOS updates,
// monitor the output of RtlIcCompletionContextSize,
// which returns its actual size.
//
typedef struct _RTL_IC_CONTEXT {
    UINT8  AddressSize;              // 0x00
    UINT8  OperandSize;              // 0x01
    UINT8  DstOperandSize;           // 0x02
    UINT8  ImmediateSizeBits;        // 0x03
    UINT8  DecodeFlags;              // 0x04
    UINT8  InstructionBufferSize;    // 0x05
    UINT8  InstructionBuffer[15];    // 0x06
    UINT8  OffsetOpcode;             // 0x15
    UINT8  ModRmLength;              // 0x16
    UINT8  ImmediateLength;          // 0x17
    UINT8  InstructionLength;        // 0x18
    UINT8  RexPrefix;                // 0x19
    UINT8  ModRm;                    // 0x1A
    UINT8  Sib;                      // 0x1B
    INT32  Displacement;             // 0x1C
    UINT64 ImmediateValue;           // 0x20
} RTL_IC_CONTEXT;

//
// This is a reconstructed type definition,
// as the structure used in the
// KiTpReadImageData routine could not be found in *ntoskrnl.pdb*. 
// 
// Notes:
// 
// This context structure is used by the 
// KiTpReadImageData routine to read data from a target image.
// The routine leverages a supplied KPROCESS pointer to attach to the
// address space of the specified process.
//
typedef struct _KTP_READ_CONTEXT {
    PKPROCESS Process;               // 0x00
    UINT64    Unk0;                  // 0x08
    VOID     *pAddress;              // 0x10
    VOID     *pBuffer;               // 0x18
    UINT32    BufferSize;            // 0x20
    UINT8     KTpReadContextPad0[4]; // 0x24
} KTP_READ_CONTEXT;

//
// A field offset table is used for the remaining NTOS structures to avoid
// redundant definitions for different NTOS builds.
//

//
// NTOS&USER-structures definitions.
//

typedef struct _DBGKD_DEBUG_DATA_HEADER64 {
    LIST_ENTRY64 List;
    UINT32 OwnerTag;
    UINT32 Size;
} DBGKD_DEBUG_DATA_HEADER64;

#define ULPTR64 UINT64

typedef struct _KDDEBUGGER_DATA64 {
    DBGKD_DEBUG_DATA_HEADER64 Header;
    UINT64 KernBase;
    ULPTR64 BreakpointWithStatus;
    UINT64 SavedContext;
    USHORT ThCallbackStack;
    USHORT NextCallback;
    USHORT FramePointer;
    USHORT PaeEnabled : 1;
    ULPTR64 KiCallUserMode;
    UINT64 KeUserCallbackDispatcher;
    ULPTR64 PsLoadedModuleList;
    ULPTR64 PsActiveProcessHead;
    ULPTR64 PspCidTable;
    ULPTR64 ExpSystemResourcesList;
    ULPTR64 ExpPagedPoolDescriptor;
    ULPTR64 ExpNumberOfPagedPools;
    ULPTR64 KeTimeIncrement;
    ULPTR64 KeBugCheckCallbackListHead;
    ULPTR64 KiBugcheckData;
    ULPTR64 IopErrorLogListHead;
    ULPTR64 ObpRootDirectoryObject;
    ULPTR64 ObpTypeObjectType;
    ULPTR64 MmSystemCacheStart;
    ULPTR64 MmSystemCacheEnd;
    ULPTR64 MmSystemCacheWs;
    ULPTR64 MmPfnDatabase;
    ULPTR64 MmSystemPtesStart;
    ULPTR64 MmSystemPtesEnd;
    ULPTR64 MmSubsectionBase;
    ULPTR64 MmNumberOfPagingFiles;
    ULPTR64 MmLowestPhysicalPage;
    ULPTR64 MmHighestPhysicalPage;
    ULPTR64 MmNumberOfPhysicalPages;
    ULPTR64 MmMaximumNonPagedPoolInBytes;
    ULPTR64 MmNonPagedSystemStart;
    ULPTR64 MmNonPagedPoolStart;
    ULPTR64 MmNonPagedPoolEnd;
    ULPTR64 MmPagedPoolStart;
    ULPTR64 MmPagedPoolEnd;
    ULPTR64 MmPagedPoolInformation;
    UINT64 MmPageSize;
    ULPTR64 MmSizeOfPagedPoolInBytes;
    ULPTR64 MmTotalCommitLimit;
    ULPTR64 MmTotalCommittedPages;
    ULPTR64 MmSharedCommit;
    ULPTR64 MmDriverCommit;
    ULPTR64 MmProcessCommit;
    ULPTR64 MmPagedPoolCommit;
    ULPTR64 MmExtendedCommit;
    ULPTR64 MmZeroedPageListHead;
    ULPTR64 MmFreePageListHead;
    ULPTR64 MmStandbyPageListHead;
    ULPTR64 MmModifiedPageListHead;
    ULPTR64 MmModifiedNoWritePageListHead;
    ULPTR64 MmAvailablePages;
    ULPTR64 MmResidentAvailablePages;
    ULPTR64 PoolTrackTable;
    ULPTR64 NonPagedPoolDescriptor;
    ULPTR64 MmHighestUserAddress;
    ULPTR64 MmSystemRangeStart;
    ULPTR64 MmUserProbeAddress;
    ULPTR64 KdPrintCircularBuffer;
    ULPTR64 KdPrintCircularBufferEnd;
    ULPTR64 KdPrintWritePointer;
    ULPTR64 KdPrintRolloverCount;
    ULPTR64 MmLoadedUserImageList;

#if (NTDDI_VERSION >= NTDDI_WINXP)
    ULPTR64 NtBuildLab;
    ULPTR64 KiNormalSystemCall;
#endif

    /* NOTE: Documented as "NT 5.0 hotfix (QFE) addition". */
#if (NTDDI_VERSION >= NTDDI_WIN2KSP4)
    ULPTR64 KiProcessorBlock;
    ULPTR64 MmUnloadedDrivers;
    ULPTR64 MmLastUnloadedDriver;
    ULPTR64 MmTriageActionTaken;
    ULPTR64 MmSpecialPoolTag;
    ULPTR64 KernelVerifier;
    ULPTR64 MmVerifierData;
    ULPTR64 MmAllocatedNonPagedPool;
    ULPTR64 MmPeakCommitment;
    ULPTR64 MmTotalCommitLimitMaximum;
    ULPTR64 CmNtCSDVersion;
#endif

#if (NTDDI_VERSION >= NTDDI_WINXP)
    ULPTR64 MmPhysicalMemoryBlock;
    ULPTR64 MmSessionBase;
    ULPTR64 MmSessionSize;
    ULPTR64 MmSystemParentTablePage;
#endif

#if (NTDDI_VERSION >= NTDDI_WS03)
    ULPTR64 MmVirtualTranslationBase;
    UINT16 OffsetKThreadNextProcessor;
    UINT16 OffsetKThreadTeb;
    UINT16 OffsetKThreadKernelStack;
    UINT16 OffsetKThreadInitialStack;
    UINT16 OffsetKThreadApcProcess;
    UINT16 OffsetKThreadState;
    UINT16 OffsetKThreadBStore;
    UINT16 OffsetKThreadBStoreLimit;
    UINT16 SizeEProcess;
    UINT16 OffsetEprocessPeb;
    UINT16 OffsetEprocessParentCID;
    UINT16 OffsetEprocessDirectoryTableBase;
    UINT16 SizePrcb;
    UINT16 OffsetPrcbDpcRoutine;
    UINT16 OffsetPrcbCurrentThread;
    UINT16 OffsetPrcbMhz;
    UINT16 OffsetPrcbCpuType;
    UINT16 OffsetPrcbVendorString;
    UINT16 OffsetPrcbProcStateContext;
    UINT16 OffsetPrcbNumber;
    UINT16 SizeEThread;
    ULPTR64 KdPrintCircularBufferPtr;
    ULPTR64 KdPrintBufferSize;
    ULPTR64 KeLoaderBlock;
    UINT16 SizePcr;
    UINT16 OffsetPcrSelfPcr;
    UINT16 OffsetPcrCurrentPrcb;
    UINT16 OffsetPcrContainedPrcb;
    UINT16 OffsetPcrInitialBStore;
    UINT16 OffsetPcrBStoreLimit;
    UINT16 OffsetPcrInitialStack;
    UINT16 OffsetPcrStackLimit;
    UINT16 OffsetPrcbPcrPage;
    UINT16 OffsetPrcbProcStateSpecialReg;
    UINT16 GdtR0Code;
    UINT16 GdtR0Data;
    UINT16 GdtR0Pcr;
    UINT16 GdtR3Code;
    UINT16 GdtR3Data;
    UINT16 GdtR3Teb;
    UINT16 GdtLdt;
    UINT16 GdtTss;
    UINT16 Gdt64R3CmCode;
    UINT16 Gdt64R3CmTeb;
    ULPTR64 IopNumTriageDumpDataBlocks;
    ULPTR64 IopTriageDumpDataBlocks;
#endif

#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    ULPTR64 VfCrashDataBlock;
    ULPTR64 MmBadPagesDetected;
    ULPTR64 MmZeroedPageSingleBitErrorsDetected;
#endif

#if (NTDDI_VERSION >= NTDDI_WIN7)
    ULPTR64 EtwpDebuggerData;
    UINT16 OffsetPrcbContext;
#endif

#if (NTDDI_VERSION >= NTDDI_WIN8)
    UINT16 OffsetPrcbMaxBreakpoints;
    UINT16 OffsetPrcbMaxWatchpoints;
    UINT32 OffsetKThreadStackLimit;
    UINT32 OffsetKThreadStackBase;
    UINT32 OffsetKThreadQueueListEntry;
    UINT32 OffsetEThreadIrpList;
    UINT16 OffsetPrcbIdleThread;
    UINT16 OffsetPrcbNormalDpcState;
    UINT16 OffsetPrcbDpcStack;
    UINT16 OffsetPrcbIsrStack;
    UINT16 SizeKDPC_STACK_FRAME;
#endif

#if (NTDDI_VERSION >= NTDDI_WINBLUE) // NTDDI_WIN81.
    UINT16 OffsetKPriQueueThreadListHead;
    UINT16 OffsetKThreadWaitReason;
#endif

#if (NTDDI_VERSION >= NTDDI_WIN10_RS1)
    UINT16 Padding;
    ULPTR64 PteBase;
#endif

#if (NTDDI_VERSION >= NTDDI_WIN10_RS5)
    ULPTR64 RetpolineStubFunctionTable;
    UINT32 RetpolineStubFunctionTableSize;
    UINT32 RetpolineStubOffset;
    UINT32 RetpolineStubSize;
#endif
} KDDEBUGGER_DATA64;

typedef struct _KNONVOLATILE_CONTEXT_POINTERS {
    union {
        M128A *FloatingContext[16];
        struct {
            M128A *pXmm0;
            M128A *pXmm1;
            M128A *pXmm2;
            M128A *pXmm3;
            M128A *pXmm4;
            M128A *pXmm5;
            M128A *pXmm6;
            M128A *pXmm7;
            M128A *pXmm8;
            M128A *pXmm9;
            M128A *pXmm10;
            M128A *pXmm11;
            M128A *pXmm12;
            M128A *pXmm13;
            M128A *pXmm14;
            M128A *pXmm15;
        } DUMMYSTRUCTNAME;
    } DUMMYUNIONNAME;
    union {
        UINT64 *IntegerContext[16];
        struct {
            UINT64 *pRax;
            UINT64 *pRcx;
            UINT64 *pRdx;
            UINT64 *pRbx;
            UINT64 *pRsp;
            UINT64 *pRbp;
            UINT64 *pRsi;
            UINT64 *pRdi;
            UINT64 *pR8;
            UINT64 *pR9;
            UINT64 *pR10;
            UINT64 *pR11;
            UINT64 *pR12;
            UINT64 *pR13;
            UINT64 *pR14;
            UINT64 *pR15;
        } DUMMYSTRUCTNAME;
    } DUMMYUNIONNAME2;
} KNONVOLATILE_CONTEXT_POINTERS;

#define UNWIND_HISTORY_TABLE_SIZE 12

typedef struct _UNWIND_HISTORY_TABLE_ENTRY {
    UINT64 ImageBase;
    RUNTIME_FUNCTION *pFunctionEntry;
} UNWIND_HISTORY_TABLE_ENTRY;

typedef struct _UNWIND_HISTORY_TABLE {
    UINT32 Count;
    UINT8  LocalHint;
    UINT8  GlobalHint;
    UINT8  Search;
    UINT8  Once;
    UINT64 LowAddress;
    UINT64 HighAddress;
    UNWIND_HISTORY_TABLE_ENTRY Entry[UNWIND_HISTORY_TABLE_SIZE];
} UNWIND_HISTORY_TABLE;

//
// HR_CONTEXT definitions.
//

#define EPILOGUE_MAXCOUNT 4

#define MMU_PAGING_LEVELS 4

#define HR_CONTEXT_PXE_BASE_IDX 0
#define HR_CONTEXT_PPE_BASE_IDX 1
#define HR_CONTEXT_PDE_BASE_IDX 2
#define HR_CONTEXT_PTE_BASE_IDX 3

typedef struct _HR_CONTEXT {
    UINT32 ContextHash32;
    UINT8 HrContextPad0[4];
    UINT64 ContextSeed;
    struct {
        PEPROCESS pProcess;
        LIST_ENTRY *pLoadedModuleList;
        struct {
            VOID *pImageBase;
            struct {
                VOID *pLowVa;
                VOID *pHighVa;
            } SECTIONS_VA_RANGE;
        } NTOS_IMAGE;
        struct {
            VOID *pImageBase;
            struct {
                VOID *pLowVa;
                VOID *pHighVa;
            } SECTIONS_VA_RANGE;
        } HR_IMAGE;
    } NTOS_PROCESS;
    struct {
        VOID* (FASTCALL *pMmAllocateIndependentPagesEx) (
            IN UINT64 NoBytes,
            IN  OPTIONAL UINT32 NumaNodeNum,
            OUT OPTIONAL UINT64 *pPfnArray,
            IN  OPTIONAL UINT32 PfnArraySize
            );
        VOID (FASTCALL *pMmFreeIndependentPages) (
            IN VOID *pAllocBase,
            IN UINT64 NoBytes
            );
        BOOLEAN (FASTCALL *pMmSetPageProtection) (
            IN VOID *pVa,
            IN UINT32 RangeSize,
            IN UINT32 Protect
            );
        RUNTIME_FUNCTION* (FASTCALL *pRtlLookupFunctionEntry) (
            IN  UINT64 ControlPc,
            OUT UINT64 *pImageBase,
            OUT OPTIONAL UNWIND_HISTORY_TABLE *pHistoryTable
            );
        UINT32 (FASTCALL *pRtlRandomEx) (
            IN OUT UINT32 *pSeed      
            );
        VOID (FASTCALL *pKdCopyDataBlock) (
            OUT VOID *pKdDataBlock
            );
        VOID (FASTCALL *pIoGetStackLimits) (
            OUT VOID *pLowLimit,
            OUT VOID *pHighLimit
            );
        NTSTATUS (FASTCALL *pRtlVirtualUnwind2) (
            IN UINT32 HandlerType,
            IN UINT64 ImageBase,
            IN UINT64 ControlPc,
            IN OPTIONAL RUNTIME_FUNCTION *pFunctionEntry,
            IN OUT CONTEXT *pContextRecord,
            IN OUT OPTIONAL BOOLEAN *pMachineFrameUnwound,
            OUT VOID **pHandlerData,
            OUT UINT64 *pEstablisherFrame,
            IN OUT OPTIONAL KNONVOLATILE_CONTEXT_POINTERS *pContextPointers,
            IN OPTIONAL UINT64 *LowLimit,
            IN OPTIONAL UINT64 *HighLimit,
            OUT OPTIONAL EXCEPTION_ROUTINE **pHandlerRoutine,
            IN UINT32 UnwindFlags
            );
        VOID* (FASTCALL *pRtlPcToFileHeader) (
            IN VOID *pPcValue,
            OUT VOID **pBaseOfImage
            );
        NTSTATUS (FASTCALL *pRtlIcParseInstruction) (
            OPTIONAL PEPROCESS pTargetProcess,
            IN OUT KTP_READ_CONTEXT *pTpReadContext,
            OPTIONAL IN UINT8 UnusedUnk0,
            OUT RTL_IC_CONTEXT *pIcContext
            );
        NTSTATUS (FASTCALL *pRtlpIcAccessMemory) (
            OUT EXCEPTION_RECORD **pExceptionRecord,
            IN OUT VOID *pIoBuffer,
            IN OUT volatile VOID *pAddress,
            IN KPROCESSOR_MODE ProcessorMode,
            IN BOOLEAN IsZx64,
            IN UINT8 RWLength,
            IN UINT8 RWFlags
            );
        UINT64 (FASTCALL *pKeIpiGenericCall) (
            IN KIPI_BROADCAST_WORKER *pBroadcastFunction,
            IN UINT64 Context
            );
        VOID (FASTCALL *pCustomxHalTimerWatchdogStop) (
            VOID
            );
        UINT32 (FASTCALL *pKeQueryActiveProcessorCountEx) (
            IN UINT16 GroupNumber
            );
        KIRQL (FASTCALL *pKzRaiseIrql) (
            IN KIRQL NewIrql
            );
        VOID (FASTCALL *pKzLowerIrql) (
            IN KIRQL NewIrql
            );
        VOID (FASTCALL *pKeBugCheckEx) (
            IN UINT32 BugCheckCode,
            IN UINT64 BugCheckParameter1,
            IN UINT64 BugCheckParameter2,
            IN UINT64 BugCheckParameter3,
            IN UINT64 BugCheckParameter4
            );
    } HR_API;
    struct {
        VOID *pKiExecuteAllDpcs;
        VOID *pKiProcessExpiredTimerList;
        VOID *pKiExpireTimer2;
        VOID *pKiDeliverApc;

        VOID *pKeRemovePriQueue;
        VOID *pKeRemovePriQueueEpi[EPILOGUE_MAXCOUNT];

        VOID *pKeWaitForSingleObject;
        VOID *pKeWaitForSingleObjectEpi[EPILOGUE_MAXCOUNT];

        VOID *pKeWaitForMultipleObjects;
        VOID *pKeWaitForMultipleObjectsEpi[EPILOGUE_MAXCOUNT];

        VOID *pKeDelayExecutionThread;
        VOID *pKeDelayExecutionThreadEpi[EPILOGUE_MAXCOUNT];

        VOID *pKiBalanceSetManagerDeferredRoutine;
        VOID *pKiCustomRecurseRoutineX;     
        VOID *pCcBcbProfiler;
        VOID *pCcBcbProfiler2;
        VOID *pKiDispatchCallout;
        VOID *pKiSwInterruptDispatch;
        VOID *pKiErrata671Present;
    } NTOS_ROUTINES;
    struct {
        UINT64 PteBases[MMU_PAGING_LEVELS];

        UINT64 *pKiWaitNever;
        UINT64 *pKiWaitAlways;

        KDPC *pKiBalanceSetManagerPeriodicDpc;
        KEVENT *pKiBalanceSetManagerPeriodicEvent;

        VOID **pPgGlobalContext;

        UINT64 *pPgCheckTimerIDT;
        UINT64 *pPgCheckTimerSSDT;
    } NTOS_ITEMS;
    struct {
        struct {
            UINT16 OffsetCurrentThread;
            UINT16 OffsetNumber;
            UINT16 OffsetHalReserved;
            UINT16 OffsetAcpiReserved;
            UINT16 OffsetDpcData;
        } KPRCB_OFFSETS;
        struct {
            UINT16 OffsetCid;
        } ETHREAD_OFFSETS;
        struct {
            UINT16 OffsetThreadLock;
            UINT16 OffsetApcState;
        } KTHREAD_OFFSETS;
        struct {
            UINT16 OffsetUniqueThread;
        } CLIENT_ID_OFFSETS;
    } NTOS_OFFSETS_TABLE;
    UINT8 HrContextPad1[6];
} HR_CONTEXT;

typedef struct _HR_CONTEXT_DESCRIPTOR {
    HR_CONTEXT *pHunterContext;
    VOID *pAllocBase;
    UINT16 LowPaddingSize;
    UINT16 HighPaddingSize;
    UINT8 HrContextDescriptorPad0[4];
} HR_CONTEXT_DESCRIPTOR;

#pragma warning(pop)

