INCLUDE common.inc

if 0

/*++
* Module Name: 
*
*     hookasm.asm
*
* Abstract:
*
*     This module contains ASM64 routines for hooks.
*
* Author:
* 
*     quokka867 (GitHub/Twitter).
*
--*/

endif 

_TEXT segment PARA 'CODE'

IF DBG
    SS_PROTECT MACRO src
        mov     ss,src
    ENDM
    DR7_PROTECT MACRO src
        mov     dr7,src
    ENDM
ELSE
    SS_PROTECT MACRO src
    ENDM
    DR7_PROTECT MACRO src
    ENDM
ENDIF

; ***
; * Routine Description:
; *
; *     This routine initializes HOOK_ASM_STUB_INFO.
; *
; * Arguments:
; *
; *     rcx - Supplies a pointer to the
; *           HOOK_ASM_STUB_INFO structure.
; *
; * Return Value:
; *
; *     Internal status.
; *
; ***
public HkGetHookAsmStubRangeAsm64
HkGetHookAsmStubRangeAsm64 proc
        test    rcx,rcx
        jnz     short success

        mov     eax,HR_ABORTED
        jmp     exit
success:
        lea     rax,[stub_base]

        mov     qword ptr [rcx],rax

        lea     rdx,[stub_end]
        sub     rdx,rax
        mov     word ptr [rcx + 8],dx

        lea     rdx,[trampoline_base_ptr]
        add     rdx,2
        sub     rdx,rax
        mov     word ptr [rcx + 10],dx

        lea     rdx,[hunter_context_ptr]
        add     rdx,2
        sub     rdx,rax
        mov     word ptr [rcx + 12],dx

        lea     rdx,[hook_routine_ptr]
        add     rdx,2
        sub     rdx,rax
        mov     word ptr [rcx + 14],dx

        mov     eax,HR_SUCCESS
exit:
        ret
stub_base:
        push    rax
        mov     rax,rsp
        and     rsp,0FFFFFFFFFFFFFFF0h
        sub     rsp,8
        push    rax
        push    rcx
        push    rdx
        push    r8
        push    r9
        sub     rsp,64
        movups  xmmword ptr [rsp + 48],xmm0
        movups  xmmword ptr [rsp + 32],xmm1
        movups  xmmword ptr [rsp + 16],xmm2
        movups  xmmword ptr [rsp],xmm3
        push    rbp
        push    rax
        add     qword ptr [rsp],8
        sub     rsp,16
trampoline_base_ptr:
        mov     rax,0AAAAAAAAAAAAAAAAh 
        mov     qword ptr [rsp + 8],rax
hunter_context_ptr:
        mov     rax,0AAAAAAAAAAAAAAAAh 
        mov     qword ptr [rsp],rax
hook_routine_ptr:
        mov     rax,0AAAAAAAAAAAAAAAAh 
        mov     rcx,rsp
        sub     rsp,32
;
; The callee (RAX) must preserve all non-volatile registers
; that it modifies.
;
        call    rax

        add     rsp,56
        pop     rbp
        movups  xmm3,xmmword ptr [rsp]
        movups  xmm2,xmmword ptr [rsp + 16]
        movups  xmm1,xmmword ptr [rsp + 32]
        movups  xmm0,xmmword ptr [rsp + 48]
        add     rsp,64
        pop     r9
        pop     r8
        pop     rdx
        pop     rcx
        pop     rax
        mov     rsp,rax
        pop     rax
stub_end:
        nop
;       jmp     qword ptr [rip]
;       db 0AAh 0AAh 0AAh 0AAh 0AAh 0AAh 0AAh 0AAh
HkGetHookAsmStubRangeAsm64 endp

; ***
; * Routine Description:
; *
; *     This routine clears the execution context of the
; *     current logical core and initiates a jmp to the
; *     specified address.
; *
; * Arguments:
; *
; *     rcx - Supplies the internal bug status.
; *
; *     rdx - Supplies a pointer to the
; *           current stack high limit.
; *
; *     r8  - Supplies a pointer to the
; *           current stack low limit.
; *
; * Return Value:
; *
; *     None.
; *
; ***
public HkCustomBugCheckAsm64
HkCustomBugCheckAsm64 proc
        cli

        mov     r14w,ss

        SS_PROTECT r14w
        xor     eax,eax

        SS_PROTECT r14w
        DR7_PROTECT rax

        SS_PROTECT r14w
        mov     rbx,rdx

        SS_PROTECT r14w
        sub     rbx,rsp

        SS_PROTECT r14w
        mov     rdi,rsp

        SS_PROTECT r14w
        lea     rsp,[rdx - 56]  

        SS_PROTECT r14w
        shr     ebx,3

        SS_PROTECT r14w
        xor     eax,eax

        SS_PROTECT r14w
        mov     r15d,ecx

        SS_PROTECT r14w
        mov     ecx,ebx

        SS_PROTECT r14w
        cld

        SS_PROTECT r14w
        rep     stosq

        SS_PROTECT r14w
        xor     ebx,ebx

        SS_PROTECT r14w
        xor     edx,edx

        SS_PROTECT r14w
        xor     edi,edi

        SS_PROTECT r14w
        xor     esi,esi

        SS_PROTECT r14w
        xor     ebp,ebp

        SS_PROTECT r14w
        mov     rax,r8

        SS_PROTECT r14w
        xor     r8d,r8d

        SS_PROTECT r14w
        xor     r9d,r9d

        SS_PROTECT r14w
        xor     r10d,r10d

        SS_PROTECT r14w
        xor     r11d,r11d

        SS_PROTECT r14w
        xor     r12d,r12d

        SS_PROTECT r14w
        xor     r13d,r13d

        SS_PROTECT r14w
        mov     ecx,r15d

        SS_PROTECT r14w
        xor     r15d,r15d

        SS_PROTECT r14w
        jmp     rax
HkCustomBugCheckAsm64 endp


; ***
; * Routine Description:
; *
; *     This routine performs a safe RSP switch by restoring the
; *     states of the non-volatile registers.
; *
; * Arguments:
; *
; *     rcx - Supplies the new RSP.
; *
; *     rdx - Supplies a pointer to the
; *           CONTEXT structure.
; *
; * Return Value:
; *
; *     None.
; *
; ***
public HkSetRspSafeAsm64
HkSetRspSafeAsm64 proc
        mov     rbx,qword ptr [rdx + 090h]
        mov     rbp,qword ptr [rdx + 0A0h]
        mov     rsi,qword ptr [rdx + 0A8h]
        mov     rdi,qword ptr [rdx + 0B0h]
        mov     r12,qword ptr [rdx + 0D8h]
        mov     r13,qword ptr [rdx + 0E0h]
        mov     r14,qword ptr [rdx + 0E8h]
        mov     r15,qword ptr [rdx + 0F0h]

        movups  xmm6,xmmword ptr [rdx + 0200h]
        movups  xmm7,xmmword ptr [rdx + 0210h]
        movups  xmm8,xmmword ptr [rdx + 0220h]
        movups  xmm9,xmmword ptr [rdx + 0230h]
        movups  xmm10,xmmword ptr [rdx + 0240h]
        movups  xmm11,xmmword ptr [rdx + 0250h]
        movups  xmm12,xmmword ptr [rdx + 0260h]
        movups  xmm13,xmmword ptr [rdx + 0270h]
        movups  xmm14,xmmword ptr [rdx + 0280h]
        movups  xmm15,xmmword ptr [rdx + 0290h]

        mov rsp,rcx
        ret
HkSetRspSafeAsm64 endp

_TEXT ends

end