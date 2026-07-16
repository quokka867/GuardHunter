#pragma once
#include "../../_common/common.h"
#include "../../init/headers/init.h"
#include "../../mem/headers/mem.h"
#include "../../utils/headers/crypto.h"
#include "../../utils/headers/pe.h"
#include "hookasm.h"

//
// Declarations of items from *hook.c*.
//

extern
HR_STATUS
FASTCALL
HkInstallRoutineHook(
    IN VOID *pRoutineAddress,
    IN BOOLEAN IsEpilogue,
    IN VOID *pHookRoutine,
    IN HR_CONTEXT *pHunterContext
);

