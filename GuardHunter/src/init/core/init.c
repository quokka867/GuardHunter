/*++
* Module Name:
*
*     init.c
*
* Abstract:
*
*     This module contains routines for initializing context structures.
*
* Author:
*
*     quokka867 (GitHub/Twitter).
*
--*/

#include "../headers/init.h"

#define NTOS_BUILD_NUMBER_25H2 26200
#define NTOS_BUILD_NUMBER_MASK 0xFFFFUI16	

#define BODY_FALLBACK_PATTERN_MAXCOUNT 4

typedef struct _NTOS_PATTERN_DATA {
    UINT64 PrimarySectionsName;
    CONST VOID *pBodyPrimaryPattern;
    CONST VOID *pBodyPrimaryMask;

    UINT8 BodyFallbackPatternCount;
    UINT8 NtosDataPatternPad0[7];
    UINT64 FallbackSectionsName[BODY_FALLBACK_PATTERN_MAXCOUNT];
    CONST VOID *pBodyFallbackPatterns[BODY_FALLBACK_PATTERN_MAXCOUNT];
    CONST VOID *pBodyFallbackMasks[BODY_FALLBACK_PATTERN_MAXCOUNT];

    UINT8 EpiloguePatternCount;
    UINT8 NtosDataPatternPad1[7];
    CONST VOID *pEpiloguePatterns[EPILOGUE_MAXCOUNT];
    CONST VOID *pEpilogueMasks[EPILOGUE_MAXCOUNT];
} NTOS_PATTERN_DATA;

#define NTOS_PATTERN_DATA_BASE_ID 0xA6A8A1BFUI32
#define NTOS_PATTERN_ID_OFFSET    0x01C9UI16

//
// NTOS routines.
//

// 01.
#define NTOS_PATTERN_DATA_MMALLOCATEINDEPENDENTPAGESEX_ID \
(NTOS_PATTERN_DATA_BASE_ID +  NTOS_PATTERN_ID_OFFSET)

// 02.
#define NTOS_PATTERN_DATA_MMFREEINDEPENDENTPAGES_ID \
(NTOS_PATTERN_DATA_BASE_ID + (NTOS_PATTERN_ID_OFFSET * 2))

// 03.
#define NTOS_PATTERN_DATA_MMSETPAGEPROTECTION_ID \
(NTOS_PATTERN_DATA_BASE_ID + (NTOS_PATTERN_ID_OFFSET * 3))

// 04.
#define NTOS_PATTERN_DATA_KDCOPYDATABLOCK_ID \
(NTOS_PATTERN_DATA_BASE_ID + (NTOS_PATTERN_ID_OFFSET * 4))

// 05.
#define NTOS_PATTERN_DATA_RTLICPARSEINSTRUCTION_ID \
(NTOS_PATTERN_DATA_BASE_ID + (NTOS_PATTERN_ID_OFFSET * 5))

// 06.
#define NTOS_PATTERN_DATA_RTLPICACCESSMEMORY_ID \
(NTOS_PATTERN_DATA_BASE_ID + (NTOS_PATTERN_ID_OFFSET * 6))

// 07.
#define NTOS_PATTERN_DATA_XHALTIMERWATCHDOGSTOPCUSTOM_ID \
(NTOS_PATTERN_DATA_BASE_ID + (NTOS_PATTERN_ID_OFFSET * 7))

// 08.
#define NTOS_PATTERN_DATA_KIEXECUTEALLDPCS_ID \
(NTOS_PATTERN_DATA_BASE_ID + (NTOS_PATTERN_ID_OFFSET * 8))

// 09.
#define NTOS_PATTERN_DATA_KIPROCESSEXPIREDTIMERLIST_ID \
(NTOS_PATTERN_DATA_BASE_ID + (NTOS_PATTERN_ID_OFFSET * 9))

// 10.
#define NTOS_PATTERN_DATA_KIEXPIRETIMER2_ID \
(NTOS_PATTERN_DATA_BASE_ID + (NTOS_PATTERN_ID_OFFSET * 10))

// 11.
#define NTOS_PATTERN_DATA_KIDELIVERAPC_ID \
(NTOS_PATTERN_DATA_BASE_ID + (NTOS_PATTERN_ID_OFFSET * 11))

//
// NTOS routines epi.
//

#define NTOS_PATTERN_DATA_EPI_BASE_ID \
(NTOS_PATTERN_DATA_BASE_ID + (NTOS_PATTERN_ID_OFFSET * 12))

// 12.
#define NTOS_PATTERN_DATA_KEREMOVEPRIQUEUE_EPI_ID \
(NTOS_PATTERN_DATA_BASE_ID + (NTOS_PATTERN_ID_OFFSET * 13))

// 13.
#define NTOS_PATTERN_DATA_KEWAITFORSINGLEOBJECT_EPI_ID \
(NTOS_PATTERN_DATA_BASE_ID + (NTOS_PATTERN_ID_OFFSET * 14))

// 14.
#define NTOS_PATTERN_DATA_KEWAITFORMULTIPLEOBJECTS_EPI_ID \
(NTOS_PATTERN_DATA_BASE_ID + (NTOS_PATTERN_ID_OFFSET * 15))

// 15.
#define NTOS_PATTERN_DATA_KEDELAYEXECUTIONTHREAD_EPI_ID \
(NTOS_PATTERN_DATA_BASE_ID + (NTOS_PATTERN_ID_OFFSET * 16))

// 16.
#define NTOS_PATTERN_DATA_KIBALANCESETMANAGERDEFERREDROUTINE_ID \
(NTOS_PATTERN_DATA_BASE_ID + (NTOS_PATTERN_ID_OFFSET * 17))

// 17.
#define NTOS_PATTERN_DATA_KICUSTOMRECURSEROUTINEX_ID \
(NTOS_PATTERN_DATA_BASE_ID + (NTOS_PATTERN_ID_OFFSET * 18))

// 18.
#define NTOS_PATTERN_DATA_CCBCBPROFILER_ID \
(NTOS_PATTERN_DATA_BASE_ID + (NTOS_PATTERN_ID_OFFSET * 19))

// 19.
#define NTOS_PATTERN_DATA_CCBCBPROFILER2_ID \
(NTOS_PATTERN_DATA_BASE_ID + (NTOS_PATTERN_ID_OFFSET * 20))

// 20.
#define NTOS_PATTERN_DATA_KIDISPATCHCALLOUT_ID \
(NTOS_PATTERN_DATA_BASE_ID + (NTOS_PATTERN_ID_OFFSET * 21))

// 21.
#define NTOS_PATTERN_DATA_KISWINTERRUPTDISPATCH_ID \
(NTOS_PATTERN_DATA_BASE_ID + (NTOS_PATTERN_ID_OFFSET * 22))

// 22.
#define NTOS_PATTERN_DATA_KIERRATA671PRESENT_ID \
(NTOS_PATTERN_DATA_BASE_ID + (NTOS_PATTERN_ID_OFFSET * 23))

//
// NTOS items.
//

// 23.
#define NTOS_PATTERN_DATA_KIWAITNEVER_ID \
(NTOS_PATTERN_DATA_BASE_ID + (NTOS_PATTERN_ID_OFFSET * 24))

// 24.
#define NTOS_PATTERN_DATA_KIWAITALWAYS_ID \
(NTOS_PATTERN_DATA_BASE_ID + (NTOS_PATTERN_ID_OFFSET * 25)) 

// 25.
#define NTOS_PATTERN_DATA_KIBALANCESETMANAGERPERIODICDPC_ID \
(NTOS_PATTERN_DATA_BASE_ID + (NTOS_PATTERN_ID_OFFSET * 26))

// 26.
#define NTOS_PATTERN_DATA_KIBALANCESETMANAGERPERIODICEVENT_ID \
(NTOS_PATTERN_DATA_BASE_ID + (NTOS_PATTERN_ID_OFFSET * 27)) 

// 27.
#define NTOS_PATTERN_DATA_PGGLOBALCONTEXT_ID \
(NTOS_PATTERN_DATA_BASE_ID + (NTOS_PATTERN_ID_OFFSET * 28))

// 28.
#define NTOS_PATTERN_DATA_PGCHECKTIMERIDT_ID \
(NTOS_PATTERN_DATA_BASE_ID + (NTOS_PATTERN_ID_OFFSET * 29))

// 29.
#define NTOS_PATTERN_DATA_PGCHECKTIMERSSDT_ID \
(NTOS_PATTERN_DATA_BASE_ID + (NTOS_PATTERN_ID_OFFSET * 30))

#define NTOS_PATTERN_ID_COUNT 29 + (1)

#define _x "\xC3"
#define _0 "\x90"

