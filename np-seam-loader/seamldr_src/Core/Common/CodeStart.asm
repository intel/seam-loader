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

.586p
;---------------------------------------------------------------------------
	OPTION LANGUAGE:C

	.XLIST
        include common.inc

        IFDEF MKF_USE_MAKEFLAG_H
        include makeflag.inc
        ENDIF

        include acmcom.equ
        include header.inc
	.LIST
        
EDATA32	SEGMENT	PUBLIC USE32
	ASSUME	ds:EDATA32

        ALIGN 16

        CodeSegmentStart    dd  00 ; size of ACM data + stack

        OriginalEIP     dd  00h
        OriginalEAX     dd  00h
        OriginalEBX     dd  00h
        OriginalECX     dd  00h
        OriginalESI     dd  00h
        OriginalEDI     dd  00h
        OriginalEDX     dd  00h
        OriginalCR3     dd  00h
        OriginalCR4     dd  00h
        OriginalIDTRLimit dw  00h         ; Original IDTR Limit
        OriginalES      dw 00h
        OriginalFS      dw 00h
        OriginalGS      dw 00h
        OriginalSS      dw 00h        
        
        PUBLIC R_ACM        
        R_ACM           dq  LBL_ACM
                        dw  0
                        dw  RFA_VALID
        AcmSize         dd  00h         ; Size of currently running ACM
                        dd  0
        AcmBase         dd  00h         ; Base address of where ACM is in memory
                        dd  0
        AcmTop          dd  00h         ; End address of ACM in memory
                        dd  0   
                        dq  _4KB_MASK
             
 EDATA32 ENDS

        
;----------------------------------------------------------------------------
;       C O D E     S E G M E N T
;----------------------------------------------------------------------------
_TEXT  SEGMENT 'CODE' USE32 BYTE
        ;
        ; Don't destroy the following values.
        ; ACM_PE2BIN tool reads [sinitAcmStart - 8] and interprets it
        ; as offset of image where to store relocation table
        ; ACM_PE2BIN tool stores offset of the beginning of CODE32 segment in
        ; [sinitAcmStart - 4] for ACM code use.
        ; Initializing of [sinitAcmStart - x] = offset HeaderStart is
        ; equivalent to initializing it as [sinitAcmStart - x] = 0 but with
        ; side effect that offset of sinitAcmStart - x is inserted into
        ; relocation table and is fixed-up by fix-up loop.
        ; Code makes use of this effect.
        ;                                            
        COM_DATA < \
                OFFSET HeaderStart, \
                OFFSET HeaderStart, \
                OFFSET HeaderStart, \
                OFFSET HeaderStart, \
                OFFSET stackStart,  \
                OFFSET HeaderStart \
                >

;-----------------------------------------------------------------------------
;-----------------------------------------------------------------------------
;  Procedure:	AcmEntryPoint
; 
;  Input:	State after GETSEC[SENTER|ENTERACCS] instruction
; 
;  Output:	None
; 
;  Description:	Performs fix-ups; transfers control to project-specific
;               entry point
; 
;-----------------------------------------------------------------------------
;-----------------------------------------------------------------------------
AcmEntryPoint PROC NEAR
        nop
        nop
        nop

        ; Right after ENTERACCS, SEAMLDR 32-bit assembly code will do the following:
        ; SIDT saved_OS_IDTR
        ; LIDT null_IDTR // a 48-bit variable that contains 0's

        sidt    fword ptr ds:[ebp + stackStart + 4*6]

        mov     dword ptr ds:[ebp + stackStart + 4], 0       ; Make sure that Null IDTR is actually zero
        mov     dword ptr ds:[ebp + stackStart + 4 + 4], 0
        lidt    fword ptr ds:[ebp + stackStart + 4]          ; Load NULL IDTR
        
        cmp     eax, ENTERACCS
        jz     _IS_ENTERACCS        
        db      0Fh, 0Bh                ; ud2        
        _IS_ENTERACCS:                

        ;
        ; Save registers to Fixup scratch area (size of 40h bytes reserved for fixup routine use)
        ;
        mov     dword ptr ds:[ebp + stackStart + 4*0], eax
        mov     dword ptr ds:[ebp + stackStart + 4*1], ebx
        mov     dword ptr ds:[ebp + stackStart + 4*2], ecx
        mov     dword ptr ds:[ebp + stackStart + 4*3], edx
        mov     dword ptr ds:[ebp + stackStart + 4*4], esi
        mov     dword ptr ds:[ebp + stackStart + 4*5], edi


        ;
        ; SE_SVN should be written as soon as possible
        ;

        mov     ecx, MSR_BIOS_SE_SVN
        rdmsr

        ; bits [63:56] (upper 8 bits of EDX) of SE_SVN MSR are the SEAMLDR_SVN that we want to write

        and     edx, 0FFFFFFh
        xor     ebx, ebx
        mov     bx, word ptr ds:[ebp + HeaderStart.SeSvn] 
        shl     ebx, 24
        or      edx, ebx

        ; Need to clear lower 8 bits containing patch-at-reset SE_SVN value, not writable and will GP if written with !0 value.

        and     eax, 0FFFFFF00h
        wrmsr
        

        mov     ecx, MSR_BIOS_DONE
        rdmsr
        test    eax, 1
        jnz _BIOS_DONE_LOCKED
        db      0Fh, 0Bh                ; ud2
        
        _BIOS_DONE_LOCKED:


        mov     esi, OFFSET stackStart+40h
        add     esi, ebp                ; point to ACM reloc table for fixup
        mov     ecx, dword ptr ds:[esi] ; get relocCount

        ; Update this section of code for 18 bits of relocation data for 256KB ACM size support
        .IF ecx != 0
          xor     edx, edx              ; # bits to shift right (Shift Right counter)
          add     esi, 4                ; point to 1st relocAddr
          mov     ebx, 3FFFFh           ; For clipping to lower 18 bits

          xor	  eax, eax
        
