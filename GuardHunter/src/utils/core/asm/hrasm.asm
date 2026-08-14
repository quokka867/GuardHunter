INCLUDE common.inc

if 0

/*++
* Module Name: 
*
*     hrasm.asm
*
* Abstract:
*
*     This module contains ASM64 routines for hr-module.
*
* Author:
* 
*     quokka867 (GitHub/Twitter).
*
--*/

endif

SyscallXorID equ 050087AD7h
SyscallExitID equ 000000000h

_TEXT segment PARA 'CODE'

; ***
; * Routine Description:
; *
; *     This routine initializes DECRYPT_ASM_STUB_INFO.
; *
; * Arguments:
; *
; *     Rcx - Supplies a pointer to the
; *           DECRYPT_ASM_STUB_INFO structure.
; *
; * Return Value:
; *
; *     Internal status.
; *
; ***
public HrGetDecryptAsmStubInfoAsm64
HrGetDecryptAsmStubInfoAsm64 proc 
        test    rcx,rcx
        jnz     short success

        mov     eax,HR_ABORTED
        jmp     short exit

success:

        lea     rax,[stub_base]

        mov     qword ptr [rcx],rax

        lea     rdx,[stub_end]
        sub     rdx,rax
        mov     word ptr [rcx + 8],dx

        lea     rdx,[data_ptr]
        add     rdx,2
        sub     rdx,rax
        mov     word ptr [rcx + 10],dx

        lea     rdx,[qword_count]
        add     rdx,1
        sub     rdx,rax
        mov     word ptr [rcx + 12],dx

        lea     rdx,[xor_key]
        sub     rdx,rax
        mov     word ptr [rcx + 14],dx
        
        mov     eax,HR_SUCCESS

exit:

        ret

stub_base:

        cli

        mov     r9w,ss

        SS_PROTECT r9w
        xor     eax,eax

        SS_PROTECT r9w
        DR7_PROTECT rax

        SS_PROTECT r9w
        push    rdi

        SS_PROTECT r9w
        xor     r10d,r10d

        SS_PROTECT r9w
        rdtsc

        SS_PROTECT r9w
        shr     rdx,32

        SS_PROTECT r9w
        lea     rax,[rax + rdx]

        SS_PROTECT r9w
        push    rax

        SS_PROTECT r9w
        rol     rax,32

        SS_PROTECT r9w
        push    rax

        SS_PROTECT r9w
        movups  xmm0,xmmword ptr [rsp]

        SS_PROTECT r9w
        lea     rsp,[rsp + 16]

        SS_PROTECT r9w
        mov     ecx,0C0000082h

        SS_PROTECT r9w
        rdmsr

        SS_PROTECT r9w
        shl     rdx,32

        SS_PROTECT r9w
        lea     r10,[r10 + rdx]

        SS_PROTECT r9w
        lea     r10,[r10 + rax]

        SS_PROTECT r9w
        xor     edx,edx

        SS_PROTECT r9w
        xor     eax,eax

        SS_PROTECT r9w
        lea     rdx,[syscall_base]

        SS_PROTECT r9w
        lea     rax,[rax + rdx]  

        SS_PROTECT r9w
        shr     rdx,32

        SS_PROTECT r9w
        wrmsr

        SS_PROTECT r9w
        lea     rcx,[decrypt_start]

        SS_PROTECT r9w
        jmp     rcx

syscall_base:

        SS_PROTECT r9w
        cmp     dword ptr [rsp],SyscallXorID

        SS_PROTECT r9w
        jnz     short syscall_exit

syscall_xor:

        SS_PROTECT r9w
        xor     qword ptr [rax],r8

        SS_PROTECT r9w
        jmp     short syscall_exit

        SS_PROTECT r9w
        ret

syscall_exit:

        SS_PROTECT r9w
        jmp     rcx

decrypt_start:

        SS_PROTECT r9w
        push    SyscallExitID

        SS_PROTECT r9w
        syscall

        SS_PROTECT r9w
        shr     r11w,8

        SS_PROTECT r9w
        test    r11w,1

        SS_PROTECT r9w
        jz      short tf_skip

tf_loop:

        SS_PROTECT r9w
        jmp     short tf_loop

tf_skip:

        SS_PROTECT r9w
data_ptr:
        mov     rax,0AAAAAAAAAAAAAAAAh

        SS_PROTECT r9w
qword_count:
        mov     edx,0AAAAAAAAh

        SS_PROTECT r9w
        mov     r8,qword ptr [xor_key]

        SS_PROTECT r9w
        bswap   r8

        SS_PROTECT r9w
        push    SyscallXorID

xor_loop:

        SS_PROTECT r9w
        syscall

        SS_PROTECT r9w
        rol     r8,8

        SS_PROTECT r9w
        lea     rax,[rax + 8]

        SS_PROTECT r9w
        dec     edx

        SS_PROTECT r9w
        jnz     short xor_loop

        SS_PROTECT r9w
        push    SyscallExitID

        SS_PROTECT r9w
        xor     r8d,r8d

        SS_PROTECT r9w
        mov     qword ptr [xor_key],r8

        SS_PROTECT r9w
        syscall

        SS_PROTECT r9w
        lea     rcx,[decrypt_exit]

        SS_PROTECT r9w
        jmp     rcx

xor_key:

        db 0AAh, 0AAh, 0AAh, 0AAh, 0AAh, 0AAh, 0AAh, 0AAh

decrypt_exit:

        SS_PROTECT r9w
        xor     edx,edx

        SS_PROTECT r9w
        xor     eax,eax

        SS_PROTECT r9w
        lea     rdx,[rdx + r10]

        SS_PROTECT r9w
        shr     rdx,32

        SS_PROTECT r9w
        lea     rax,[rax + r10]

        SS_PROTECT r9w
        syscall

        SS_PROTECT r9w
        mov     ecx,0C0000082h

        SS_PROTECT r9w
        wrmsr

        SS_PROTECT r9w
        lea     rdi,[stub_base + 1]

        SS_PROTECT r9w
        xor     eax,eax

        SS_PROTECT r9w
        lea     rcx,[cleanup_end]

        SS_PROTECT r9w
        sub     rcx,rdi

        SS_PROTECT r9w
        mov     byte ptr [rdi - 1],0C3h

        SS_PROTECT r9w
        lea     rsp,[rsp + 24]

        SS_PROTECT r9w
        sti

        SS_PROTECT r9w
        cld

        SS_PROTECT r9w
        rep     stosb

cleanup_end:

        pop     rdi

        ret

stub_end:

        nop
HrGetDecryptAsmStubInfoAsm64 endp

_TEXT ends

end