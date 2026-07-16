/*++
* Module Name:
*
*     fltmgr.c
*
* Abstract:
*
*     This module contains routines for managing filters.
*
* Author:
*
*     quokka867 (GitHub/Twitter).
*
--*/

#include "../../headers/fltmgr.h"

FILTER_DATA g_FltMgrDpcFilter = { 0 };
UINT32      g_FltMgrDpcFilterTypeId = 0;

FILTER_DATA g_FltMgrTimerFilter = { 0 };
UINT32      g_FltMgrTimerFilterTypeId = 0;

FILTER_DATA g_FltMgrTimer2Filter = { 0 };
UINT32      g_FltMgrTimer2FilterTypeId = 0;

FILTER_DATA g_FltMgrApcFilter = { 0 };
UINT32      g_FltMgrApcFilterTypeId = 0;

FILTER_DATA g_FltMgrWorkItemFilter = { 0 };
UINT32      g_FltMgrWorkItemFilterTypeId = 0;

FILTER_DATA g_FltMgrWaitThreadFilter = { 0 };
UINT32      g_FltMgrWaitThreadFilterTypeId = 0;

HR_STATUS
FASTCALL
FltMgrInitFilterData(
    IN UINT32 TypeSelectorId,
    IN UINT32 TypeId
)
/*++
* Routine Description:
*
*     This routine initializes a filter of a
*     specific type using its selector ID.
*
* Arguments:
*
*     TypeSelectorId - Supplies the type selector ID.
* 
*     TypeId         - Supplies the type ID for the
*                      specified filter.
* 
* Return Value:
*
*     Internal status.
*
--*/
{
    FILTER_DATA *pSelectedFilter = NULL;

    switch (TypeSelectorId) {
    case DPC_FILTER_TYPE_SELECTOR_ID:
        g_FltMgrDpcFilter.TypeId = TypeId;
        g_FltMgrDpcFilterTypeId = TypeId;
        pSelectedFilter = &g_FltMgrDpcFilter;
        break;
    case TIMER_FILTER_TYPE_SELECTOR_ID:
        g_FltMgrTimerFilter.TypeId = TypeId;
        g_FltMgrTimerFilterTypeId = TypeId;
        pSelectedFilter = &g_FltMgrTimerFilter;
        break;
    case TIMER2_FILTER_TYPE_SELECTOR_ID:
        g_FltMgrTimer2Filter.TypeId = TypeId;
        g_FltMgrTimer2FilterTypeId = TypeId;
        pSelectedFilter = &g_FltMgrTimer2Filter;
        break;
    case APC_FILTER_TYPE_SELECTOR_ID:
        g_FltMgrApcFilter.TypeId = TypeId;
        g_FltMgrApcFilterTypeId = TypeId;
        pSelectedFilter = &g_FltMgrApcFilter;
        break;
    case WORK_ITEM_FILTER_TYPE_SELECTOR_ID:
        g_FltMgrWorkItemFilter.TypeId = TypeId;
        g_FltMgrWorkItemFilterTypeId = TypeId;
        pSelectedFilter = &g_FltMgrWorkItemFilter;
        break;
    case WAIT_THREAD_FILTER_TYPE_SELECTOR_ID:
        g_FltMgrWaitThreadFilter.TypeId = TypeId;
        g_FltMgrWaitThreadFilterTypeId = TypeId;
        pSelectedFilter = &g_FltMgrWaitThreadFilter;
        break;
    default:
        DBG_BREAK;
        return HR_ABORTED;
    }

    InitializeListHead(&pSelectedFilter->FilterListHead);

    return HR_SUCCESS;
}

HR_STATUS
FASTCALL
FltMgrGetFilterByTypeId(
    IN  UINT32 TypeId,
    OUT FILTER_DATA **pFilter
)
/*++
* Routine Description:
*
*     This routine returns the
*     filter base address using its type ID.
*
* Arguments:
*
*     TypeId  - Supplies the type ID for the
*               specified filter.
*
*     pFilter - Supplies a pointer to a variable that
*               receives a pointer to the
*               FILTER_DATA structure.
*
* Return Value:
*
*     Internal status.
*
--*/
{
    FILTER_DATA *pSelectedFilter = NULL;

    if (!pFilter) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    if (TypeId == g_FltMgrDpcFilterTypeId) {
        pSelectedFilter = &g_FltMgrDpcFilter;
    } else if (TypeId == g_FltMgrTimerFilterTypeId) {
        pSelectedFilter = &g_FltMgrTimerFilter;
    } else if (TypeId == g_FltMgrTimer2FilterTypeId) {
        pSelectedFilter = &g_FltMgrTimer2Filter;
    } else if (TypeId == g_FltMgrApcFilterTypeId) {
        pSelectedFilter = &g_FltMgrApcFilter;
    } else if (TypeId == g_FltMgrWorkItemFilterTypeId) {
        pSelectedFilter = &g_FltMgrWorkItemFilter;
    } else if (TypeId == g_FltMgrWaitThreadFilterTypeId) {
        pSelectedFilter = &g_FltMgrWaitThreadFilter;
    } else {
        DBG_BREAK;
        return HR_ABORTED;
    }

    *pFilter = pSelectedFilter;

    return HR_SUCCESS;
}

