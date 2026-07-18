if 0

/*++
* Module Name: 
*
*     main.asm
*
* Abstract:
*
*     This module contains the image-module entry point.
*
* Author:
* 
*     quokka867 (GitHub/Twitter).
*
--*/

endif 

extern __ImageBase:qword

extern BsMain:proc

_TEXT segment PARA 'CODE'

; ***
; * Routine Description:
; *
; *     This routine serves as the entry point into the image.
; *
; * Arguments:
; *
; *     None.
; *
; * Return Value:
; *
; *     Pointer to the hunter export table, or NULL.
; *
; ***
public HrStart
HrStart proc
        lea     rax,[__ImageBase]
        lea     rcx,[call_bs_main - 0AEC7h]
        push    rcx
        add     qword ptr [rsp],0AEC8h
        sub     rcx,rax
        add     rax,rcx
        lea     rax,[rax + 0AEC7h]
        jmp     rax
        ret
call_bs_main:
        ret
        lea     rax,[__ImageBase]
        lea     rcx,[BsMain - 02AE8h]
        sub     rcx,rax
        add     rax,rcx
        push    rax
        add     qword ptr [rsp],02AE8h
        ret  
HrStart endp

_TEXT ends

end