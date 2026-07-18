#pragma once
#include "../../_common/common.h"
#include "../../mem/headers/mem.h"
#include "../../utils/headers/crypto.h"

//
// Declarations of items from *hr.c*.
//

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

typedef struct _CRITICAL_TABLE {
    UINT32 TableHash32;
    UINT8 CriticalTablePad0[4];
    UINT64 TableSeed;
    VOID* (FASTCALL *pMmAllocateIndependentPagesEx) (
        IN UINT64 NoBytes,
        IN  OPTIONAL UINT32 NumaNodeNum,
        OUT OPTIONAL UINT64 *pPfnArray,
        IN  OPTIONAL UINT32 PfnArraySize
        );
    VOID (FASTCALL *pMmFreeIndependentPages) (
        IN VOID *pAllocBase,
        IN UINT64 NoBytes
        );
    VOID (FASTCALL *pIoGetStackLimits) (
        OUT VOID *pLowLimit,
        OUT VOID *pHighLimit
        );
    UINT32 (FASTCALL *pRtlRandomEx) (
        IN OUT UINT32 *pSeed
        );
    KIRQL (FASTCALL *pKzRaiseIrql) (
        IN KIRQL NewIrql
        );
    VOID (FASTCALL *pKzLowerIrql) (
        IN KIRQL NewIrql
        );
    VOID (FASTCALL *pKeBugCheckEx) (
        IN UINT32 BugCheckCode,
        IN UINT64 BugCheckParameter1,
        IN UINT64 BugCheckParameter2,
        IN UINT64 BugCheckParameter3,
        IN UINT64 BugCheckParameter4
        );
} CRITICAL_TABLE;

extern
HR_STATUS
FASTCALL
HrCheckHunterContextIntegrity(
    IN  HR_CONTEXT *pHunterContext,
    OUT BOOLEAN *pCheckStatus
);

extern
HR_STATUS
FASTCALL
HrInitCriticalTable(
    IN  HR_CONTEXT *pHunterContext
);

extern
HR_STATUS
FASTCALL
HrReadCriticalTableQword(
    IN  UINT16 Offset,
    OUT UINT64 *pQword
);

extern
HR_STATUS
FASTCALL
HrCheckCriticalTableIntegrity(
    IN  CRITICAL_TABLE *pCriticalTable,
    OUT BOOLEAN *pCheckStatus
);

extern
HR_STATUS
FASTCALL
HrGetAsciiStringLength(
    IN  CONST UINT8 *pString,
    OUT UINT64 *pLength,
    IN  UINT64 MaxCount
);

