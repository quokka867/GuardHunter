/*++
* Module Name:
*
*     flt.c
*
* Abstract:
*
*     This module contains the basic filter callback routines.
*
* Author:
*
*     quokka867 (GitHub/Twitter).
*
--*/

#include "../../../headers/flt.h"

typedef struct _WHITE_TABLE_ENTRY {
    UINT64 RoutineBase;
    UINT64 RoutineEnd;
} WHITE_TABLE_ENTRY;

typedef struct _WHITE_TABLE_DATA {
    WHITE_TABLE_ENTRY *pWhiteTable;
    volatile UINT32 WhiteTableLock;
    volatile UINT16 EntryCount;
    UINT8 WhiteTablePad0[2];
} WHITE_TABLE_DATA;

WHITE_TABLE_DATA g_FltWhiteTableData = { 0 };

HR_STATUS
FASTCALL
FltInitWhiteRoutineTable(
    IN HR_CONTEXT *pHunterContext
)
/*++
* Routine Description:
*
*     This routine initializes the white routines table.
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

    if (!(g_FltWhiteTableData.pWhiteTable =
        pHunterContext->HR_API.pMmAllocateIndependentPagesEx(
            (PAGE_SIZE * 8),
            (UINT32)-1,
            NULL,
            0))) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    return HR_SUCCESS;
}

HR_STATUS
FASTCALL
FltIsWhiteRoutine(
    IN  UINT64 RoutineBase,
    IN  UINT64 RoutineEnd,
    OUT BOOLEAN *pIsWhiteRoutine
)
/*++
* Routine Description:
*
*     This routine checks whether the specified routine address
*     range belongs to a registered white routine.
*
* Arguments:
*
*     RoutineBase     - Supplies the routine base.
*
*     RoutineEnd      - Supplies the routine end.
*
*     pIsWhiteRoutine - Supplies a pointer to a variable that
*                       receives the check status. 
*
* Return Value:
*
*     Internal status.
*
--*/
{
    if (!RoutineBase || !RoutineEnd || !pIsWhiteRoutine) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    WHITE_TABLE_ENTRY *pCurrentEntry =
        g_FltWhiteTableData.pWhiteTable;
    UINT16 EntryCount =
        g_FltWhiteTableData.EntryCount;

    KeMemoryBarrier();

    BOOLEAN EntryFound = FALSE;

    for (UINT16 i = 0; i < EntryCount; i++) {
        if (RoutineBase >= pCurrentEntry->RoutineBase &&
            RoutineEnd <= pCurrentEntry->RoutineEnd) {
            EntryFound = TRUE;
            break;
        }
        pCurrentEntry++;
    }

    *pIsWhiteRoutine = EntryFound;

    return HR_SUCCESS;
}

HR_STATUS
FASTCALL
FltRegisterWhiteRoutine(
    IN UINT64 RoutineBase,
    IN UINT64 RoutineEnd,
    IN HR_CONTEXT *pHunterContext
)
/*++
* Routine Description:
*
*     This routine registers the specified routine address
*     range in the white routines table.
*
* Arguments:
*
*     RoutineBase     - Supplies the routine base.
*
*     RoutineEnd      - Supplies the routine end.
*
*     pHunterContext  - Supplies a pointer to the
*                       HR_CONTEXT structure.
*
* Return Value:
*
*     Internal status.
*
--*/
{
    KIRQL OldIrql = 0;

    WHITE_TABLE_ENTRY *pWhiteTableNewEntry = NULL;

    BOOLEAN IsLocked = FALSE;
    BOOLEAN IsAborted = TRUE;

    BOOLEAN IsWhiteRoutine = FALSE;

    if (!RoutineBase || !RoutineEnd || !pHunterContext) {
        DBG_BREAK;
        goto aborted;
    }

    OldIrql = pHunterContext->HR_API.pKzRaiseIrql(DISPATCH_LEVEL);

    while (_interlockedbittestandset(
        (volatile LONG*)&g_FltWhiteTableData.WhiteTableLock,
        0)) {
        _mm_pause();
    }

    IsLocked = TRUE;

    if (g_FltWhiteTableData.EntryCount ==
        ((PAGE_SIZE * 8) / sizeof(WHITE_TABLE_ENTRY))) {
        DBG_BREAK;
        goto aborted;
    } 

    if (HR_ERROR(FltIsWhiteRoutine(
        RoutineBase,
        RoutineEnd,
        &IsWhiteRoutine))) {
        DBG_BREAK;
        goto aborted;
    }

    if (!IsWhiteRoutine) {
        pWhiteTableNewEntry = (g_FltWhiteTableData.pWhiteTable +
            g_FltWhiteTableData.EntryCount);
        pWhiteTableNewEntry->RoutineBase = RoutineBase;
        pWhiteTableNewEntry->RoutineEnd = RoutineEnd;

        KeMemoryBarrier();

        g_FltWhiteTableData.EntryCount++;
    }

    IsAborted = FALSE;

aborted:

    if (IsLocked) {
        _InterlockedExchange(
            (volatile LONG*)&g_FltWhiteTableData.WhiteTableLock,
            0);
        pHunterContext->HR_API.pKzLowerIrql(OldIrql);
    } 

    if (IsAborted) {
        return HR_ABORTED;
    }

    return HR_SUCCESS;
}

#define PG_CODE_PATTERN_COUNT 1
#define PG_CODE_FRAGMENTED_PATTERN_COUNT 1

