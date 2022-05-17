;**********************************************************************;
;*                                                                    *;
;* Intel Proprietary                                                  *;
;*                                                                    *;
;* Copyright 2021 Intel Corporation All Rights Reserved.              *;
;*                                                                    *;
;* Your use of this software is governed by the TDX Source Code       *;
;* LIMITED USE LICENSE.                                               *;
;*                                                                    *;
;* The Materials are provided "as is," without any express or         *;
;* implied warranty of any kind including warranties of               *;
;* merchantability, non-infringement, title, or fitness for a         *;
;* particular purpose.                                                *;
;*                                                                    *;
;**********************************************************************;


	.XLIST
	include common.inc
	include	common64.inc
	include	entry64.equ
    include acmcom64.equ
	.LIST

	.DATA
	ALIGN 16

	.CODE

        ALIGN 16

               
;-----------------------------------------------------------------------------
;-----------------------------------------------------------------------------
;  Procedure:	Entry64
;
;  Input:	None
;
;  Output:	None
;
;  Description:	Entry procedure of 64-bit code.
;
;-----------------------------------------------------------------------------
;-----------------------------------------------------------------------------
Entry64 PROC FRAME
        START_FRAME
        MAKE_LOCAL   pCom64data:QWORD
        MAKE_LOCAL   pIdt64[2]:QWORD
        END_FRAME

        ;
        ; Save entry parameter
        ;
        mov     pCom64data, rcx
        
        mov     rcx, pCom64data
        ; backup registers
        mov     QWORD PTR [rcx].SEAMLDR_COM64_DATA.OriginalR8, r8
        mov     QWORD PTR [rcx].SEAMLDR_COM64_DATA.OriginalR9, r9
        mov     QWORD PTR [rcx].SEAMLDR_COM64_DATA.OriginalR10, r10
        mov     QWORD PTR [rcx].SEAMLDR_COM64_DATA.OriginalR11, r11
        mov     QWORD PTR [rcx].SEAMLDR_COM64_DATA.OriginalR12, r12

        ; zero non-input registers
        mov     r13, 0
        mov     r14, 0
        mov     r15, 0

        ; Align the stack pointer to 16-byte before running the main 64-bit code - required from proper crypto usage
        and     rsp, 0fffffff0h
        sub     rsp, 20h
        
        call    Target64
        
        ;; SEAMLDR64 finished running here.
        ;; Exit procedure:
        
        mov     rcx, pCom64data
        
        
        lgdt    FWORD PTR [rcx].SEAMLDR_COM64_DATA.NewGdtr
        
        
        mov     ax, WORD PTR [rcx].SEAMLDR_COM64_DATA.OriginalES
        mov     es, ax
        mov     ds, ax
        mov     ax, WORD PTR [rcx].SEAMLDR_COM64_DATA.OriginalFS
        mov     fs, ax
        mov     ax, WORD PTR [rcx].SEAMLDR_COM64_DATA.OriginalGS
        mov     gs, ax        
        mov     ax, WORD PTR [rcx].SEAMLDR_COM64_DATA.OriginalSS
        mov     ss, ax        
        
        mov     eax, DWORD PTR [rcx].SEAMLDR_COM64_DATA.OriginalECX        
        push    rax
        mov     rax,  OFFSET  _restored_cs
        push    rax

        retfq   
        _restored_cs:
        
        lgdt    FWORD PTR [rcx].SEAMLDR_COM64_DATA.OriginalGdtr
        
        ;; turn off PCIDE bit in current CR4
        mov     rdx, cr4
        and     edx, (NOT 020000h)
        mov     cr4, rdx 

        xor     rdx, rdx
        mov     edx, DWORD PTR [rcx].SEAMLDR_COM64_DATA.OriginalCR4

        ;; Obtain the saved OS CR4 and retain following bits - LA57, and PGE. Mask off all other bits to 0 and set the SMXE bit to 1.
        ;; PAE should be set regardless of whether it was set originally or not.
        and     edx, (CR4_LA57 OR CR4_PGE)
        or      edx, (CR4_SMXE OR CR4_PAE)
        mov     cr4, rdx
        
        mov     rdx, cr0
        and     edx, (not CR0_WP)
        mov     cr0, rdx
        
DoExitAC:

        ;; Parameters for EXITAC
        mov     rbx, QWORD PTR [rcx].SEAMLDR_COM64_DATA.ResumeRip          ; uCode restores the RIP from RBX during EXITAC
        mov     r8, QWORD PTR [rcx].SEAMLDR_COM64_DATA.OriginalCR3         ; uCode restores the CR3 from R8 during EXITAC
        mov     r9, QWORD PTR [rcx].SEAMLDR_COM64_DATA.RetVal              ; SEAMLDR Error code is reported in R9
        mov     rdx, 0                                                     ; Clear all flags
        mov     rax, EXITAC                                                ; Do ExitAC
        
        push    2
        popfq

        lidt    FWORD PTR [rcx].SEAMLDR_COM64_DATA.NewIDTR        
     

        ;; Clear other registers as described in spec - not xor to avoid changing flags
        mov     rcx, 0
        mov     rbp, 0
        mov     rsp, 0
        mov     r10, 0
        mov     r11, 0
        mov     r12, 0
        mov     r13, 0
        mov     r14, 0
        mov     r15, 0

        ; REX.W EXITAC to exit AC mode.
        db 48h
        _GETSEC
        
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ;;;;;;;;;;;;;;;;; SHOULD NOT RETURN HERE AFTER GETSEC ;;;;;;;;;;;;;;;;;
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ;; Cannot use ud2 -> triple fault for TXT shutdown here, because OS
        ;; IDT is already restored at that point
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        
        jmp     $

Entry64 ENDP


END
