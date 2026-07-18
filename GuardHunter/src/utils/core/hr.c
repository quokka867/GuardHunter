/*++
* Module Name:
*
*     hr.c
*
* Abstract:
*
*     This module contains routines specific to Hunter.
*
* Author:
*
*     quokka867 (GitHub/Twitter).
*
--*/

#include "../headers/hr.h"

HR_STATUS
FASTCALL
HrCheckHunterContextIntegrity(
    IN  HR_CONTEXT *pHunterContext,
    OUT BOOLEAN *pCheckStatus
)
/*++
* Routine Description:
*
*     This routine checks the
*     integrity of a hunter context by comparing hashes.
*
* Arguments:
*
*     pHunterContext - Supplies a pointer to the
*                      HR_CONTEXT structure.
*
*     pCheckStatus   - Supplies a pointer to a variable that
*                      receives the check status.
*
* Return Value:
*
*     Internal status.
*
--*/
{
    UINT32 CurrentHash32 = 0;

    if (!pHunterContext || !pCheckStatus) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    if (HR_ERROR(CryCrc32DataHash(
        (UINT8*)&pHunterContext->ContextSeed,
        sizeof(HR_CONTEXT) - FIELD_OFFSET(HR_CONTEXT, ContextSeed),
        &CurrentHash32))) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    *pCheckStatus = (CurrentHash32 ==
        QUICK_XOR32(pHunterContext->ContextHash32));

    return HR_SUCCESS;
}

CRITICAL_TABLE *g_pCriticalTable = NULL;
volatile UINT32 g_CriticalTableLock = 0;
MMPTE_HARDWARE *g_pCriticalTablePte = NULL;
UINT64 g_CriticalTablePfn = 0;