HR_STATUS
FASTCALL
FltMgrInitFilterCallback(
    OUT FILTER_CALLBACK *pFilterCallback,
    OUT UINT32 *pCallbackId,
    IN  VOID *pCallbackRoutine,
    IN  UINT64 CallbackContext
)
/*++
* Routine Description:
*
*     This routine initializes the filter callback.
*
* Arguments:
*
*     pFilterCallback  - Supplies a pointer to the
*                        FILTER_CALLBACK structure.
* 
*     pCallbackId      - Supplies a pointer to a variable that
*                        receives the callback ID.
*
*     pCallbackRoutine - Supplies a pointer to the
*                        callback routine.
* 
*     CallbackContext  - Supplies the callback context.
*
* Return Value:
*
*     Internal status.
*
--*/
{
    UINT32 (FASTCALL *pRtlRandomEx) (
        IN OUT UINT32 *pSeed
        ) = NULL;

    UINT64 CriticalTableQword = 0;

    UINT32 Seed = 0;

    if (!pFilterCallback || !pCallbackId || !pCallbackRoutine) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    if (!IS_CANONICAL_SYSTEM_VA(pFilterCallback) ||
        !IS_CANONICAL_SYSTEM_VA(pCallbackRoutine)) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    if (HR_ERROR(HrReadCriticalTableQword(
        FIELD_OFFSET(CRITICAL_TABLE, pRtlRandomEx),
        &CriticalTableQword))) {
        DBG_BREAK;
        return HR_ABORTED;
    } 
    
    pRtlRandomEx =
        (UINT32(FASTCALL*)(UINT32*))
        CriticalTableQword;

    pFilterCallback->CallbackId = pRtlRandomEx(&Seed);
    pFilterCallback->pCallback =
        (FILTER_STATUS(FASTCALL*)(UINT64, FILTER_CONTEXT*, FILTER_DETECT_STATUS*))
        pCallbackRoutine;
    pFilterCallback->CallbackContext = CallbackContext;

    *pCallbackId = pFilterCallback->CallbackId;

    return HR_SUCCESS;
}

HR_STATUS
FASTCALL
FltMgrRegisterFilterCallback(
    IN UINT32 TypeId,
    IN FILTER_CALLBACK *pFilterCallback
)
/*++
* Routine Description:
*
*     This routine registers a callback for the
*     specified filter using its type ID.
*
* Arguments:
*
*     TypeId          - Supplies the type ID for the
*                       specified filter.
*
*     pFilterCallback - Supplies a pointer to the
*                       FILTER_CALLBACK structure.
*
* Return Value:
*
*     Internal status.
*
--*/
{
    BOOLEAN CurrentIF = FALSE;

    FILTER_DATA *pSelectedFilter = NULL;

    if (!pFilterCallback) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    if (HR_ERROR(FltMgrGetFilterByTypeId(
        TypeId,
        &pSelectedFilter))) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    if ((CurrentIF = IS_INTERRUPTS_ENABLED)) {
        _disable();
    }

    while (_interlockedbittestandset(
        (volatile LONG*)&pSelectedFilter->FilterLock,
        0)) {
        _mm_pause();
    }

    InsertTailList(
        &pSelectedFilter->FilterListHead,
        &pFilterCallback->pFilterListEntry);

    pSelectedFilter->FilterListDepth++;

    _InterlockedExchange(
        (volatile LONG*)&pSelectedFilter->FilterLock,
        0);

    if (CurrentIF) {
        _enable();
    }

    return HR_SUCCESS;
}