HR_STATUS
FASTCALL
InitGetPatternData(
    IN  UINT32 PatternDataId,
    OUT NTOS_PATTERN_DATA *pPatternData
)
/*++
* Routine Description:
*
*     This routine initializes the
*     specified NTOS_PATTERN_DATA structure based on the
*     specified pattern data ID.
*
* Arguments:
*
*     PatternDataId - Supplies the pattern data ID.
*
*     pPatternData  - Supplies a pointer to the
*                     NTOS_PATTERN_DATA structure.
*
* Return Value:
*
*     Internal status.
*
--*/
{
    if (!PatternDataId || !pPatternData) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    switch (SharedUserData->NtBuildNumber & NTOS_BUILD_NUMBER_MASK) {
    case NTOS_BUILD_NUMBER_25H2:
        switch (PatternDataId) {
        case NTOS_PATTERN_DATA_MMALLOCATEINDEPENDENTPAGESEX_ID:
            pPatternData->pBodyPrimaryPattern = "\x48\x00\x00\x00\x00\x00\x00\x00\xFF\xFF\x48\xC1\x00\x19\x00\x00\x48\x00\x00\x41\x00\x04";
            pPatternData->pBodyPrimaryMask    = _x _0 _0 _0 _0 _0 _0 _0 _x _x _x _x _0 _x _0 _0 _x _0 _0 _x _0 _x;

            pPatternData->pBodyFallbackPatterns[0] = "\x48\x00\x00\x00\x00\x00\x00\xDE\xFF\xFF\x48\xC1\x00\x0C\x48\x00\xFF\x0F";
            pPatternData->pBodyFallbackMasks[0]    = _x _0 _x _x _x _x _x _x _x _x _x _x _0 _x _x _0 _x _x;
            pPatternData->pBodyFallbackPatterns[1] = "\x0F\x57\x00\x40\x0F\x95\x00\x4D\x00\x00\x48\xC1\x00\x00\x48\x00\x00";
            pPatternData->pBodyFallbackMasks[1]    = _x _x _0 _x _x _x _0 _x _0 _0 _x _x _0 _0 _x _0 _0;

            pPatternData->PrimarySectionsName = QUICK_XOR64(NTOS_PAGE_SECTION_NAME);

            pPatternData->FallbackSectionsName[0] = QUICK_XOR64(NTOS_PAGE_SECTION_NAME);
            pPatternData->FallbackSectionsName[1] = QUICK_XOR64(NTOS_PAGE_SECTION_NAME);

            pPatternData->BodyFallbackPatternCount = 2;

            pPatternData->EpiloguePatternCount = 0;
            break;
        case NTOS_PATTERN_DATA_MMFREEINDEPENDENTPAGES_ID:
            pPatternData->pBodyPrimaryPattern = "\x41\x0F\x95\x00\x48\xC1\x00\x0C\x00\x00\x00\x48\xC1\x00\x09\x0F\x11\x45";
            pPatternData->pBodyPrimaryMask    = _x _x _x _0 _x _x _0 _x _0 _0 _0 _x _x _0 _x _x _x _x;

            pPatternData->pBodyFallbackPatterns[0] = "\x41\x0F\xB6\x00\x44\x0F\x22\x00\x48\x8B\x00\x00\x48\x8D\x00\x00\x00\x00\x00\x00\x00\x00\xE8";
            pPatternData->pBodyFallbackMasks[0]    = _x _x _x _0 _x _x _x _0 _x _x _0 _0 _x _x _0 _0 _0 _0 _0 _0 _0 _0 _x;
            pPatternData->pBodyFallbackPatterns[1] = "\x0F\x57\x00\x00\x00\x00\x48\x89\x00\x00\x48\xF7\x00\xFF\x0F\x00\x00";
            pPatternData->pBodyFallbackMasks[1]    = _x _x _0 _0 _0 _0 _x _x _0 _0 _x _x _0 _x _x _x _x;

            pPatternData->PrimarySectionsName = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);

            pPatternData->FallbackSectionsName[0] = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);
            pPatternData->FallbackSectionsName[1] = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);

            pPatternData->BodyFallbackPatternCount = 2;

            pPatternData->EpiloguePatternCount = 0;
            break;
        case NTOS_PATTERN_DATA_MMSETPAGEPROTECTION_ID:
            pPatternData->pBodyPrimaryPattern = "\xE8\x00\x00\x00\x00\x85\x00\x75\x00\x00\x00\xE8\x00\x00\x00\x00\x83\x00\x07\x77\x00\x00\x00\x83\x00\x05";
            pPatternData->pBodyPrimaryMask    = _x _0 _0 _0 _0 _x _0 _x _0 _0 _0 _x _0 _0 _0 _0 _x _0 _x _x _0 _0 _0 _x _0 _x;

            pPatternData->pBodyFallbackPatterns[0] = "\x80\x00\x05\x74\x00\x00\x00\x74\x00\x48\xF7\x05\x00\x00\x00\x00\x00\x80\x00\x00";
            pPatternData->pBodyFallbackMasks[0]    = _x _0 _x _x _0 _0 _0 _x _0 _x _x _x _0 _0 _0 _0 _x _x _x _x;
            pPatternData->pBodyFallbackPatterns[1] = "\x48\xF7\x05\x00\x00\x00\x00\x00\x80\x00\x00\x75\x00\x44\x8B\x00\x48\x8B\x00\x48\x8B";
            pPatternData->pBodyFallbackMasks[1]    = _x _x _x _0 _0 _0 _0 _x _x _x _x _x _0 _x _x _0 _x _x _0 _x _x;

            pPatternData->PrimarySectionsName = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);

            pPatternData->FallbackSectionsName[0] = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);
            pPatternData->FallbackSectionsName[1] = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);

            pPatternData->BodyFallbackPatternCount = 2;

            pPatternData->EpiloguePatternCount = 0;
            break;
        case NTOS_PATTERN_DATA_KDCOPYDATABLOCK_ID:
            pPatternData->pBodyPrimaryPattern = "\x48\x0F\x00\x48\x33\x00\x00\x00\x00\x00\x49\x89\x00\x00\x48\x8D\x00\x00\x41\x83\x00\xFF\x75\x00\xC3\xCC";
            pPatternData->pBodyPrimaryMask    = _x _x _0 _x _x _0 _0 _0 _0 _x _x _x _0 _0 _x _x _0 _0 _x _x _0 _x _x _0 _x _x;

            pPatternData->pBodyFallbackPatterns[0] = "\x41\x0F\x11\x00\x00\x00\x00\x00\x0F\x10\x00\x00\x00\x00\x00\x41\x0F\x11\x00\x00\x48\x83\x00\x00\x75\x00\x0F\x10";
            pPatternData->pBodyFallbackMasks[0]    = _x _x _x _0 _0 _0 _0 _0 _x _x _0 _0 _0 _0 _0 _x _x _x _0 _0 _x _x _0 _0 _x _0 _x _x;
            pPatternData->pBodyFallbackPatterns[1] = "\x48\x83\x00\x00\x75\x00\x0F\x10\x00\x41\x0F\x11\x00\x0F\x10\x00\x00\x41\x0F\x11\x00\x00\xC3\xCC";
            pPatternData->pBodyFallbackMasks[1]    = _x _x _0 _0 _x _0 _x _x _0 _x _x _x _0 _x _x _0 _0 _x _x _x _0 _0 _x _x;

            pPatternData->PrimarySectionsName = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);

            pPatternData->FallbackSectionsName[0] = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);
            pPatternData->FallbackSectionsName[1] = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);

            pPatternData->BodyFallbackPatternCount = 2;

            pPatternData->EpiloguePatternCount = 0;
            break;
        case NTOS_PATTERN_DATA_RTLICPARSEINSTRUCTION_ID:
            pPatternData->pBodyPrimaryPattern = "\x21\x00\x24\x00\x0F\x57\x00\x41\x0F\x11\x00\x48\x00\x00\x41\x0F\x11";
            pPatternData->pBodyPrimaryMask    = _x _0 _x _0 _x _x _0 _x _x _x _0 _x _0 _0 _x _x _x;

            pPatternData->pBodyFallbackPatterns[0] = "\x0F\x57\x00\x45\x00\x00\x0F\x11\x00\x48\x00\x00\x0F\x11\x00\x00\x44\x8D";
            pPatternData->pBodyFallbackMasks[0]    = _x _x _0 _x _0 _0 _x _x _0 _x _0 _0 _x _x _0 _0 _x _x;
            pPatternData->pBodyFallbackPatterns[1] = "\x48\x00\x00\x88\x00\x00\xE8\x00\x00\x00\x00\x3D\x06\x02\x00\xC0\x75\x00\x83\x7C\x24\x00\x00\x73";
            pPatternData->pBodyFallbackMasks[1]    = _x _0 _0 _x _0 _0 _x _0 _0 _0 _0 _x _x _x _x _x _x _0 _x _x _x _0 _0 _x;

            pPatternData->PrimarySectionsName = QUICK_XOR64(NTOS_TRACESUP_SECTION_NAME);

            pPatternData->FallbackSectionsName[0] = QUICK_XOR64(NTOS_TRACESUP_SECTION_NAME);
            pPatternData->FallbackSectionsName[1] = QUICK_XOR64(NTOS_TRACESUP_SECTION_NAME);

            pPatternData->BodyFallbackPatternCount = 2;

            pPatternData->EpiloguePatternCount = 0;
            break;
        case NTOS_PATTERN_DATA_RTLPICACCESSMEMORY_ID:

            pPatternData->pBodyPrimaryPattern = "\x41\x00\x01\x00\x00\x00\x00\x00\x45\x00\x00\x75\x00\x00\x84\x00\x74\x00\x0F\xB6\x00\x24\x00\x48\x00\x00\xE8";
            pPatternData->pBodyPrimaryMask    = _x _0 _x _x _x _x _0 _0 _x _0 _0 _x _0 _0 _x _0 _x _0 _x _x _0 _x _0 _x _0 _0 _x;

            pPatternData->pBodyFallbackPatterns[0] = "\x89\x00\x79\x00\x49\x8B\x00\xC7\x00\x05\x00\x00\xC0\x45\x84\x00\x0F\x95";
            pPatternData->pBodyFallbackMasks[0]    = _x _0 _x _0 _x _x _0 _x _0 _x _x _x _x _x _x _0 _x _x;
            pPatternData->pBodyFallbackPatterns[1] = "\x48\x00\x00\x45\x84\x00\x48\x0F\x44\x00\x48\x0F\x44\x00\x0F\xB6\x00\x24\x00\x83\x00\x01\x74";
            pPatternData->pBodyFallbackMasks[1]    = _x _0 _0 _x _x _0 _x _x _x _0 _x _x _x _0 _x _x _0 _x _0 _x _0 _x _x;

            pPatternData->PrimarySectionsName = QUICK_XOR64(NTOS_TRACESUP_SECTION_NAME);

            pPatternData->FallbackSectionsName[0] = QUICK_XOR64(NTOS_TRACESUP_SECTION_NAME);
            pPatternData->FallbackSectionsName[1] = QUICK_XOR64(NTOS_TRACESUP_SECTION_NAME);

            pPatternData->BodyFallbackPatternCount = 2;

            pPatternData->EpiloguePatternCount = 0;
            break;
        case NTOS_PATTERN_DATA_XHALTIMERWATCHDOGSTOPCUSTOM_ID:
            pPatternData->pBodyPrimaryPattern = "\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC";
            pPatternData->pBodyPrimaryMask    = _x _x _x _x _x _x _x _x _x _x _x _x _x;

            pPatternData->pBodyFallbackPatterns[0] = pPatternData->pBodyPrimaryPattern;
            pPatternData->pBodyFallbackMasks[0]    = pPatternData->pBodyPrimaryMask;
            pPatternData->pBodyFallbackPatterns[1] = pPatternData->pBodyPrimaryPattern;
            pPatternData->pBodyFallbackMasks[1]    = pPatternData->pBodyPrimaryMask;

            pPatternData->PrimarySectionsName = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);

            pPatternData->FallbackSectionsName[0] = QUICK_XOR64(NTOS_PAGE_SECTION_NAME);
            pPatternData->FallbackSectionsName[1] = QUICK_XOR64(NTOS_INITKDBG_SECTION_NAME);

            pPatternData->BodyFallbackPatternCount = 2;

            pPatternData->EpiloguePatternCount = 0;
            break;
        case NTOS_PATTERN_DATA_KIEXECUTEALLDPCS_ID:
            pPatternData->pBodyPrimaryPattern = "\x1F\x00\x00\x00\x41\x00\x00\x01\x00\x2F\x00\x00\x00\x0F\x00\x00\x48";
            pPatternData->pBodyPrimaryMask    = _x _x _x _x _x _0 _0 _x _0 _x _x _x _x _x _0 _0 _x;

            pPatternData->pBodyFallbackPatterns[0] = "\x46\x0F\xB6\x00\x00\x41\xC1\x00\x06\x0F\xB6\x00\x00\x01\x44";
            pPatternData->pBodyFallbackMasks[0]    = _x _x _x _0 _0 _x _x _0 _x _x _x _0 _0 _x _x;
            pPatternData->pBodyFallbackPatterns[1] = "\x75\x00\xFA\x00\x01\x48\x8B\x00\x00\x00\x00\x00\x00\x48";
            pPatternData->pBodyFallbackMasks[1]    = _x _0 _x _0 _x _x _x _0 _0 _0 _0 _0 _0 _x;

            pPatternData->PrimarySectionsName = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);

            pPatternData->FallbackSectionsName[0] = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);
            pPatternData->FallbackSectionsName[1] = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);

            pPatternData->BodyFallbackPatternCount = 2;

            pPatternData->EpiloguePatternCount = 0;
            break;
        case NTOS_PATTERN_DATA_KIPROCESSEXPIREDTIMERLIST_ID:
            pPatternData->pBodyPrimaryPattern = "\x45\x00\x00\x44\x89\x00\x00\x0F\x57\x00\x4C\x89\x00\x00\x0F\x57\x00\x48\x89\x00\x00\x00\x48";
            pPatternData->pBodyPrimaryMask    = _x _0 _0 _x _x _0 _0 _x _x _0 _x _x _0 _0 _x _x _0 _x _x _0 _0 _0 _x;

            pPatternData->pBodyFallbackPatterns[0] = "\xFF\x00\x49\x87\x00\x00\x48\x85\x00\x74\x00\xF0\x0F\xBA\x00\x00\x00\x00\x00\x0F\x82";
            pPatternData->pBodyFallbackMasks[0]    = _x _0 _x _x _0 _0 _x _x _0 _x _0 _x _x _x _0 _0 _0 _0 _0 _x _x;
            pPatternData->pBodyFallbackPatterns[1] = "\x0F\xBC\x00\x75\x00\x89\x00\x24\x00\x00\x00\xF6\x00\x00\x0F\x85\x00\x00\x00\x00\x48";
            pPatternData->pBodyFallbackMasks[1]    = _x _x _0 _x _0 _x _0 _x _0 _0 _0 _x _0 _0 _x _x _0 _0 _0 _0 _x;

            pPatternData->PrimarySectionsName = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);

            pPatternData->FallbackSectionsName[0] = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);
            pPatternData->FallbackSectionsName[1] = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);

            pPatternData->BodyFallbackPatternCount = 2;

            pPatternData->EpiloguePatternCount = 0;
            break;
        case NTOS_PATTERN_DATA_KIEXPIRETIMER2_ID:
            pPatternData->pBodyPrimaryPattern = "\x89\x00\x00\x00\x89\x00\x00\x0F\x11\x00\x00\x00\x89\x00\x00\x0F\x11\x00\x00\x00\x88\x00\x00\x00\x88";
            pPatternData->pBodyPrimaryMask    = _x _0 _0 _0 _x _0 _0 _x _x _0 _0 _0 _x _0 _0 _x _x _0 _0 _0 _x _0 _0 _0 _x;

            pPatternData->pBodyFallbackPatterns[0] = "\x81\x00\x7F\xF0\xFF\xFF\x00\x00\x00\x00\x00\xF0\x0F\xB1\x00\x00\x00\x74";
            pPatternData->pBodyFallbackMasks[0]    = _x _0 _x _x _x _x _0 _0 _0 _0 _0 _x _x _x _0 _0 _0 _x;
            pPatternData->pBodyFallbackPatterns[1] = "\xF0\x0F\xB1\x00\x00\x00\x74\x00\x00\x00\x00\x00\x00\x00\x00\x7F\xF0\xFF\xFF\x00\x00\x00\xF0\x0F\xB1";
            pPatternData->pBodyFallbackMasks[1]    = _x _x _x _0 _0 _0 _x _0 _0 _0 _0 _0 _0 _0 _0 _x _x _x _x _0 _0 _0 _x _x _x;

            pPatternData->PrimarySectionsName = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);

            pPatternData->FallbackSectionsName[0] = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);
            pPatternData->FallbackSectionsName[1] = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);

            pPatternData->BodyFallbackPatternCount = 2;

            pPatternData->EpiloguePatternCount = 0;
            break;
        case NTOS_PATTERN_DATA_KIDELIVERAPC_ID:
            pPatternData->pBodyPrimaryPattern = "\x66\x83\x00\x33\x0F\x85\x00\x00\x00\x00\x48\x8B\x00\x00\x00\x00\x00\x00\x00\x00\x0F\x87";
            pPatternData->pBodyPrimaryMask    = _x _x _0 _x _x _x _0 _0 _0 _0 _x _x _0 _0 _0 _0 _0 _0 _0 _0 _x _x;

            pPatternData->pBodyFallbackPatterns[0] = "\x41\x00\x01\x00\x00\x00\x41\x00\x02\x00\x00\x00\x00\x39\x00\x0F\x84";
            pPatternData->pBodyFallbackMasks[0]    = _x _0 _x _x _x _x _x _0 _x _x _x _x _0 _x _0 _x _x;
            pPatternData->pBodyFallbackPatterns[1] = "\x75\x00\x00\x8B\x00\x00\x48\x39\x00\x0F\x84\x00\x00\x00\x00\x00\x03\x00\x00\x00\xCD\x29\x66\x83\x00\x10";
            pPatternData->pBodyFallbackMasks[1]    = _x _0 _0 _x _0 _0 _x _x _0 _x _x _0 _0 _0 _0 _0 _x _x _x _x _x _x _x _x _0 _x;

            pPatternData->PrimarySectionsName = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);

            pPatternData->FallbackSectionsName[0] = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);
            pPatternData->FallbackSectionsName[1] = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);

            pPatternData->BodyFallbackPatternCount = 2;

            pPatternData->EpiloguePatternCount = 0;
            break;
        case NTOS_PATTERN_DATA_KEREMOVEPRIQUEUE_EPI_ID:
            pPatternData->pBodyPrimaryPattern = "\xF0\x45\x21\x00\xC6\x00\x00\x00\x00\x00\x00\x65\x48\x8B\x00\x25\x20\x00\x00\x00\x48\x83";
            pPatternData->pBodyPrimaryMask    = _x _x _x _0 _x _0 _0 _0 _x _x _x _x _x _x _0 _x _x _x _x _x _x _x;

            pPatternData->pBodyFallbackPatterns[0] = "\x0F\x85\x00\x00\x00\x00\x85\x00\x0F\x84\x00\x00\x00\x00\x00\x00\x48\x03\x00\x00\xE9\x1E";
            pPatternData->pBodyFallbackMasks[0]    = _x _x _0 _0 _0 _0 _x _0 _x _x _0 _0 _0 _0 _0 _0 _x _x _0 _0 _x _x;
            pPatternData->pBodyFallbackPatterns[1] = "\x74\x00\xF0\x0F\xBA\x00\x00\x00\x44\x8B\x00\x00\x00\x00\x48\x00\x00\x00\x48\x00\x00\xE8";
            pPatternData->pBodyFallbackMasks[1]    = _x _0 _x _x _x _0 _0 _0 _x _x _0 _0 _0 _0 _x _0 _0 _0 _x _0 _0 _x;

            pPatternData->PrimarySectionsName = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);

            pPatternData->FallbackSectionsName[0] = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);
            pPatternData->FallbackSectionsName[1] = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);

            pPatternData->pEpiloguePatterns[0] = "\xC3\xCC\x44\x0F\x20\x00\x00\x00\x00\x00\x00";
            pPatternData->pEpilogueMasks[0]    = _x _x _x _x _x _0 _0 _0 _0 _x _x;

            pPatternData->BodyFallbackPatternCount = 2;

            pPatternData->EpiloguePatternCount = 1;
            break;
        case NTOS_PATTERN_DATA_KEWAITFORSINGLEOBJECT_EPI_ID:
            pPatternData->pEpiloguePatterns[0] = "\xC3\xCC\x66\x66\x66\x0F";
            pPatternData->pEpilogueMasks[0]    = _x _x _x _x _x _x;

            pPatternData->BodyFallbackPatternCount = 0;

            pPatternData->EpiloguePatternCount = 1;
            break;
        case NTOS_PATTERN_DATA_KEWAITFORMULTIPLEOBJECTS_EPI_ID:
            pPatternData->pEpiloguePatterns[0] = "\xC3\xCC\x83\x00\x00\x77";
            pPatternData->pEpilogueMasks[0]    = _x _x _x _0 _0 _x;

            pPatternData->BodyFallbackPatternCount = 0;

            pPatternData->EpiloguePatternCount = 1;
            break;
        case NTOS_PATTERN_DATA_KEDELAYEXECUTIONTHREAD_EPI_ID:
            pPatternData->pEpiloguePatterns[0] = "\xC3\xCC\x48\x89\x00\x24\x00\x00\x00\x00";
            pPatternData->pEpilogueMasks[0]    = _x _x _x _x _0 _x _0 _0 _x _x;

            pPatternData->BodyFallbackPatternCount = 0;

            pPatternData->EpiloguePatternCount = 1;
            break;
        case NTOS_PATTERN_DATA_KIBALANCESETMANAGERDEFERREDROUTINE_ID:
            pPatternData->pBodyPrimaryPattern = "\xE8\x00\x00\x00\x00\x48\x00\x00\x48\xC1\x00\x00\x48\xFF\x00\x48\x83\x00\x00\x0F\x87\x00\x00\x00\x00\x45";
            pPatternData->pBodyPrimaryMask    = _x _0 _0 _0 _0 _x _0 _0 _x _x _0 _0 _x _x _0 _x _x _0 _0 _x _x _0 _0 _0 _0 _x;

            pPatternData->PrimarySectionsName = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);

            pPatternData->BodyFallbackPatternCount = 0;

            pPatternData->EpiloguePatternCount = 0;
            break;
        case NTOS_PATTERN_DATA_KICUSTOMRECURSEROUTINEX_ID:
            pPatternData->pBodyPrimaryPattern = "\x48\x83\xEC\x00\xFF\x00\x74\x00\xE8\x00\x00\x00\x00\x8B\x00\x48\x83\xC4\x00\xC3\xCC\xCC\xCC";
            pPatternData->pBodyPrimaryMask    = _x _x _x _0 _x _0 _x _0 _x _0 _0 _0 _0 _x _0 _x _x _x _0 _x _x _x _x;

            pPatternData->PrimarySectionsName = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);

            pPatternData->BodyFallbackPatternCount = 0;

            pPatternData->EpiloguePatternCount = 0;
            break;
        case NTOS_PATTERN_DATA_CCBCBPROFILER_ID:
        case NTOS_PATTERN_DATA_CCBCBPROFILER2_ID:
            pPatternData->pBodyPrimaryPattern = "\xC7\x84\x24\x00\x00\x00\x00\x00\x00\x90\x10\x8B\x00\x24\x00\x00\x00\x00\x48\xC1\x00\x00\xC1\x00\x00\xE8";
            pPatternData->pBodyPrimaryMask    = _x _x _x _0 _x _x _x _x _x _x _x _x _0 _x _0 _0 _x _x _x _x _0 _0 _x _0 _0 _x;

            if (PatternDataId == NTOS_PATTERN_DATA_CCBCBPROFILER_ID) {
                pPatternData->PrimarySectionsName = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);
            } else {
                pPatternData->PrimarySectionsName = QUICK_XOR64(NTOS_PAGE_SECTION_NAME);
            }

            pPatternData->BodyFallbackPatternCount = 0;

            pPatternData->EpiloguePatternCount = 0;
            break;
        case NTOS_PATTERN_DATA_KIDISPATCHCALLOUT_ID:
            pPatternData->pBodyPrimaryPattern = "\x44\x0F\x22\x00\x00\x3D\x00\x00\x00\x00\x00\x0F\x85\x00\x00\x00\x00\x00\x3D\x00\x00\x00\x00\x00\x75";
            pPatternData->pBodyPrimaryMask    = _x _x _x _0 _0 _x _0 _0 _0 _x _x _x _x _0 _0 _0 _0 _0 _x _0 _0 _0 _x _x _x;

            pPatternData->PrimarySectionsName = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);

            pPatternData->BodyFallbackPatternCount = 0;

            pPatternData->EpiloguePatternCount = 0;
            break;
        case NTOS_PATTERN_DATA_KISWINTERRUPTDISPATCH_ID:
            pPatternData->pBodyPrimaryPattern = "\xEB\x00\x0F\xAE\xE8\x0F\x31\x48\xC1\x00\x00\x4C\x8D\x00\x00\x00\x00\x00\x48\x00\x00\x48";
            pPatternData->pBodyPrimaryMask    = _x _0 _x _x _x _x _x _x _x _0 _0 _x _x _0 _0 _0 _0 _0 _x _0 _0 _x;

            pPatternData->PrimarySectionsName = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);

            pPatternData->BodyFallbackPatternCount = 0;

            pPatternData->EpiloguePatternCount = 0;
            break;
        case NTOS_PATTERN_DATA_KIERRATA671PRESENT_ID:
            pPatternData->pBodyPrimaryPattern = "\x33\xC0\xFF\xC0\xC3\x66\x66\x66";
            pPatternData->pBodyPrimaryMask    = _x _x _x _x _x _x _x _x;

            pPatternData->PrimarySectionsName = QUICK_XOR64(NTOS_INITKDBG_SECTION_NAME);

            pPatternData->BodyFallbackPatternCount = 0;

            pPatternData->EpiloguePatternCount = 0;
            break;
        case NTOS_PATTERN_DATA_KIWAITNEVER_ID:
            pPatternData->pBodyPrimaryPattern = "\x48\x8B\x00\x00\x00\x00\x00\x48\x8B\x00\x48\x00\x00\x48\xD3\x00\x48\x8D\x00\x00\x00\x00\x00\x48\x00\x00\x48\x0F";
            pPatternData->pBodyPrimaryMask    = _x _x _0 _0 _0 _0 _x _x _x _0 _x _0 _0 _x _x _0 _x _x _0 _0 _0 _0 _x _x _0 _0 _x _x;

            pPatternData->pBodyFallbackPatterns[0] = "\x48\x8B\x00\x00\x00\x00\x00\x48\x8B\x00\x00\x49\x0F\x00\x4C\x33\x00\x00\x00\x49\xD3";
            pPatternData->pBodyFallbackMasks[0]    = _x _x _0 _0 _0 _0 _x _x _x _0 _0 _x _x _0 _x _x _0 _0 _0 _x _x;
            pPatternData->pBodyFallbackPatterns[1] = "\x48\x8B\x00\x00\x00\x00\x00\x00\x00\x00\x48\x33\x00\x00\x00\x00\x00\x48\x00\x00\x48\x0F\x00\x48";
            pPatternData->pBodyFallbackMasks[1]    = _x _x _0 _0 _0 _0 _x _0 _0 _0 _x _x _0 _0 _0 _0 _x _x _0 _0 _x _x _0 _x;

            pPatternData->PrimarySectionsName = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);

            pPatternData->FallbackSectionsName[0] = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);
            pPatternData->FallbackSectionsName[1] = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);

            pPatternData->BodyFallbackPatternCount = 2;

            pPatternData->EpiloguePatternCount = 0;
            break;
        case NTOS_PATTERN_DATA_KIWAITALWAYS_ID:
            pPatternData->pBodyPrimaryPattern = "\x00\x33\x00\x00\x00\x00\x00\x49\x89\x00\x00\x48\x8D\x00\x00\x41\x83\x00\xFF\x75\x00\xC3\xCC";
            pPatternData->pBodyPrimaryMask    = _0 _x _0 _0 _0 _0 _x _x _x _0 _0 _x _x _0 _0 _x _x _0 _x _x _0 _x _x;

            pPatternData->pBodyFallbackPatterns[0] = "\x00\x33\x00\x00\x00\x00\x00\x48\x8B\x00\x00\x00\x00\x00\x48\x8B\x00\x00\x49\x0F\x00\x4C\x33\x00\x00\x00\x49\xD3";
            pPatternData->pBodyFallbackMasks[0]    = _0 _x _0 _0 _0 _0 _x _x _x _0 _0 _0 _0 _x _x _x _0 _0 _x _x _0 _x _x _0 _0 _0 _x _x;
            pPatternData->pBodyFallbackPatterns[1] = "\x48\x33\x00\x00\x00\x00\x00\x48\x00\x00\x48\x0F\x00\x48\x00\x00\x4C\x89\x00\x24";
            pPatternData->pBodyFallbackMasks[1]    = _x _x _0 _0 _0 _0 _x _x _0 _0 _x _x _0 _x _0 _0 _x _x _0 _x;

            pPatternData->PrimarySectionsName = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);

            pPatternData->FallbackSectionsName[0] = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);
            pPatternData->FallbackSectionsName[1] = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);

            pPatternData->BodyFallbackPatternCount = 2;

            pPatternData->EpiloguePatternCount = 0;
            break;
        case NTOS_PATTERN_DATA_KIBALANCESETMANAGERPERIODICDPC_ID:
            pPatternData->pBodyPrimaryPattern = "\x48\x8D\x00\x00\x00\x00\x00\x45\x00\x00\x89\x00\x00\x00\x00\x00\x00\x00\xE8\x00\x00\x00\x00\xE9\x00\x00\x00\x00\x44\x8B";
            pPatternData->pBodyPrimaryMask    = _x _x _0 _0 _0 _0 _0 _x _0 _0 _x _0 _0 _0 _0 _0 _0 _0 _x _0 _0 _0 _0 _x _0 _0 _0 _0 _x _x;

            pPatternData->PrimarySectionsName = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);

            pPatternData->BodyFallbackPatternCount = 0;

            pPatternData->EpiloguePatternCount = 0;
            break;
        case NTOS_PATTERN_DATA_KIBALANCESETMANAGERPERIODICEVENT_ID:
            pPatternData->pBodyPrimaryPattern = "\x48\x8D\x00\x00\x00\x00\x00\x48\x89\x00\x24\x00\x8D\x00\x00\x48\x8B\x00\x00\x00\x00\x00\x49\x00\x04";
            pPatternData->pBodyPrimaryMask    = _x _x _0 _0 _0 _0 _0 _x _x _0 _x _0 _x _0 _0 _x _x _0 _0 _0 _0 _0 _x _0 _x;

            pPatternData->PrimarySectionsName = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);

            pPatternData->BodyFallbackPatternCount = 0;

            pPatternData->EpiloguePatternCount = 0;
            break;
        case NTOS_PATTERN_DATA_PGGLOBALCONTEXT_ID:
            pPatternData->pBodyPrimaryPattern = "\x48\x8B\x00\x00\x00\x00\x00\x48\x8B\x00\x00\x00\x00\x00\x48\x8B\x00\x00\xFB\x0F\xBA";
            pPatternData->pBodyPrimaryMask    = _x _x _0 _0 _0 _0 _x _x _x _0 _0 _0 _0 _x _x _x _0 _0 _x _x _x;

            pPatternData->PrimarySectionsName = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);

            pPatternData->BodyFallbackPatternCount = 0;

            pPatternData->EpiloguePatternCount = 0;
            break;
        case NTOS_PATTERN_DATA_PGCHECKTIMERIDT_ID:
            pPatternData->pBodyPrimaryPattern = "\x48\x3B\x00\x00\x00\x00\x00\x0F\x93\x00\x85\x00\x0F\x84\x00\x00\x00\x00\x0F\x01\x8C\x24";
            pPatternData->pBodyPrimaryMask    = _x _x _0 _0 _0 _0 _x _x _x _0 _x _0 _x _x _0 _0 _0 _0 _x _x _x _x;

            pPatternData->PrimarySectionsName = QUICK_XOR64(NTOS_TEXT_SECTION_NAME);

            pPatternData->BodyFallbackPatternCount = 0;

            pPatternData->EpiloguePatternCount = 0;
            break;
        case NTOS_PATTERN_DATA_PGCHECKTIMERSSDT_ID:
            pPatternData->pBodyPrimaryPattern = "\x48\x3B\x00\x00\x00\x00\x00\x45\x8D\x00\x00\x0F\x82\x00\x00\x00\x00\x4C\x8D\x00\x00\x00\x00\x00\x00\x00\x00\x0F\x18\x05";
            pPatternData->pBodyPrimaryMask    = _x _x _0 _0 _0 _0 _x _x _x _0 _0 _x _x _0 _0 _0 _0 _x _x _0 _0 _0 _0 _x _0 _0 _0 _x _x _x;

            pPatternData->PrimarySectionsName = QUICK_XOR64(NTOS_PAGE_SECTION_NAME);

            pPatternData->BodyFallbackPatternCount = 0;

            pPatternData->EpiloguePatternCount = 0;
            break;
        default:
            DBG_BREAK;
            return HR_ABORTED;
        }
        break;
    default:
        DBG_BREAK;
        return HR_ABORTED;
    }

    return HR_SUCCESS;
}

