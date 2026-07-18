/*++
* Module Name:
*
*     hook.c
*
* Abstract:
*
*     This module contains routines for 
*     autonomously hooking the execution of target routines.
*
* Author:
*
*     quokka867 (GitHub/Twitter).
*
--*/

#include "../headers/hook.h"

typedef struct _HOOK_CONTEXT {
    VOID *pTrampolineBase;
    UINT8 NopPaddingSize;
    UINT8 RestoreInstructions[64];
    UINT8 RestoreInstructionsSize;
    UINT8 HookDataPad0[6];
} HOOK_CONTEXT;

#define IS_ADD_RSP_IMMX_INSTRUCTION(x) \
((((((UINT32)(x)) & 0x00FFFFFF) == 0x00C48348)) || \
(((((UINT32)(x)) & 0x00FFFFFF) == 0x00C48148)))

#define IS_RELATIVE_IP_MODRM(x) \
((((UINT8)(x)) & 0xC7) == 0x05)

#define IS_INC86_OPCODE(x) \
((((UINT16)(x)) & 0xF8FF) == 0xC0FF)

#define IS_DEC86_OPCODE(x) \
((((UINT16)(x)) & 0xF8FF) == 0xC8FF)

#define IS_CALL_OPCODE(x) \
(((UINT8)(x)) == 0xE8)

#define IS_RELX_JMP_OPCODE(x) \
((((UINT8)(x)) == 0xE9) || \
(((UINT8)(x)) == 0xEB))

#define IS_RELX_JCC_OPCODE(x) \
(((((UINT16)(x)) & 0xF0FF) == 0x800F) || \
((((UINT8)(x)) & 0xF0) == 0x70))

#define IS_RELX_LOOP_OPCODE(x) \
((((UINT8)(x)) & 0xFC) == 0xE0)

