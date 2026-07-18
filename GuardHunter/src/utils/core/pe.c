/*++
* Module Name:
*
*     pe.c
*
* Abstract:
*
*     This module contains routines for parsing PE32+ images in memory.
*
* Author:
*
*     quokka867 (GitHub/Twitter).
*
--*/

#include "../headers/pe.h"

HR_STATUS
FASTCALL
PeFindSectionBaseVa(
    IN  VOID *pImageBase,
    IN  UINT64 SectionName,
    IN  BOOLEAN IsXoredName,
    OUT UINT32 *pSectionSize,
    OUT VOID **pSectionBase
)
/*++
* Routine Description:
*
*     This routine searches for the base virtual address
*     of a section with the specified name in the PE32+ image.
*
* Arguments:
*
*     pImageBase   - Supplies a pointer to the image base.
*
*     SectionName  - Supplies the section name.
*
*     IsXoredName  - Supplies a boolean flag for the
*                    XORed state of SectionName.
*
*     pSectionSize - Supplies a pointer to a variable that
*                    receives the section size.
*
*     pSectionBase - Supplies a pointer to a variable that
*                    receives the section base.
*
* Return Value:
*
*     Internal status.
*
--*/
{
    INT32 Elfanew = 0;
    IMAGE_NT_HEADERS64 *pNtHeader64 = NULL;

    UINT16 NoSections = 0;
    IMAGE_SECTION_HEADER *pTableSectionHeader = NULL;

    if (!pImageBase || !SectionName || !pSectionSize || !pSectionBase) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    if (IsXoredName) {
        SectionName = QUICK_XOR64(SectionName);
    }

    if (((IMAGE_DOS_HEADER*)pImageBase)->e_magic != IMAGE_DOS_SIGNATURE) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    Elfanew = ((IMAGE_DOS_HEADER*)pImageBase)->e_lfanew;
    if (Elfanew > 0x400 || Elfanew < 0) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    pNtHeader64 = (IMAGE_NT_HEADERS64*)((UINT8*)pImageBase + Elfanew);
    if (pNtHeader64->Signature != IMAGE_NT_SIGNATURE ||
        pNtHeader64->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64 ||
        pNtHeader64->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    NoSections = pNtHeader64->FileHeader.NumberOfSections;
    if (!NoSections || NoSections > 256) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    pTableSectionHeader = 
        (IMAGE_SECTION_HEADER*)((UINT8*)&pNtHeader64->OptionalHeader +
            pNtHeader64->FileHeader.SizeOfOptionalHeader);

    SectionName = _byteswap_uint64(SectionName);

    for (UINT16 i = 0; i < NoSections; i++) {
        if (SectionName == *(UINT64*)&pTableSectionHeader->Name) {
            *pSectionSize = pTableSectionHeader->Misc.VirtualSize;
            *pSectionBase = (VOID*)((UINT8*)pImageBase +
                pTableSectionHeader->VirtualAddress);
            break;
        }
        pTableSectionHeader++;
    }

    return HR_SUCCESS;
}

HR_STATUS
FASTCALL
PeGetImageSectionsRange(
    IN  VOID *pImageBase,
    OUT VOID **pLowVa,
    OUT VOID **pHighVa
)
/*++
* Routine Description:
*
*     This routine calculates the virtual address range
*     for the sections of the PE32+ image.
*
* Arguments:
*
*     pImageBase - Supplies a pointer to the image base.
*
*     pLowVa     - Supplies a pointer to a variable that
*                  receives the low virtual address.
*
*     pHighVa    - Supplies a pointer to a variable that
*                  receives the high virtual address.
*
* Return Value:
*
*     Internal status.
*
--*/
{
    INT32 Elfanew = 0;
    IMAGE_NT_HEADERS64 *pNtHeader64 = NULL;

    UINT16 NoSections = 0;
    IMAGE_SECTION_HEADER *pTableSectionHeader = NULL;

    UINT32 LowSectionRva = 0;
    UINT32 HighSectionRva = 0;

    UINT32 CurrentSectionRva = 0;

    if (!pImageBase) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    if (((IMAGE_DOS_HEADER*)pImageBase)->e_magic != IMAGE_DOS_SIGNATURE) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    Elfanew = ((IMAGE_DOS_HEADER*)pImageBase)->e_lfanew;
    if (Elfanew > 0x400 || Elfanew < 0) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    pNtHeader64 = (IMAGE_NT_HEADERS64*)((UINT8*)pImageBase + Elfanew);
    if (pNtHeader64->Signature != IMAGE_NT_SIGNATURE ||
        pNtHeader64->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64 ||
        pNtHeader64->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    NoSections = pNtHeader64->FileHeader.NumberOfSections;
    if (!NoSections || NoSections > 256) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    pTableSectionHeader = 
        (IMAGE_SECTION_HEADER*)((UINT8*)&pNtHeader64->OptionalHeader +
            pNtHeader64->FileHeader.SizeOfOptionalHeader);

    LowSectionRva = pTableSectionHeader->VirtualAddress;
    HighSectionRva = LowSectionRva +
        (pTableSectionHeader->Misc.VirtualSize - 1);

    NoSections -= 1;
    pTableSectionHeader += 1;

    for (UINT16 i = 0; i < NoSections; i++) {
        CurrentSectionRva = pTableSectionHeader->VirtualAddress;
        if (CurrentSectionRva < LowSectionRva) {
            LowSectionRva = CurrentSectionRva;
        } else if (CurrentSectionRva > HighSectionRva) {   
            HighSectionRva = CurrentSectionRva +
                (pTableSectionHeader->Misc.VirtualSize - 1);
        }
        pTableSectionHeader++;
    }

    *pLowVa =  (VOID*)((UINT64)pImageBase + LowSectionRva);
    *pHighVa = (VOID*)((UINT64)pImageBase + HighSectionRva);

    return HR_SUCCESS;
}

#define ITEM_NAME_MAXLEN 4096

HR_STATUS
FASTCALL
PeFindExportItemCrc32Hash(
    IN  VOID *pImageBase,
    IN  UINT32 ItemNameHash,
    IN  BOOLEAN IsXoredHash,
    OUT VOID **pItemAddress
)
/*++
* Routine Description:
*
*     This routine resolves the absolute address
*     of an exported item within a PE32+ image by walking the EAT
*	  and matching a 32-bit hash of the target name.
*
* Arguments:
*
*     pImageBase   - Supplies a pointer to the image base.
*
*     ItemNameHash - Supplies the item name hash.
*
*     IsXoredHash  - Supplies a boolean flag for the
*                    XORed state of ItemNameHash.
*
*     pItemAddress - Supplies a pointer to a variable that
*                    receives the item address.
*
* Return Value:
*
*     Internal status.
*
--*/
{
    INT32 Elfanew = 0;
    IMAGE_NT_HEADERS64 *pNtHeader64 = NULL;

    UINT32 ImageExportDirRva = 0;
    IMAGE_EXPORT_DIRECTORY *pImageExportDir = NULL;

    UINT32 *pTableAddressOfNames = NULL;
    UINT16 *pTableAddressOfNameOrdinals = NULL;
    UINT32 *pTableAddressOfFunctions = NULL;

    UINT32 NoNames = 0;
    UINT8 *pNameAddress = NULL;
    UINT64 NameLength = 0;
    UINT32 NameHash = 0;

    if (!pImageBase || !ItemNameHash || !pItemAddress) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    if (IsXoredHash) {
        ItemNameHash = (UINT32)QUICK_XOR32(ItemNameHash);
    }

    if (((IMAGE_DOS_HEADER*)pImageBase)->e_magic != IMAGE_DOS_SIGNATURE) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    Elfanew = ((IMAGE_DOS_HEADER*)pImageBase)->e_lfanew;
    if (Elfanew > 0x400 || Elfanew < 0) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    pNtHeader64 = (IMAGE_NT_HEADERS64*)((UINT8*)pImageBase + Elfanew);
    if (pNtHeader64->Signature != IMAGE_NT_SIGNATURE ||
        pNtHeader64->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64 ||
        pNtHeader64->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    ImageExportDirRva = pNtHeader64->OptionalHeader.
        DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    if (!ImageExportDirRva) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    pImageExportDir = (IMAGE_EXPORT_DIRECTORY*)((UINT8*)pImageBase +
        ImageExportDirRva);

    NoNames = pImageExportDir->NumberOfNames;
    if (!NoNames) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    pTableAddressOfNames =
        (UINT32*)((UINT8*)pImageBase + 
            pImageExportDir->AddressOfNames);

    pTableAddressOfNameOrdinals =
        (UINT16*)((UINT8*)pImageBase + 
            pImageExportDir->AddressOfNameOrdinals);

    pTableAddressOfFunctions =
        (UINT32*)((UINT8*)pImageBase + 
            pImageExportDir->AddressOfFunctions);

    for (UINT32 i = 0; i < NoNames; i++) {
        pNameAddress = ((UINT8*)pImageBase + pTableAddressOfNames[i]);
        if (HR_ERROR(HrGetAsciiStringLength(
            pNameAddress,
            &NameLength,
            ITEM_NAME_MAXLEN + 1))) {
            DBG_BREAK;
            return HR_ABORTED;
        } else if (!NameLength || NameLength == (ITEM_NAME_MAXLEN + 1)) {
            continue;
        }
        if (HR_ERROR(CryCrc32DataHash(
            pNameAddress,
            NameLength,
            &NameHash))) {
            DBG_BREAK;
            return HR_ABORTED;
        } else if (ItemNameHash == NameHash) {
            *pItemAddress = ((UINT8*)pImageBase +
                pTableAddressOfFunctions[pTableAddressOfNameOrdinals[i]]);
            break;
        }
    }

    return HR_SUCCESS;
}

HR_STATUS
FASTCALL
PeFindSectionMemoryPattern(
    IN  VOID *pImageBase,
    IN  UINT64 SectionName,
    IN  BOOLEAN IsXoredName,
    IN  CONST UINT8 *pPattern,
    IN  CONST UINT8 *pMask,
    OUT VOID **pPatternBase
)
/*++
* Routine Description:
*
*     This routine searches for a memory pattern in the
*     specified PE32+ image section.
*
* Arguments:
*
*     pImageBase   - Supplies a pointer to the image base.
*
*     SectionName  - Supplies the section name.
*
*     IsXoredName  - Supplies a boolean flag for the
*                    XORed state of SectionName.
*
*     pPattern     - Supplies a pointer to the pattern array.
*
*     pMask        - Supplies a pointer to the mask string.
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
    UINT32 SectionSize = 0;

    VOID *pSectionBase = NULL;

    VOID *pPatternBase2 = NULL;

    if (HR_ERROR(PeFindSectionBaseVa(
        pImageBase,
        SectionName,
        IsXoredName,
        &SectionSize,
        &pSectionBase))) {
        DBG_BREAK;
        return HR_ABORTED;
    } else if (!SectionSize || !pSectionBase) {  
        DBG_BREAK;
        return HR_ABORTED;
    }

    if (HR_ERROR(MemFindMemoryPattern(
        pSectionBase,
        (UINT64)SectionSize,
        pPattern,
        pMask,
        &pPatternBase2))) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    *pPatternBase = pPatternBase2;

    return HR_SUCCESS;
}

HR_STATUS
FASTCALL
PeTruncateImageHeaders(
    IN VOID *pImageBase
)
/*++
* Routine Description:
*
*     This routine truncates the
*     PE32+ headers of the specified image.
*
* Arguments:
*
*     pImageBase - Supplies a pointer to the
*                  image base.
*
* Return Value:
*
*     Internal status.
*
--*/
{
    UINT64 SectionsLowVa = 0;
    UINT64 SectionsHighVa = 0;

    if (!pImageBase) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    if (HR_ERROR(PeGetImageSectionsRange(
        pImageBase,
        (VOID**)&SectionsLowVa,
        (VOID**)&SectionsHighVa))) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    if (HR_ERROR(MemSetRomData(
        pImageBase,
        0,
        SectionsLowVa - (UINT64)pImageBase))) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    return HR_SUCCESS;
}