HR_STATUS
FASTCALL
InitHunterContextOffsetsTable(
    IN HR_CONTEXT *pHunterContext
)
/*++
* Routine Description:
*
*     This routine initializes the
*     hunter context offsets table.
*
* Arguments:
*
*     pHunterContext - Supplies a pointer to the
*                      HR_CONTEXT structure.
*
* Return Value:
*
*     Internal status.
*
--*/
{
    if (!pHunterContext) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    switch (SharedUserData->NtBuildNumber & NTOS_BUILD_NUMBER_MASK) {
    case NTOS_BUILD_NUMBER_25H2:
        pHunterContext->NTOS_OFFSETS_TABLE.KPRCB_OFFSETS.
            OffsetCurrentThread = 0x08;
        pHunterContext->NTOS_OFFSETS_TABLE.KPRCB_OFFSETS.
            OffsetNumber        = 0x24;
        pHunterContext->NTOS_OFFSETS_TABLE.KPRCB_OFFSETS.
            OffsetHalReserved   = 0x48;
        pHunterContext->NTOS_OFFSETS_TABLE.KPRCB_OFFSETS.
            OffsetAcpiReserved  = 0xE0;
        pHunterContext->NTOS_OFFSETS_TABLE.KPRCB_OFFSETS.
            OffsetDpcData       = 0x3840;
        pHunterContext->NTOS_OFFSETS_TABLE.ETHREAD_OFFSETS.
            OffsetCid           = 0x508; 
        pHunterContext->NTOS_OFFSETS_TABLE.CLIENT_ID_OFFSETS.
            OffsetUniqueThread  = 0x08;
        pHunterContext->NTOS_OFFSETS_TABLE.KTHREAD_OFFSETS.
            OffsetThreadLock    = 0x40;
        pHunterContext->NTOS_OFFSETS_TABLE.KTHREAD_OFFSETS.
            OffsetApcState      = 0x98;
        break;
    default:
        DBG_BREAK;
        return HR_ABORTED;
    }

    return HR_SUCCESS;
}

