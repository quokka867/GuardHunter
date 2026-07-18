#pragma once
#include "../../_common/common.h"
#include "../../mem/headers/mem.h"
#include "../headers/hr.h"
#include "../headers/crypto.h"

//
// Declarations of items from *pe.c*.
//

extern
HR_STATUS
FASTCALL
PeFindSectionBaseVa(
    IN  VOID *pImageBase,
    IN  UINT64 SectionName,
    IN  BOOLEAN IsXoredName,
    OUT UINT32 *pSectionSize,
    OUT VOID **pSectionBase
);

extern
HR_STATUS
FASTCALL
PeGetImageSectionsRange(
    IN  VOID *pImageBase,
    OUT VOID **pLowVa,
    OUT VOID **pHighVa
);

extern
HR_STATUS
FASTCALL
PeFindExportItemCrc32Hash(
    IN  VOID *pImageBase,
    IN  UINT32 ItemNameHash,
    IN  BOOLEAN IsXoredHash,
    OUT VOID **pItemAddress
);

extern
HR_STATUS
FASTCALL
PeFindSectionMemoryPattern(
    IN  VOID *pImageBase,
    IN  UINT64 SectionName,
    IN  BOOLEAN IsXoredName,
    IN  CONST UINT8 *pPattern,
    IN  CONST UINT8 *pMask,
    OUT VOID **pPatternBase
);

extern
HR_STATUS
FASTCALL
PeTruncateImageHeaders(
    IN VOID *pImageBase
);

