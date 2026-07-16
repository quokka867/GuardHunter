/*++
* Module Name:
*
*     mem.c
*
* Abstract:
*
*     This module contains routines for memory operation.
*
* Author:
*
*     quokka867 (GitHub/Twitter).
*
--*/

#include "../headers/mem.h"

HR_STATUS
FASTCALL
MemWriteRomData(
    OUT UINT8 *pDest,
    IN  CONST UINT8 *pSrc,
    IN  UINT64 SrcSize
)
/*++
* Routine Description:
*
*     This routine writes data to read-only memory by
*     clearing the WriteProtect (bit 16) hardware flag in
*     the CR0 register.
*
* Arguments:
*
*     pDest   - Supplies a pointer to the destination.
*
*     pSrc    - Supplies a pointer to the source.
*
*     SrcSize - Supplies the size of the source.
*
* Return Value:
*
*     Internal status.
*
--*/
{
    BOOLEAN CurrentIF = FALSE;

    BOOLEAN CurrentWP = FALSE;

    if (!pDest || !pSrc || !SrcSize) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    if ((CurrentIF = IS_INTERRUPTS_ENABLED)) {
        _disable();
    }

    if ((CurrentWP = IS_WRITE_PROTECTION_ENABLED)) {
        __writecr0(__readcr0() & ~CR0_WRITE_PROTECT_FLAG);
    }

    KeMemoryBarrier();

    memmove(pDest, pSrc, SrcSize);

    KeMemoryBarrier();

    if (CurrentWP) {
        __writecr0(__readcr0() | CR0_WRITE_PROTECT_FLAG);
    }

    if (CurrentIF) {
        _enable();
    }

    return HR_SUCCESS;
}

HR_STATUS
FASTCALL
MemSetRomData(
    OUT UINT8 *pDest,
    IN  UINT32 Src,
    IN  UINT64 SrcSize
)
/*++
* Routine Description:
*
*     This routine sets data to read-only memory by
*     clearing the WriteProtect (bit 16) hardware flag in
*     the CR0 register.
*
* Arguments:
*
*     pDest   - Supplies a pointer to the destination.
*
*     Src     - Supplies the source value.
*
*     SrcSize - Supplies the size of the source.
*
* Return Value:
*
*     Internal status.
*
--*/
{
    BOOLEAN CurrentIF = FALSE;

    BOOLEAN CurrentWP = FALSE;

    if (!pDest || !SrcSize) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    if ((CurrentIF = IS_INTERRUPTS_ENABLED)) {
        _disable();
    }

    if ((CurrentWP = IS_WRITE_PROTECTION_ENABLED)) {
        __writecr0(__readcr0() & ~CR0_WRITE_PROTECT_FLAG);
    }

    KeMemoryBarrier();

    switch (SrcSize) {
    case 1:
        _InterlockedExchange8(
            (volatile CHAR*)pDest,
            (UINT8)Src);
        break;
    case 2:
        if (!((UINT64)pDest % 2)) {
            _InterlockedExchange16(
                (volatile SHORT*)pDest,
                (UINT16)Src);
            goto success;
        }
        break;
    case 4:
        if (!((UINT64)pDest % 4)) {
            _InterlockedExchange(
                (volatile LONG*)pDest,
                Src);
            goto success;
        }
        break;
    case 8:
        if (!((UINT64)pDest % 8)) {
            _InterlockedExchange64(
                (volatile LONG64*)pDest,
                ((UINT64)Src << 32) | (UINT64)Src);
            goto success;
        }
        break;
    }

    memset(pDest, Src, SrcSize);

success:

    KeMemoryBarrier();

    if (CurrentWP) {
        __writecr0(__readcr0() | CR0_WRITE_PROTECT_FLAG);
    }

    if (CurrentIF) {
        _enable();
    }

    return HR_SUCCESS;
}

