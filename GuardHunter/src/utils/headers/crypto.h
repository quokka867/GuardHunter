#pragma once
#include "../../_common/common.h"

//
// Declarations of items from *crypto.c*.
//

extern
HR_STATUS
FASTCALL
CryDiffusionDataFlow(
    IN OUT UINT8 *pData,
    IN UINT64 DataSize
);

extern
HR_STATUS
FASTCALL
CryDeDiffusionDataFlow(
    IN OUT UINT8 *pData,
    IN UINT64 DataSize
);

extern
HR_STATUS
FASTCALL
CryFillBufferRandomDword(
    OUT UINT32 *pBuffer,
    IN  UINT32 NoDword,
    IN  UINT32 Seed
);

extern
HR_STATUS
FASTCALL
CryCrc32DataHash(
    IN  CONST UINT8 *pData,
    IN  UINT64 DataSize,
    OUT UINT32 *pHash
);