#define PG_CODE_FRAGMENTED_PATTERN_INSTANCE \
((CONST VOID*)0xB6052E895E563F23UI64)

CONST VOID *g_pPgCodePattern[PG_CODE_PATTERN_COUNT] = {
    "\x01\x20\x00\x04\x80\x00\x10\x70"
};
CONST VOID *g_pPgCodeMask[PG_CODE_PATTERN_COUNT] = {
    _sx _sx _sx _sx _sx _sx _sx _sx
};
CONST VOID *g_pPgCodeFragmentedPattern[] = {
    "\x41\xC6\x00\x2E",
    "\x41\xC6\x00\x00\x48",
    "\x41\xC6\x00\x00\x31",
    "\x41\xC6\x00\x00\x11",
    PG_CODE_FRAGMENTED_PATTERN_INSTANCE
};
CONST VOID *g_pPgCodeFragmentedMask[] = {
    _sx _sx _s0 _sx,
    _sx _sx _s0 _s0 _sx,
    _sx _sx _s0 _s0 _sx,
    _sx _sx _s0 _s0 _sx,
    PG_CODE_FRAGMENTED_PATTERN_INSTANCE
};

FILTER_STATUS
FASTCALL
FltIsPatchGuardDpc(
    IN  UINT64 CallbackContext,
    IN  FILTER_CONTEXT *pFilterContext,
    OUT FILTER_DETECT_STATUS *pFilterDetectStatus
)
/*++
* Routine Description:
*
*     This routine performs a series of heuristic checks
*     on the specified DPC to determine whether it is
*     associated with PatchGuard.
*
* Arguments:
*
*     CallbackContext     - Supplies the callback context.
*
*     pFilterContext      - Supplies a pointer to the
*                           FILTER_CONTEXT structure.
*
*     pFilterDetectStatus - Supplies a pointer to a variable that
*                           receives the filter detect status.
*
* Return Value:
*
*     Filter status.
*
--*/
{
    UNREFERENCED_PARAMETER(CallbackContext);

    HR_CONTEXT *pHunterContext = pFilterContext->pHunterContext;

    KDPC *pDpc = (KDPC*)pFilterContext->AddArgument1;

    UINT64 ImageBase = 0;

    RUNTIME_FUNCTION *pFunctionEntry = NULL;

    UINT64 RoutineBase = 0;
    UINT64 RoutineEnd = 0;

    CONST VOID **pCurrentFragmentedPattern = NULL;
    CONST VOID **pCurrentFragmentedMask = NULL;

    BOOLEAN FragmentedPatternFound = TRUE;

    VOID *pPatternBase = NULL;

    BOOLEAN IsWhiteRoutine = FALSE;

    BOOLEAN IsAddressValid = FALSE;

    MMPTE_HARDWARE *pPte = NULL;

#if DBG
    UINT8 FilterDetectReason = 0;
#endif

    FILTER_DETECT_STATUS FilterDetectStatus = FILTER_DETECTED;

    if (!(pHunterContext->HR_API.pRtlPcToFileHeader(
        (VOID*)pDpc->DeferredRoutine,
        (VOID**)&ImageBase))) {
#if DBG
        FilterDetectReason = UnbackedRoutine;
#endif
        goto detected;
    }

    if (HR_ERROR(NtosLookupKernelFunctionEntry(
        (UINT64)pDpc->DeferredRoutine,
        &pFunctionEntry,
        pHunterContext))) {
        DBG_BREAK;
        goto aborted;
    } else if (!pFunctionEntry) {   
        goto none_detected;
    }

    RoutineBase = (ImageBase + pFunctionEntry->BeginAddress);
    RoutineEnd = (ImageBase + pFunctionEntry->EndAddress);

    if ((RoutineEnd - RoutineBase) < 8) {
        goto none_detected;
    }

    if (HR_ERROR(FltIsWhiteRoutine(
        RoutineBase,
        RoutineEnd,
        &IsWhiteRoutine))) {
        DBG_BREAK;
        goto aborted;
    } else if (IsWhiteRoutine) {    
        goto none_detected;
    }

    for (UINT8 i = 0; i < PG_CODE_PATTERN_COUNT; i++) {
        if (HR_ERROR(MemFindMemoryPattern(
            (UINT8*)RoutineBase,
            (RoutineEnd - RoutineBase),
            g_pPgCodePattern[i],
            g_pPgCodeMask[i],
            &pPatternBase))) {
            DBG_BREAK;
            goto aborted;
        } else if (pPatternBase) {
#if DBG
            FilterDetectReason = PgCodePattern;
#endif
            goto detected;
        }
    }

    pCurrentFragmentedPattern = g_pPgCodeFragmentedPattern;
    pCurrentFragmentedMask = g_pPgCodeFragmentedMask;
    if (*pCurrentFragmentedPattern !=
        PG_CODE_FRAGMENTED_PATTERN_INSTANCE) {
        for (UINT8 i = 0; i < PG_CODE_FRAGMENTED_PATTERN_COUNT; i++) {
            for (UINT8 i2 = 0; ; i2++) {
                if (pCurrentFragmentedPattern[i2] ==
                    PG_CODE_FRAGMENTED_PATTERN_INSTANCE) {
                    pCurrentFragmentedPattern += (i2 + 1);
                    pCurrentFragmentedMask += (i2 + 1);
                    break;
                }
                if (HR_ERROR(MemFindMemoryPattern(
                    (UINT8*)RoutineBase,
                    (RoutineEnd - RoutineBase),
                    pCurrentFragmentedPattern[i2],
                    pCurrentFragmentedMask[i2],
                    &pPatternBase))) {
                    DBG_BREAK;
                    goto aborted;
                } else if (!pPatternBase) {               
                    FragmentedPatternFound = FALSE;
                    while (TRUE) {
                        if (pCurrentFragmentedPattern[i2++] ==
                            PG_CODE_FRAGMENTED_PATTERN_INSTANCE) {
                            pCurrentFragmentedPattern += i2;
                            pCurrentFragmentedMask += i2;
                            break;
                        }
                    }
                    break;
                }
            }
            if (!FragmentedPatternFound) {
                FragmentedPatternFound = TRUE;
            } else {
#if DBG
                FilterDetectReason = PgCodePattern;
#endif
                goto detected;
            }
        }
    }

    if (pDpc->DeferredContext) {
        if (__popcnt64((UINT64)pDpc->DeferredContext) > 16) {
            if (!(IS_CANONICAL_SYSTEM_VA(pDpc->DeferredContext))) {
#if DBG
                FilterDetectReason = NoneCanonicalContext;
#endif
                goto detected;
            }

            if (HR_ERROR(MemIsSystemAddressValid(
                pDpc->DeferredContext,
                &IsAddressValid,
                pHunterContext))) {
                DBG_BREAK;
                goto aborted;
            } else if (!IsAddressValid) {
#if DBG
                FilterDetectReason = InvalidContext;
#endif
                goto detected;
            }

            if (HR_ERROR(MemGetPteAddressSafe(
                pDpc->DeferredContext,
                &pPte,
                pHunterContext))) {
                DBG_BREAK;
                goto aborted;
            } else if (!pPte) {            
#if DBG
                FilterDetectReason = InvalidContext;
#endif
                goto detected;
            } else if (IS_RWX_TABLE_ENTRY(pPte)) {            
#if DBG
                FilterDetectReason = RwxContext;
#endif
                goto detected;
            }
        }
    }

    if (HR_ERROR(FltRegisterWhiteRoutine(
        RoutineBase,
        RoutineEnd,
        pHunterContext))) {
        DBG_BREAK;
        goto aborted;
    }

none_detected:

    FilterDetectStatus = FILTER_NOT_DETECTED;

detected:

#if DBG

    if (FilterDetectStatus == FILTER_DETECTED) {
        DbgLog(DBG_WARNING_PREFIX
               "A potential PatchGuard DPC has been detected.\n");
        DbgLog(DBG_WARNING_PREFIX
               "Reason: 0x%02X\n",
               FilterDetectReason);
        DbgLog(DBG_WARNING_PREFIX
               "DPC: 0x%I64X\n",
               pDpc);
        DbgLog(DBG_WARNING_PREFIX
               "DPC.DeferredRoutine: 0x%I64X\n",
               pDpc->DeferredRoutine);
        DbgLog(DBG_WARNING_PREFIX
               "DPC.DeferredContext: 0x%I64X\n",
               pDpc->DeferredContext);
    }

#endif

    *pFilterDetectStatus = FilterDetectStatus;

    return FILTER_SUCCESS;

aborted:

    return FILTER_ABORTED;
}

