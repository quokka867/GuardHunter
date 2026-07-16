#pragma once
#include "../../_common/common.h"
#include "../../utils/headers/pe.h"
#include "../../utils/headers/ntos.h"
#include "../../utils/headers/hr.h"
#include "../../utils/headers/crypto.h"

//
// Declarations of items from *init.c*.
//

#define NTOS_TEXT_SECTION_NAME     0x2E74657874000000UI64
#define NTOS_PAGE_SECTION_NAME     0x5041474500000000UI64
#define NTOS_TRACESUP_SECTION_NAME 0x5452414345535550UI64
#define NTOS_INITKDBG_SECTION_NAME 0x494E49544B444247UI64

#define _bx 0xC3UI8
#define _b0 0x90UI8
#define _sx "\xC3"
#define _s0 "\x90"

extern
HR_STATUS
FASTCALL
InitHunterContext(
    OUT HR_CONTEXT_DESCRIPTOR *pHunterContextDesc
);