HR_STATUS
FASTCALL
HrInitCriticalTable(
    IN HR_CONTEXT *pHunterContext
)
/*++
* Routine Description:
*
*     This routine initializes the critical table.
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
    KIRQL OldIrql = 0;

    UINT32 Seed = 0;

    UINT16 TableLowPaddingSize = 0;
    UINT16 TableHighPaddingSize = 0;
    UINT32 *pTableLowPaddingBase = NULL;
    UINT32 *pTableHighPaddingBase = NULL;

    CRITICAL_TABLE *pCriticalTable = NULL;
    MMPTE_HARDWARE *pCriticalTablePte = NULL;

    BOOLEAN IsAborted = TRUE;
    BOOLEAN IsLocked = FALSE;

    if (!pHunterContext) {
        DBG_BREAK;
        goto aborted;
    }

    OldIrql = pHunterContext->HR_API.pKzRaiseIrql(DISPATCH_LEVEL);

    while (_interlockedbittestandset(
        (volatile LONG*)&g_CriticalTableLock,
        0)) {
        _mm_pause();
    }

    IsLocked = TRUE;

    if (g_pCriticalTable) {
        DBG_BREAK;
        goto aborted;
    }

    TableLowPaddingSize =
        (UINT16)pHunterContext->HR_API.pRtlRandomEx(&Seed);
    TableHighPaddingSize =
        (UINT16)pHunterContext->HR_API.pRtlRandomEx(&Seed);

    TableLowPaddingSize =
        ((TableLowPaddingSize & 0x3FF) + 0x201) & ~0x07;
    TableHighPaddingSize =
        ((TableHighPaddingSize & 0x3FF) + 0x201) & ~0x07;

    if (!(pTableLowPaddingBase =
        pHunterContext->HR_API.pMmAllocateIndependentPagesEx(
        PAGE_SIZE,
        (UINT32)-1,
        NULL,
        0))) {
        DBG_BREAK;
        goto aborted;
    }

    pCriticalTable =
        (CRITICAL_TABLE*)
        (((UINT8*)pTableLowPaddingBase) + TableLowPaddingSize);

    pTableHighPaddingBase = (UINT32*)(pCriticalTable + 1);

    pCriticalTable->TableSeed =
        ((UINT64)pHunterContext->HR_API.pRtlRandomEx(&Seed)) << 32;

    pCriticalTable->TableSeed +=
        ((UINT64)pHunterContext->HR_API.pRtlRandomEx(&Seed));

    pCriticalTable->pMmAllocateIndependentPagesEx =
        pHunterContext->HR_API.pMmAllocateIndependentPagesEx;
    pCriticalTable->pMmFreeIndependentPages =
        pHunterContext->HR_API.pMmFreeIndependentPages;
    pCriticalTable->pIoGetStackLimits =
        pHunterContext->HR_API.pIoGetStackLimits;
    pCriticalTable->pRtlRandomEx =
        pHunterContext->HR_API.pRtlRandomEx;
    pCriticalTable->pKzRaiseIrql =
        pHunterContext->HR_API.pKzRaiseIrql;
    pCriticalTable->pKzLowerIrql =
        pHunterContext->HR_API.pKzLowerIrql;
    pCriticalTable->pKeBugCheckEx =
        pHunterContext->HR_API.pKeBugCheckEx;

    if (HR_ERROR(CryCrc32DataHash(
        (UINT8*)&pCriticalTable->TableSeed,
        sizeof(HR_CONTEXT) - FIELD_OFFSET(CRITICAL_TABLE, TableSeed),
        &pCriticalTable->TableHash32))) {
        DBG_BREAK;
        goto aborted;
    }

    pCriticalTable->TableHash32 =
        QUICK_XOR32(pCriticalTable->TableHash32);

    if (HR_ERROR(CryDiffusionDataFlow(
        (UINT8*)pCriticalTable,
        sizeof(CRITICAL_TABLE)))) {
        DBG_BREAK;
        goto aborted;
    }

    pHunterContext->HR_API.pRtlRandomEx(&Seed);
    if (HR_ERROR(CryFillBufferRandomDword(
        pTableLowPaddingBase,
        (TableLowPaddingSize / 4),
        Seed))) {
        DBG_BREAK;
        goto aborted;
    }

    pHunterContext->HR_API.pRtlRandomEx(&Seed);
    if (HR_ERROR(CryFillBufferRandomDword(
        pTableHighPaddingBase,
        (TableHighPaddingSize / 4),
        Seed))) {
        DBG_BREAK;
        goto aborted;
    }

    if (NT_ERROR(pHunterContext->HR_API.pMmSetPageProtection(
        pTableLowPaddingBase,
        PAGE_SIZE,
        PAGE_READONLY))) {
        DBG_BREAK;
        goto aborted;
    }

    if (HR_ERROR(MemGetPteAddressSafe(
        pTableLowPaddingBase,
        &pCriticalTablePte,
        pHunterContext))) {
        DBG_BREAK;
        goto aborted;
    } else if (!pCriticalTablePte) {
        DBG_BREAK;
        goto aborted;
    }

    g_pCriticalTable =
        (CRITICAL_TABLE*)QUICK_XOR64(pCriticalTable);
    g_pCriticalTablePte =
        (MMPTE_HARDWARE*)QUICK_XOR64(pCriticalTablePte);
    g_CriticalTablePfn =
        (UINT64)QUICK_XOR64(pCriticalTablePte->PageFrameNumber);

    pCriticalTablePte->Valid = 0;

    pCriticalTablePte->PageFrameNumber = 0;

    pHunterContext->HR_API.pKeIpiGenericCall(
        (KIPI_BROADCAST_WORKER*)&MemFlushPageTb,
        (UINT64)pTableLowPaddingBase);

    IsAborted = FALSE;

aborted:

    if (IsLocked) {
        _InterlockedExchange(
            (volatile LONG*)&g_CriticalTableLock,
            0);     
        pHunterContext->HR_API.pKzLowerIrql(OldIrql);
    }
    
    if (IsAborted) {
        if (pCriticalTable) {
            RtlSecureZeroMemory(pTableLowPaddingBase, PAGE_SIZE);
            pHunterContext->HR_API.pMmFreeIndependentPages(
                pTableLowPaddingBase,
                PAGE_SIZE);
        }
        return HR_ABORTED;
    }

    return HR_SUCCESS;
}

HR_STATUS
FASTCALL
HrReadCriticalTableQword(
    IN  UINT16 Offset,
    OUT UINT64 *pQword
)
/*++
* Routine Description:
*
*     This routine reads a qword from the critical table.
*
* Arguments:
*
*     Offset - Supplies the critical table offset
*
*     pQword - Supplies a pointer to a variable that
*              receives the qword.
*
* Return Value:
*
*     Internal status.
*
--*/
{
    CRITICAL_TABLE StackCriticalTable = { 0 };

    CRITICAL_TABLE *pCriticalTable = NULL;
    MMPTE_HARDWARE *pCriticalTablePte = NULL;
    UINT64 CriticalTablePfn = 0;

    BOOLEAN CurrentIF = FALSE;

    BOOLEAN IsLocked = FALSE;
    BOOLEAN IsAborted = TRUE;

    if (!pQword) {
        DBG_BREAK;
        goto aborted;
    }

    if ((CurrentIF = IS_INTERRUPTS_ENABLED)) {
        _disable();
    }

    while (_interlockedbittestandset(
        (volatile LONG*)&g_CriticalTableLock,
        0)) {
        _mm_pause();
    }

    IsLocked = TRUE;

    if (!g_pCriticalTable) {
        DBG_BREAK;
        goto aborted;
    }
    
    pCriticalTable = (CRITICAL_TABLE*)QUICK_XOR64(g_pCriticalTable);

    pCriticalTablePte =
        (MMPTE_HARDWARE*)QUICK_XOR64(g_pCriticalTablePte);

    CriticalTablePfn =
        (UINT64)QUICK_XOR64(g_CriticalTablePfn);
    
    pCriticalTablePte->Valid = 1;
    pCriticalTablePte->PageFrameNumber = CriticalTablePfn;

    __invlpg(pCriticalTable);

    memmove(
        &StackCriticalTable,
        (VOID*)pCriticalTable,
        sizeof(CRITICAL_TABLE));

    if (HR_ERROR(CryDeDiffusionDataFlow(
        (UINT8*)&StackCriticalTable,
        sizeof(CRITICAL_TABLE)))) {
        RtlSecureZeroMemory(&StackCriticalTable, sizeof(CRITICAL_TABLE));
        DBG_BREAK;
        goto aborted;
    }

    *pQword =
        *(UINT64*)(((UINT8*)&StackCriticalTable) + Offset);

    RtlSecureZeroMemory(&StackCriticalTable, sizeof(CRITICAL_TABLE));

    pCriticalTablePte->Valid = 0;
    pCriticalTablePte->PageFrameNumber = 0;

    __invlpg(pCriticalTable);

    IsAborted = FALSE;

aborted:

    if (IsLocked) {
        _InterlockedExchange(
            (volatile LONG*)&g_CriticalTableLock,
            0);
        if (CurrentIF) {
            _enable();
        }
    }

    if (IsAborted) {
        return HR_ABORTED;
    }

    return HR_SUCCESS;
}

