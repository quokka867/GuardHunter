/*++
* Module Name:
*
*     hkrtn.c
*
* Abstract:
*
*     This module contains hook routines.
*
* Author:
*
*     quokka867 (GitHub/Twitter).
*
--*/

#include "../../headers/hkrtn.h"

VOID
FASTCALL
HkKiExecuteAllDpcs(
    IN HOOK_ASM_STUB_CONTEXT *pHookAsmStubContext
)
/*++
* Routine Description:
*
*     This routine filters the
*     DPC Normal/Threaded queues of the
*     current PRCB.
*
* Arguments:
*
*     pHookAsmStubContext - Supplies a pointer to the
*                           HOOK_ASM_STUB_CONTEXT structure.
*
* Return Value:
*
*     None.
*
--*/
{
    VOID (FASTCALL *pIoGetStackLimits) (
        OUT VOID *pLowLimit,
        OUT VOID *pHighLimit
        ) = NULL;

    VOID *pKeBugCheckEx = NULL;

    HR_CONTEXT *pHunterContext =
        (HR_CONTEXT*)QUICK_XOR64(pHookAsmStubContext->pHunterContext);

    HR_CONTEXT HunterContext = { 0 };

    BOOLEAN CheckStatus = FALSE;

    BOOLEAN IsContextCorrupted = FALSE;
    BOOLEAN IsContextBadDecrypt = FALSE;
    BOOLEAN IsContextBadCheck = FALSE;

    UINT64 CriticalTableQword = 0;

    UINT64 StackLowLimit = 0;
    UINT64 StackHighLimit = 0;

    UINT8 *pCurrentPrcb = NULL;
         
    KDPC_DATA *pDpcData = NULL;

    SINGLE_LIST_ENTRY *pCurrentListHead = NULL;
    SINGLE_LIST_ENTRY *pPreviousListEntry = NULL;
    SINGLE_LIST_ENTRY *pCurrentListEntry = NULL;
    KDPC *pCurrentDpc = NULL;

    FILTER_CONTEXT FilterContext = { 0 };

    FILTER_DETECT_STATUS FilterDetectStatus = FILTER_NOT_DETECTED;

    if (!IS_CANONICAL_SYSTEM_VA(pHunterContext)) {
        IsContextCorrupted = TRUE;
        DBG_BREAK;
        goto aborted;
    }

    memmove(&HunterContext, pHunterContext, sizeof(HR_CONTEXT));

    if (HR_ERROR(CryDeDiffusionDataFlow(
        (UINT8*)&HunterContext,
        sizeof(HR_CONTEXT)))) {
        IsContextBadDecrypt = TRUE;
        DBG_BREAK;
        goto aborted;
    }

    if (HR_ERROR(HrCheckHunterContextIntegrity(
        &HunterContext,
        &CheckStatus))) {
        IsContextBadCheck = TRUE;
        DBG_BREAK;
        goto aborted;
    } else if (!CheckStatus) {
        IsContextCorrupted = TRUE;
        DBG_BREAK;
        goto aborted;
    }

    FilterContext.pHunterContext = &HunterContext;
    FilterContext.pHookAsmStubContext = pHookAsmStubContext;

    pCurrentPrcb = (UINT8*)__readgsqword(FIELD_OFFSET(KPCR, CurrentPrcb));

    pDpcData = (KDPC_DATA*)(pCurrentPrcb +
        HunterContext.NTOS_OFFSETS_TABLE.KPRCB_OFFSETS.OffsetDpcData);

    for (UINT8 i = 0; i < 2; i++) {
        if (pDpcData[i].DpcQueueDepth) {
            pCurrentListHead = &pDpcData[i].DpcList.ListHead;
            pPreviousListEntry = pCurrentListHead;
            pCurrentListEntry = pCurrentListHead->Next;
            while (pCurrentListEntry) {
                pCurrentDpc =
                    CONTAINING_RECORD(pCurrentListEntry, KDPC, DpcListEntry);
                FilterContext.AddArgument1 = (UINT64)pCurrentDpc;
                if (HR_ERROR(FltMgrExecuteFilterCallbacks(
                    g_FltMgrDpcFilterTypeId,
                    &FilterContext,
                    &FilterDetectStatus,
                    &HunterContext))) {
                    DBG_BREAK;
                    goto aborted;
                } else if (FilterDetectStatus == FILTER_DETECTED) {
                    while (_interlockedbittestandset64(
                        (volatile LONG64*)&pDpcData[i].DpcLock,
                        0)) {
                        _mm_pause();
                    }
                    if (!(--pDpcData[i].DpcQueueDepth)) {
                        pDpcData[i].DpcList.pLastEntry =
                            &pDpcData[i].DpcList.ListHead;
                    } else if (!pCurrentListEntry->Next) {
                        pDpcData[i].DpcList.pLastEntry =
                            pPreviousListEntry;
                    }
                    pPreviousListEntry->Next =
                        pCurrentListEntry->Next;
                    pCurrentListEntry =
                        pCurrentListEntry->Next;
                    _InterlockedExchange64(
                        (volatile LONG64*)&pDpcData[i].DpcLock,
                        0);
                    continue;
                }
                pPreviousListEntry = pCurrentListEntry;
                pCurrentListEntry = pCurrentListEntry->Next;
            }
        }
    }

    RtlSecureZeroMemory(&HunterContext, sizeof(HR_CONTEXT));

    return;
    
aborted:

    if (IsContextCorrupted || IsContextBadDecrypt || IsContextBadCheck) {
        RtlSecureZeroMemory(&HunterContext, sizeof(HR_CONTEXT));
        StackHighLimit = (UINT64)_AddressOfReturnAddress();
        pKeBugCheckEx = NULL;
        if (HR_ERROR(HrReadCriticalTableQword(
            FIELD_OFFSET(CRITICAL_TABLE, pIoGetStackLimits),
            &CriticalTableQword))) {
            DBG_BREAK;
            goto aborted2;
        } 
        pIoGetStackLimits =
            (VOID(FASTCALL*)(VOID*, VOID*))CriticalTableQword;
        if (HR_ERROR(HrReadCriticalTableQword(
            FIELD_OFFSET(CRITICAL_TABLE, pKeBugCheckEx),
            &CriticalTableQword))) {
            DBG_BREAK;
            goto aborted2;
        } 
        pKeBugCheckEx =
            (VOID*)CriticalTableQword;
    } else {
        pIoGetStackLimits = HunterContext.HR_API.pIoGetStackLimits;
        pKeBugCheckEx = (VOID*)HunterContext.HR_API.pKeBugCheckEx;
    }

    pIoGetStackLimits(
        &StackLowLimit,
        &StackHighLimit);

aborted2:

    HkCustomBugCheckAsm64(
        DPC_HR_CONTEXT_CORRUPTION,
        (VOID*)StackHighLimit,
        pKeBugCheckEx);
}

