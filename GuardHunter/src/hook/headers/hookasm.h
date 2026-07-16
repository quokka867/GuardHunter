#pragma once
#include "../../_common/common.h"

//
// Declarations of items from *hookasm.asm*.
//

typedef struct _HOOK_ASM_STUB_CONTEXT {
    HR_CONTEXT *pHunterContext;
    UINT64 TrampolineBase;
    UINT64 Rsp;
    UINT64 Rbp;
    M128A Xmm3;
    M128A Xmm2;
    M128A Xmm1;
    M128A Xmm0;
    UINT64 R9;
    UINT64 R8;
    UINT64 Rdx;
    UINT64 Rcx;
    UINT64 *pRax;
    UINT8 HoookAsmStubContextPad0[8];
} HOOK_ASM_STUB_CONTEXT;

typedef struct _HOOK_ASM_STUB_INFO {
    VOID *pStubBase;
    UINT16 StubSize;
    UINT16 OffsetTrampolineBasePtr;
    UINT16 OffsetHunterContextPtr;
    UINT16 OffsetHookRoutinePtr;
    UINT8 HookAsmStubInfoPad0[4];
} HOOK_ASM_STUB_INFO;

extern
HR_STATUS
FASTCALL
HkGetHookAsmStubRangeAsm64(
    OUT HOOK_ASM_STUB_INFO *pHookAsmStubInfo
);

extern
DECLSPEC_NORETURN
VOID
FASTCALL
HkCustomBugCheckAsm64(
    IN UINT32 InternalBugStatus,
    IN VOID *pStackHighLimit,
    IN VOID *pKeBugCheckEx
);

extern
DECLSPEC_NORETURN
VOID
FASTCALL
HkSetRspSafeAsm64(
    IN UINT64 NewRsp,
    IN CONTEXT *pContext
);