HR_STATUS
FASTCALL
MemIsHyperProtectedCr0Unsafe(
    OUT BOOLEAN *pIsHyperProtectedCr0,
    IN  HR_CONTEXT *pHunterContext
)
/*++
* Routine Description:
*
*     This routine performs introspection to determine the
*     hyper-protection settings for the CR0 register based on the
*     CR0 bit mask configured in the hypervisor VMCS/VMCB settings.
*
* Arguments:
*
*     pIsHyperProtectedCr0 - Supplies a pointer to a variable that
*                            receives the introspection status.
*
*     pHunterContext       - Supplies a pointer to the
*                            HR_CONTEXT structure.
*
* Return Value:
*
*     Internal status.
*
--*/
{
    KIRQL OldIrql = 0;

    BOOLEAN CurrentWP = FALSE;

    UINT8 *pOpcodePatch = NULL;
    UINT8 RetOpcode = 0;
    UINT8 SaveOpcode = 0;

    EXCEPTION_RECORD ExceptionRecord = { 0 };
    EXCEPTION_RECORD *pExceptionRecord = &ExceptionRecord;

    BOOLEAN IsHyperProtectedCr0 = FALSE;

    if (!pIsHyperProtectedCr0 || !pHunterContext) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    OldIrql = pHunterContext->HR_API.pKzRaiseIrql(HIGH_LEVEL);

    if ((CurrentWP = IS_WRITE_PROTECTION_ENABLED)) {
        __writecr0(__readcr0() & ~CR0_WRITE_PROTECT_FLAG);
    }

    pOpcodePatch =
        (((UINT8*)pHunterContext->NTOS_ROUTINES.pKiErrata671Present) + 2);
    SaveOpcode = *pOpcodePatch;

    RetOpcode = 0xC3;

    if (NT_ERROR(pHunterContext->HR_API.pRtlpIcAccessMemory(
        &pExceptionRecord,
        &RetOpcode,
        pOpcodePatch,
        KernelMode,
        FALSE,
        1,
        1))) {
        IsHyperProtectedCr0 = TRUE;
        goto success;
    }

    if (((BOOLEAN(FASTCALL*)(VOID))
        pHunterContext->NTOS_ROUTINES.pKiErrata671Present)()) {
        IsHyperProtectedCr0 = TRUE;
    }

success:

    if (!IsHyperProtectedCr0) {
        *pOpcodePatch = SaveOpcode;
    }

    *pIsHyperProtectedCr0 = IsHyperProtectedCr0;

    if (CurrentWP) {
        __writecr0(__readcr0() | CR0_WRITE_PROTECT_FLAG);
    }

    pHunterContext->HR_API.pKzLowerIrql(OldIrql);

    return HR_SUCCESS;
}

#define MEM_PATTERN_MAXLEN 64

HR_STATUS
FASTCALL
MemFindMemoryPattern(
    IN  UINT8 *pBaseRange,
    IN  UINT64 RangeSize,
    IN  CONST UINT8 *pPattern,
    IN  CONST UINT8 *pMask,
    OUT VOID **pPatternBase
)
/*++
* Routine Description:
*
*     This routine searches for a pattern in the specified memory range.
*
* Arguments:
*
*     pBaseRange   - Supplies the base address of the memory range.
*
*     RangeSize    - Supplies the size of the memory range.
*
*     pPattern     - Supplies a pointer to the pattern.
*
*     pMask        - Supplies a pointer to the mask.
*
*     pPatternBase - Supplies a pointer to a variable that
*                    receives the pattern base.
*
* Return Value:
*
*     Internal status.
*
--*/
{
    UINT64 PatternLen = strnlen_s(
        (const char*)pMask,
        MEM_PATTERN_MAXLEN + 1);

    UINT64 FinalRangeLen = (RangeSize - PatternLen);

    UINT8 *pCurrentAddress = pBaseRange;

    BOOLEAN MatchFound = FALSE;

    if (!pBaseRange ||
        !RangeSize  ||
        !pPattern   ||
        !pMask      ||
        !pPatternBase) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    if (!PatternLen || PatternLen == (MEM_PATTERN_MAXLEN + 1) || 
        PatternLen > RangeSize) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    for (UINT64 i = 0; i <= FinalRangeLen; i++) {
        MatchFound = TRUE;
        for (UINT64 i2 = 0; i2 < PatternLen; i2++) {
            if (pMask[i2] == _bx && pCurrentAddress[i2] != pPattern[i2]) {
                MatchFound = FALSE;
                break;
            }
        }
        if (MatchFound) {
            *pPatternBase = (VOID*)pCurrentAddress;
            break;
        }
        pCurrentAddress++;
    }

    return HR_SUCCESS;
}

