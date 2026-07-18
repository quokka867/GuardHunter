#pragma once
#include "../../_common/common.h"
#include "../../init/headers/init.h"
#include "../../mem/headers/mem.h"
#include "fltmgr.h"

//
// Declarations of items from *flt.c*.
//

#if DBG

typedef enum _FILTER_DETECT_REASON {
    Reserved0,
    NonBackedCode,
    PgCodePattern,
    NonCanonicalContext,
    InvalidContext,
    RwxContext
} FILTER_DETECT_REASON;

#endif

extern
HR_STATUS
FASTCALL
FltInitWhiteRoutineTable(
    IN HR_CONTEXT *pHunterContext
);

extern
FILTER_STATUS
FASTCALL
FltIsPatchGuardDpc(
    IN  UINT64 CallbackContext,
    IN  FILTER_CONTEXT *pFilterContext,
    OUT FILTER_DETECT_STATUS *pFilterDetectStatus
);

extern
FILTER_STATUS
FASTCALL
FltIsPatchGuardTimer2(
    IN  UINT64 CallbackContext,
    IN  FILTER_CONTEXT *pFilterContext,
    OUT FILTER_DETECT_STATUS *pFilterDetectStatus
);

extern
FILTER_STATUS
FASTCALL
FltIsPatchGuardApc(
    IN  UINT64 CallbackContext,
    IN  FILTER_CONTEXT *pFilterContext,
    OUT FILTER_DETECT_STATUS *pFilterDetectStatus
);

extern
FILTER_STATUS
FASTCALL
FltIsPatchGuardWorkItem(
    IN  UINT64 CallbackContext,
    IN  FILTER_CONTEXT *pFilterContext,
    OUT FILTER_DETECT_STATUS *pFilterDetectStatus
);

extern
FILTER_STATUS
FASTCALL
FltIsPatchGuardWaitThread(
    IN  UINT64 CallbackContext,
    IN  FILTER_CONTEXT *pFilterContext,
    OUT FILTER_DETECT_STATUS *pFilterDetectStatus
);