HR_STATUS
FASTCALL
HkInitHookContext(
    IN  VOID *pRoutineAddress,
    IN  BOOLEAN IsEpilogue,
    OUT HOOK_CONTEXT *pHookContext,
    IN  HR_CONTEXT *pHunterContext
)
/*++
* Routine Description:
*
*     This routine initializes the
*     hook context.
*
* Arguments:
*
*     pRoutineAddress - Supplies a pointer to the
*                       target routine address.
*
*     IsEpilogue      - Supplies the
*                       epilogue hook flag.
*
*     pHookContext    - Supplies a pointer to the
*                       HOOK_CONTEXT structure.
*
*     pHunterContext  - Supplies a pointer to the
*                       HR_CONTEXT structure.
*
* Return Value:
*
*     Internal status.
*
--*/
{
    VOID *pTrampolineBase = NULL;
    UINT8 AccTrampolineSize = 0;

    BOOLEAN EpiTrampolineBaseFound = FALSE;

    UINT32 StackFrameCleanupInstruction = 0;

    UINT64 ImageBase = 0;

    RUNTIME_FUNCTION *pFunctionEntry = NULL;

    UINT64 RoutineStart = 0;
    UINT64 RoutineEnd = 0;

    KTP_READ_CONTEXT TpReadContext = { 0 };
    RTL_IC_CONTEXT IcContext = { 0 };

    UINT16 Instruction = 0;
    UINT16 Opcode = 0;
    UINT8 ModRm = 0;

    UINT8 InstructionLength = 0;

    if (!pRoutineAddress || !pHookContext || !pHunterContext) {
        DBG_BREAK;
        return HR_ABORTED;
    }

    if (IsEpilogue) {
        if ((*(UINT8*)pRoutineAddress) != 0xC3) {
            DBG_BREAK;
            return HR_ABORTED;
        }
        for (UINT8 i = 0; i < 64; i++) {
            StackFrameCleanupInstruction = *(UINT32*)(pTrampolineBase =
                (((UINT8*)pRoutineAddress) - i));
            if (IS_ADD_RSP_IMMX_INSTRUCTION(StackFrameCleanupInstruction)) {
                EpiTrampolineBaseFound = TRUE;
                break;
            }
        }
        if (!EpiTrampolineBaseFound) {
            DBG_BREAK;
            return HR_ABORTED;
        } else if (((UINT64)pRoutineAddress - 
            (UINT64)pTrampolineBase) < 6) {
            DBG_BREAK;
            return HR_ABORTED;
        }
    } else {
        if (!(pFunctionEntry =
            pHunterContext->HR_API.pRtlLookupFunctionEntry(
                (UINT64)pRoutineAddress,
                &ImageBase,
                NULL))) {
            DBG_BREAK;
            return HR_ABORTED;
        }
        RoutineStart = (ImageBase + pFunctionEntry->BeginAddress);
        if (RoutineStart != (UINT64)pRoutineAddress) {
            DBG_BREAK;
            return HR_ABORTED;
        }
        RoutineEnd = (ImageBase + pFunctionEntry->EndAddress);
        if ((RoutineEnd - RoutineStart) < 7) {
            DBG_BREAK;
            return HR_ABORTED;
        }
        pTrampolineBase = pRoutineAddress;
    }

    TpReadContext.Process = NULL;
    TpReadContext.pAddress = pTrampolineBase;

    for (UINT8 i = 0; i < 6; i++) {
        Instruction = *(UINT16*)(TpReadContext.pAddress);
        if (IS_INC86_OPCODE(Instruction) ||
            IS_DEC86_OPCODE(Instruction)) {
            InstructionLength = 2;
        } else {    
            if (NT_ERROR(
                pHunterContext->HR_API.pRtlIcParseInstruction(
                    NULL,
                    &TpReadContext,
                    0,
                    &IcContext))) {
                DBG_BREAK;
                return HR_ABORTED;
            }
            Opcode = *(UINT16*)(((UINT8*)TpReadContext.pAddress) +
                IcContext.OffsetOpcode);
            ModRm = IcContext.ModRm;
            if (IS_RELATIVE_IP_MODRM(ModRm) ||
                IS_CALL_OPCODE(Opcode)      ||
                IS_RELX_JMP_OPCODE(Opcode)  ||
                IS_RELX_JCC_OPCODE(Opcode)  ||
                IS_RELX_LOOP_OPCODE(Opcode)) {
                DBG_BREAK;
                return HR_ABORTED;
            }
            InstructionLength = IcContext.InstructionLength;
        }
        if ((AccTrampolineSize +=
            InstructionLength) > 5) {
            break;
        }
        TpReadContext.pAddress = ((UINT8*)pTrampolineBase) +
            AccTrampolineSize;
    }

    pHookContext->pTrampolineBase = pTrampolineBase;
    pHookContext->NopPaddingSize = (AccTrampolineSize - 6);
    memmove(
        pHookContext->RestoreInstructions,
        pTrampolineBase,
        AccTrampolineSize);
    pHookContext->RestoreInstructionsSize = AccTrampolineSize;

    return HR_SUCCESS;
}

typedef struct _INSTALL_HOOK_CONTEXT {
    HR_CONTEXT *pHunterContext;
    HOOK_CONTEXT *pHookContext;
    VOID **pStubBase;
    UINT32 ActiveLogicalCoreCount;
    volatile UINT32 CoreSyncBarrier[16];
    volatile UINT32 CoreSyncLock;
    HR_STATUS Status;
} INSTALL_HOOK_CONTEXT;

static
VOID
FASTCALL
HkEnterBarrierGateIpi(
    IN OUT volatile UINT32 *pBarrier
)
/*++
* Routine Description:
*
*     This routine synchronizes logical cores executing
*     from the IPI context using a counter-based barrier.
*
* Arguments:
*
*     pBarrier - Supplies a pointer to the
*                volatile barrier.
*
* Return Value:
*
*     None.
*
--*/
{
    if (_InterlockedDecrement(
        (volatile LONG*)pBarrier)) {
        while (*pBarrier) {
            _mm_pause();
        }
    }

    return;
}