HR_STATUS
FASTCALL
MemIsSystemAddressValid(
    IN  VOID *pVa,
    OUT BOOLEAN *pIsAddressValid,
    IN  HR_CONTEXT *pHunterContext
) 
/*++
* Routine Description:
*
*     This routine checks the validity of the
*     specified system VA.
*
* Arguments:
*
*     pVa             - Supplies a pointer to the
*                       virtual address.
* 
*     pIsAddressValid - Supplies a pointer to a variable that
*                       receives the validity status.
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
    MMPTE_HARDWARE *pPte = NULL;

    BOOLEAN IsAddressValid = TRUE;

    if (!pHunterContext || !pVa || !pIsAddressValid) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    if (!IS_CANONICAL_SYSTEM_VA(pVa)) {
        IsAddressValid = FALSE;
        goto success;
    }

    if (HR_ERROR(MemGetPteAddressSafe(
        pVa,
        &pPte,
        pHunterContext))) {
        DBG_BREAK;
        return HR_ABORTED;
    } else if (!pPte) {
        IsAddressValid = FALSE;
    }

success:

    *pIsAddressValid = IsAddressValid;
   
    return HR_SUCCESS;
}

HR_STATUS
FASTCALL
MemGetPteAddressSafe(
    IN  VOID *pVa,
    OUT VOID **pPteAddress,
    IN  HR_CONTEXT *pHunterContext
)
/*++
* Routine Description:
*
*     This routine returns the
*     PTE address of the specified VA.
*
* Arguments:
*
*     pVa            - Supplies a pointer to the 
*                      virtual address.
*
*     pPteAddress    - Supplies a pointer to a variable that
*                      receives the PTE address.
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
    MMPTE_HARDWARE *pPte = NULL;

    if (!pVa || !pPteAddress || !pHunterContext) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    if (!(pPte = (MMPTE_HARDWARE*)GET_PML4_ENTRY_ADDRESS(
        pHunterContext->NTOS_ITEMS.PteBases[HR_CONTEXT_PXE_BASE_IDX],
        pVa))->Valid) {
        pPte = NULL;
        goto success;
    } else if (pPte->LargePage) {
        goto success;
    }
    if (!(pPte = (MMPTE_HARDWARE*)GET_PDPT_ENTRY_ADDRESS(
        pHunterContext->NTOS_ITEMS.PteBases[HR_CONTEXT_PPE_BASE_IDX],
        pVa))->Valid) {
        pPte = NULL;
        goto success;
    } else if (pPte->LargePage) {
        goto success;
    }
    if (!(pPte = (MMPTE_HARDWARE*)GET_PD_ENTRY_ADDRESS(
        pHunterContext->NTOS_ITEMS.PteBases[HR_CONTEXT_PDE_BASE_IDX],
        pVa))->Valid) {
        pPte = NULL;
        goto success;
    } else if (pPte->LargePage) {
        goto success;
    }
    if (!(pPte = (MMPTE_HARDWARE*)GET_PT_ENTRY_ADDRESS(
        pHunterContext->NTOS_ITEMS.PteBases[HR_CONTEXT_PTE_BASE_IDX],
        pVa))->Valid) {
        pPte = NULL;
    }

success:

    *pPteAddress = pPte;

    return HR_SUCCESS;
}

VOID
FASTCALL
MemFlushPageTb(
    IN VOID *pPage
)
/*++
* Routine Description:
*
*     This routine flushes the
*     TLB entry for the specified page.
*
* Arguments:
*
*     pPage - Supplies a pointer to the
*             page base.
* 
* Return Value:
*
*     None.
*
--*/
{
    __invlpg(pPage);

    return;
}

