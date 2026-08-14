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

#define BS_INITIALIZATION_ABORTED 0xD38FF4A1UI32

extern
HR_EXPORT_TABLE*
FASTCALL
BsMain(
    VOID
);

