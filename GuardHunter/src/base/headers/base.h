#pragma once
#include "../../_common/common.h"
#include "../../hook/headers/fltmgr.h"
#include "../../hook/headers/flt.h"
#include "../../hook/headers/hkrtn.h"
#include "../../hook/headers/hook.h"
#include "../../hook/headers/hookasm.h"
#include "../../init/headers/init.h"
#include "../../mem/headers/mem.h"
#include "../../utils/headers/crypto.h"
#include "../../utils/headers/hr.h"
#include "../../utils/headers/pe.h"

//
// Declarations of items from *base.c*.
//

typedef struct _HR_EXPORT_TABLE {
    UINT32 XorKey32;
    UINT8 HrExportTablePad0[4];
    struct {
        struct {
            HR_STATUS(FASTCALL *pFltMgrInitFilterCallback) (
                OUT FILTER_CALLBACK *pFilterCallback,
                OUT UINT32 *pCallbackId,
                IN  VOID *pCallbackRoutine,
                IN  UINT64 CallbackContext
                );
            HR_STATUS(FASTCALL *pFltMgrRegisterFilterCallback) (
                IN UINT32 TypeId,
                IN FILTER_CALLBACK *pFilterCallback
                );
            HR_STATUS(FASTCALL *pFltMgrDeregisterFilterCallback) (
                IN  UINT32 TypeId,
                IN  UINT32 CallbackId,
                OUT BOOLEAN *pCallbackFound
                );
        } API;
        struct {
            UINT32 DpcFilterTypeId;
            UINT32 TimerFilterTypeId;
            UINT32 Timer2FilterTypeId;
            UINT32 ApcFilterTypeId;
            UINT32 WorkItemFilterTypeId;
            UINT32 WaitThreadFilterTypeId;
        } DATA;
    } FLTMGR;
} HR_EXPORT_TABLE;

#define BS_MAIN_INITIALIZATION_ABORTED 0xD38FF4A1UI32

extern
HR_EXPORT_TABLE*
FASTCALL
BsMain(
    VOID
);