static
VOID
FASTCALL
HkInstallRoutineHookIpi(
    IN OUT INSTALL_HOOK_CONTEXT *pInstallHookContext
)
/*++
* Routine Description:
*
*     This routine performs a synchronous insertion of a
*     trampoline into the target routine from the IPI context.
*
* Arguments:
*
*     pInstallHookContext - Supplies a pointer to the
*                           INSTALL_HOOK_CONTEXT structure.
*
* Return Value:
*
*     None.
*
--*/
{
    UINT64 TrampolineBase = 0;
    UINT64 TrampolineEnd = 0;

    HOOK_ASM_STUB_INFO HookAsmStubInfo = { 0 };

    UINT8 TrampolineBody[20] = {
        0xFF, 0x25, 0xAA, 0xAA, 0xAA, 0xAA,
        0x90, 0x90,
        0x90, 0x90,
        0x90, 0x90,
        0x90, 0x90,
        0x90, 0x90,
        0x90, 0x90,
        0x90, 0x90
    };

    INT32 ActiveStubBaseJmpRel32 = 0;

    TrampolineBase = 
        (UINT64)(pInstallHookContext->pHookContext->pTrampolineBase);
    TrampolineEnd = 
        (TrampolineBase +
            (6 + pInstallHookContext->pHookContext->NopPaddingSize));

    HkEnterBarrierGateIpi(&pInstallHookContext->CoreSyncBarrier[0]);

    if (HR_ERROR(pInstallHookContext->Status)) {
        goto aborted;
    }

    if (!_interlockedbittestandset(
        (volatile LONG*)&pInstallHookContext->CoreSyncLock,
        0)) {
        if (HR_ERROR(HkGetHookAsmStubRangeAsm64(&HookAsmStubInfo))) {
            _InterlockedExchange(
                (volatile LONG*)&pInstallHookContext->Status,
                HR_ABORTED);
            goto aborted2;
        }

        ActiveStubBaseJmpRel32 =
            (INT32)(((UINT64)pInstallHookContext->pStubBase) -
                (((UINT64)pInstallHookContext->pHookContext->
                    pTrampolineBase) + 6));
        *(INT32*)(TrampolineBody + 2) = ActiveStubBaseJmpRel32;
        MemWriteRomData(
            pInstallHookContext->pHookContext->pTrampolineBase,
            TrampolineBody,
            6 + pInstallHookContext->pHookContext->NopPaddingSize);

    aborted2:

        if (pInstallHookContext->ActiveLogicalCoreCount > 1) {
            while (pInstallHookContext->CoreSyncBarrier[1] != 1) {
                _mm_pause();
            }
        }

        _InterlockedExchange(
            (volatile LONG*)&pInstallHookContext->CoreSyncLock,
            0);
    }

aborted:

    HkEnterBarrierGateIpi(&pInstallHookContext->CoreSyncBarrier[1]);
  
    return;
}

#define HOOK_INSTALL_ATTEMPT_MAXCOUNT 64