VOID
FASTCALL
HkKiProcessExpiredTimerList(
    IN HOOK_ASM_STUB_CONTEXT *pHookAsmStubContext
)
/*++
* Routine Description:
*
*     This routine filters expired KTIMERs for the
*     current PRCB.
*
* Arguments:
*
*     pHookAsmStubContext - Supplies a pointer to the
*                           HOOK_ASM_STUB_CONTEXT structure.
*
* Return Value:
*
*     None.
*
--*/
{
    VOID (FASTCALL *pIoGetStackLimits) (
        OUT VOID *pLowLimit,
        OUT VOID *pHighLimit
        ) = NULL;

    VOID *pKeBugCheckEx = NULL;

    HR_CONTEXT *pHunterContext =
        (HR_CONTEXT*)QUICK_XOR64(pHookAsmStubContext->pHunterContext);

    HR_CONTEXT HunterContext = { 0 };

    BOOLEAN CheckStatus = FALSE;

    BOOLEAN IsContextCorrupted = FALSE;
    BOOLEAN IsContextBadDecrypt = FALSE;
    BOOLEAN IsContextBadCheck = FALSE;

    UINT64 CriticalTableQword = 0;

    UINT64 StackLowLimit = 0;
    UINT64 StackHighLimit = 0;

    UINT64 KiWaitAlways = 0;
    UINT64 KiWaitNever = 0;
    
    KTIMER **pTimerTableBase = NULL;
    KTIMER *pCurrentTimer = NULL;
    UINT64 NumberOfTimers = 0;

    FILTER_CONTEXT FilterContext = { 0 };

    FILTER_DETECT_STATUS FilterDetectStatus = FILTER_NOT_DETECTED;

    if (!IS_CANONICAL_SYSTEM_VA(pHunterContext)) {
        IsContextCorrupted = TRUE;
        DBG_BREAK;
        goto aborted;
    }

    memmove(&HunterContext, pHunterContext, sizeof(HR_CONTEXT));

    if (HR_ERROR(CryDeDiffusionDataFlow(
        (UINT8*)&HunterContext,
        sizeof(HR_CONTEXT)))) {
        IsContextBadDecrypt = TRUE;
        DBG_BREAK;
        goto aborted;
    }

    if (HR_ERROR(HrCheckHunterContextIntegrity(
        &HunterContext,
        &CheckStatus))) {
        IsContextBadCheck = TRUE;
        DBG_BREAK;
        goto aborted;
    } else if (!CheckStatus) {   
        IsContextCorrupted = TRUE;
        DBG_BREAK;
        goto aborted;
    }

    FilterContext.pHunterContext = &HunterContext;
    FilterContext.pHookAsmStubContext = pHookAsmStubContext;

    KiWaitAlways = *(HunterContext.NTOS_ITEMS.pKiWaitAlways);
    KiWaitNever = *(HunterContext.NTOS_ITEMS.pKiWaitNever);

    pTimerTableBase = (KTIMER**)pHookAsmStubContext->R8;
    NumberOfTimers = pHookAsmStubContext->R9;

    for (UINT64 i = 0; i < NumberOfTimers; i++) {
        pCurrentTimer = (KTIMER*)_InterlockedAnd64(
            (volatile LONG64*)(pTimerTableBase + i),
            MAXUINT64);
        if (!pCurrentTimer) {
            continue;
        }
        while (_interlockedbittestandset(
            (volatile LONG*)pCurrentTimer,
            7)) {
            _mm_pause();
        }
        FilterContext.AddArgument1 = (UINT64)(KiWaitAlways ^
            _byteswap_uint64(
                (UINT64)pCurrentTimer ^ _rotl64(
                    KiWaitNever ^ (UINT64)pCurrentTimer->Dpc,
                    (UINT8)KiWaitNever)));
        if (FilterContext.AddArgument1) {
            if (HR_ERROR(FltMgrExecuteFilterCallbacks(
                g_FltMgrTimerFilterTypeId,
                &FilterContext,
                &FilterDetectStatus,
                &HunterContext))) {
                DBG_BREAK;
                goto aborted;
            } else if (FilterDetectStatus == FILTER_DETECTED) {      
                _InterlockedAnd64(
                    (volatile LONG64*)(pTimerTableBase + i),
                    0);
            }
        }
        _interlockedbittestandreset(
            (volatile LONG*)pCurrentTimer,
            7);
    }

    RtlSecureZeroMemory(&HunterContext, sizeof(HR_CONTEXT));

    return;

aborted:

    if (IsContextCorrupted || IsContextBadDecrypt || IsContextBadCheck) {
        RtlSecureZeroMemory(&HunterContext, sizeof(HR_CONTEXT));
        StackHighLimit = (UINT64)_AddressOfReturnAddress();
        pKeBugCheckEx = NULL;
        if (HR_ERROR(HrReadCriticalTableQword(
            FIELD_OFFSET(CRITICAL_TABLE, pIoGetStackLimits),
            &CriticalTableQword))) {
            DBG_BREAK;
            goto aborted2;
        } 
        pIoGetStackLimits =
            (VOID(FASTCALL*)(VOID*, VOID*))CriticalTableQword;
        if (HR_ERROR(HrReadCriticalTableQword(
            FIELD_OFFSET(CRITICAL_TABLE, pKeBugCheckEx),
            &CriticalTableQword))) {
            DBG_BREAK;
            goto aborted2;
        } 
        pKeBugCheckEx =
            (VOID*)CriticalTableQword;
    } else {
        pIoGetStackLimits = HunterContext.HR_API.pIoGetStackLimits;
        pKeBugCheckEx = (VOID*)HunterContext.HR_API.pKeBugCheckEx;
    }

    pIoGetStackLimits(
        &StackLowLimit,
        &StackHighLimit);

aborted2:

    HkCustomBugCheckAsm64(
        TIMER_HR_CONTEXT_CORRUPTION,
        (VOID*)StackHighLimit,
        pKeBugCheckEx);
}

