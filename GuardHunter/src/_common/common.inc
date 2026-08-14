IFNDEF COMMON_INC
    COMMON_INC equ 1

    DBG equ 1

    HR_SUCCESS equ 002FC9E0Ah
    HR_ABORTED equ 0A9F34231h

    IF DBG
        SS_PROTECT MACRO src
        ENDM
        DR7_PROTECT MACRO src
        ENDM
    ELSE
        SS_PROTECT MACRO src
            mov     ss,src
        ENDM
        DR7_PROTECT MACRO src
            mov     dr7,src
        ENDM
    ENDIF

ENDIF