#pragma once
#include "../../_common/common.h"
#include "../../utils/headers/crypto.h"
#include "../../utils/headers/hr.h"
#include "fltmgr.h"
#include "hookasm.h"


//
// Declarations of items from *hkrtn.c*.
//

#define DPC_HR_CONTEXT_CORRUPTION                0xF7612D46UI32
#define TIMER_HR_CONTEXT_CORRUPTION              0xB5FC863CUI32
#define TIMER2_HR_CONTEXT_CORRUPTION             0xFEE269B9UI32
#define APC_HR_CONTEXT_CORRUPTION                0xDA17D27DUI32
#define WORK_ITEM_HR_CONTEXT_CORRUPTION          0x3E5A93BCUI32
#define WAIT_THREAD_HR_CONTEXT_CORRUPTION        0xB86C993CUI32
#define PG_RECURSE_ROUTINE_HR_CONTEXT_CORRUPTION 0xE7CAD531UI32

extern
VOID
FASTCALL
HkKiExecuteAllDpcs(
    HOOK_ASM_STUB_CONTEXT *pHookAsmStubContext
);

extern
VOID
FASTCALL
HkKiProcessExpiredTimerList(
    HOOK_ASM_STUB_CONTEXT *pHookAsmStubContext
);

extern
VOID
FASTCALL
HkKiExpireTimer2(
    HOOK_ASM_STUB_CONTEXT *pHookAsmStubContext
);

extern
VOID
FASTCALL
HkKiDeliverApc(
    IN HOOK_ASM_STUB_CONTEXT *pHookAsmStubContext
);

extern
VOID
FASTCALL
HkKeRemovePriQueueEpi(
    IN HOOK_ASM_STUB_CONTEXT *pHookAsmStubContext
);

extern
VOID
FASTCALL
HkWaitThreadEpi(
    IN HOOK_ASM_STUB_CONTEXT *pHookAsmStubContext
);

extern
VOID
FASTCALL
HkKiCustomRecurseRoutineX(
    IN HOOK_ASM_STUB_CONTEXT *pHookAsmStubContext
);