VOID
FASTCALL
HkKiExpireTimer2(
    IN HOOK_ASM_STUB_CONTEXT *pHookAsmStubContext
)
/*++
* Routine Description:
*
*     This procedure filters the
*     expired KTIMER2 passed to the
*     KiExpireTimer2 routine.
*
* Arguments:
*
*     pHookAsmStubContext - Supplies a pointer to the
*                           HOOK_ASM_STUB_CONTEXT structure.
*
* Return Value:
*
*     None.
*
--*/
{
    VOID (FASTCALL *pIoGetStackLimits) (
        OUT VOID *pLowLimit,
        OUT VOID *pHighLimit
        ) = NULL;

    VOID *pKeBugCheckEx = NULL;

    HR_CONTEXT *pHunterContext =
        (HR_CONTEXT*)QUICK_XOR64(pHookAsmStubContext->pHunterContext);

    HR_CONTEXT HunterContext = { 0 };

    BOOLEAN CheckStatus = FALSE;

    BOOLEAN IsContextCorrupted = FALSE;
    BOOLEAN IsContextBadDecrypt = FALSE;
    BOOLEAN IsContextBadCheck = FALSE;

    UINT64 CriticalTableQword = 0;

    UINT64 StackLowLimit = 0;
    UINT64 StackHighLimit = 0;

    UINT64 KiWaitAlways = 0;
    UINT64 KiWaitNever = 0;

    KTIMER2 *pTimer2 = (KTIMER2*)pHookAsmStubContext->Rcx;

    FILTER_CONTEXT FilterContext = { 0 };

    FILTER_DETECT_STATUS FilterDetectStatus = FILTER_NOT_DETECTED;

    if (!IS_CANONICAL_SYSTEM_VA(pHunterContext)) {
        IsContextCorrupted = TRUE;
        DBG_BREAK;
        goto aborted;
    }

    memmove(&HunterContext, pHunterContext, sizeof(HR_CONTEXT));

    if (HR_ERROR(CryDeDiffusionDataFlow(
        (UINT8*)&HunterContext,
        sizeof(HR_CONTEXT)))) {
        IsContextBadDecrypt = TRUE;
        DBG_BREAK;
        goto aborted;
    }

    if (HR_ERROR(HrCheckHunterContextIntegrity(
        &HunterContext,
        &CheckStatus))) {
        IsContextBadCheck = TRUE;
        DBG_BREAK;
        goto aborted;
    } else if (!CheckStatus) {   
        IsContextCorrupted = TRUE;
        DBG_BREAK;
        goto aborted;
    }

    FilterContext.pHunterContext = &HunterContext;
    FilterContext.pHookAsmStubContext = pHookAsmStubContext;

    KiWaitAlways = *(HunterContext.NTOS_ITEMS.pKiWaitAlways);
    KiWaitNever = *(HunterContext.NTOS_ITEMS.pKiWaitNever);

    FilterContext.AddArgument1 = (UINT64)pTimer2;

    if (HR_ERROR(FltMgrExecuteFilterCallbacks(
        g_FltMgrTimer2FilterTypeId,
        &FilterContext,
        &FilterDetectStatus,
        &HunterContext))) {
        DBG_BREAK;
        goto aborted;
    } else if (FilterDetectStatus == FILTER_DETECTED) {
        pTimer2->pCallback = 
            (VOID(FASTCALL*)(VOID*, VOID*))
            (KiWaitNever ^
                _rotr64(((UINT64)pTimer2 ^
                        _byteswap_uint64(KiWaitAlways ^ 0)),
                        (UINT32)KiWaitNever));
        pTimer2->pCallbackContext = (VOID*)pTimer2->pCallback;
    }

    RtlSecureZeroMemory(&HunterContext, sizeof(HR_CONTEXT));

    return;

aborted:

    if (IsContextCorrupted || IsContextBadDecrypt || IsContextBadCheck) {
        RtlSecureZeroMemory(&HunterContext, sizeof(HR_CONTEXT));
        StackHighLimit = (UINT64)_AddressOfReturnAddress();
        pKeBugCheckEx = NULL;
        if (HR_ERROR(HrReadCriticalTableQword(
            FIELD_OFFSET(CRITICAL_TABLE, pIoGetStackLimits),
            &CriticalTableQword))) {
            DBG_BREAK;
            goto aborted2;
        } 
        pIoGetStackLimits =
            (VOID(FASTCALL*)(VOID*, VOID*))CriticalTableQword;
        if (HR_ERROR(HrReadCriticalTableQword(
            FIELD_OFFSET(CRITICAL_TABLE, pKeBugCheckEx),
            &CriticalTableQword))) {
            DBG_BREAK;
            goto aborted2;
        } 
        pKeBugCheckEx =
            (VOID*)CriticalTableQword;
    } else {
        pIoGetStackLimits = HunterContext.HR_API.pIoGetStackLimits;
        pKeBugCheckEx = (VOID*)HunterContext.HR_API.pKeBugCheckEx;
    }

    pIoGetStackLimits(
        &StackLowLimit,
        &StackHighLimit);

aborted2:

    HkCustomBugCheckAsm64(
        TIMER2_HR_CONTEXT_CORRUPTION,
        (VOID*)StackHighLimit,
        pKeBugCheckEx);
}