FILTER_STATUS
FASTCALL
FltIsPatchGuardTimer2(
    IN  UINT64 CallbackContext,
    IN  FILTER_CONTEXT *pFilterContext,
    OUT FILTER_DETECT_STATUS *pFilterDetectStatus
)
/*++
* Routine Description:
*
*     This routine performs a series of heuristic checks
*     on the specified TIMER2 to determine whether it is
*     associated with PatchGuard.
*
* Arguments:
*
*     CallbackContext     - Supplies the callback context.
*
*     pFilterContext      - Supplies a pointer to the
*                           FILTER_CONTEXT structure.
*
*     pFilterDetectStatus - Supplies a pointer to a variable that
*                           receives the filter detect status.
*
* Return Value:
*
*     Filter status.
*
--*/
{
    UNREFERENCED_PARAMETER(CallbackContext);

    HR_CONTEXT *pHunterContext = pFilterContext->pHunterContext;

    UINT64 KiWaitAlways = *(pHunterContext->NTOS_ITEMS.pKiWaitAlways);
    UINT64 KiWaitNever = *(pHunterContext->NTOS_ITEMS.pKiWaitNever);

    KTIMER2 *pTimer2 = (KTIMER2*)pFilterContext->AddArgument1;
    VOID *pCallback =
        (VOID*)(KiWaitAlways ^
            _byteswap_uint64(
                (UINT64)pTimer2 ^ _rotl64(
                    KiWaitNever ^ (UINT64)pTimer2->pCallback,
                    (UINT8)KiWaitNever)));
    VOID* pCallbackContext =
        (VOID*)(KiWaitAlways ^
            _byteswap_uint64(
                (UINT64)pTimer2 ^ _rotl64(
                    KiWaitNever ^ (UINT64)pTimer2->pCallbackContext,
                    (UINT8)KiWaitNever)));

    UINT64 ImageBase = 0;

    RUNTIME_FUNCTION *pFunctionEntry = NULL;

    UINT64 RoutineBase = 0;
    UINT64 RoutineEnd = 0;

    CONST VOID **pCurrentFragmentedPattern = NULL;
    CONST VOID **pCurrentFragmentedMask = NULL;

    BOOLEAN FragmentedPatternFound = TRUE;

    VOID *pPatternBase = NULL;

    BOOLEAN IsWhiteRoutine = FALSE;

    BOOLEAN IsAddressValid = FALSE;

    MMPTE_HARDWARE *pPte = NULL;

#if DBG
    UINT8 FilterDetectReason = 0;
#endif

    FILTER_DETECT_STATUS FilterDetectStatus = FILTER_DETECTED;

    if (!pCallback) {
        goto none_detected;
    }

    if (!(pHunterContext->HR_API.pRtlPcToFileHeader(
        pCallback,
        (VOID**)&ImageBase))) {
#if DBG
        FilterDetectReason = UnbackedRoutine;
#endif
        goto detected;
    }

    if (HR_ERROR(NtosLookupKernelFunctionEntry(
        (UINT64)pCallback,
        &pFunctionEntry,
        pHunterContext))) {
        DBG_BREAK;
        goto aborted;
    } else if (!pFunctionEntry) {   
        goto none_detected;
    }

    RoutineBase = (ImageBase + pFunctionEntry->BeginAddress);
    RoutineEnd = (ImageBase + pFunctionEntry->EndAddress);

    if ((RoutineEnd - RoutineBase) < 8) {
        goto none_detected;
    }

    if (HR_ERROR(FltIsWhiteRoutine(
        RoutineBase,
        RoutineEnd,
        &IsWhiteRoutine))) {
        DBG_BREAK;
        goto aborted;
    } else if (IsWhiteRoutine) {   
        goto none_detected;
    }

    for (UINT8 i = 0; i < PG_CODE_PATTERN_COUNT; i++) {
        if (HR_ERROR(MemFindMemoryPattern(
            (UINT8*)RoutineBase,
            (RoutineEnd - RoutineBase),
            g_pPgCodePattern[i],
            g_pPgCodeMask[i],
            &pPatternBase))) {
            DBG_BREAK;
            goto aborted;
        } else if (pPatternBase) {
#if DBG
            FilterDetectReason = PgCodePattern;
#endif
            goto detected;
        }
    }

    pCurrentFragmentedPattern = g_pPgCodeFragmentedPattern;
    pCurrentFragmentedMask = g_pPgCodeFragmentedMask;
    if (*pCurrentFragmentedPattern !=
        PG_CODE_FRAGMENTED_PATTERN_INSTANCE) {
        for (UINT8 i = 0; i < PG_CODE_FRAGMENTED_PATTERN_COUNT; i++) {
            for (UINT8 i2 = 0; ; i2++) {
                if (pCurrentFragmentedPattern[i2] ==
                    PG_CODE_FRAGMENTED_PATTERN_INSTANCE) {
                    pCurrentFragmentedPattern += (i2 + 1);
                    pCurrentFragmentedMask += (i2 + 1);
                    break;
                }
                if (HR_ERROR(MemFindMemoryPattern(
                    (UINT8*)RoutineBase,
                    (RoutineEnd - RoutineBase),
                    pCurrentFragmentedPattern[i2],
                    pCurrentFragmentedMask[i2],
                    &pPatternBase))) {
                    DBG_BREAK;
                    goto aborted;
                } else if (!pPatternBase) {               
                    FragmentedPatternFound = FALSE;
                    while (TRUE) {
                        if (pCurrentFragmentedPattern[i2++] ==
                            PG_CODE_FRAGMENTED_PATTERN_INSTANCE) {
                            pCurrentFragmentedPattern += i2;
                            pCurrentFragmentedMask += i2;
                            break;
                        }
                    }
                    break;
                }
            }
            if (!FragmentedPatternFound) {
                FragmentedPatternFound = TRUE;
            } else {
#if DBG
                FilterDetectReason = PgCodePattern;
#endif
                goto detected;
            }
        }
    }

    if (pCallbackContext) {
        if (__popcnt64((UINT64)pCallbackContext) > 16) {
            if (!(IS_CANONICAL_SYSTEM_VA(pCallbackContext))) {
#if DBG
                FilterDetectReason = NoneCanonicalContext;
#endif
                goto detected;
            }

            if (HR_ERROR(MemIsSystemAddressValid(
                pCallbackContext,
                &IsAddressValid,
                pHunterContext))) {
                DBG_BREAK;
                goto aborted;
            } else if (!IsAddressValid) {
#if DBG
                FilterDetectReason = InvalidContext;
#endif
                goto detected;
            }

            if (HR_ERROR(MemGetPteAddressSafe(
                pCallbackContext,
                &pPte,
                pHunterContext))) {
                DBG_BREAK;
                goto aborted;
            } else if (!pPte) {            
#if DBG
                FilterDetectReason = InvalidContext;
#endif
                goto detected;
            } else if (IS_RWX_TABLE_ENTRY(pPte)) {           
#if DBG
                FilterDetectReason = RwxContext;
#endif
                goto detected;
            }
        }
    }

    if (HR_ERROR(FltRegisterWhiteRoutine(
        RoutineBase,
        RoutineEnd,
        pHunterContext))) {
        DBG_BREAK;
        goto aborted;
    }

none_detected:

    FilterDetectStatus = FILTER_NOT_DETECTED;

detected:

#if DBG

    if (FilterDetectStatus == FILTER_DETECTED) {
        DbgLog(DBG_WARNING_PREFIX
               "A potential PatchGuard TIMER2 has been detected.\n");
        DbgLog(DBG_WARNING_PREFIX
               "Reason: 0x%02X\n",
               FilterDetectReason);
        DbgLog(DBG_WARNING_PREFIX
               "TIMER2: 0x%I64X\n",
               pTimer2);
        DbgLog(DBG_WARNING_PREFIX
               "TIMER2.Callback: 0x%I64X\n",
               pCallback);
        DbgLog(DBG_WARNING_PREFIX
               "TIMER2.CallbackContext: 0x%I64X\n",
               pCallbackContext);
    }

#endif

    *pFilterDetectStatus = FilterDetectStatus;

    return FILTER_SUCCESS;

aborted:

    return FILTER_ABORTED;
}

