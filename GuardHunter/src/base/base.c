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

#include "headers/base.h"

//
// Filter.
//

#define BS_MAIN_FILTER_CALLBACK_BASE_ID   0x5EA4D8B3UI32
#define BS_MAIN_FILTER_CALLBACK_ID_OFFSET 0x17A3UI16

// 01.
#define BS_MAIN_DPC_FILTER_CALLBACK_ID \
(BS_MAIN_FILTER_CALLBACK_BASE_ID + \
BS_MAIN_FILTER_CALLBACK_ID_OFFSET)

// 02.
#define BS_MAIN_TIMER_FILTER_CALLBACK_ID \
(BS_MAIN_FILTER_CALLBACK_BASE_ID + \
(BS_MAIN_FILTER_CALLBACK_ID_OFFSET * 2))

// 03.
#define BS_MAIN_TIMER2_FILTER_CALLBACK_ID \
(BS_MAIN_FILTER_CALLBACK_BASE_ID + \
(BS_MAIN_FILTER_CALLBACK_ID_OFFSET * 3))

// 04.
#define BS_MAIN_APC_FILTER_CALLBACK_ID \
(BS_MAIN_FILTER_CALLBACK_BASE_ID + \
(BS_MAIN_FILTER_CALLBACK_ID_OFFSET * 4))

// 05.
#define BS_MAIN_WORK_ITEM_FILTER_CALLBACK_ID \
(BS_MAIN_FILTER_CALLBACK_BASE_ID + \
(BS_MAIN_FILTER_CALLBACK_ID_OFFSET * 5))

// 06.
#define BS_MAIN_WAIT_THREAD_FILTER_CALLBACK_ID \
(BS_MAIN_FILTER_CALLBACK_BASE_ID + \
(BS_MAIN_FILTER_CALLBACK_ID_OFFSET * 6))

#define BS_MAIN_FILTER_CALLBACK_COUNT 6

//
// Hook.
//

#define BS_MAIN_HOOK_TARGET_BASE_ID   0xE0943BB3UI32
#define BS_MAIN_HOOK_TARGET_ID_OFFSET 0xF5E4UI16

// 01.
#define BS_MAIN_HOOK_TARGET_KICUSTOMRECURSEROUTINEX_ID \
(BS_MAIN_HOOK_TARGET_BASE_ID + \
BS_MAIN_HOOK_TARGET_ID_OFFSET)

// 02.
#define BS_MAIN_HOOK_TARGET_KIEXECUTEALLDPCS_ID \
(BS_MAIN_HOOK_TARGET_BASE_ID + \
(BS_MAIN_HOOK_TARGET_ID_OFFSET * 2))

// 03.
#define BS_MAIN_HOOK_TARGET_KIPROCESSEXPIREDTIMERLIST_ID \
(BS_MAIN_HOOK_TARGET_BASE_ID + \
(BS_MAIN_HOOK_TARGET_ID_OFFSET * 3))

// 04.
#define BS_MAIN_HOOK_TARGET_KIEXPIRETIMER2_ID \
(BS_MAIN_HOOK_TARGET_BASE_ID + \
(BS_MAIN_HOOK_TARGET_ID_OFFSET * 4))

// 05.
#define BS_MAIN_HOOK_TARGET_KIDELIVERAPC_ID \
(BS_MAIN_HOOK_TARGET_BASE_ID + \
(BS_MAIN_HOOK_TARGET_ID_OFFSET * 5))

#define BS_MAIN_HOOK_TARGET_COUNT 5

//
// Epi hook.
//

#define BS_MAIN_EPI_HOOK_TARGET_BASE_ID   0x0CCCAEF9UI32
#define BS_MAIN_EPI_HOOK_TARGET_ID_OFFSET 0x5AA6UI16

// 01.
#define BS_MAIN_HOOK_TARGET_EPI_KEREMOVEPRIQUEUEEPI_ID \
(BS_MAIN_EPI_HOOK_TARGET_BASE_ID + \
BS_MAIN_EPI_HOOK_TARGET_ID_OFFSET)

