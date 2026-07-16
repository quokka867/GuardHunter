#pragma once
#include "../../_common/common.h"
#include "../../mem/headers/mem.h"
#include "hookasm.h"

//
// Declarations of items from *fltmgr.c*.
//

#define FILTER_TYPE_SELECTOR_BASE_ID   0xA9D3E505UI32
#define FILTER_TYPE_SELECTOR_ID_OFFSET 0xDC51UI16

// 01.
#define DPC_FILTER_TYPE_SELECTOR_ID \
(FILTER_TYPE_SELECTOR_BASE_ID +  FILTER_TYPE_SELECTOR_ID_OFFSET)

// 02.
#define TIMER_FILTER_TYPE_SELECTOR_ID \
(FILTER_TYPE_SELECTOR_BASE_ID + (FILTER_TYPE_SELECTOR_ID_OFFSET * 2))

// 03.
#define TIMER2_FILTER_TYPE_SELECTOR_ID \
(FILTER_TYPE_SELECTOR_BASE_ID + (FILTER_TYPE_SELECTOR_ID_OFFSET * 3))

// 04.
#define APC_FILTER_TYPE_SELECTOR_ID \
(FILTER_TYPE_SELECTOR_BASE_ID + (FILTER_TYPE_SELECTOR_ID_OFFSET * 4))

// 05.
#define WORK_ITEM_FILTER_TYPE_SELECTOR_ID \
(FILTER_TYPE_SELECTOR_BASE_ID + (FILTER_TYPE_SELECTOR_ID_OFFSET * 5))

// 06.
#define WAIT_THREAD_FILTER_TYPE_SELECTOR_ID \
(FILTER_TYPE_SELECTOR_BASE_ID + (FILTER_TYPE_SELECTOR_ID_OFFSET * 6))

#define FILTER_TYPE_SELECTOR_ID_COUNT 6

typedef UINT32 FILTER_STATUS;
#define FILTER_SUCCESS 0x0B3C85B5UI32
#define FILTER_ABORTED 0xBAB93E5CUI32
#define FILTER_ERROR(status) ((status) != FILTER_SUCCESS)

typedef UINT32 FILTER_DETECT_STATUS;
#define FILTER_DETECTED     0xFD16A76BUI32
#define FILTER_NOT_DETECTED 0x8DDD1738UI32

typedef struct _FILTER_DATA {
    UINT32 TypeId;
    UINT8 FilterDataPad0[4];
    LIST_ENTRY FilterListHead;
    volatile UINT32 FilterLock;
    volatile UINT32 FilterListDepth;
} FILTER_DATA;

typedef struct _FILTER_CONTEXT {
    HR_CONTEXT *pHunterContext;
    HOOK_ASM_STUB_CONTEXT *pHookAsmStubContext;
    UINT64 AddArgument1;
    UINT64 AddArgument2;
} FILTER_CONTEXT;

typedef struct FILTER_CALLBACK {
    UINT32 CallbackId;
    UINT8 FilterCallbackPad0[4];
    FILTER_STATUS (FASTCALL *pCallback) (
        IN  UINT64 CallbackContext,
        IN  FILTER_CONTEXT *pFilterContext,
        OUT FILTER_DETECT_STATUS *pFilterDetectStatus);
    UINT64 CallbackContext;
    LIST_ENTRY pFilterListEntry;
} FILTER_CALLBACK;

extern UINT32 g_FltMgrDpcFilterTypeId;
extern UINT32 g_FltMgrTimerFilterTypeId;
extern UINT32 g_FltMgrTimer2FilterTypeId;
extern UINT32 g_FltMgrApcFilterTypeId;
extern UINT32 g_FltMgrWorkItemFilterTypeId;
extern UINT32 g_FltMgrWaitThreadFilterTypeId;

extern
HR_STATUS
FASTCALL
FltMgrInitFilterData(
    IN UINT32 TypeSelectorId,
    IN UINT32 TypeId
);

extern
HR_STATUS
FASTCALL
FltMgrGetFilterByTypeId(
    IN  UINT32 TypeId,
    OUT FILTER_DATA **pFilter
);

extern
HR_STATUS
FASTCALL
FltMgrInitFilterCallback(
    OUT FILTER_CALLBACK *pFilterCallback,
    OUT UINT32 *pCallbackId,
    IN  VOID *pCallbackRoutine,
    IN  UINT64 CallbackContext
);

extern
HR_STATUS
FASTCALL
FltMgrRegisterFilterCallback(
    IN UINT32 TypeId,
    IN FILTER_CALLBACK *pFilterCallback
);

extern
HR_STATUS
FASTCALL
FltMgrDeregisterFilterCallback(
    IN  UINT32 TypeId,
    IN  UINT32 CallbackId,
    OUT BOOLEAN *pCallbackFound
);

extern
HR_STATUS
FASTCALL
FltMgrExecuteFilterCallbacks(
    IN  UINT32 TypeId,
    IN  FILTER_CONTEXT *pFilterContext,
    OUT FILTER_DETECT_STATUS *pFilterDetectStatus,
    IN  HR_CONTEXT *pHunterContext
);