FILTER_STATUS
FASTCALL
FltIsPatchGuardApc(
    IN  UINT64 CallbackContext,
    IN  FILTER_CONTEXT *pFilterContext,
    OUT FILTER_DETECT_STATUS *pFilterDetectStatus
)
/*++
* Routine Description:
*
*     This routine performs a series of heuristic checks
*     on the specified APC to determine whether it is
*     associated with PatchGuard.
*
* Arguments:
*
*     CallbackContext     - Supplies the callback context.
*
*     pFilterContext      - Supplies a pointer to the
*                           FILTER_CONTEXT structure.
*
*     pFilterDetectStatus - Supplies a pointer to a variable that
*                           receives the filter detect status.
*
* Return Value:
*
*     Filter status.
*
--*/
{
    UNREFERENCED_PARAMETER(CallbackContext);

    HR_CONTEXT *pHunterContext = pFilterContext->pHunterContext;

    KAPC2 *pApc = (KAPC2*)pFilterContext->AddArgument1;

    UINT64 ImageBase = 0;

    RUNTIME_FUNCTION *pFunctionEntry = NULL;

    UINT64 RoutineBase = 0;
    UINT64 RoutineEnd = 0;

    CONST VOID **pCurrentFragmentedPattern = NULL;
    CONST VOID **pCurrentFragmentedMask = NULL;

    BOOLEAN FragmentedPatternFound = TRUE;

    VOID *pPatternBase = NULL;

    BOOLEAN IsWhiteRoutine = FALSE;

    BOOLEAN IsAddressValid = FALSE;

    MMPTE_HARDWARE *pPte = NULL;

#if DBG
    UINT8 FilterDetectReason = 0;
#endif

    FILTER_DETECT_STATUS FilterDetectStatus = FILTER_DETECTED;

    if (!(pHunterContext->HR_API.pRtlPcToFileHeader(
        (VOID*)pApc->pKernelRoutine,
        (VOID**)&ImageBase))) {
#if DBG
        FilterDetectReason = UnbackedRoutine;
#endif
        goto detected;
    }

    if (HR_ERROR(NtosLookupKernelFunctionEntry(
        (UINT64)pApc->pKernelRoutine,
        &pFunctionEntry,
        pHunterContext))) {
        DBG_BREAK;
        goto aborted;
    } else if (!pFunctionEntry) {   
        goto none_detected;
    }

    RoutineBase = (ImageBase + pFunctionEntry->BeginAddress);
    RoutineEnd = (ImageBase + pFunctionEntry->EndAddress);

    if ((RoutineEnd - RoutineBase) < 8) {
        goto none_detected;
    }

    if (HR_ERROR(FltIsWhiteRoutine(
        RoutineBase,
        RoutineEnd,
        &IsWhiteRoutine))) {
        DBG_BREAK;
        goto aborted;
    } else if (IsWhiteRoutine) {   
        goto none_detected;
    }

    for (UINT8 i = 0; i < PG_CODE_PATTERN_COUNT; i++) {
        if (HR_ERROR(MemFindMemoryPattern(
            (UINT8*)RoutineBase,
            (RoutineEnd - RoutineBase),
            g_pPgCodePattern[i],
            g_pPgCodeMask[i],
            &pPatternBase))) {
            DBG_BREAK;
            goto aborted;
        } else if (pPatternBase) {
#if DBG
            FilterDetectReason = PgCodePattern;
#endif
            goto detected;
        }
    }

    pCurrentFragmentedPattern = g_pPgCodeFragmentedPattern;
    pCurrentFragmentedMask = g_pPgCodeFragmentedMask;
    if (*pCurrentFragmentedPattern !=
        PG_CODE_FRAGMENTED_PATTERN_INSTANCE) {
        for (UINT8 i = 0; i < PG_CODE_FRAGMENTED_PATTERN_COUNT; i++) {
            for (UINT8 i2 = 0; ; i2++) {
                if (pCurrentFragmentedPattern[i2] ==
                    PG_CODE_FRAGMENTED_PATTERN_INSTANCE) {
                    pCurrentFragmentedPattern += (i2 + 1);
                    pCurrentFragmentedMask += (i2 + 1);
                    break;
                }
                if (HR_ERROR(MemFindMemoryPattern(
                    (UINT8*)RoutineBase,
                    (RoutineEnd - RoutineBase),
                    pCurrentFragmentedPattern[i2],
                    pCurrentFragmentedMask[i2],
                    &pPatternBase))) {
                    DBG_BREAK;
                    goto aborted;
                } else if (!pPatternBase) {               
                    FragmentedPatternFound = FALSE;
                    while (TRUE) {
                        if (pCurrentFragmentedPattern[i2++] ==
                            PG_CODE_FRAGMENTED_PATTERN_INSTANCE) {
                            pCurrentFragmentedPattern += i2;
                            pCurrentFragmentedMask += i2;
                            break;
                        }
                    }
                    break;
                }
            }
            if (!FragmentedPatternFound) {
                FragmentedPatternFound = TRUE;
            } else {
#if DBG
                FilterDetectReason = PgCodePattern;
#endif
                goto detected;
            }
        }
    }

    if (pApc->pSystemArgument1) {
        if (__popcnt64((UINT64)pApc->pSystemArgument1) > 16) {
            if (!(IS_CANONICAL_SYSTEM_VA(pApc->pSystemArgument1))) {
#if DBG
                FilterDetectReason = NoneCanonicalContext;
#endif
                goto detected;
            }

            if (HR_ERROR(MemIsSystemAddressValid(
                pApc->pSystemArgument1,
                &IsAddressValid,
                pHunterContext))) {
                DBG_BREAK;
                goto aborted;
            } else if (!IsAddressValid) {
#if DBG
                FilterDetectReason = InvalidContext;
#endif
                goto detected;
            }

            if (HR_ERROR(MemGetPteAddressSafe(
                pApc->pSystemArgument1,
                &pPte,
                pHunterContext))) {
                DBG_BREAK;
                goto aborted;
            } else if (!pPte) {           
#if DBG
                FilterDetectReason = InvalidContext;
#endif
                goto detected;
            } else if (IS_RWX_TABLE_ENTRY(pPte)) {           
#if DBG
                FilterDetectReason = RwxContext;
#endif
                goto detected;
            }
        }
    }

    if (HR_ERROR(FltRegisterWhiteRoutine(
        RoutineBase,
        RoutineEnd,
        pHunterContext))) {
        DBG_BREAK;
        goto aborted;
    }

none_detected:

    FilterDetectStatus = FILTER_NOT_DETECTED;

detected:

#if DBG

    if (FilterDetectStatus == FILTER_DETECTED) {
        DbgLog(DBG_WARNING_PREFIX
               "A potential PatchGuard APC has been detected.\n");
        DbgLog(DBG_WARNING_PREFIX
               "Reason: 0x%02X\n",
               FilterDetectReason);
        DbgLog(DBG_WARNING_PREFIX
               "APC: 0x%I64X\n",
               pApc);
        DbgLog(DBG_WARNING_PREFIX
               "APC.KernelRoutine: 0x%I64X\n",
               pApc->pKernelRoutine);
        DbgLog(DBG_WARNING_PREFIX
               "APC.SystemArgument1: 0x%I64X\n",
               pApc->pSystemArgument1);
    }

#endif

    *pFilterDetectStatus = FilterDetectStatus;

    return FILTER_SUCCESS;

aborted:

    return FILTER_ABORTED;
}