HR_STATUS
FASTCALL
HkInstallRoutineHook(
    IN VOID *pRoutineAddress,
    IN BOOLEAN IsEpilogue,
    IN VOID *pHookRoutine,
    IN HR_CONTEXT *pHunterContext
)
/*++
* Routine Description:
*
*     This routine installs a hook in the
*     specified routine.
*
* Arguments:
*
*     pRoutineAddress - Supplies a pointer to the
*                       target routine address.
*
*     IsEpilogue      - Supplies the
*                       epilogue hook flag.
*
*     pHookRoutine    - Supplies a pointer to the
*                       hook routine.
*
*     pHunterContext  - Supplies a pointer to the
*                       HR_CONTEXT structure.
*
* Return Value:
*
*     Internal status.
*
--*/
{
    HOOK_CONTEXT HookContext = { 0 };
    INSTALL_HOOK_CONTEXT InstallHookContext = { 0 };
    HOOK_ASM_STUB_INFO HookAsmStubInfo = { 0 };

    UINT32 Seed = 0;

    UINT16 StubLowPaddingSize = 0;
    UINT16 StubHighPaddingSize = 0;
    UINT32 *pStubLowPaddingBase = NULL;
    UINT32 *pStubHighPaddingBase = NULL;

    VOID *pStubBase = NULL;

    UINT16 StubHrCtxLowPaddingSize = 0;
    UINT16 StubHrCtxHighPaddingSize = 0;
    UINT32 *pStubHrCtxLowPaddingBase = NULL;
    UINT32 *pStubHrCtxHighPaddingBase = NULL;

    HR_CONTEXT *pStubHunterContext = NULL;

    VOID *pPatternBase = NULL;

    UINT8 TrampolineReturnBody[14] = {
        0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    };

    BOOLEAN IsAborted = TRUE;

    if (!pHunterContext) {
        DBG_BREAK;
        goto aborted;
    }

    if (HR_ERROR(HkInitHookContext(
        pRoutineAddress,
        IsEpilogue,
        &HookContext,
        pHunterContext))) {
        DBG_BREAK;
        goto aborted;
    }

    if (HR_ERROR(HkGetHookAsmStubRangeAsm64(&HookAsmStubInfo))) {
        DBG_BREAK;
        goto aborted;
    }

    if (HR_ERROR(PeFindSectionMemoryPattern(
        pHunterContext->NTOS_PROCESS.NTOS_IMAGE.pImageBase,
        QUICK_XOR64(NTOS_TEXT_SECTION_NAME),
        TRUE,
        (UINT8*)"\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC",
        (UINT8*)_sx _sx _sx _sx _sx _sx _sx _sx _sx _sx,
        &pPatternBase))) {
        DBG_BREAK;
        goto aborted;
    }

    if (!pPatternBase) {
        DBG_BREAK;
        goto aborted;
    }

    pPatternBase = ((UINT8*)pPatternBase) + 1;

    StubLowPaddingSize = 
        (UINT16)pHunterContext->HR_API.pRtlRandomEx(&Seed);
    StubHighPaddingSize = 
        (UINT16)pHunterContext->HR_API.pRtlRandomEx(&Seed);

    StubLowPaddingSize = 
        ((StubLowPaddingSize & 0x3FF) + 0x401) & ~0x0F;
    StubHighPaddingSize = 
        ((StubHighPaddingSize & 0x3FF) + 0x401) & ~0x0F;

    if (!(pStubLowPaddingBase =
        pHunterContext->HR_API.pMmAllocateIndependentPagesEx(
            REQUIRED_NUMBER_OF_PAGES(StubLowPaddingSize +
            HookAsmStubInfo.StubSize +
            HookContext.RestoreInstructionsSize +
            sizeof(TrampolineReturnBody) +
            StubHighPaddingSize)
            << PAGE_SHIFT,
            (UINT32)-1,
            NULL,
            0))) {
        DBG_BREAK;
        goto aborted;
    }

    pStubBase = ((UINT8*)pStubLowPaddingBase) + StubLowPaddingSize;

    pStubHighPaddingBase = (UINT32*)(((UINT8*)pStubBase) +
        HookAsmStubInfo.StubSize +
        HookContext.RestoreInstructionsSize +
        sizeof(TrampolineReturnBody));

    pHunterContext->HR_API.pRtlRandomEx(&Seed);
    if (HR_ERROR(CryFillBufferRandomDword(
        pStubLowPaddingBase,
        (StubLowPaddingSize / 4),
        Seed))) {
        DBG_BREAK;
        goto aborted;
    }

    pHunterContext->HR_API.pRtlRandomEx(&Seed);
    if (HR_ERROR(CryFillBufferRandomDword(
        pStubHighPaddingBase,
        (StubHighPaddingSize / 4),
        Seed))) {
        DBG_BREAK;
        goto aborted;
    }

    memmove(
        pStubBase,
        HookAsmStubInfo.pStubBase,
        HookAsmStubInfo.StubSize);

    StubHrCtxLowPaddingSize = 
        (UINT16)pHunterContext->HR_API.pRtlRandomEx(&Seed);
    StubHrCtxHighPaddingSize = 
        (UINT16)pHunterContext->HR_API.pRtlRandomEx(&Seed);

    StubHrCtxLowPaddingSize = 
        ((StubHrCtxLowPaddingSize & 0x3FF) + 0x401) & ~0x07;
    StubHrCtxHighPaddingSize = 
        ((StubHrCtxHighPaddingSize & 0x3FF) + 0x401) & ~0x07;

    if (!(pStubHrCtxLowPaddingBase =
        pHunterContext->HR_API.pMmAllocateIndependentPagesEx(
            REQUIRED_NUMBER_OF_PAGES(
            StubHrCtxLowPaddingSize +
            sizeof(HR_CONTEXT) +
            StubHrCtxHighPaddingSize)
            << PAGE_SHIFT,
            (UINT32)-1,
            NULL,
            0))) {
        DBG_BREAK;
        goto aborted;
    }

    pStubHunterContext = (HR_CONTEXT*)(((UINT8*)pStubHrCtxLowPaddingBase) +
        StubHrCtxLowPaddingSize);

    pStubHrCtxHighPaddingBase = (UINT32*)(pStubHunterContext + 1);

    pHunterContext->HR_API.pRtlRandomEx(&Seed);
    if (HR_ERROR(CryFillBufferRandomDword(
        pStubHrCtxLowPaddingBase,
        (StubHrCtxLowPaddingSize / 4),
        Seed))) {
        DBG_BREAK;
        goto aborted;
    }

    pHunterContext->HR_API.pRtlRandomEx(&Seed);
    if (HR_ERROR(CryFillBufferRandomDword(
        pStubHrCtxHighPaddingBase,
        (StubHrCtxHighPaddingSize / 4),
        Seed))) {
        DBG_BREAK;
        goto aborted;
    }

    memmove(
        pStubHunterContext,
        pHunterContext,
        sizeof(HR_CONTEXT));

    pStubHunterContext->ContextSeed =
        ((UINT64)pStubHunterContext->HR_API.pRtlRandomEx(&Seed)) << 32;
        
    pStubHunterContext->ContextSeed +=
        ((UINT64)pStubHunterContext->HR_API.pRtlRandomEx(&Seed));

    if (HR_ERROR(CryCrc32DataHash(
        (UINT8*)&pStubHunterContext->ContextSeed,
        sizeof(HR_CONTEXT) - FIELD_OFFSET(HR_CONTEXT, ContextSeed),
        &pStubHunterContext->ContextHash32))) {
        DBG_BREAK;
        goto aborted;
    }

    pStubHunterContext->ContextHash32 =
        QUICK_XOR32(pStubHunterContext->ContextHash32);

    if (HR_ERROR(CryDiffusionDataFlow(
        (UINT8*)pStubHunterContext,
        sizeof(HR_CONTEXT)))) {
        DBG_BREAK;
        goto aborted;
    }

    if (NT_ERROR(pHunterContext->HR_API.pMmSetPageProtection(
        pStubHrCtxLowPaddingBase,
        REQUIRED_NUMBER_OF_PAGES(
        StubHrCtxLowPaddingSize +
        sizeof(HR_CONTEXT) +
        StubHrCtxHighPaddingSize)
        << PAGE_SHIFT,
        PAGE_READONLY))) {
        DBG_BREAK;
        goto aborted;
    }

    *(VOID**)(((UINT8*)pStubBase) +
        HookAsmStubInfo.OffsetTrampolineBasePtr) =
        (VOID*)QUICK_XOR64((UINT64)HookContext.pTrampolineBase);
    *(VOID**)(((UINT8*)pStubBase) +
        HookAsmStubInfo.OffsetHunterContextPtr) =
        (VOID*)QUICK_XOR64((UINT64)pStubHunterContext);
    *(VOID**)(((UINT8*)pStubBase) +
        HookAsmStubInfo.OffsetHookRoutinePtr) =
        pHookRoutine;

    *(UINT64*)(TrampolineReturnBody + 6) =
        (UINT64)((((UINT8*)HookContext.pTrampolineBase) + 6) +
            HookContext.NopPaddingSize);

    memmove(
        ((UINT8*)pStubBase) + 
        HookAsmStubInfo.StubSize,
        HookContext.RestoreInstructions,
        HookContext.RestoreInstructionsSize);
    memmove(
        ((UINT8*)pStubBase) +
        HookAsmStubInfo.StubSize +
        HookContext.RestoreInstructionsSize,
        TrampolineReturnBody,
        sizeof(TrampolineReturnBody));

    if (HR_ERROR(MemWriteRomData(
        (UINT8*)pPatternBase,
        (VOID*)&pStubBase,
        8))) {
        DBG_BREAK;
        goto aborted;
    }

    if (NT_ERROR(pHunterContext->HR_API.pMmSetPageProtection(
        pStubLowPaddingBase,
        REQUIRED_NUMBER_OF_PAGES(
        StubLowPaddingSize +
        HookAsmStubInfo.StubSize +
        HookContext.RestoreInstructionsSize +
        sizeof(TrampolineReturnBody) +
        StubHighPaddingSize)
        << PAGE_SHIFT,
        PAGE_EXECUTE_READ))) {
        DBG_BREAK;
        goto aborted;
    }

    InstallHookContext.pHunterContext = pHunterContext;
    InstallHookContext.pHookContext = &HookContext;
    InstallHookContext.pStubBase = pPatternBase;
    InstallHookContext.ActiveLogicalCoreCount =
        pHunterContext->HR_API.pKeQueryActiveProcessorCountEx(
            ALL_PROCESSOR_GROUPS);

    for (UINT8 i = 0; i < HOOK_INSTALL_ATTEMPT_MAXCOUNT; i++) {
        for (UINT8 i2 = 0;
            i2 < (sizeof(InstallHookContext.CoreSyncBarrier) / 4); i2++) {
            InstallHookContext.CoreSyncBarrier[i2] =
                InstallHookContext.ActiveLogicalCoreCount;
        }
        InstallHookContext.Status = HR_SUCCESS;
        pHunterContext->HR_API.pKeIpiGenericCall(
            (KIPI_BROADCAST_WORKER*)&HkInstallRoutineHookIpi,
            (UINT64)&InstallHookContext);
        if (HR_ERROR(InstallHookContext.Status)) {
            continue;
        } else {
            break;
        }
    }

    if (HR_ERROR(InstallHookContext.Status)) {
        DBG_BREAK;
        goto aborted;
    }

    IsAborted = FALSE;

aborted:

    if (IsAborted) {
        if (pHunterContext) {
            if (pStubBase) {
                RtlSecureZeroMemory(
                    pStubLowPaddingBase,
                    (StubLowPaddingSize +
                    HookAsmStubInfo.StubSize +
                    HookContext.RestoreInstructionsSize +
                    sizeof(TrampolineReturnBody) +
                    StubHighPaddingSize));
                pHunterContext->HR_API.pMmFreeIndependentPages(
                    pStubLowPaddingBase,
                    REQUIRED_NUMBER_OF_PAGES(
                    StubLowPaddingSize +
                    HookAsmStubInfo.StubSize +
                    HookContext.RestoreInstructionsSize +
                    sizeof(TrampolineReturnBody) +
                    StubHighPaddingSize)
                    << PAGE_SHIFT);
            }
            if (pStubHunterContext) {
                RtlSecureZeroMemory(
                    pStubHrCtxLowPaddingBase,
                    (StubHrCtxLowPaddingSize +
                    sizeof(HR_CONTEXT) +
                    StubHrCtxHighPaddingSize));
                pHunterContext->HR_API.pMmFreeIndependentPages(
                    pStubHrCtxLowPaddingBase,
                    REQUIRED_NUMBER_OF_PAGES(
                    (StubHrCtxLowPaddingSize +
                    sizeof(HR_CONTEXT) +
                    StubHrCtxHighPaddingSize))
                    << PAGE_SHIFT);
            }
        }
        return HR_ABORTED;
    }

    return HR_SUCCESS;
}