HR_STATUS
FASTCALL
HrCheckCriticalTableIntegrity(
    IN  CRITICAL_TABLE *pCriticalTable,
    OUT BOOLEAN *pCheckStatus
)
/*++
* Routine Description:
*
*     This routine checks the
*     integrity of a critical table by comparing hashes.
*
* Arguments:
*
*     pCriticalTable - Supplies a pointer to the
*                      CRITICAL_TABLE structure.
* 
*     pCheckStatus   - Supplies a pointer to a variable that
*                      receives the check status.
*
* Return Value:
*
*     Internal status.
*
--*/
{
    UINT32 CurrentHash32 = 0;

    if (!pCriticalTable || !pCheckStatus) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    if (HR_ERROR(CryCrc32DataHash(
        (UINT8*)&pCriticalTable->TableSeed,
        sizeof(CRITICAL_TABLE) - FIELD_OFFSET(CRITICAL_TABLE, TableSeed),
        &CurrentHash32))) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    *pCheckStatus = (CurrentHash32 ==
        QUICK_XOR32(pCriticalTable->TableHash32));

    return HR_SUCCESS;
}

HR_STATUS
FASTCALL
HrGetAsciiStringLength(
    IN  CONST UINT8 *pString,
    OUT UINT64 *pLength,
    IN  UINT64 MaxCount
)
/*++
* Routine Description:
*
*     This routine computes the length of the
*     specified ASCII string.
*
* Arguments:
*
*     pString  - Supplies a pointer to the
*                string.
*
*     pLength  - Supplies a pointer to a variable that
*                receives the string length.
* 
*     MaxCount - Supplies the maximum count.
*
* Return Value:
*
*     Internal status.
*
--*/
{
    UINT64 StrLength = 0;

    if (!pString || !pLength) {
        return HR_ABORTED;
    }

    while ((StrLength < MaxCount) && pString[StrLength]) {
        StrLength++;
    }

    *pLength = StrLength;
    
    return HR_SUCCESS;
}