HR_STATUS
FASTCALL
FltMgrDeregisterFilterCallback(
    IN  UINT32 TypeId,
    IN  UINT32 CallbackId,
    OUT BOOLEAN *pCallbackFound
)
/*++
* Routine Description:
*
*     This routine deregisters a callback for the
*     specified filter using its type ID.
*
* Arguments:
*
*     TypeId         - Supplies the type ID for the
*                      specified filter.
*
*     CallbackId     - Supplies the callback ID.
*
*     pCallbackFound - Supplies a pointer to a variable that
*                      receives the find status.
*
* Return Value:
*
*     Internal status.
*
--*/
{
    BOOLEAN CurrentIF = FALSE;

    FILTER_DATA *pSelectedFilter = NULL;
    LIST_ENTRY *pCurrentListEntry = NULL;
    FILTER_CALLBACK *pCurrentFilterCallback = NULL;

    BOOLEAN CallbackFound = FALSE;

    if (!pCallbackFound) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    if (HR_ERROR(FltMgrGetFilterByTypeId(
        TypeId,
        &pSelectedFilter))) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    if ((CurrentIF = IS_INTERRUPTS_ENABLED)) {
        _disable();
    }

    while (_interlockedbittestandset(
        (volatile LONG*)&pSelectedFilter->FilterLock,
        0)) {
        _mm_pause();
    }

    if (pSelectedFilter->FilterListDepth) {
        pCurrentListEntry = pSelectedFilter->FilterListHead.Flink;
        while (pCurrentListEntry != &pSelectedFilter->FilterListHead) {
            pCurrentFilterCallback =
                CONTAINING_RECORD(
                pCurrentListEntry,
                FILTER_CALLBACK,
                pFilterListEntry);   
            if (pCurrentFilterCallback->CallbackId == CallbackId) {
                RemoveEntryList(&pCurrentFilterCallback->pFilterListEntry);
                pSelectedFilter->FilterListDepth--;
                CallbackFound = TRUE;
                break;
            }
            pCurrentListEntry = pCurrentListEntry->Flink;
        }
    }

    _InterlockedExchange(
        (volatile LONG*)&pSelectedFilter->FilterLock,
        0);

    if (CurrentIF) {
        _enable();
    }

    *pCallbackFound = CallbackFound;

    return HR_SUCCESS;
}

HR_STATUS
FASTCALL
FltMgrExecuteFilterCallbacks(
    IN  UINT32 TypeId,
    IN  FILTER_CONTEXT *pFilterContext,
    OUT FILTER_DETECT_STATUS *pFilterDetectStatus,
    IN  HR_CONTEXT *pHunterContext
)
/*++
* Routine Description:
*
*     This routine executes all registered callbacks for the
*     specified filter atomically using its type ID.
*
* Arguments:
*
*     TypeId              - Supplies the type ID for the
*                           specified filter.
*
*     pFilterContext      - Supplies a pointer to the
*                           FILTER_CONTEXT structure.
*
*     pFilterDetectStatus - Supplies a pointer to a variable that
*                           receives the filter detect status.
* 
*     pHunterContext      - Supplies a pointer to the
*                           HR_CONTEXT structure.
*
* Return Value:
*
*     Internal status.
*
--*/
{
    KIRQL OldIrql = 0;

    FILTER_DATA *pSelectedFilter = NULL;
    LIST_ENTRY *pCurrentListEntry = NULL;
    FILTER_CALLBACK *pCurrentFilterCallback = NULL;
    FILTER_DETECT_STATUS FilterDetectStatus = FILTER_NOT_DETECTED;

    if (!pFilterContext || !pFilterDetectStatus || !pHunterContext) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    if (HR_ERROR(FltMgrGetFilterByTypeId(
        TypeId,
        &pSelectedFilter))) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    OldIrql = pHunterContext->HR_API.pKzRaiseIrql(DISPATCH_LEVEL);

    while (_interlockedbittestandset(
        (volatile LONG*)&pSelectedFilter->FilterLock,
        0)) {
        _mm_pause();
    }

    if (pSelectedFilter->FilterListDepth) {
        pCurrentListEntry = pSelectedFilter->FilterListHead.Flink;
        while (pCurrentListEntry != &pSelectedFilter->FilterListHead) {
            pCurrentFilterCallback =
                CONTAINING_RECORD(
                pCurrentListEntry,
                FILTER_CALLBACK,
                pFilterListEntry);
            pCurrentListEntry = pCurrentListEntry->Flink;
            if (FILTER_ERROR(pCurrentFilterCallback->
                pCallback(
                    pCurrentFilterCallback->CallbackContext,
                    pFilterContext,
                    &FilterDetectStatus))) {
                //
                // Log.
                //
            } else if (FilterDetectStatus == FILTER_DETECTED) {
                break;
            }    
        }
    }

    _InterlockedExchange(
        (volatile LONG*)&pSelectedFilter->FilterLock,
        0);

    pHunterContext->HR_API.pKzLowerIrql(OldIrql);

    *pFilterDetectStatus = FilterDetectStatus;

    return HR_SUCCESS;
}

