#pragma once
#include "../../_common/common.h"

//
// Declarations of items from *hrasm.c*.
//

typedef struct _DECRYPT_ASM_STUB_INFO {
    VOID *pStubBase;
    UINT16 StubSize;
    UINT16 OffsetDataPtr;
    UINT16 OffsetQwordCount;
    UINT16 OffsetXorKey;
} DECRYPT_ASM_STUB_INFO;

extern
FASTCALL
HR_STATUS
HrGetDecryptAsmStubInfoAsm64(
    DECRYPT_ASM_STUB_INFO *pDecryptAsmStubInfo
);