VOID
FASTCALL
HkKiDeliverApc(
    IN HOOK_ASM_STUB_CONTEXT *pHookAsmStubContext
)
/*++
* Routine Description:
* 
*     This routine filters the
*     APCs in the current thread kernel queue.
*
* Arguments:
*
*     pHookAsmStubContext - Supplies a pointer to the
*                           HOOK_ASM_STUB_CONTEXT structure.
*
* Return Value:
*
*     None.
*
--*/
{
    VOID (FASTCALL *pIoGetStackLimits) (
        OUT VOID *pLowLimit,
        OUT VOID *pHighLimit
        ) = NULL;

    VOID *pKeBugCheckEx = NULL;

    HR_CONTEXT *pHunterContext =
        (HR_CONTEXT*)QUICK_XOR64(pHookAsmStubContext->pHunterContext);

    HR_CONTEXT HunterContext = { 0 };

    BOOLEAN CheckStatus = FALSE;

    BOOLEAN IsContextCorrupted = FALSE;
    BOOLEAN IsContextBadDecrypt = FALSE;
    BOOLEAN IsContextBadCheck = FALSE;

    UINT64 CriticalTableQword = 0;

    UINT64 StackLowLimit = 0;
    UINT64 StackHighLimit = 0;

    UINT8 *pCurrentPrcb = NULL;

    UINT8 *pCurrentThread = NULL;

    UINT64 *pThreadLock = NULL;

    KAPC_STATE *pApcState = NULL;

    LIST_ENTRY *pCurrentListHead = NULL;
    LIST_ENTRY *pPreviousListEntry = NULL;
    LIST_ENTRY *pCurrentListEntry = NULL;
    LIST_ENTRY *pNextListEntry = NULL;

    KAPC2 *pCurrentApc = NULL;

    FILTER_CONTEXT FilterContext = { 0 };

    FILTER_DETECT_STATUS FilterDetectStatus = FILTER_NOT_DETECTED;

    if (!IS_CANONICAL_SYSTEM_VA(pHunterContext)) {
        IsContextCorrupted = TRUE;
        DBG_BREAK;
        goto aborted;
    }

    memmove(&HunterContext, pHunterContext, sizeof(HR_CONTEXT));

    if (HR_ERROR(CryDeDiffusionDataFlow(
        (UINT8*)&HunterContext,
        sizeof(HR_CONTEXT)))) {
        IsContextBadDecrypt = TRUE;
        DBG_BREAK;
        goto aborted;
    }

    if (HR_ERROR(HrCheckHunterContextIntegrity(
        &HunterContext,
        &CheckStatus))) {
        IsContextBadCheck = TRUE;
        DBG_BREAK;
        goto aborted;
    }
    else if (!CheckStatus) {
        IsContextCorrupted = TRUE;
        DBG_BREAK;
        goto aborted;
    }

    FilterContext.pHunterContext = &HunterContext;
    FilterContext.pHookAsmStubContext = pHookAsmStubContext;

    pCurrentPrcb = (UINT8*)__readgsqword(FIELD_OFFSET(KPCR, CurrentPrcb));

    pCurrentThread = *(VOID**)(pCurrentPrcb +
        HunterContext.
        NTOS_OFFSETS_TABLE.KPRCB_OFFSETS.OffsetCurrentThread);

    pThreadLock = (UINT64*)(pCurrentThread +
        HunterContext.
        NTOS_OFFSETS_TABLE.KTHREAD_OFFSETS.OffsetThreadLock);

    pApcState = (KAPC_STATE*)(pCurrentThread +
        HunterContext.
        NTOS_OFFSETS_TABLE.KTHREAD_OFFSETS.OffsetApcState);

    pCurrentListHead = &pApcState->ApcListHead[KernelMode];
    pPreviousListEntry = pCurrentListHead;
    pCurrentListEntry = pCurrentListHead->Flink;

    while (pCurrentListEntry != pCurrentListHead) {
        pNextListEntry = pCurrentListEntry->Flink;
        pCurrentApc =
            CONTAINING_RECORD(pCurrentListEntry, KAPC2, ApcListEntry);
        FilterContext.AddArgument1 = (UINT64)pCurrentApc;
        if (HR_ERROR(FltMgrExecuteFilterCallbacks(
            g_FltMgrApcFilterTypeId,
            &FilterContext,
            &FilterDetectStatus,
            &HunterContext))) {
            DBG_BREAK;
            goto aborted;
        } else if (FilterDetectStatus == FILTER_DETECTED) {      
            while (_interlockedbittestandset64(
                (volatile LONG64*)pThreadLock,
                0)) {
                _mm_pause();
            }
            pPreviousListEntry->Flink = pNextListEntry;
            pCurrentListEntry = pNextListEntry;
            pNextListEntry->Blink = pPreviousListEntry;
            _InterlockedExchange64(
                (volatile LONG64*)pThreadLock,
                0);
            continue;
        }
        pPreviousListEntry = pCurrentListEntry;
        pCurrentListEntry = pCurrentListEntry->Flink;
    }

    RtlSecureZeroMemory(&HunterContext, sizeof(HR_CONTEXT));

    return;

aborted:

    if (IsContextCorrupted || IsContextBadDecrypt || IsContextBadCheck) {
        RtlSecureZeroMemory(&HunterContext, sizeof(HR_CONTEXT));
        StackHighLimit = (UINT64)_AddressOfReturnAddress();
        pKeBugCheckEx = NULL;
        if (HR_ERROR(HrReadCriticalTableQword(
            FIELD_OFFSET(CRITICAL_TABLE, pIoGetStackLimits),
            &CriticalTableQword))) {
            DBG_BREAK;
            goto aborted2;
        } 
        pIoGetStackLimits =
            (VOID(FASTCALL*)(VOID*, VOID*))CriticalTableQword;
        if (HR_ERROR(HrReadCriticalTableQword(
            FIELD_OFFSET(CRITICAL_TABLE, pKeBugCheckEx),
            &CriticalTableQword))) {
            DBG_BREAK;
            goto aborted2;
        } 
        pKeBugCheckEx =
            (VOID*)CriticalTableQword;
    } else {
        pIoGetStackLimits = HunterContext.HR_API.pIoGetStackLimits;
        pKeBugCheckEx = (VOID*)HunterContext.HR_API.pKeBugCheckEx;
    }

    pIoGetStackLimits(
        &StackLowLimit,
        &StackHighLimit);

aborted2:

    HkCustomBugCheckAsm64(
        APC_HR_CONTEXT_CORRUPTION,
        (VOID*)StackHighLimit,
        pKeBugCheckEx);
}

