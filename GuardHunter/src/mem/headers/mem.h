#pragma once
#include "../../_common/common.h"
#include "../../init/headers/init.h"

//
// Declarations of items from *mem.c*.
//

extern
HR_STATUS
FASTCALL
MemWriteRomData(
    OUT UINT8 *pDest,
    IN  CONST UINT8 *pSrc,
    IN  UINT64 SrcSize
);

extern
HR_STATUS
FASTCALL
MemSetRomData(
    OUT UINT8 *pDest,
    IN  UINT32 Src,
    IN  UINT64 SrcSize
);

extern
HR_STATUS
FASTCALL
MemIsHyperProtectedCr0Unsafe(
    OUT BOOLEAN *pIsHyperProtectedCr0,
    IN  HR_CONTEXT *pHunterContext
);

extern
HR_STATUS
FASTCALL
MemFindMemoryPattern(
    IN  UINT8 *pBaseRange,
    IN  UINT64 RangeSize,
    IN  CONST UINT8 *pPattern,
    IN  CONST UINT8 *pMask,
    OUT VOID **pPatternBase
);

extern
HR_STATUS
FASTCALL
MemIsSystemAddressValid(
    IN  VOID *pVa,
    OUT BOOLEAN *pIsAddressValid,
    IN  HR_CONTEXT *pHunterContext
);

extern 
HR_STATUS
FASTCALL
MemGetPteAddressSafe(
    IN  VOID *pVa,
    OUT VOID **pPteAddress,
    IN  HR_CONTEXT *pHunterContext
);

extern
VOID
FASTCALL
MemFlushPageTb(
    IN VOID *pPage
);