FILTER_STATUS
FASTCALL
FltIsPatchGuardWorkItem(
    IN  UINT64 CallbackContext,
    IN  FILTER_CONTEXT *pFilterContext,
    OUT FILTER_DETECT_STATUS *pFilterDetectStatus
)
/*++
* Routine Description:
*
*     This routine performs a series of heuristic checks
*     on the specified WorkItem to determine whether it is
*     associated with PatchGuard.
*
* Arguments:
*
*     CallbackContext     - Supplies the callback context.
*
*     pFilterContext      - Supplies a pointer to the
*                           FILTER_CONTEXT structure.
*
*     pFilterDetectStatus - Supplies a pointer to a variable that
*                           receives the filter detect status.
*
* Return Value:
*
*     Filter status.
*
--*/
{
    UNREFERENCED_PARAMETER(CallbackContext);

    HR_CONTEXT *pHunterContext = pFilterContext->pHunterContext;

    WORK_QUEUE_ITEM *pWorkItem = (WORK_QUEUE_ITEM*)pFilterContext->AddArgument1;

    UINT64 ImageBase = 0;

    RUNTIME_FUNCTION *pFunctionEntry = NULL;

    UINT64 RoutineBase = 0;
    UINT64 RoutineEnd = 0;

    CONST VOID **pCurrentFragmentedPattern = NULL;
    CONST VOID **pCurrentFragmentedMask = NULL;

    BOOLEAN FragmentedPatternFound = TRUE;

    VOID *pPatternBase = NULL;

    BOOLEAN IsWhiteRoutine = FALSE;

    BOOLEAN IsAddressValid = FALSE;

    MMPTE_HARDWARE *pPte = NULL;

#if DBG
    UINT8 FilterDetectReason = 0;
#endif

    FILTER_DETECT_STATUS FilterDetectStatus = FILTER_DETECTED;

    if (!(pHunterContext->HR_API.pRtlPcToFileHeader(
        (VOID*)pWorkItem->WorkerRoutine,
        (VOID**)&ImageBase))) {
#if DBG
        FilterDetectReason = UnbackedRoutine;
#endif
        goto detected;
    }

    if (HR_ERROR(NtosLookupKernelFunctionEntry(
        (UINT64)pWorkItem->WorkerRoutine,
        &pFunctionEntry,
        pHunterContext))) {
        DBG_BREAK;
        goto aborted;
    } else if (!pFunctionEntry) {   
        goto none_detected;
    }

    RoutineBase = (ImageBase + pFunctionEntry->BeginAddress);
    RoutineEnd = (ImageBase + pFunctionEntry->EndAddress);

    if ((RoutineEnd - RoutineBase) < 8) {
        goto none_detected;
    }

    if (HR_ERROR(FltIsWhiteRoutine(
        RoutineBase,
        RoutineEnd,
        &IsWhiteRoutine))) {
        DBG_BREAK;
        goto aborted;
    } else if (IsWhiteRoutine) {   
        goto none_detected;
    }

    for (UINT8 i = 0; i < PG_CODE_PATTERN_COUNT; i++) {
        if (HR_ERROR(MemFindMemoryPattern(
            (UINT8*)RoutineBase,
            (RoutineEnd - RoutineBase),
            g_pPgCodePattern[i],
            g_pPgCodeMask[i],
            &pPatternBase))) {
            DBG_BREAK;
            goto aborted;
        } else if (pPatternBase) {
#if DBG
            FilterDetectReason = PgCodePattern;
#endif
            goto detected;
        }
    }

    pCurrentFragmentedPattern = g_pPgCodeFragmentedPattern;
    pCurrentFragmentedMask = g_pPgCodeFragmentedMask;
    if (*pCurrentFragmentedPattern !=
        PG_CODE_FRAGMENTED_PATTERN_INSTANCE) {
        for (UINT8 i = 0; i < PG_CODE_FRAGMENTED_PATTERN_COUNT; i++) {
            for (UINT8 i2 = 0; ; i2++) {
                if (pCurrentFragmentedPattern[i2] ==
                    PG_CODE_FRAGMENTED_PATTERN_INSTANCE) {
                    pCurrentFragmentedPattern += (i2 + 1);
                    pCurrentFragmentedMask += (i2 + 1);
                    break;
                }
                if (HR_ERROR(MemFindMemoryPattern(
                    (UINT8*)RoutineBase,
                    (RoutineEnd - RoutineBase),
                    pCurrentFragmentedPattern[i2],
                    pCurrentFragmentedMask[i2],
                    &pPatternBase))) {
                    DBG_BREAK;
                    goto aborted;
                } else if (!pPatternBase) {                
                    FragmentedPatternFound = FALSE;
                    while (TRUE) {
                        if (pCurrentFragmentedPattern[i2++] ==
                            PG_CODE_FRAGMENTED_PATTERN_INSTANCE) {
                            pCurrentFragmentedPattern += i2;
                            pCurrentFragmentedMask += i2;
                            break;
                        }
                    }
                    break;
                }
            }
            if (!FragmentedPatternFound) {
                FragmentedPatternFound = TRUE;
            } else {
#if DBG
                FilterDetectReason = PgCodePattern;
#endif
                goto detected;
            }
        }
    }

    if (pWorkItem->Parameter) {
        if (__popcnt64((UINT64)pWorkItem->Parameter) > 16) {
            if (!(IS_CANONICAL_SYSTEM_VA(pWorkItem->Parameter))) {
#if DBG
                FilterDetectReason = NoneCanonicalContext;
#endif
                goto detected;
            }

            if (HR_ERROR(MemIsSystemAddressValid(
                pWorkItem->Parameter,
                &IsAddressValid,
                pHunterContext))) {
                DBG_BREAK;
                goto aborted;
            } else if (!IsAddressValid) {
#if DBG
                FilterDetectReason = InvalidContext;
#endif
                goto detected;
            }

            if (HR_ERROR(MemGetPteAddressSafe(
                pWorkItem->Parameter,
                &pPte,
                pHunterContext))) {
                DBG_BREAK;
                goto aborted;
            } else if (!pPte) {           
#if DBG
                FilterDetectReason = InvalidContext;
#endif
                goto detected;
            } else if (IS_RWX_TABLE_ENTRY(pPte)) {           
#if DBG
                FilterDetectReason = RwxContext;
#endif
                goto detected;
            }
        }
    }

    if (HR_ERROR(FltRegisterWhiteRoutine(
        RoutineBase,
        RoutineEnd,
        pHunterContext))) {
        DBG_BREAK;
        goto aborted;
    }

none_detected:

    FilterDetectStatus = FILTER_NOT_DETECTED;

detected:

#if DBG

    if (FilterDetectStatus == FILTER_DETECTED) {
        DbgLog(DBG_WARNING_PREFIX
               "A potential PatchGuard WorkItem has been detected.\n");
        DbgLog(DBG_WARNING_PREFIX
               "Reason: 0x%02X\n",
               FilterDetectReason);
        DbgLog(DBG_WARNING_PREFIX
               "WorkItem: 0x%I64X\n",
               pWorkItem);
        DbgLog(DBG_WARNING_PREFIX
               "WorkItem.WorkerRoutine: 0x%I64X\n",
               pWorkItem->WorkerRoutine);
        DbgLog(DBG_WARNING_PREFIX
               "WorkItem.Parameter: 0x%I64X\n",
               pWorkItem->Parameter);
    }

#endif

    *pFilterDetectStatus = FilterDetectStatus;

    return FILTER_SUCCESS;

aborted:

    return FILTER_ABORTED;
}