VOID
FASTCALL
HkKeRemovePriQueueEpi(
    IN HOOK_ASM_STUB_CONTEXT *pHookAsmStubContext
)
/*++
* Routine Description:
*
*     This routine filters the
*     WorkItem removed from the
*     priority queue by the
*     KeRemovePriQueue routine.
*
* Arguments:
*
*     pHookAsmStubContext - Supplies a pointer to the
*                           HOOK_ASM_STUB_CONTEXT structure.
*
* Return Value:
*
*     None.
*
--*/
{
    VOID (FASTCALL *pIoGetStackLimits) (
        OUT VOID *pLowLimit,
        OUT VOID *pHighLimit
        ) = NULL;

    VOID *pKeBugCheckEx = NULL;

    HR_CONTEXT *pHunterContext =
        (HR_CONTEXT*)QUICK_XOR64(pHookAsmStubContext->pHunterContext);

    HR_CONTEXT HunterContext = { 0 };

    BOOLEAN CheckStatus = FALSE;

    BOOLEAN IsContextCorrupted = FALSE;
    BOOLEAN IsContextBadDecrypt = FALSE;
    BOOLEAN IsContextBadCheck = FALSE;

    UINT64 CriticalTableQword = 0;

    UINT64 StackLowLimit = 0;
    UINT64 StackHighLimit = 0;

    WORK_QUEUE_ITEM *pWorkItem = NULL;
        
    FILTER_CONTEXT FilterContext = { 0 };

    FILTER_DETECT_STATUS FilterDetectStatus = FILTER_NOT_DETECTED;

    if (!IS_CANONICAL_SYSTEM_VA(pHunterContext)) {
        IsContextCorrupted = TRUE;
        DBG_BREAK;
        goto aborted;
    }

    memmove(&HunterContext, pHunterContext, sizeof(HR_CONTEXT));

    if (HR_ERROR(CryDeDiffusionDataFlow(
        (UINT8*)&HunterContext,
        sizeof(HR_CONTEXT)))) {
        IsContextBadDecrypt = TRUE;
        DBG_BREAK;
        goto aborted;
    }

    if (HR_ERROR(HrCheckHunterContextIntegrity(
        &HunterContext,
        &CheckStatus))) {
        IsContextBadCheck = TRUE;
        DBG_BREAK;
        goto aborted;
    } else if (!CheckStatus) {   
        IsContextCorrupted = TRUE;
        DBG_BREAK;
        goto aborted;
    }

    FilterContext.pHunterContext = &HunterContext;
    FilterContext.pHookAsmStubContext = pHookAsmStubContext;

    pWorkItem = (WORK_QUEUE_ITEM*)*(pHookAsmStubContext->pRax);

    if (IS_CANONICAL_SYSTEM_VA(pWorkItem)) {
        FilterContext.AddArgument1 = (UINT64)pWorkItem;
        if (HR_ERROR(FltMgrExecuteFilterCallbacks(
            g_FltMgrWorkItemFilterTypeId,
            &FilterContext,
            &FilterDetectStatus,
            &HunterContext))) {
            DBG_BREAK;
            goto aborted;
        } else if (FilterDetectStatus == FILTER_DETECTED) {    
            *pHookAsmStubContext->pRax = STATUS_TIMEOUT;
        }
    }
    
    RtlSecureZeroMemory(&HunterContext, sizeof(HR_CONTEXT));

    return;

aborted:

    if (IsContextCorrupted || IsContextBadDecrypt || IsContextBadCheck) {
        RtlSecureZeroMemory(&HunterContext, sizeof(HR_CONTEXT));
        StackHighLimit = (UINT64)_AddressOfReturnAddress();
        pKeBugCheckEx = NULL;
        if (HR_ERROR(HrReadCriticalTableQword(
            FIELD_OFFSET(CRITICAL_TABLE, pIoGetStackLimits),
            &CriticalTableQword))) {
            DBG_BREAK;
            goto aborted2;
        } 
        pIoGetStackLimits =
            (VOID(FASTCALL*)(VOID*, VOID*))CriticalTableQword;
        if (HR_ERROR(HrReadCriticalTableQword(
            FIELD_OFFSET(CRITICAL_TABLE, pKeBugCheckEx),
            &CriticalTableQword))) {
            DBG_BREAK;
            goto aborted2;
        } 
        pKeBugCheckEx =
            (VOID*)CriticalTableQword;
    } else {
        pIoGetStackLimits = HunterContext.HR_API.pIoGetStackLimits;
        pKeBugCheckEx = (VOID*)HunterContext.HR_API.pKeBugCheckEx;
    }

    pIoGetStackLimits(
        &StackLowLimit,
        &StackHighLimit);

aborted2:

    HkCustomBugCheckAsm64(
        TIMER2_HR_CONTEXT_CORRUPTION,
        (VOID*)StackHighLimit,
        pKeBugCheckEx);
}

