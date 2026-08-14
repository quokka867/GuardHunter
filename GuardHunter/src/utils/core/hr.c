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

HR_STATUS
FASTCALL
HrInitHunterExportTable(
    IN  HR_CONTEXT *pHunterContext,
    OUT HR_EXPORT_TABLE **pHunterExportTable
)
/*++
* Routine Description:
*
*     This routine initializes
*     the hunter export table.
*
* Arguments:
*
*     pHunterContext     - Supplies a pointer to the
*                          HR_CONTEXT structure.
* 
*     pHunterExportTable - Supplies a pointer to the
*                          HR_EXPORT_TABLE structure.
*
* Return Value:
*
*     Internal status.
*
--*/
{
    UINT32 Seed = 0;

    UINT16 TableLowPaddingSize = 0;
    UINT16 TableHighPaddingSize = 0;
    UINT32 *pTableLowPaddingBase = NULL;
    UINT32 *pTableHighPaddingBase = NULL;

    UINT64 ExportTableDataBase = 0;
    UINT64 ExportTableDataEnd = 0;
    UINT16 ExportTableDataQwordCount = 0;
    UINT64 ExportTableDataKey = 0;
    DECRYPT_ASM_STUB_INFO DecryptAsmStubInfo;
    UINT8 StubLowPaddingSize = 0;

    HR_EXPORT_TABLE *pHrExportTable = NULL;

    BOOLEAN IsAborted = TRUE;

    if (!pHunterContext || !pHunterExportTable) {
        DBG_BREAK;
        goto aborted;
    }

    TableLowPaddingSize =
        (UINT16)pHunterContext->HR_API.pRtlRandomEx(&Seed);
    TableHighPaddingSize =
        (UINT16)pHunterContext->HR_API.pRtlRandomEx(&Seed);

    TableLowPaddingSize =
        ((TableLowPaddingSize & 0x3FF) + 0x401) & ~0x0F;
    TableHighPaddingSize =
        ((TableHighPaddingSize & 0x3FF) + 0x401) & ~0x0F;

    if (!(pTableLowPaddingBase =
        (UINT32*)
        pHunterContext->HR_API.pMmAllocateIndependentPagesEx(
            REQUIRED_NUMBER_OF_PAGES(
            (TableLowPaddingSize +
            sizeof(HR_EXPORT_TABLE) +
            TableHighPaddingSize))
            << PAGE_SHIFT,
            (UINT32)-1,
            NULL,
            0))) {
        DBG_BREAK;
        goto aborted;    
    }

    pHrExportTable =
        (HR_EXPORT_TABLE*)(((UINT8*)pTableLowPaddingBase) +
            TableLowPaddingSize);

    pTableHighPaddingBase = (UINT32*)(pHrExportTable + 1);

    if (NT_ERROR(pHunterContext->HR_API.pMmSetPageProtection(
        pTableLowPaddingBase,
        REQUIRED_NUMBER_OF_PAGES(
        (TableLowPaddingSize +
        sizeof(HR_EXPORT_TABLE) +
        TableHighPaddingSize))
        << PAGE_SHIFT,
        PAGE_EXECUTE_READWRITE))) {
        DBG_BREAK;
        goto aborted;
    }

    if (HR_ERROR(HrGetDecryptAsmStubInfoAsm64(
        &DecryptAsmStubInfo))) {
        DBG_BREAK;
        goto aborted;
    }

    if (DecryptAsmStubInfo.StubSize > DECRYPT_ASM_STUB_MAXSIZE) {
        DBG_BREAK;
        goto aborted;
    }

    pHunterContext->HR_API.pRtlRandomEx(&Seed);
    if (HR_ERROR(CryFillBufferRandomDword(
        (UINT32*)pHrExportTable->DecryptAsmStub,
        DECRYPT_ASM_STUB_MAXSIZE / 4,
        Seed))) {
        DBG_BREAK;
        goto aborted;
    }

    memmove(
        pHrExportTable->DecryptAsmStub,
        DecryptAsmStubInfo.pStubBase,
        DecryptAsmStubInfo.StubSize);

    StubLowPaddingSize =
        8 - (DecryptAsmStubInfo.StubSize % 8);

    ExportTableDataBase = ((UINT64)pHrExportTable->DecryptAsmStub) +
        (DecryptAsmStubInfo.StubSize + StubLowPaddingSize);

    ExportTableDataEnd = (UINT64)(pHrExportTable + 1);

    ExportTableDataQwordCount =
        (UINT16)((ExportTableDataEnd - ExportTableDataBase) / 8);

    *(UINT64*)(((UINT8*)pHrExportTable->DecryptAsmStub) +
        DecryptAsmStubInfo.OffsetDataPtr) =
        ExportTableDataBase;

    *(UINT32*)(((UINT8*)pHrExportTable->DecryptAsmStub) +
        DecryptAsmStubInfo.OffsetQwordCount) =
        ExportTableDataQwordCount;

    *(UINT64*)(((UINT8*)pHrExportTable->DecryptAsmStub) +
        DecryptAsmStubInfo.OffsetXorKey) =
        _byteswap_uint64(ExportTableDataKey = QUICK_XOR64(__rdtsc()));

    pHrExportTable->pDecryptStub =
        (VOID(FASTCALL*)(VOID))pHrExportTable->DecryptAsmStub;

    pHrExportTable->FLTMGR.API.pFltMgrInitFilterCallback =      
        &FltMgrInitFilterCallback;
    pHrExportTable->FLTMGR.API.pFltMgrRegisterFilterCallback =
        &FltMgrRegisterFilterCallback;
    pHrExportTable->FLTMGR.API.pFltMgrDeregisterFilterCallback =
        &FltMgrDeregisterFilterCallback;
    
    pHrExportTable->FLTMGR.DATA.DpcFilterTypeId =
        g_FltMgrDpcFilterTypeId;  
    pHrExportTable->FLTMGR.DATA.TimerFilterTypeId =
        g_FltMgrTimerFilterTypeId;  
    pHrExportTable->FLTMGR.DATA.Timer2FilterTypeId =
        g_FltMgrTimer2FilterTypeId; 
    pHrExportTable->FLTMGR.DATA.ApcFilterTypeId =
        g_FltMgrApcFilterTypeId; 
    pHrExportTable->FLTMGR.DATA.WorkItemFilterTypeId =
        g_FltMgrWorkItemFilterTypeId;  
    pHrExportTable->FLTMGR.DATA.WaitThreadFilterTypeId =
        g_FltMgrWaitThreadFilterTypeId;
    
    for (UINT16 i = 0; i < ExportTableDataQwordCount; i++) {
        ((UINT64*)ExportTableDataBase)[i] ^= ExportTableDataKey;
        ExportTableDataKey = _rotl64(ExportTableDataKey, 8);
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

    *pHunterExportTable = pHrExportTable;

    IsAborted = FALSE;

aborted:

    if (IsAborted) {
        if (pHrExportTable) {
            RtlSecureZeroMemory(
                pTableLowPaddingBase,
                (TableLowPaddingSize +
                sizeof(HR_EXPORT_TABLE) +
                TableHighPaddingSize));
            pHunterContext->HR_API.pMmFreeIndependentPages(
                pHrExportTable,
                REQUIRED_NUMBER_OF_PAGES(
                (TableLowPaddingSize +
                sizeof(HR_EXPORT_TABLE) +
                TableHighPaddingSize))
                << PAGE_SHIFT);
        }

        return HR_ABORTED;
    }

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
*     This routine initializes 
*     the critical table.
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
    MMPFN *pCriticalTablePfn = NULL;

    BOOLEAN IsLocked = FALSE;
    BOOLEAN IsAborted = TRUE;

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
        (CRITICAL_TABLE*)(((UINT8*)pTableLowPaddingBase) +
            TableLowPaddingSize);

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
        sizeof(CRITICAL_TABLE) - FIELD_OFFSET(CRITICAL_TABLE, TableSeed),
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
        pCriticalTable,
        &pCriticalTablePte,
        pHunterContext))) {
        DBG_BREAK;
        goto aborted;
    } else if (!pCriticalTablePte) {   
        DBG_BREAK;
        goto aborted;
    }

    pCriticalTablePfn =
        (MMPFN*)(pHunterContext->NTOS_ITEMS.PfnDatabase +
            (pCriticalTablePte->PageFrameNumber *
                sizeof(MMPFN)));

    //
    // This prevents stealing the PFN of the allocated page
    // without using dirty tricks,
    // such as calling the MmRemovePhysicalMemory.
    //
    _InterlockedIncrement16(
        (volatile INT16*)&pCriticalTablePfn->u3.ReferenceCount);

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

    BOOLEAN CheckStatus = FALSE;

    CRITICAL_TABLE *pCriticalTable = NULL;
    MMPTE_HARDWARE *pCriticalTablePte = NULL;
    UINT64 CriticalTablePfn = 0;

    BOOLEAN IsLocked = FALSE;
    BOOLEAN IsAborted = TRUE;

    if (!pQword || Offset > (sizeof(CRITICAL_TABLE) - 8)) {
        DBG_BREAK;
        goto aborted;
    }

    _disable();

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
        DBG_BREAK;
        goto aborted;
    }

    if (HR_ERROR(HrCheckCriticalTableIntegrity(
        &StackCriticalTable,
        &CheckStatus))) {
        DBG_BREAK;
        goto aborted;
    } else if (!CheckStatus) {
        DBG_BREAK;
        goto aborted;
    }

    *pQword =
        *(UINT64*)(((UINT8*)&StackCriticalTable) + Offset);

    IsAborted = FALSE;

aborted:

    RtlSecureZeroMemory(&StackCriticalTable, sizeof(CRITICAL_TABLE));

    if (pCriticalTable) {
        pCriticalTablePte->Valid = 0;
        pCriticalTablePte->PageFrameNumber = 0;
        __invlpg(pCriticalTable);

        pCriticalTable    = NULL;
        pCriticalTablePte = NULL;
        CriticalTablePfn  = 0;
    }

    if (IsLocked) {
        _InterlockedExchange(
            (volatile LONG*)&g_CriticalTableLock,
            0);
        _enable();
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