#define NTOS_NAME_HASH32_BASE_ID   0xE1BF2105UI32
#define NTOS_NAME_HASH32_ID_OFFSET 0x02E1UI16

// 01.
#define NTOS_PSINITIALSYSTEMPROCESS_NAME_HASH32_ID \
(NTOS_NAME_HASH32_BASE_ID +  NTOS_NAME_HASH32_ID_OFFSET)

// 02.
#define NTOS_PSLOADEDMODULELIST_NAME_HASH32_ID \
(NTOS_NAME_HASH32_BASE_ID + (NTOS_NAME_HASH32_ID_OFFSET * 2))

// 03.
#define NTOS_RTLLOOKUPFUNCTIONENTRY_NAME_HASH32_ID \
(NTOS_NAME_HASH32_BASE_ID + (NTOS_NAME_HASH32_ID_OFFSET * 3))

// 04.
#define NTOS_RTLRANDOMEX_NAME_HASH32_ID \
(NTOS_NAME_HASH32_BASE_ID + (NTOS_NAME_HASH32_ID_OFFSET * 4))

// 05.
#define NTOS_IOGETSTACKLIMITS_NAME_HASH32_ID \
(NTOS_NAME_HASH32_BASE_ID + (NTOS_NAME_HASH32_ID_OFFSET * 5))