VOID
FASTCALL
HkWaitThreadEpi(
    IN HOOK_ASM_STUB_CONTEXT *pHookAsmStubContext
)
/*++
* Routine Description:
*
*     This routine filters threads that are 
*     attempting to exit the waiting state.
*
* Arguments:
*
*     pHookAsmStubContext - Supplies a pointer to the
*                           HOOK_ASM_STUB_CONTEXT structure.
*
* Return Value:
*
*     None.
*
--*/
{
    VOID (FASTCALL *pIoGetStackLimits) (
        OUT VOID *pLowLimit,
        OUT VOID *pHighLimit
        ) = NULL;

    VOID *pKeBugCheckEx = NULL;

    HR_CONTEXT *pHunterContext =
        (HR_CONTEXT*)QUICK_XOR64(pHookAsmStubContext->pHunterContext);

    HR_CONTEXT HunterContext = { 0 };

    BOOLEAN CheckStatus = FALSE;

    BOOLEAN IsContextCorrupted = FALSE;
    BOOLEAN IsContextBadDecrypt = FALSE;
    BOOLEAN IsContextBadCheck = FALSE;

    UINT64 CriticalTableQword = 0;

    UINT64 TrampolineBase = 0;

    UINT64 StackLowLimit = 0;
    UINT64 StackHighLimit = 0;

    CONTEXT VirtualContext = { 0 };
    UINT64 ImageBase = 0;
    RUNTIME_FUNCTION *pFunctionEntry = NULL;
    VOID *pHandlerData = NULL;
    UINT64 EstablisherFrame = 0;

    LARGE_INTEGER Interval = { 0 };

    NTSTATUS (FASTCALL *pKeDelayExecutionThread) (
        IN KPROCESSOR_MODE WaitMode,
        IN BOOLEAN Alertable,
        IN LARGE_INTEGER *pInterval
        ) = NULL;

    FILTER_CONTEXT FilterContext = { 0 };

    FILTER_DETECT_STATUS FilterDetectStatus = FILTER_NOT_DETECTED;

    if (!IS_CANONICAL_SYSTEM_VA(pHunterContext)) {
        IsContextCorrupted = TRUE;
        DBG_BREAK;
        goto aborted;
    }

    memmove(&HunterContext, pHunterContext, sizeof(HR_CONTEXT));

    if (HR_ERROR(CryDeDiffusionDataFlow(
        (UINT8*)&HunterContext,
        sizeof(HR_CONTEXT)))) {
        IsContextBadDecrypt = TRUE;
        DBG_BREAK;
        goto aborted;
    }

    if (HR_ERROR(HrCheckHunterContextIntegrity(
        &HunterContext,
        &CheckStatus))) {
        IsContextBadCheck = TRUE;
        DBG_BREAK;
        goto aborted;
    } else if (!CheckStatus) {   
        IsContextCorrupted = TRUE;
        DBG_BREAK;
        goto aborted;
    }

    TrampolineBase =
        QUICK_XOR64(pHookAsmStubContext->TrampolineBase);

    if (!(pFunctionEntry =
        HunterContext.HR_API.pRtlLookupFunctionEntry(
            TrampolineBase,
            &ImageBase,
            NULL))) {
        DBG_BREAK;
        goto aborted;
    }

    FilterContext.pHunterContext = &HunterContext;
    FilterContext.pHookAsmStubContext = pHookAsmStubContext;

    VirtualContext.ContextFlags = CONTEXT_CONTROL;
    VirtualContext.Rip          = TrampolineBase;
    VirtualContext.Rsp          = pHookAsmStubContext->Rsp;
    VirtualContext.Rbp          = pHookAsmStubContext->Rbp;

    HunterContext.HR_API.pIoGetStackLimits(
        &StackLowLimit,
        &StackHighLimit);
 
    if (!NT_SUCCESS(HunterContext.HR_API.pRtlVirtualUnwind2(
        0,
        ImageBase,
        TrampolineBase,
        pFunctionEntry,
        &VirtualContext,
        NULL,
        &pHandlerData,
        &EstablisherFrame,
        NULL,
        &StackLowLimit,
        &StackHighLimit,
        NULL,
        0))) {
        DBG_BREAK;
        goto aborted;
    }

    FilterContext.AddArgument1 = VirtualContext.Rip;

    if (HR_ERROR(FltMgrExecuteFilterCallbacks(
        g_FltMgrWaitThreadFilterTypeId,
        &FilterContext,
        &FilterDetectStatus,
        &HunterContext))) {
        DBG_BREAK;
        goto aborted;
    } else if (FilterDetectStatus == FILTER_DETECTED) {
        Interval.QuadPart = ~(MAXINT64 / 2);
        pKeDelayExecutionThread =
            (NTSTATUS(FASTCALL*)(KPROCESSOR_MODE, BOOLEAN, LARGE_INTEGER*))
            HunterContext.NTOS_ROUTINES.pKeDelayExecutionThread;
        if (!NT_SUCCESS(pKeDelayExecutionThread(
            KernelMode,
            FALSE,
            &Interval))) {
            DBG_BREAK;
            goto aborted;
        }
    }

    RtlSecureZeroMemory(&HunterContext, sizeof(HR_CONTEXT));

    return;

aborted:

    if (IsContextCorrupted || IsContextBadDecrypt || IsContextBadCheck) {
        RtlSecureZeroMemory(&HunterContext, sizeof(HR_CONTEXT));
        StackHighLimit = (UINT64)_AddressOfReturnAddress();
        pKeBugCheckEx = NULL;
        if (HR_ERROR(HrReadCriticalTableQword(
            FIELD_OFFSET(CRITICAL_TABLE, pIoGetStackLimits),
            &CriticalTableQword))) {
            DBG_BREAK;
            goto aborted2;
        } 
        pIoGetStackLimits =
            (VOID(FASTCALL*)(VOID*, VOID*))CriticalTableQword;
        if (HR_ERROR(HrReadCriticalTableQword(
            FIELD_OFFSET(CRITICAL_TABLE, pKeBugCheckEx),
            &CriticalTableQword))) {
            DBG_BREAK;
            goto aborted2;
        } 
        pKeBugCheckEx =
            (VOID*)CriticalTableQword;
    } else {
        pIoGetStackLimits = HunterContext.HR_API.pIoGetStackLimits;
        pKeBugCheckEx = (VOID*)HunterContext.HR_API.pKeBugCheckEx;
    }

    pIoGetStackLimits(
        &StackLowLimit,
        &StackHighLimit);

aborted2:

    HkCustomBugCheckAsm64(
        WAIT_THREAD_HR_CONTEXT_CORRUPTION,
        (VOID*)StackHighLimit,
        pKeBugCheckEx);
}