FixupLoop:
          mov     edi, dword ptr ds:[esi] ; get relocAddr
          xchg    ecx, edx              ; Keep ecx = Shift Right count
          shr     edi, cl               ; Shifting right by Shift Right count
          and     edi, ebx              ; Clipped to lower 18 bits
          add     dword ptr ds:[edi+ebp], ebp ; fix it up
          xchg    ecx, edx              ; Restoring ecx

          add     edx,2                 ; Increment Shift Right counter
          and     edx, 0Fh              ; Clipped Shift Right counter to 0-15 count

          add     esi, 2                ; point to next relocAddr
          .IF edx == 0
            add     esi, 2              ; Adjust relocAddr for proper alignment if Shift Right counter == 0
          .ENDIF
          
          loop    FixupLoop
          
          ;lfence
          DB 0Fh 
          DB 0AEh
          DB 0E8h
          
          IFDEF STACK_GAP
          mov ecx, 120h
          mov eax, 100h
          mov edx, 0
          wrmsr
          ENDIF

        .ENDIF
        ;
        ; Find offset of code segment start, updated by ACM_PE2BIN tool
        ; Also serves as stack end
        ;
        mov     esi, OFFSET AcmEntryPoint
        mov     eax, cs:[esi - SIZEOF COM_DATA].COM_DATA.Code32Start
        mov     CodeSegmentStart, eax
        ;
        ; Restore registers from Fixup scratch area (size of 40h bytes reserved for fixup routine use)
        ;
        mov     eax, DWORD PTR ds:[stackStart + 4*0]
        mov     ebx, DWORD PTR ds:[stackStart + 4*1]
        mov     ecx, DWORD PTR ds:[stackStart + 4*2]
        mov     edx, DWORD PTR ds:[stackStart + 4*3]
        mov     esi, DWORD PTR ds:[stackStart + 4*4]
        mov     edi, DWORD PTR ds:[stackStart + 4*5]

        mov     OriginalEIP, ebx
        mov     OriginalEAX, eax
        mov     OriginalECX, ecx
        mov     OriginalESI, esi
        mov     OriginalEDI, edi
        mov     OriginalEDX, edx
        mov     eax, cr3
        mov     OriginalCR3, eax

        xor     eax, eax
        mov     ax, WORD ptr ds:[stackStart + 4*6]
        mov     OriginalIDTRLimit, ax

        ;                      
        ; Initialize Stack and Extended Data segments
        ;
        mov     ax, ES
        mov     OriginalES, ax
        mov     ax, FS
        mov     OriginalFS, ax
        mov     ax, GS
        mov     OriginalGS, ax
        mov     ax, SS
        mov     OriginalSS, ax        
        mov     ax, ACM_DATA_SELECTOR
        mov     es, ax
        mov     fs, ax
        mov     ss, ax                  ; init stack segment
        mov     esp, CodeSegmentStart  

        IFDEF   STACK_GAP
        sub     esp, STACK_GAP          ; Safety margin
        ELSE
        sub     esp, 10h                ; Safety margin
        ENDIF
        
        STACK_TEST_FILL_IN
        
        mov     AcmBase, ebp            ; save ac,.base
        mov     DWORD PTR AcmTop, ebp
        xor     edx, edx
        mov     eax, HeaderStart.Sizeb
        shl     eax, 02h                ; mult by 4
        mov     AcmSize, eax            ; save acm.Size in bytes
        add     AcmTop, eax             ; Save end address of ACM
        
        ;
        ; make sure TXT private space is already open before continuing
        ; 

        ; retry a long time
        RETRIES          EQU     080000000h
        
        mov     ecx, RETRIES
        mov     edi, LT_PRV_BASE + TXT.LT_STS
        
        .REPEAT
          mov   eax, DWORD PTR [edi]
          .BREAK .IF eax != 0FFFFFFFFh
          pause          
        .UNTILCXZ
        .if ecx == 0
           db      0Fh, 0Bh                ; ud2
        .endif
        
   
        mov     eax, cr4
        mov     OriginalCR4, eax
        INVOKE  Update_CRx
        
        jmp     commonAcmEntryPoint
;--------------------------------------------------------------------------------------
AcmEntryPoint ENDP
                                                       
_TEXT  ENDS

END