// 06.
#define NTOS_RTLVIRTUALUNWIND2_NAME_HASH32_ID \
(NTOS_NAME_HASH32_BASE_ID + (NTOS_NAME_HASH32_ID_OFFSET * 6))

// 07.
#define NTOS_RTLPCTOFILEHEADER_NAME_HASH32_ID \
(NTOS_NAME_HASH32_BASE_ID + (NTOS_NAME_HASH32_ID_OFFSET * 7))

// 08.
#define NTOS_KEIPIGENERICCALL_NAME_HASH32_ID \
(NTOS_NAME_HASH32_BASE_ID + (NTOS_NAME_HASH32_ID_OFFSET * 8))

// 09.
#define NTOS_KEQUERYACTIVEPROCESSORCOUNTEX_NAME_HASH32_ID \
(NTOS_NAME_HASH32_BASE_ID + (NTOS_NAME_HASH32_ID_OFFSET * 9))

// 10.
#define NTOS_KZRAISEIRQL_NAME_HASH32_ID \
(NTOS_NAME_HASH32_BASE_ID + (NTOS_NAME_HASH32_ID_OFFSET * 10))

// 11.
#define NTOS_KZLOWERIRQL_NAME_HASH32_ID \
(NTOS_NAME_HASH32_BASE_ID + (NTOS_NAME_HASH32_ID_OFFSET * 11))

// 12.
#define NTOS_KEBUGCHECKEX_NAME_HASH32_ID \
(NTOS_NAME_HASH32_BASE_ID + (NTOS_NAME_HASH32_ID_OFFSET * 12))

// 13.
#define NTOS_KEWAITFORSINGLEOBJECT_NAME_HASH32_ID \
(NTOS_NAME_HASH32_BASE_ID + (NTOS_NAME_HASH32_ID_OFFSET * 13))

// 14.
#define NTOS_KEWAITFORMULTIPLEOBJECTS_NAME_HASH32_ID \
(NTOS_NAME_HASH32_BASE_ID + (NTOS_NAME_HASH32_ID_OFFSET * 14))

// 15.
#define NTOS_KEDELAYEXECUTIONTHREAD_NAME_HASH32_ID \
(NTOS_NAME_HASH32_BASE_ID + (NTOS_NAME_HASH32_ID_OFFSET * 15))

#define NTOS_PSINITIALSYSTEMPROCESS_NAME_HASH32         0x0742A560UI32
#define NTOS_PSLOADEDMODULELIST_NAME_HASH32             0x3DA0D935UI32
#define NTOS_RTLLOOKUPFUNCTIONENTRY_NAME_HASH32         0x0E20A343UI32
#define NTOS_RTLRANDOMEX_NAME_HASH32                    0x9AB4737EUI32
#define NTOS_IOGETSTACKLIMITS_NAME_HASH32               0x7B7293B1UI32
#define NTOS_RTLVIRTUALUNWIND2_NAME_HASH32              0x9281BB47UI32
#define NTOS_RTLPCTOFILEHEADER_NAME_HASH32              0xD6896A10UI32
#define NTOS_KEIPIGENERICCALL_NAME_HASH32               0x0A36B576UI32
#define NTOS_KEQUERYACTIVEPROCESSORCOUNTEX_NAME_HASH32  0xDDBB4CF4UI32
#define NTOS_KZRAISEIRQL_NAME_HASH32                    0x03F1D28CUI32
#define NTOS_KZLOWERIRQL_NAME_HASH32                    0xA9E0AFD9UI32
#define NTOS_KEBUGCHECKEX_NAME_HASH32                   0x9FA533C2UI32
#define NTOS_KEWAITFORSINGLEOBJECT_NAME_HASH32          0xD5D751C7UI32
#define NTOS_KEWAITFORMULTIPLEOBJECTS_NAME_HASH32       0x82A81EC8UI32
#define NTOS_KEDELAYEXECUTIONTHREAD_NAME_HASH32         0x96680457UI32

#define NTOS_NAME_HASH32_ID_COUNT 15

#define INIT_FIND_EXCEPTION_ENTRY_ATTEMPT_MAXCOUNT 8

