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
data_ptr:

        mov     rcx,0AAAAAAAAAAAAAAAAh

qword_count:

        mov     edx,0AAAAAAAAh

        mov     r8,qword ptr [xor_key]
        bswap   r8

decrypt_loop:

        xor     qword ptr [rcx],r8
        lea     rcx,[rcx + 8]
        dec     edx
        jnz     short decrypt_loop

        xor     r8d,r8d

        lea     rcx,[decrypt_exit]
        lfence
        jmp     rcx

xor_key:

        db 0AAh, 0AAh, 0AAh, 0AAh, 0AAh, 0AAh, 0AAh, 0AAh

decrypt_exit:

        lea     rcx,[stub_base]
        lea     rdx,[cleanup_loop]

cleanup_loop:

        xor     byte ptr [rcx],cl
        lea     rcx,[rcx + 1]
        cmp     rcx,rdx
        jnz     short cleanup_loop

        lfence
        ret

stub_end:

        nop
HrGetDecryptAsmStubInfoAsm64 endp

_TEXT ends

END