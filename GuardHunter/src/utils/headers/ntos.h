#pragma once
#include "../../_common/common.h"
#include "../../mem/headers/mem.h"

//
// Declarations of items from *ntos.c*.
//

typedef struct _WRITE_PRCB_QWORD {
    HR_CONTEXT *pHunterContext;
    UINT64 Qword;
    UINT16 OffsetQword;
    BOOLEAN IsIpi;
    UINT8 NtosWritePrcbQwordPad0[1];
    volatile UINT32 IpiSuccessCount;
} WRITE_PRCB_QWORD;

extern
HR_STATUS
FASTCALL
NtosFindKernelBaseVa(
    OUT VOID **pImageBase
);

extern
HR_STATUS
FASTCALL
NtosLookupKernelFunctionEntry(
    IN  UINT64 ControlPc,
    OUT RUNTIME_FUNCTION **pFunctionEntry,
    IN  HR_CONTEXT *pHunterContext
);

extern
VOID
FASTCALL
NtosWritePrcbQword(
    WRITE_PRCB_QWORD *pWritePrcbQword
);