HR_STATUS
FASTCALL
InitHunterContext(
    OUT HR_CONTEXT_DESCRIPTOR *pHunterContextDesc
)
/*++
* Routine Description:
*
*     This routine initializes the
*     hunter context.
*
* Arguments:
*
*     pHunterContextDesc - Supplies a pointer to the
*                          HR_CONTEXT_DESCRIPTOR structure.
*
* Return Value:
*
*     Internal status.
*
--*/
{
    VOID *pNtosBase = NULL;

    UINT64 ImageBase = 0;

    RUNTIME_FUNCTION *pFunctionEntry = NULL;

    RUNTIME_FUNCTION* (FASTCALL *pRtlLookupFunctionEntry) (
        IN  UINT64 ControlPc,
        OUT UINT64 *pImageBase,
        OUT UNWIND_HISTORY_TABLE *pHistoryTable
        ) = NULL;

    VOID* (FASTCALL *pMmAllocateIndependentPagesEx) (
        IN UINT64 NoBytes,
        IN  OPTIONAL UINT32 NumaNodeNum,
        OUT OPTIONAL UINT64 *pPfnArray,
        IN  OPTIONAL UINT32 PfnArraySize
        ) = NULL;

    NTOS_PATTERN_DATA NtosPatternData = { 0 };

    UINT32 CurrentPatternId = 0;
    UINT32 CurrentItemNameHashId = 0;
    UINT32 CurrentItemNameHash = 0;

    UINT8 EpiCount = 0;

    BOOLEAN EpiFound = FALSE;

    VOID **pSelectedItem = NULL;
    VOID **pSelectedItem2 = NULL;

    KDDEBUGGER_DATA64 *pKdDataBlock = NULL;

    UINT64 SelfReferencePml4eIdx = 0;
    UINT64 AccPteBase = 0;

    UINT32 Seed = 0;
    UINT16 ContextLowPaddingSize = 0;
    UINT16 ContextHighPaddingSize = 0;
    UINT32 *pContextLowPaddingBase = NULL;
    UINT32 *pContextHighPaddingBase = NULL;

    HR_CONTEXT *pHunterContext = NULL;
    HR_CONTEXT *pHunterContext2 = NULL;

    VOID (FASTCALL *pMmFreeIndependentPages) (
        IN VOID *pAllocBase,
        IN UINT64 NoBytes
        ) = NULL;

    HR_CONTEXT *pActiveContext = NULL;
    VOID *pActiveContextAllocBase = NULL;
    UINT32 ActiveContextAllocSize = 0;
    BOOLEAN IsAborted = TRUE;

    if (!pHunterContextDesc) {
        DBG_BREAK;
        goto aborted;
    }

    if (HR_ERROR(NtosFindKernelBaseVa(&pNtosBase))) {
        DBG_BREAK;
        goto aborted;
    } else if (!pNtosBase) {
        DBG_BREAK;
        goto aborted;
    }

    if (HR_ERROR(PeFindExportItemCrc32Hash(
        pNtosBase,
        QUICK_XOR32(NTOS_RTLLOOKUPFUNCTIONENTRY_NAME_HASH32),
        TRUE,
        (VOID**)&pRtlLookupFunctionEntry))) {
        DBG_BREAK;
        goto aborted;
    } else if (!pRtlLookupFunctionEntry) {
        DBG_BREAK;
        goto aborted;
    }

    if (HR_ERROR(InitGetPatternData(
        NTOS_PATTERN_DATA_MMALLOCATEINDEPENDENTPAGESEX_ID,
        &NtosPatternData))) {
        DBG_BREAK;
        goto aborted;
    }
    if (HR_ERROR(PeFindSectionMemoryPattern(
        pNtosBase,
        NtosPatternData.PrimarySectionsName,
        TRUE,
        NtosPatternData.pBodyPrimaryPattern,
        NtosPatternData.pBodyPrimaryMask,
        (VOID*)&pMmAllocateIndependentPagesEx))) {
        DBG_BREAK;
        goto aborted;
    } 
    if (!pMmAllocateIndependentPagesEx) {
        for (UINT8 i2 = 0;
            i2 < NtosPatternData.BodyFallbackPatternCount; i2++) {
            if (HR_ERROR(PeFindSectionMemoryPattern(
                pNtosBase,
                NtosPatternData.FallbackSectionsName[i2],
                TRUE,
                NtosPatternData.pBodyFallbackPatterns[i2],
                NtosPatternData.pBodyFallbackMasks[i2],
                (VOID*)&pMmAllocateIndependentPagesEx))) {
                DBG_BREAK;
                goto aborted;
            }
            if (pMmAllocateIndependentPagesEx) {
                break;
            }
        }
    }
    if (!pMmAllocateIndependentPagesEx) {
        DBG_BREAK;
        goto aborted;
    }
      
    if (!(pFunctionEntry = pRtlLookupFunctionEntry(
        (UINT64)pMmAllocateIndependentPagesEx,
        &ImageBase,
        NULL))) {
        DBG_BREAK;
        goto aborted;
    }

    pMmAllocateIndependentPagesEx =
        (VOID*(FASTCALL*)(UINT64, UINT32, UINT64*, UINT32))
        (ImageBase + pFunctionEntry->BeginAddress);

    if (!(pHunterContext = pMmAllocateIndependentPagesEx(
        REQUIRED_NUMBER_OF_PAGES(sizeof(HR_CONTEXT))
        << PAGE_SHIFT,
        (UINT32)-1,
        NULL,
        0))) {
        DBG_BREAK;
        goto aborted;
    }

    RtlSecureZeroMemory(pHunterContext, sizeof(HR_CONTEXT));

    pHunterContext->HR_API.pMmAllocateIndependentPagesEx =
        pMmAllocateIndependentPagesEx;

    pHunterContext->HR_API.pRtlLookupFunctionEntry =
        pRtlLookupFunctionEntry;

    CurrentPatternId = NTOS_PATTERN_DATA_BASE_ID;
    for (UINT8 i = 0; i < NTOS_PATTERN_ID_COUNT; i++) {
        CurrentPatternId += NTOS_PATTERN_ID_OFFSET;
        switch (CurrentPatternId) {
        case NTOS_PATTERN_DATA_MMALLOCATEINDEPENDENTPAGESEX_ID:
        case NTOS_PATTERN_DATA_EPI_BASE_ID:
            continue;
        case NTOS_PATTERN_DATA_KEREMOVEPRIQUEUE_EPI_ID:
            EpiCount++;
            break;
        case NTOS_PATTERN_DATA_KEWAITFORSINGLEOBJECT_EPI_ID:
        case NTOS_PATTERN_DATA_KEWAITFORMULTIPLEOBJECTS_EPI_ID:
        case NTOS_PATTERN_DATA_KEDELAYEXECUTIONTHREAD_EPI_ID:
            EpiCount++;
            continue;
        default:
            break;
        }
        if (HR_ERROR(InitGetPatternData(
            CurrentPatternId,
            &NtosPatternData))) {
            DBG_BREAK;
            goto aborted;
        }
        switch (CurrentPatternId) {
        case NTOS_PATTERN_DATA_MMFREEINDEPENDENTPAGES_ID:
            pSelectedItem = (VOID**)&pHunterContext->HR_API.
                pMmFreeIndependentPages;
            break;
        case NTOS_PATTERN_DATA_MMSETPAGEPROTECTION_ID:
            pSelectedItem = (VOID**)&pHunterContext->HR_API.
                pMmSetPageProtection;
            break;
        case NTOS_PATTERN_DATA_KDCOPYDATABLOCK_ID:
            pSelectedItem = (VOID**)&pHunterContext->HR_API.
                pKdCopyDataBlock;
            break;
        case NTOS_PATTERN_DATA_RTLICPARSEINSTRUCTION_ID:
            pSelectedItem = (VOID**)&pHunterContext->HR_API.
                pRtlIcParseInstruction;
            break;
        case NTOS_PATTERN_DATA_RTLPICACCESSMEMORY_ID:
            pSelectedItem = (VOID**)&pHunterContext->HR_API.
                pRtlpIcAccessMemory;
            break;
        case NTOS_PATTERN_DATA_XHALTIMERWATCHDOGSTOPCUSTOM_ID:
            pSelectedItem = (VOID**)&pHunterContext->HR_API.
                pCustomxHalTimerWatchdogStop;
            break;
        case NTOS_PATTERN_DATA_KIEXECUTEALLDPCS_ID:
            pSelectedItem = (VOID**)&pHunterContext->NTOS_ROUTINES.
                pKiExecuteAllDpcs;
            break;
        case NTOS_PATTERN_DATA_KIPROCESSEXPIREDTIMERLIST_ID:
            pSelectedItem = (VOID**)&pHunterContext->NTOS_ROUTINES.
                pKiProcessExpiredTimerList;
            break;
        case NTOS_PATTERN_DATA_KIEXPIRETIMER2_ID:
            pSelectedItem = (VOID**)&pHunterContext->NTOS_ROUTINES.
                pKiExpireTimer2;
            break;
        case NTOS_PATTERN_DATA_KIDELIVERAPC_ID:
            pSelectedItem = (VOID**)&pHunterContext->NTOS_ROUTINES.
                pKiDeliverApc;
            break;
        case NTOS_PATTERN_DATA_KEREMOVEPRIQUEUE_EPI_ID:
            pSelectedItem = (VOID**)&pHunterContext->NTOS_ROUTINES.
                pKeRemovePriQueue;
            break;
        case NTOS_PATTERN_DATA_KIBALANCESETMANAGERDEFERREDROUTINE_ID:
            pSelectedItem = (VOID**)&pHunterContext->NTOS_ROUTINES.
                pKiBalanceSetManagerDeferredRoutine;
            break;
        case NTOS_PATTERN_DATA_KICUSTOMRECURSEROUTINEX_ID:
            pSelectedItem = (VOID**)&pHunterContext->NTOS_ROUTINES.
                pKiCustomRecurseRoutineX;
            break;
        case NTOS_PATTERN_DATA_CCBCBPROFILER_ID:
            pSelectedItem = (VOID**)&pHunterContext->NTOS_ROUTINES.
                pCcBcbProfiler;
            break;
        case NTOS_PATTERN_DATA_CCBCBPROFILER2_ID:
            pSelectedItem = (VOID**)&pHunterContext->NTOS_ROUTINES.
                pCcBcbProfiler2;
            break;
        case NTOS_PATTERN_DATA_KIDISPATCHCALLOUT_ID:
            pSelectedItem = (VOID**)&pHunterContext->NTOS_ROUTINES.
                pKiDispatchCallout;
            break;
        case NTOS_PATTERN_DATA_KISWINTERRUPTDISPATCH_ID:
            pSelectedItem = (VOID**)&pHunterContext->NTOS_ROUTINES.
                pKiSwInterruptDispatch;
            break;
        case NTOS_PATTERN_DATA_KIERRATA671PRESENT_ID:
            pSelectedItem = (VOID**)&pHunterContext->NTOS_ROUTINES.
                pKiErrata671Present;
            break;
        case NTOS_PATTERN_DATA_KIWAITNEVER_ID:
            pSelectedItem = (VOID**)&pHunterContext->NTOS_ITEMS.
                pKiWaitNever;
            break;
        case NTOS_PATTERN_DATA_KIWAITALWAYS_ID:
            pSelectedItem = (VOID**)&pHunterContext->NTOS_ITEMS.
                pKiWaitAlways;
            break;
        case NTOS_PATTERN_DATA_KIBALANCESETMANAGERPERIODICDPC_ID:
            pSelectedItem = (VOID**)&pHunterContext->NTOS_ITEMS.
                pKiBalanceSetManagerPeriodicDpc;
            break;
        case NTOS_PATTERN_DATA_KIBALANCESETMANAGERPERIODICEVENT_ID:
            pSelectedItem = (VOID**)&pHunterContext->NTOS_ITEMS.
                pKiBalanceSetManagerPeriodicEvent;
            break;
        case NTOS_PATTERN_DATA_PGGLOBALCONTEXT_ID:
            pSelectedItem = (VOID**)&pHunterContext->NTOS_ITEMS.
                pPgGlobalContext;
            break;
        case NTOS_PATTERN_DATA_PGCHECKTIMERIDT_ID:
            pSelectedItem = (VOID**)&pHunterContext->NTOS_ITEMS.
                pPgCheckTimerIDT;
            break;
        case NTOS_PATTERN_DATA_PGCHECKTIMERSSDT_ID:
            pSelectedItem = (VOID**)&pHunterContext->NTOS_ITEMS.
                pPgCheckTimerSSDT;
            break;
        default:
            DBG_BREAK;
            goto aborted;
        }
        if (HR_ERROR(PeFindSectionMemoryPattern(
            pNtosBase,
            NtosPatternData.PrimarySectionsName,
            TRUE,
            NtosPatternData.pBodyPrimaryPattern,
            NtosPatternData.pBodyPrimaryMask,
            pSelectedItem))) {
            DBG_BREAK;
            goto aborted;
        } else if (!(*pSelectedItem)){
            for (UINT8 i2 = 0;
                i2 < NtosPatternData.BodyFallbackPatternCount; i2++) {
                if (HR_ERROR(PeFindSectionMemoryPattern(
                    pNtosBase,
                    NtosPatternData.FallbackSectionsName[i2],
                    TRUE,
                    NtosPatternData.pBodyFallbackPatterns[i2],
                    NtosPatternData.pBodyFallbackMasks[i2],
                    pSelectedItem))) {
                    DBG_BREAK;
                    goto aborted;
                }
                if (*pSelectedItem) {
                    break;
                }
            }
        }
        if (!(*pSelectedItem)) {
            DBG_BREAK;
            goto aborted;
        }
        switch (CurrentPatternId) {
        case NTOS_PATTERN_DATA_XHALTIMERWATCHDOGSTOPCUSTOM_ID:
            break;
        case NTOS_PATTERN_DATA_KIWAITNEVER_ID:
        case NTOS_PATTERN_DATA_KIWAITALWAYS_ID:
        case NTOS_PATTERN_DATA_KIBALANCESETMANAGERPERIODICDPC_ID:
        case NTOS_PATTERN_DATA_KIBALANCESETMANAGERPERIODICEVENT_ID:
        case NTOS_PATTERN_DATA_PGGLOBALCONTEXT_ID:
        case NTOS_PATTERN_DATA_PGCHECKTIMERIDT_ID:
        case NTOS_PATTERN_DATA_PGCHECKTIMERSSDT_ID:
            *pSelectedItem = (VOID**)((((UINT64)*pSelectedItem) + 7) +
                *(INT32*)(((UINT64)*pSelectedItem) + 3));
            break;
        default:
            if (!(pFunctionEntry = pRtlLookupFunctionEntry(
                (UINT64)*pSelectedItem,
                &ImageBase,
                NULL))) {
                DBG_BREAK;
                goto aborted;
            }
            *pSelectedItem =
                (VOID*)(ImageBase + pFunctionEntry->BeginAddress);
            break;
        }
    }

    CurrentItemNameHashId = NTOS_NAME_HASH32_BASE_ID;
    for (UINT8 i = 0; i < NTOS_NAME_HASH32_ID_COUNT; i++) {
        CurrentItemNameHashId += NTOS_NAME_HASH32_ID_OFFSET;
        switch (CurrentItemNameHashId) {
        case NTOS_RTLLOOKUPFUNCTIONENTRY_NAME_HASH32_ID:
            continue;
        }
        switch (CurrentItemNameHashId) {
        case NTOS_PSINITIALSYSTEMPROCESS_NAME_HASH32_ID:
            CurrentItemNameHash = QUICK_XOR32(
                NTOS_PSINITIALSYSTEMPROCESS_NAME_HASH32);
            pSelectedItem = (VOID**)&pHunterContext->NTOS_PROCESS.
                pProcess;
            break;
        case NTOS_PSLOADEDMODULELIST_NAME_HASH32_ID:
            CurrentItemNameHash = QUICK_XOR32(
                NTOS_PSLOADEDMODULELIST_NAME_HASH32);
            pSelectedItem = (VOID**)&pHunterContext->NTOS_PROCESS.
                pLoadedModuleList;
            break;
        case NTOS_RTLRANDOMEX_NAME_HASH32_ID:
            CurrentItemNameHash = QUICK_XOR32(
                NTOS_RTLRANDOMEX_NAME_HASH32);
            pSelectedItem = (VOID**)&pHunterContext->HR_API.
                pRtlRandomEx;
            break;
        case NTOS_IOGETSTACKLIMITS_NAME_HASH32_ID:
            CurrentItemNameHash = QUICK_XOR32(
                NTOS_IOGETSTACKLIMITS_NAME_HASH32);
            pSelectedItem = (VOID**)&pHunterContext->HR_API.
                pIoGetStackLimits;
            break;
        case NTOS_RTLVIRTUALUNWIND2_NAME_HASH32_ID:
            CurrentItemNameHash = QUICK_XOR32(
                NTOS_RTLVIRTUALUNWIND2_NAME_HASH32);
            pSelectedItem = (VOID**)&pHunterContext->HR_API.
                pRtlVirtualUnwind2;
            break;
        case NTOS_RTLPCTOFILEHEADER_NAME_HASH32_ID:
            CurrentItemNameHash = QUICK_XOR32(
                NTOS_RTLPCTOFILEHEADER_NAME_HASH32);
            pSelectedItem = (VOID**)&pHunterContext->HR_API.
                pRtlPcToFileHeader;
            break;
        case NTOS_KEIPIGENERICCALL_NAME_HASH32_ID:
            CurrentItemNameHash = QUICK_XOR32(
                NTOS_KEIPIGENERICCALL_NAME_HASH32);
            pSelectedItem = (VOID**)&pHunterContext->HR_API.
                pKeIpiGenericCall;
            break;
        case NTOS_KEQUERYACTIVEPROCESSORCOUNTEX_NAME_HASH32_ID:
            CurrentItemNameHash = QUICK_XOR32(
                NTOS_KEQUERYACTIVEPROCESSORCOUNTEX_NAME_HASH32);
            pSelectedItem = (VOID**)&pHunterContext->HR_API.
                pKeQueryActiveProcessorCountEx;
            break;
        case NTOS_KZRAISEIRQL_NAME_HASH32_ID:
            CurrentItemNameHash = QUICK_XOR32(
                NTOS_KZRAISEIRQL_NAME_HASH32);
            pSelectedItem = (VOID**)&pHunterContext->HR_API.
                pKzRaiseIrql;
            break;
        case NTOS_KZLOWERIRQL_NAME_HASH32_ID:
            CurrentItemNameHash = QUICK_XOR32(
                NTOS_KZLOWERIRQL_NAME_HASH32);
            pSelectedItem = (VOID**)&pHunterContext->HR_API.
                pKzLowerIrql;
            break;
        case NTOS_KEBUGCHECKEX_NAME_HASH32_ID:
            CurrentItemNameHash = QUICK_XOR32(
                NTOS_KEBUGCHECKEX_NAME_HASH32);
            pSelectedItem = (VOID**)&pHunterContext->HR_API.
                pKeBugCheckEx;
            break;
        case NTOS_KEWAITFORSINGLEOBJECT_NAME_HASH32_ID:
            CurrentItemNameHash = QUICK_XOR32(
                NTOS_KEWAITFORSINGLEOBJECT_NAME_HASH32);
            pSelectedItem = (VOID**)&pHunterContext->NTOS_ROUTINES.
                pKeWaitForSingleObject;
            break;
            case NTOS_KEWAITFORMULTIPLEOBJECTS_NAME_HASH32_ID:
            CurrentItemNameHash = QUICK_XOR32(
                NTOS_KEWAITFORMULTIPLEOBJECTS_NAME_HASH32);
            pSelectedItem = (VOID**)&pHunterContext->NTOS_ROUTINES.
                pKeWaitForMultipleObjects;
            break;
            case NTOS_KEDELAYEXECUTIONTHREAD_NAME_HASH32_ID:
                CurrentItemNameHash = QUICK_XOR32(
                    NTOS_KEDELAYEXECUTIONTHREAD_NAME_HASH32);
                pSelectedItem = (VOID**)&pHunterContext->NTOS_ROUTINES.
                    pKeDelayExecutionThread;
                break;
        default:
            DBG_BREAK;
            goto aborted;
        }
        if (HR_ERROR(PeFindExportItemCrc32Hash(
            pNtosBase,
            CurrentItemNameHash,
            TRUE,
            pSelectedItem))) {
            DBG_BREAK;
            goto aborted;
        } else if (!(*pSelectedItem)) {
            DBG_BREAK;
            goto aborted;
        }
    }

    CurrentPatternId = NTOS_PATTERN_DATA_EPI_BASE_ID;
    for (UINT8 i = 0; i < EpiCount; i++) {
        CurrentPatternId += NTOS_PATTERN_ID_OFFSET;
        if (HR_ERROR(InitGetPatternData(
            CurrentPatternId,
            &NtosPatternData))) {
            DBG_BREAK;
            goto aborted;
        }
        switch (CurrentPatternId) {
        case NTOS_PATTERN_DATA_KEREMOVEPRIQUEUE_EPI_ID:
            pSelectedItem = (VOID**)&pHunterContext->NTOS_ROUTINES.
                pKeRemovePriQueue;
            pSelectedItem2 = (VOID**)&pHunterContext->NTOS_ROUTINES.
                pKeRemovePriQueueEpi;
            break;
        case NTOS_PATTERN_DATA_KEWAITFORSINGLEOBJECT_EPI_ID:
            pSelectedItem = (VOID**)&pHunterContext->NTOS_ROUTINES.
                pKeWaitForSingleObject;
            pSelectedItem2 = (VOID**)&pHunterContext->NTOS_ROUTINES.
                pKeWaitForSingleObjectEpi;
            break;
        case NTOS_PATTERN_DATA_KEWAITFORMULTIPLEOBJECTS_EPI_ID:
            pSelectedItem = (VOID**)&pHunterContext->NTOS_ROUTINES.
                pKeWaitForMultipleObjects;
            pSelectedItem2 = (VOID**)&pHunterContext->NTOS_ROUTINES.
                pKeWaitForMultipleObjectsEpi;
            break;
        case NTOS_PATTERN_DATA_KEDELAYEXECUTIONTHREAD_EPI_ID:
            pSelectedItem = (VOID**)&pHunterContext->NTOS_ROUTINES.
                pKeDelayExecutionThread;
            pSelectedItem2 = (VOID**)&pHunterContext->NTOS_ROUTINES.
                pKeDelayExecutionThreadEpi;
            break;
        default:
            DBG_BREAK;
            goto aborted;
        }
        if (!(pFunctionEntry = pRtlLookupFunctionEntry(
            (UINT64)*pSelectedItem,
            &ImageBase,
            NULL))) {
            DBG_BREAK;
            goto aborted;
        }
        for (UINT8 i2 = 0;
            i2 < INIT_FIND_EXCEPTION_ENTRY_ATTEMPT_MAXCOUNT; i2++) {
            EpiFound = TRUE;
            for (UINT8 i3 = 0;
                i3 < NtosPatternData.EpiloguePatternCount; i3++) {
                if (HR_ERROR(MemFindMemoryPattern(
                    *pSelectedItem,
                    ((ImageBase + pFunctionEntry->EndAddress) -
                    (UINT64)*pSelectedItem),
                    NtosPatternData.pEpiloguePatterns[i3],
                    NtosPatternData.pEpilogueMasks[i3],
                    &pSelectedItem2[i3]))) {
                    DBG_BREAK;
                    goto aborted;
                } else if (!(pSelectedItem2[i3])) {             
                    if (!(pFunctionEntry = pRtlLookupFunctionEntry(
                        (ImageBase + (pFunctionEntry->EndAddress + 1)),
                        &ImageBase,
                        NULL))) {
                        DBG_BREAK;
                        goto aborted;
                    }
                    EpiFound = FALSE;
                    break;
                }
            }
            if (EpiFound) {
                break;
            }
        }
        if (!EpiFound) {
            DBG_BREAK;
            goto aborted;
        }
    }

    if (!(pKdDataBlock = pMmAllocateIndependentPagesEx(
        REQUIRED_NUMBER_OF_PAGES(sizeof(KDDEBUGGER_DATA64)) << PAGE_SHIFT,
        (UINT32)-1,
        NULL,
        0))) {
        DBG_BREAK;
        goto aborted;
    }

    pHunterContext->HR_API.pKdCopyDataBlock(pKdDataBlock);

    AccPteBase = pKdDataBlock->PteBase;
    SelfReferencePml4eIdx = 
        (AccPteBase >> (9 * (MMU_PAGING_LEVELS - 1))) & 0x1FF000;
    pHunterContext->NTOS_ITEMS.PteBases[HR_CONTEXT_PTE_BASE_IDX] =
        AccPteBase;
    for (UINT8 i = 0, i2 = HR_CONTEXT_PDE_BASE_IDX;
        i < (MMU_PAGING_LEVELS - 1); i++, i2--) {
        AccPteBase += SelfReferencePml4eIdx << (9 * i2);
        pHunterContext->NTOS_ITEMS.PteBases[i2] = AccPteBase;
    }

    if (HR_ERROR(InitHunterContextOffsetsTable(pHunterContext))) {
        DBG_BREAK;
        goto aborted;
    }

    pHunterContext->NTOS_PROCESS.NTOS_IMAGE.pImageBase = pNtosBase;
    if (HR_ERROR(PeGetImageSectionsRange(
        pNtosBase,
        &pHunterContext->
        NTOS_PROCESS.NTOS_IMAGE.SECTIONS_VA_RANGE.pLowVa,
        &pHunterContext->
        NTOS_PROCESS.NTOS_IMAGE.SECTIONS_VA_RANGE.pHighVa))) {
        DBG_BREAK;
        goto aborted;
    }

    pHunterContext->NTOS_PROCESS.HR_IMAGE.pImageBase = &__ImageBase;
    if (HR_ERROR(PeGetImageSectionsRange(
        &__ImageBase,
        &pHunterContext->
        NTOS_PROCESS.HR_IMAGE.SECTIONS_VA_RANGE.pLowVa,
        &pHunterContext->
        NTOS_PROCESS.HR_IMAGE.SECTIONS_VA_RANGE.pHighVa))) {
        DBG_BREAK;
        goto aborted;
    }

    RtlSecureZeroMemory(pKdDataBlock, sizeof(KDDEBUGGER_DATA64));

    pHunterContext->HR_API.pMmFreeIndependentPages(
        pKdDataBlock,
        REQUIRED_NUMBER_OF_PAGES(sizeof(KDDEBUGGER_DATA64)) << PAGE_SHIFT);

    pKdDataBlock = NULL;

    pHunterContext->ContextSeed =
        ((UINT64)pHunterContext->HR_API.pRtlRandomEx(&Seed)) << 32;

    pHunterContext->ContextSeed +=
        ((UINT64)pHunterContext->HR_API.pRtlRandomEx(&Seed));

    if (HR_ERROR(CryCrc32DataHash(
        (UINT8*)&pHunterContext->ContextSeed,
        sizeof(HR_CONTEXT) - FIELD_OFFSET(HR_CONTEXT, ContextSeed),
        &pHunterContext->ContextHash32))) {
        DBG_BREAK;
        goto aborted;
    }
    pHunterContext->ContextHash32 = 
        QUICK_XOR32(pHunterContext->ContextHash32);

    ContextLowPaddingSize = 
        (UINT16)pHunterContext->HR_API.pRtlRandomEx(&Seed);
    ContextHighPaddingSize = 
        (UINT16)pHunterContext->HR_API.pRtlRandomEx(&Seed);

    ContextLowPaddingSize = 
        ((ContextLowPaddingSize & 0x3FF) + 0x401) & ~0x07;
    ContextHighPaddingSize = 
        ((ContextHighPaddingSize & 0x3FF) + 0x401) & ~0x07;

    if (!(pContextLowPaddingBase = pMmAllocateIndependentPagesEx(
        REQUIRED_NUMBER_OF_PAGES(
        ContextLowPaddingSize +
        sizeof(HR_CONTEXT) +
        ContextHighPaddingSize)
        << PAGE_SHIFT,
        (UINT32)-1,
        NULL,
        0))) {
        DBG_BREAK;
        goto aborted;
    }

    pHunterContext2 = (HR_CONTEXT*)(((UINT8*)pContextLowPaddingBase) +
        ContextLowPaddingSize);

    pContextHighPaddingBase = (UINT32*)(pHunterContext2 + 1);

    memmove(pHunterContext2, pHunterContext, sizeof(HR_CONTEXT));

    RtlSecureZeroMemory(pHunterContext, sizeof(HR_CONTEXT));

    pHunterContext2->HR_API.pMmFreeIndependentPages(
        pHunterContext,
        REQUIRED_NUMBER_OF_PAGES(sizeof(HR_CONTEXT)) << PAGE_SHIFT);

    pHunterContext = NULL;

    pHunterContext2->HR_API.pRtlRandomEx(&Seed);
    if (HR_ERROR(CryFillBufferRandomDword(
        pContextLowPaddingBase,
        (ContextLowPaddingSize / 4),
        Seed))) {
        DBG_BREAK;
        goto aborted;
    }

    pHunterContext2->HR_API.pRtlRandomEx(&Seed);
    if (HR_ERROR(CryFillBufferRandomDword(
        pContextHighPaddingBase,
        (ContextHighPaddingSize / 4),
        Seed))) {
        DBG_BREAK;
        goto aborted;
    }

    pHunterContextDesc->pHunterContext  = pHunterContext2;
    pHunterContextDesc->pAllocBase      = pContextLowPaddingBase;
    pHunterContextDesc->LowPaddingSize  = ContextLowPaddingSize;
    pHunterContextDesc->HighPaddingSize = ContextHighPaddingSize;

    IsAborted = FALSE;

aborted:

    if (IsAborted) {
        if (pHunterContext) {
            pActiveContext = pHunterContext;
            pActiveContextAllocBase = pHunterContext;
            ActiveContextAllocSize = sizeof(HR_CONTEXT);

        } else if (pHunterContext2) { 
            pActiveContext = pHunterContext2;
            pActiveContextAllocBase = pContextLowPaddingBase;
            ActiveContextAllocSize = 
                (ContextLowPaddingSize +
                sizeof(HR_CONTEXT) +
                ContextHighPaddingSize);
        }
        if (pActiveContext) {
            pMmFreeIndependentPages = 
                pActiveContext->HR_API.pMmFreeIndependentPages;

            RtlSecureZeroMemory(
                pActiveContextAllocBase,
                ActiveContextAllocSize);
            pMmFreeIndependentPages(
                pActiveContextAllocBase,
                REQUIRED_NUMBER_OF_PAGES(ActiveContextAllocSize)
                << PAGE_SHIFT);
            if (pKdDataBlock) {
                RtlSecureZeroMemory(
                    pKdDataBlock,
                    sizeof(KDDEBUGGER_DATA64));
                pMmFreeIndependentPages(
                    pKdDataBlock,
                    REQUIRED_NUMBER_OF_PAGES(sizeof(KDDEBUGGER_DATA64))
                    << PAGE_SHIFT);
            }
        }
        return HR_ABORTED;
    }

    return HR_SUCCESS;
}

