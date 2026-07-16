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
        sub     rsp,40
        lea     rax,[HrStart]
        lea     rcx,[call_bs_main]
        push    rcx
        inc     qword ptr [rsp]
        sub     rcx,rax
        add     rax,rcx
        jmp     rax
        ret
call_bs_main:
        ret
        call    BsMain
        add     rsp,40
        mov     rax,0FFFFFFFFFFFFFFFFh
        ret  
HrStart endp

_TEXT ends

end