VOID
FASTCALL
HkKiCustomRecurseRoutineX(
    IN HOOK_ASM_STUB_CONTEXT *pHookAsmStubContext
)
/*++
* Routine Description:
*
*     This routine performs a partial emulation of
*     longjmp to return execution to a
*     legitimate kernel routine.
*
* Arguments:
*
*     pHookAsmStubContext - Supplies a pointer to the
*                           HOOK_ASM_STUB_CONTEXT structure.
*
* Return Value:
*
*     None.
*
--*/
{
    VOID (FASTCALL *pIoGetStackLimits) (
        OUT VOID *pLowLimit,
        OUT VOID *pHighLimit
        ) = NULL;

    VOID *pKeBugCheckEx = NULL;

    HR_CONTEXT *pHunterContext =
        (HR_CONTEXT*)QUICK_XOR64(pHookAsmStubContext->pHunterContext);

    HR_CONTEXT HunterContext = { 0 };

    BOOLEAN CheckStatus = FALSE;

    BOOLEAN IsContextCorrupted = FALSE;
    BOOLEAN IsContextBadDecrypt = FALSE;
    BOOLEAN IsContextBadCheck = FALSE;

    UINT64 CriticalTableQword = 0;

    UINT64 TrampolineBase = 0;

    UINT64 StackLowLimit = 0;
    UINT64 StackHighLimit = 0;

    CONTEXT VirtualContext = { 0 };
    UINT64 ImageBase = 0;
    RUNTIME_FUNCTION *pFunctionEntry = NULL;
    VOID *pHandlerData = NULL;
    UINT64 EstablisherFrame = 0;

    if (!IS_CANONICAL_SYSTEM_VA(pHunterContext)) {
        IsContextCorrupted = TRUE;
        DBG_BREAK;
        goto aborted;
    }

    memmove(&HunterContext, pHunterContext, sizeof(HR_CONTEXT));

    if (HR_ERROR(CryDeDiffusionDataFlow(
        (UINT8*)&HunterContext,
        sizeof(HR_CONTEXT)))) {
        IsContextBadDecrypt = TRUE;
        DBG_BREAK;
        goto aborted;
    }

    if (HR_ERROR(HrCheckHunterContextIntegrity(
        &HunterContext,
        &CheckStatus))) {
        IsContextBadCheck = TRUE;
        DBG_BREAK;
        goto aborted;
    } else if (!CheckStatus) {   
        IsContextCorrupted = TRUE;
        DBG_BREAK;
        goto aborted;
    }

    TrampolineBase =
        QUICK_XOR64(pHookAsmStubContext->TrampolineBase);

    VirtualContext.ContextFlags = CONTEXT_FULL;
    VirtualContext.Rsp          = pHookAsmStubContext->Rsp;
    VirtualContext.Rbp          = pHookAsmStubContext->Rbp;
    VirtualContext.Rip          = TrampolineBase;

    HunterContext.HR_API.pIoGetStackLimits(
        &StackLowLimit,
        &StackHighLimit);

    for (UINT8 i = 0; i < 3; i++) {
        if (!(pFunctionEntry =
            HunterContext.HR_API.pRtlLookupFunctionEntry(
                VirtualContext.Rip,
                &ImageBase,
                NULL))) {
            DBG_BREAK;
            goto aborted;
        }
        if (NT_ERROR(HunterContext.HR_API.pRtlVirtualUnwind2(
            0,
            ImageBase,
            VirtualContext.Rip,
            pFunctionEntry,
            &VirtualContext,
            NULL,
            &pHandlerData,
            &EstablisherFrame,
            NULL,
            &StackLowLimit,
            &StackHighLimit,
            NULL,
            0))) {
            DBG_BREAK;
            goto aborted;
        }
    }

    RtlSecureZeroMemory(&HunterContext, sizeof(HR_CONTEXT));

    DbgLog(DBG_WARNING_PREFIX
           "An attempt to recursively invoke "
           "KiCustomRecurseRoutineX has been detected.\n");
    DbgLog(DBG_WARNING_PREFIX
           "Longjmp partial emulation RIP: 0x%I64X\n",
           VirtualContext.Rip);

    HkSetRspSafeAsm64(
        (VirtualContext.Rsp - 8),
        &VirtualContext);

    return;

aborted:

    if (IsContextCorrupted || IsContextBadDecrypt || IsContextBadCheck) {
        RtlSecureZeroMemory(&HunterContext, sizeof(HR_CONTEXT));
        StackHighLimit = (UINT64)_AddressOfReturnAddress();
        pKeBugCheckEx = NULL;
        if (HR_ERROR(HrReadCriticalTableQword(
            FIELD_OFFSET(CRITICAL_TABLE, pIoGetStackLimits),
            &CriticalTableQword))) {
            DBG_BREAK;
            goto aborted2;
        } 
        pIoGetStackLimits =
            (VOID(FASTCALL*)(VOID*, VOID*))CriticalTableQword;
        if (HR_ERROR(HrReadCriticalTableQword(
            FIELD_OFFSET(CRITICAL_TABLE, pKeBugCheckEx),
            &CriticalTableQword))) {
            DBG_BREAK;
            goto aborted2;
        } 
        pKeBugCheckEx =
            (VOID*)CriticalTableQword;
    } else {
        pIoGetStackLimits = HunterContext.HR_API.pIoGetStackLimits;
        pKeBugCheckEx = (VOID*)HunterContext.HR_API.pKeBugCheckEx;
    }

    pIoGetStackLimits(
        &StackLowLimit,
        &StackHighLimit);

aborted2:

    HkCustomBugCheckAsm64(
        PG_RECURSE_ROUTINE_HR_CONTEXT_CORRUPTION,
        (VOID*)StackHighLimit,
        pKeBugCheckEx);
}