// 02.
#define BS_MAIN_HOOK_TARGET_EPI_KEWAITFORSINGLEOBJECTEPI_ID \
(BS_MAIN_EPI_HOOK_TARGET_BASE_ID + \
(BS_MAIN_EPI_HOOK_TARGET_ID_OFFSET * 2))

// 03.
#define BS_MAIN_HOOK_TARGET_EPI_KEWAITFORMULTIPLEOBJECTSEPI_ID \
(BS_MAIN_EPI_HOOK_TARGET_BASE_ID + \
(BS_MAIN_EPI_HOOK_TARGET_ID_OFFSET * 3))

// 04.
#define BS_MAIN_HOOK_TARGET_EPI_KEDELAYEXECUTIONTHREADEPI_ID \
(BS_MAIN_EPI_HOOK_TARGET_BASE_ID + \
(BS_MAIN_EPI_HOOK_TARGET_ID_OFFSET * 4))

#define BS_MAIN_EPI_HOOK_TARGET_COUNT 4

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
*     Pointer to the hunter export table, or NULL.
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

    const UINT8 RoutineExecuteFix[] = {
        //
        // mov al,al.
        //
        0x88, 0xC0,

        //
        // nop qword ptr [0xbee * 0xbee].
        //
        0x48, 0x0F, 0x1F, 0x04, 0x25, 0x44, 0x51, 0x8E, 0x00,

        //
        // ret.
        //
        0xC3
    };

    UINT32 Seed = 0;

    UINT32 FilterTypeSelectorId = 0;

    UINT32 BsMainFilterCallbackId = 0;

    UINT32 FilterTypeId = 0;
    VOID *pFilterCallbackRoutine = 0;

    FILTER_CALLBACK *pFilterCallback = { 0 };
    UINT32 CallbackId = 0;

    WRITE_PRCB_QWORD WritePrcbQword = { 0 };

    UINT8 *pStartPgRecurseRoutine = NULL;
    UINT8 *pCurrentPgRecurseRoutine = NULL;
    UINT8 *pNextPgRecurseRoutine = NULL;

    UINT32 BsMainHookTargetId = 0;
    VOID *pHookTarget = 0;
    VOID *pHookRoutine = 0;

    UINT32 BsMainEpiHookTargetId = 0;
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
    } else if (IsHyperProtectedCr0) {
        DbgLog(DBG_ABORTED_PREFIX
               "CR0 is hyper-protected via VMCB/VMCS bitmask.\n");
    } 

    DbgLog(DBG_SUCCESS_PREFIX
        "CR0 is not hyper-protected.\n");

    if (HR_ERROR(PeTruncateImageHeaders(
        pHunterContext->NTOS_PROCESS.HR_IMAGE.pImageBase))) {
        DbgLog(DBG_ABORTED_PREFIX
               "Module PE32+ headers truncation failed.\n");
        DBG_BREAK;
        goto aborted;
    }

    DbgLog(DBG_SUCCESS_PREFIX
        "Module PE32+ headers truncated successfully.\n");

    if (HR_ERROR(HrInitCriticalTable(pHunterContext))) {
        DbgLog(DBG_ABORTED_PREFIX
               "Critical table initialization failed.\n");
        DBG_BREAK;
        goto aborted;
    }

    DbgLog(DBG_SUCCESS_PREFIX
           "Critical table initialized successfully.\n");
 
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
                   "Filter selector id: 0x%I32X\n",
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
               "Filter callbacks page allocation failed.\n");
        DBG_BREAK;
        goto aborted;
    }

    DbgLog(DBG_SUCCESS_PREFIX
           "Filter callbacks page allocated successfully.\n");

    BsMainFilterCallbackId = BS_MAIN_FILTER_CALLBACK_BASE_ID;

    for (UINT8 i = 0; i < BS_MAIN_FILTER_CALLBACK_COUNT; i++) {
        BsMainFilterCallbackId += BS_MAIN_FILTER_CALLBACK_ID_OFFSET;
        switch (BsMainFilterCallbackId) {
        case BS_MAIN_DPC_FILTER_CALLBACK_ID:
            FilterTypeId = g_FltMgrDpcFilterTypeId;
            pFilterCallbackRoutine = (VOID*)&FltIsPatchGuardDpc;
            break;
        case BS_MAIN_TIMER_FILTER_CALLBACK_ID:
            FilterTypeId = g_FltMgrTimerFilterTypeId;
            pFilterCallbackRoutine = (VOID*)&FltIsPatchGuardDpc;
            break;
        case BS_MAIN_TIMER2_FILTER_CALLBACK_ID:
            FilterTypeId = g_FltMgrTimer2FilterTypeId;
            pFilterCallbackRoutine = (VOID*)&FltIsPatchGuardTimer2;
            break;
        case BS_MAIN_APC_FILTER_CALLBACK_ID:
            FilterTypeId = g_FltMgrApcFilterTypeId;
            pFilterCallbackRoutine = (VOID*)&FltIsPatchGuardApc;
            break;
        case BS_MAIN_WORK_ITEM_FILTER_CALLBACK_ID:
            FilterTypeId = g_FltMgrWorkItemFilterTypeId;
            pFilterCallbackRoutine = (VOID*)&FltIsPatchGuardWorkItem;
            break;
        case BS_MAIN_WAIT_THREAD_FILTER_CALLBACK_ID:
            FilterTypeId = g_FltMgrWaitThreadFilterTypeId;
            pFilterCallbackRoutine = (VOID*)&FltIsPatchGuardWaitThread;
            break;
        default:
            DbgLog(DBG_ABORTED_PREFIX
                   "Unknown filter callback id.\n");
            DbgLog(DBG_ABORTED_PREFIX
                   "Filter callback id: 0x%I32X\n",
                   BsMainFilterCallbackId);
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
                   "Filter callback id: 0x%I32X\n",
                   BsMainFilterCallbackId);
            DBG_BREAK;
            goto aborted;
        } else if (HR_ERROR(FltMgrRegisterFilterCallback(    
            FilterTypeId,
            pFilterCallback + i))) {
            DbgLog(DBG_ABORTED_PREFIX
                   "Filter callback registration failed.\n",
                   BsMainFilterCallbackId);
            DbgLog(DBG_ABORTED_PREFIX
                   "Filter callback id: 0x%I32X\n",
                   BsMainFilterCallbackId);
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
               "PRCB processing failed.\n");
        DBG_BREAK;
        goto aborted;
    }
    
    DbgLog(DBG_SUCCESS_PREFIX
           "PRCB processed successfully.\n");

    if (HR_ERROR(MemWriteRomData(
        (UINT8*)pHunterContext->NTOS_ROUTINES.pCcBcbProfiler,
        RoutineExecuteFix,
        sizeof(RoutineExecuteFix)))) {
        DbgLog(DBG_ABORTED_PREFIX
               "NTOS routine processing failed.\n");
        DbgLog(DBG_ABORTED_PREFIX
               "NTOS routine: 0x%I64X\n",
            pHunterContext->NTOS_ROUTINES.pCcBcbProfiler);
        DBG_BREAK;
        goto aborted;
    }

    if (HR_ERROR(MemWriteRomData(
        (UINT8*)pHunterContext->NTOS_ROUTINES.pCcBcbProfiler2,
        RoutineExecuteFix,
        sizeof(RoutineExecuteFix)))) {
        DbgLog(DBG_ABORTED_PREFIX
               "NTOS routine processing failed.\n");
        DbgLog(DBG_ABORTED_PREFIX
               "NTOS routine: 0x%I64X\n",
            pHunterContext->NTOS_ROUTINES.pCcBcbProfiler2);
        DBG_BREAK;
        goto aborted;
    }

    if (HR_ERROR(MemWriteRomData(
        (UINT8*)pHunterContext->NTOS_ROUTINES.pKiDispatchCallout,
        RoutineExecuteFix,
        sizeof(RoutineExecuteFix)))) {
        DbgLog(DBG_ABORTED_PREFIX
               "NTOS routine processing failed.\n");
        DbgLog(DBG_ABORTED_PREFIX
               "NTOS routine: 0x%I64X\n",
            pHunterContext->NTOS_ROUTINES.pKiDispatchCallout);
        DBG_BREAK;
        goto aborted;
    }

    if (HR_ERROR(MemWriteRomData(
        (UINT8*)pHunterContext->NTOS_ROUTINES.pKiSwInterruptDispatch,
        RoutineExecuteFix,
        sizeof(RoutineExecuteFix)))) {
        DbgLog(DBG_ABORTED_PREFIX
               "NTOS routine processing failed.\n");
        DbgLog(DBG_ABORTED_PREFIX
               "NTOS routine: 0x%I64X\n",
            pHunterContext->NTOS_ROUTINES.pKiSwInterruptDispatch);
        DBG_BREAK;
        goto aborted;
    }

    pHunterContext->
        NTOS_ITEMS.pKiBalanceSetManagerPeriodicDpc->DeferredRoutine =
        (PKDEFERRED_ROUTINE)pHunterContext->
        NTOS_ROUTINES.pKiBalanceSetManagerDeferredRoutine;

    pHunterContext->
        NTOS_ITEMS.pKiBalanceSetManagerPeriodicDpc->DeferredContext =
        (VOID*)pHunterContext->
        NTOS_ITEMS.pKiBalanceSetManagerPeriodicEvent;

    if (HR_ERROR(MemSetRomData(
        (UINT8*)pHunterContext->NTOS_ITEMS.pPgGlobalContext,
        0,
        8))) {
        DbgLog(DBG_ABORTED_PREFIX
               "NTOS item processing failed.\n");
        DbgLog(DBG_ABORTED_PREFIX
               "NTOS item: 0x%I64X\n",
               pHunterContext->NTOS_ITEMS.pPgGlobalContext);
        DBG_BREAK;
        goto aborted;
    }

    if (HR_ERROR(MemSetRomData(
        (UINT8*)pHunterContext->NTOS_ITEMS.pPgCheckTimerIDT,
        MAXUINT32,
        8))) {
        DbgLog(DBG_ABORTED_PREFIX
               "NTOS item processing failed.\n");
        DbgLog(DBG_ABORTED_PREFIX
               "NTOS item: 0x%I64X\n",
               pHunterContext->NTOS_ITEMS.pPgCheckTimerIDT);
        DBG_BREAK;
        goto aborted;
    }

    if (HR_ERROR(MemSetRomData(
        (UINT8*)pHunterContext->NTOS_ITEMS.pPgCheckTimerSSDT,
        MAXUINT32,
        8))) {
        DbgLog(DBG_ABORTED_PREFIX
               "NTOS item processing failed.\n");
        DbgLog(DBG_ABORTED_PREFIX
               "NTOS item: 0x%I64X\n",
               pHunterContext->NTOS_ITEMS.pPgCheckTimerSSDT);
        DBG_BREAK;
        goto aborted;
    }

    DbgLog(DBG_SUCCESS_PREFIX
           "NTOS routines and items processed successfully.\n");

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
                   "NTOS routine hook installation failed.\n");
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

    BsMainHookTargetId = BS_MAIN_HOOK_TARGET_BASE_ID;

    for (UINT8 i = 0; i < BS_MAIN_HOOK_TARGET_COUNT; i++) {
        BsMainHookTargetId += BS_MAIN_HOOK_TARGET_ID_OFFSET;
        switch (BsMainHookTargetId) {
        case BS_MAIN_HOOK_TARGET_KICUSTOMRECURSEROUTINEX_ID:
            continue;
        default:
            break;
        }
        switch (BsMainHookTargetId) {
        case BS_MAIN_HOOK_TARGET_KIEXECUTEALLDPCS_ID:
            pHookTarget =
                (VOID*)pHunterContext->
                NTOS_ROUTINES.pKiExecuteAllDpcs;
            pHookRoutine = (VOID*)&HkKiExecuteAllDpcs;
            break;
        case BS_MAIN_HOOK_TARGET_KIPROCESSEXPIREDTIMERLIST_ID:
            pHookTarget =
                (VOID*)pHunterContext->
                NTOS_ROUTINES.pKiProcessExpiredTimerList;
            pHookRoutine = (VOID*)&HkKiProcessExpiredTimerList;
            break;
        case BS_MAIN_HOOK_TARGET_KIEXPIRETIMER2_ID:
            pHookTarget =
                (VOID*)pHunterContext->
                NTOS_ROUTINES.pKiExpireTimer2;
            pHookRoutine = (VOID*)&HkKiExpireTimer2;
            break;
        case BS_MAIN_HOOK_TARGET_KIDELIVERAPC_ID:
            pHookTarget =
                (VOID*)pHunterContext->
                NTOS_ROUTINES.pKiDeliverApc;
            pHookRoutine = (VOID*)&HkKiDeliverApc;
            break;
        default:
            DbgLog(DBG_ABORTED_PREFIX
                   "Unknown hook target id.\n");
            DbgLog(DBG_ABORTED_PREFIX
                   "Hook target id: 0x%I32X\n",
                   BsMainHookTargetId);
            DBG_BREAK;
            goto aborted;
        }
        if (HR_ERROR(HkInstallRoutineHook(
            (VOID*)pHookTarget,
            FALSE,
            (VOID*)pHookRoutine,
            pHunterContext))) {
            DbgLog(DBG_ABORTED_PREFIX
                   "NTOS routine hook installation failed.\n");
            DbgLog(DBG_ABORTED_PREFIX
                   "NTOS routine: 0x%I64X\n",
                   pHookTarget);
            DBG_BREAK;
            goto aborted;
        }
    }

    DbgLog(DBG_SUCCESS_PREFIX
           "NTOS routine hooks installed successfully.\n");

    BsMainEpiHookTargetId = BS_MAIN_EPI_HOOK_TARGET_BASE_ID;

    for (UINT8 i = 0; i < BS_MAIN_EPI_HOOK_TARGET_COUNT; i++) {
        BsMainEpiHookTargetId += BS_MAIN_EPI_HOOK_TARGET_ID_OFFSET;
        switch (BsMainEpiHookTargetId) {
        case BS_MAIN_HOOK_TARGET_EPI_KEREMOVEPRIQUEUEEPI_ID:
            pEpiHookTarget =
                (UINT64*)&pHunterContext->
                NTOS_ROUTINES.pKeRemovePriQueueEpi[0];
            pEpiHookRoutine = (VOID*)&HkKeRemovePriQueueEpi;
            break;
        case BS_MAIN_HOOK_TARGET_EPI_KEWAITFORSINGLEOBJECTEPI_ID:
            pEpiHookTarget =
                (UINT64*)&pHunterContext->
                NTOS_ROUTINES.pKeWaitForSingleObjectEpi[0];
            pEpiHookRoutine = (VOID*)&HkWaitThreadEpi;
            break;
        case BS_MAIN_HOOK_TARGET_EPI_KEWAITFORMULTIPLEOBJECTSEPI_ID:
            pEpiHookTarget =
                (UINT64*)&pHunterContext->
                NTOS_ROUTINES.pKeWaitForMultipleObjectsEpi[0];
            pEpiHookRoutine = (VOID*)&HkWaitThreadEpi;
            break;
        case BS_MAIN_HOOK_TARGET_EPI_KEDELAYEXECUTIONTHREADEPI_ID:
            pEpiHookTarget =
                (UINT64*)&pHunterContext->
                NTOS_ROUTINES.pKeDelayExecutionThreadEpi[0];
            pEpiHookRoutine = (VOID*)&HkWaitThreadEpi;
            break;
        default:
            DbgLog(DBG_ABORTED_PREFIX
                   "Unknown hook target epi id.\n");
            DbgLog(DBG_ABORTED_PREFIX
                   "Hook target epi id: 0x%I32X\n",
                   BsMainEpiHookTargetId);
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
                           "NTOS routine epi hook installation failed.\n");
                    DbgLog(DBG_ABORTED_PREFIX
                           "NTOS routine: 0x%I64X\n",
                            pEpiHookTarget[i2]);
                    DBG_BREAK;
                    goto aborted;
                }
            }
        }
    }

    if (!(pHunterExportTable =
        (HR_EXPORT_TABLE*)
        pHunterContext->HR_API.pMmAllocateIndependentPagesEx(
            PAGE_SIZE,
            (UINT32)-1,
            NULL,
            0))) {
        DbgLog(DBG_ABORTED_PREFIX
               "Export table page allocation failed.\n");
        DBG_BREAK;
        goto aborted;
    }

    DbgLog(DBG_SUCCESS_PREFIX
           "Export table page allocated successfully.\n");
    DbgLog(DBG_SUCCESS_PREFIX
           "Export table: 0x%I64X\n",
           pHunterExportTable);

    pHunterExportTable->XorKey32 =
        (UINT32)__rdtsc();

    pHunterExportTable->FLTMGR.API.pFltMgrInitFilterCallback =
        (HR_STATUS (FASTCALL*) (FILTER_CALLBACK*, UINT32*, VOID*, UINT64))
        ((UINT64)&FltMgrInitFilterCallback ^
            (UINT64)pHunterExportTable->XorKey32);

    pHunterExportTable->FLTMGR.API.pFltMgrRegisterFilterCallback =
        (HR_STATUS(FASTCALL*) (UINT32, FILTER_CALLBACK*))
        ((UINT64)&FltMgrRegisterFilterCallback ^
            (UINT64)pHunterExportTable->XorKey32);

    pHunterExportTable->FLTMGR.API.pFltMgrDeregisterFilterCallback =
    (HR_STATUS(FASTCALL*) (UINT32, UINT32, BOOLEAN*))
        ((UINT64)&FltMgrDeregisterFilterCallback ^
            (UINT64)pHunterExportTable->XorKey32);

    pHunterExportTable->FLTMGR.DATA.DpcFilterTypeId =
        (g_FltMgrDpcFilterTypeId ^ pHunterExportTable->XorKey32);

    pHunterExportTable->FLTMGR.DATA.TimerFilterTypeId =
        (g_FltMgrTimerFilterTypeId ^ pHunterExportTable->XorKey32);

    pHunterExportTable->FLTMGR.DATA.Timer2FilterTypeId =
        (g_FltMgrTimer2FilterTypeId ^ pHunterExportTable->XorKey32);

    pHunterExportTable->FLTMGR.DATA.ApcFilterTypeId =
        (g_FltMgrApcFilterTypeId ^ pHunterExportTable->XorKey32);

    pHunterExportTable->FLTMGR.DATA.WorkItemFilterTypeId =
        (g_FltMgrWorkItemFilterTypeId ^ pHunterExportTable->XorKey32);

    pHunterExportTable->FLTMGR.DATA.WaitThreadFilterTypeId =
        (g_FltMgrWaitThreadFilterTypeId ^ pHunterExportTable->XorKey32);

    pHunterExportTable->XorKey32 =
        _byteswap_ulong(pHunterExportTable->XorKey32);

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
                HunterContextDesc.LowPaddingSize +
                HunterContextDesc.HighPaddingSize);
            pMmFreeIndependentPages(
                HunterContextDesc.pAllocBase,
                REQUIRED_NUMBER_OF_PAGES(
                HunterContextDesc.LowPaddingSize +
                HunterContextDesc.HighPaddingSize)
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
            BS_MAIN_INITIALIZATION_ABORTED,
            (VOID*)StackHighLimit,
            pKeBugCheckEx);
    }

    return pHunterExportTable;
}