FILTER_STATUS
FASTCALL
FltIsPatchGuardWaitThread(
    IN  UINT64 CallbackContext,
    IN  FILTER_CONTEXT *pFilterContext,
    OUT FILTER_DETECT_STATUS *pFilterDetectStatus
)
/*++
* Routine Description:
*
*     This routine performs a series of heuristic checks
*     on the specified WaitThread to determine whether it is
*     associated with PatchGuard.
*
* Arguments:
*
*     CallbackContext     - Supplies the callback context.
*
*     pFilterContext      - Supplies a pointer to the
*                           FILTER_CONTEXT structure.
*
*     pFilterDetectStatus - Supplies a pointer to a variable that
*                           receives the filter detect status.
*
* Return Value:
*
*     Filter status.
*
--*/
{
    UNREFERENCED_PARAMETER(CallbackContext);

    HR_CONTEXT *pHunterContext = pFilterContext->pHunterContext;

    UINT64 *pWaitThreadReturnAddress = (UINT64*)pFilterContext->AddArgument1;

    UINT64 ImageBase = 0;

    RUNTIME_FUNCTION *pFunctionEntry = NULL;

    UINT64 RoutineBase = 0;
    UINT64 RoutineEnd = 0;

    CONST VOID **pCurrentFragmentedPattern = NULL;
    CONST VOID **pCurrentFragmentedMask = NULL;

    BOOLEAN FragmentedPatternFound = TRUE;

    VOID *pPatternBase = NULL;

    BOOLEAN IsWhiteRoutine = FALSE;

#if DBG
    UINT8 FilterDetectReason = 0;
#endif

    FILTER_DETECT_STATUS FilterDetectStatus = FILTER_DETECTED;

    if (!(pHunterContext->HR_API.pRtlPcToFileHeader(
        (VOID*)pWaitThreadReturnAddress,
        (VOID**)&ImageBase))) {
#if DBG
        FilterDetectReason = UnbackedRoutine;
#endif
        goto detected;
    }

    if (HR_ERROR(NtosLookupKernelFunctionEntry(
        (UINT64)pWaitThreadReturnAddress,
        &pFunctionEntry,
        pHunterContext))) {
        DBG_BREAK;
        goto aborted;
    } else if (!pFunctionEntry) {    
        goto none_detected;
    }

    RoutineBase = (ImageBase + pFunctionEntry->BeginAddress);
    RoutineEnd = (ImageBase + pFunctionEntry->EndAddress);

    if ((RoutineEnd - RoutineBase) < 8) {
        goto none_detected;
    }

    if (HR_ERROR(FltIsWhiteRoutine(
        RoutineBase,
        RoutineEnd,
        &IsWhiteRoutine))) {
        DBG_BREAK;
        goto aborted;
    } else if (IsWhiteRoutine) { 
        goto none_detected;
    }

    for (UINT8 i = 0; i < PG_CODE_PATTERN_COUNT; i++) {
        if (HR_ERROR(MemFindMemoryPattern(
            (UINT8*)RoutineBase,
            (RoutineEnd - RoutineBase),
            g_pPgCodePattern[i],
            g_pPgCodeMask[i],
            &pPatternBase))) {
            DBG_BREAK;
            goto aborted;
        } else if (pPatternBase) {
#if DBG
            FilterDetectReason = PgCodePattern;
#endif
            goto detected;
        }
    }

    pCurrentFragmentedPattern = g_pPgCodeFragmentedPattern;
    pCurrentFragmentedMask = g_pPgCodeFragmentedMask;
    if (*pCurrentFragmentedPattern !=
        PG_CODE_FRAGMENTED_PATTERN_INSTANCE) {
        for (UINT8 i = 0; i < PG_CODE_FRAGMENTED_PATTERN_COUNT; i++) {
            for (UINT8 i2 = 0; ; i2++) {
                if (pCurrentFragmentedPattern[i2] ==
                    PG_CODE_FRAGMENTED_PATTERN_INSTANCE) {
                    pCurrentFragmentedPattern += (i2 + 1);
                    pCurrentFragmentedMask += (i2 + 1);
                    break;
                }
                if (HR_ERROR(MemFindMemoryPattern(
                    (UINT8*)RoutineBase,
                    (RoutineEnd - RoutineBase),
                    pCurrentFragmentedPattern[i2],
                    pCurrentFragmentedMask[i2],
                    &pPatternBase))) {
                    DBG_BREAK;
                    goto aborted;
                } else if (!pPatternBase) {               
                    FragmentedPatternFound = FALSE;
                    while (TRUE) {
                        if (pCurrentFragmentedPattern[i2++] ==
                            PG_CODE_FRAGMENTED_PATTERN_INSTANCE) {
                            pCurrentFragmentedPattern += i2;
                            pCurrentFragmentedMask += i2;
                            break;
                        }
                    }
                    break;
                }
            }
            if (!FragmentedPatternFound) {
                FragmentedPatternFound = TRUE;
            } else {
#if DBG
                FilterDetectReason = PgCodePattern;
#endif
                goto detected;
            }
        }
    }

    if (HR_ERROR(FltRegisterWhiteRoutine(
        RoutineBase,
        RoutineEnd,
        pHunterContext))) {
        DBG_BREAK;
        goto aborted;
    }

none_detected:

    FilterDetectStatus = FILTER_NOT_DETECTED;

detected:

#if DBG

    if (FilterDetectStatus == FILTER_DETECTED) {
        DbgLog(DBG_WARNING_PREFIX
               "A potential PatchGuard WaitThread has been detected.\n");
        DbgLog(DBG_WARNING_PREFIX
               "Reason: 0x%02X\n",
               FilterDetectReason);
        DbgLog(DBG_WARNING_PREFIX
               "WaitThread: 0x%I64X\n",
               KeGetCurrentThread());
    }

#endif

    *pFilterDetectStatus = FilterDetectStatus;

    return FILTER_SUCCESS;

aborted:

    return FILTER_ABORTED;
}

