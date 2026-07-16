/*++
* Module Name:
*
*     ntos.c
*
* Abstract:
*
*     This module contains routines for retrieving essential
*     NTOS information.
*
* Author:
*
*     quokka867 (GitHub/Twitter).
*
--*/

#include "../headers/ntos.h"

HR_STATUS
FASTCALL
NtosFindKernelBaseVa(
    OUT VOID **pImageBase
)
/*++
* Routine Description:
*
*     This routine searches for the kernel base.
*
* Arguments:
*
*     pImageBase -  Supplies a pointer to a variable that
*                   receives the image base.
*
* Return Value:
*
*     Internal status.
*
--*/
{
    INT32 Elfanew = 0;
    IMAGE_NT_HEADERS64 *pNtHeader64 = NULL;

    UINT64 KernelAddress = __readmsr(IA32_LSTAR);

    if (!KernelAddress || !pImageBase) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    KernelAddress &= ~0xFFF;
    KernelAddress += PAGE_SIZE;

    while (TRUE) {
        KernelAddress -= PAGE_SIZE;
        if (((IMAGE_DOS_HEADER*)KernelAddress)->e_magic ==
            IMAGE_DOS_SIGNATURE) {
            Elfanew = ((IMAGE_DOS_HEADER*)KernelAddress)->e_lfanew;
            if (Elfanew > 0x400 || Elfanew < 0) {
                continue;
            }
            pNtHeader64 = (IMAGE_NT_HEADERS64*)(KernelAddress + 
                (UINT32)Elfanew);
            if ((pNtHeader64->Signature != 
                IMAGE_NT_SIGNATURE) ||
                (pNtHeader64->FileHeader.Machine != 
                IMAGE_FILE_MACHINE_AMD64) ||
                (pNtHeader64->OptionalHeader.Magic != 
                IMAGE_NT_OPTIONAL_HDR64_MAGIC) ||
                (pNtHeader64->OptionalHeader.ImageBase !=
                KernelAddress)) {
                continue;
            }
            *pImageBase = (VOID*)KernelAddress;

            break;
        }
    }

    return HR_SUCCESS;
}

HR_STATUS
FASTCALL
NtosLookupKernelFunctionEntry(
    IN  UINT64 ControlPc,
    OUT RUNTIME_FUNCTION **pFunctionEntry,
    IN  HR_CONTEXT *pHunterContext
)
/*++
* Routine Description:
*
*     This routine searches for an entry for the
*     specified IP only in the
*     kernel exception table.
*
* Arguments:
*
*     ControlPc      - Supplies the IP.
* 
*     pFunctionEntry - Supplies a pointer to a variable that
*                      receives a pointer to the
*                      RUNTIME_FUNCTION structure.
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
    UINT64 ImageBase = 0;

    UINT64 SectionsLow = 0;
    UINT64 SectionsHigh = 0;

    RUNTIME_FUNCTION *pFunctionEntry2 = NULL;

    if (!ControlPc || !pFunctionEntry || !pHunterContext) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    SectionsLow = (UINT64)pHunterContext->
        NTOS_PROCESS.NTOS_IMAGE.SECTIONS_VA_RANGE.pLowVa;
    SectionsHigh = (UINT64)pHunterContext->
        NTOS_PROCESS.NTOS_IMAGE.SECTIONS_VA_RANGE.pHighVa;

    if ((UINT64)ControlPc >= SectionsLow &&
        (UINT64)ControlPc <= SectionsHigh) {
        pFunctionEntry2 = pHunterContext->HR_API.pRtlLookupFunctionEntry(
            (UINT64)ControlPc,
            &ImageBase,
            NULL);
    } 

    *pFunctionEntry = pFunctionEntry2;

    return HR_SUCCESS;
}

VOID
FASTCALL
NtosWritePrcbQword(
    WRITE_PRCB_QWORD *pWritePrcbQword
)
/*++
* Routine Description:
*
*     This routine writes the
*     specified QWORD to the PRCB of the
*     current logical core at the specified offset.
*
* Arguments:
*
*     pWritePrcbQword - Supplies a pointer to the
*                       WRITE_PRCB_QWORD structure.
* 
* Return Value:
*
*     None.
*
--*/
{
    UINT64 pPrcb = 0;

    UINT64 *pQword = NULL;

    if (!pWritePrcbQword) {
        goto aborted;
    }

    pPrcb = __readgsqword(FIELD_OFFSET(KPCR, CurrentPrcb));

    pQword = (UINT64*)(pPrcb + pWritePrcbQword->OffsetQword);

    *pQword = pWritePrcbQword->Qword;
    
    if (pWritePrcbQword->IsIpi) {
        _InterlockedIncrement(
            (volatile LONG*)&pWritePrcbQword->IpiSuccessCount);
    }

aborted:

    return;
}