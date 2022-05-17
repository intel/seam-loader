//**********************************************************************;
//*                                                                    *;
//* Intel Proprietary                                                  *;
//*                                                                    *;
//* Copyright 2021 Intel Corporation All Rights Reserved.              *;
//*                                                                    *;
//* Your use of this software is governed by the TDX Source Code       *;
//* LIMITED USE LICENSE.                                               *;
//*                                                                    *;
//* The Materials are provided "as is," without any express or         *;
//* implied warranty of any kind including warranties of               *;
//* merchantability, non-infringement, title, or fitness for a         *;
//* particular purpose.                                                *;
//*                                                                    *;
//**********************************************************************;

#include <common.h>
#include <common64.h>
#include <Header.h>
#include <NpSeamldr.h>
#include <paging.h>

static volatile UINT8 STSRegister;

static void CloseTPMLocality(PT_CTX* PtCtx)
{
    // Map the LT_PRV MMIO page as a single 4KB page with UC memtype
    // This is the first mapping the we do, so it should always succeed with enough page table entries left
    volatile TXT* TXTPrivateBase = (volatile TXT*)MapPhysicalRange(PtCtx, LT_PRV_BASE, sizeof(TXT), PAGE_WRITABLE, PAGE_4K, PAGE_UC_MEMTYPE);
    volatile UINT8 ReadByte;

    if ((UINT64)TXTPrivateBase == (UINT64)BAD_MAPPING) {
        _ud2();
    }

    // Note that access to MMIO registers must be done as UC accesses.

    // To close locality 3 do a
    // 1 - byte write of 0x0 to MMIO address 0xFED203A8 (CMD.CLOSE.LOCALITY3) and then do a 1 - byte read to the same address.
    *((UINT8*)(&TXTPrivateBase->LT_CMD_CLOSE_LOCALITY3)) = 0x0;
    do {
        ReadByte = *((UINT8*)(&TXTPrivateBase->LT_CMD_CLOSE_LOCALITY3));
        PauseCpu();
    } while ((ReadByte != 0) && (ReadByte != 0xFF));

    // The TXT private space and TPM locality 2 must be closed only if the platform is not post - SENTER.
    // The post - SENTER state can be determined by reading 1 - byte at LT.STS register at MMIO address 0xFED20000. 
    STSRegister = *((UINT8*)(&TXTPrivateBase->LT_STS));

    // If the value read has bit 0 set to 1 (i.e.LT.STS[0] == 1) then do not close the TXT private space or the TPM locality 2. 
    if ((STSRegister & 0x1) == 0) {
        // To close TXT private space do a 1 - byte write of 0x0 to MMIO address 0xFED20048 (CMD.CLOSE - PRIVATE) and then do a 1 - byte read to the same address.
        *((UINT8*)(&TXTPrivateBase->LT_CMD_CLOSE_PRIVATE)) = 0x0;
        do {
            ReadByte = *((UINT8*)(&TXTPrivateBase->LT_CMD_CLOSE_PRIVATE));
            PauseCpu();
        } while ((ReadByte != 0) && (ReadByte != 0xFF));

        // 1308911717 - Closing LT private closes locality 2 as well, so closing locality 2 explicitly is redundant
    }

    RemoveLinearMapping(PtCtx, (UINT64)TXTPrivateBase, FALSE);
}

static void ReopenTPMLocality(PT_CTX* PtCtx)
{
    // Map the LT_PRV MMIO page as a single 4KB page with UC memtype
    // This is the first mapping the we do, so it should always succeed with enough page table entries left
    volatile TXT* TXTPrivateBase = (volatile TXT*)MapPhysicalRange(PtCtx, LT_PRV_BASE, sizeof(TXT), PAGE_WRITABLE, PAGE_4K, PAGE_UC_MEMTYPE);
    volatile UINT8 ReadByte;

    if ((UINT64)TXTPrivateBase == (UINT64)BAD_MAPPING) {
        _ud2();
    }
    // If the value read has bit 0 set to 1 (i.e.LT.STS[0] == 1) then re-open the TXT private space and the TPM locality 2. 
    if ((STSRegister & 0x1) != 0) {
        // The post - SENTER state can be determined by reading 1 - byte at LT.STS register at MMIO address 0xFED20000. 

        // If the value read has bit 0 set to 1 (i.e.LT.STS[0] == 1) then re-open locality 2 for the MLE
        // Note that access to MMIO registers must be done as UC accesses.

        *((UINT8*)(&TXTPrivateBase->LT_CMD_OPEN_LOCALITY2)) = 0x0;
        do {
            ReadByte = *((UINT8*)(&TXTPrivateBase->LT_CMD_OPEN_LOCALITY2));
            PauseCpu();
        } while ((ReadByte != 0) && (ReadByte != 0xFF));

        RemoveLinearMapping(PtCtx, (UINT64)TXTPrivateBase, FALSE);
    }
}

void Target64 (SEAMLDR_COM64_DATA *pCom64)
{
    UINT64 canonicity_mask = 0;
    __security_init_cookie();
    pCom64->NewIDTR.Limit = pCom64->OriginalIDTRLimit;
    pCom64->NewIDTR.Base = pCom64->OriginalR12;
    *(UINT64 *)(pCom64->OriginalGdtr + 2) = pCom64->OriginalR9;
    pCom64->ResumeRip   = pCom64->OriginalR10;
    pCom64->OriginalCR3 = pCom64->OriginalR11;

    PT_CTX* PtCtx = (PT_CTX*)pCom64->PtCtxPtr;

    CloseTPMLocality(PtCtx);
    canonicity_mask = ((pCom64->OriginalCR4 & CR4_LA57) != 0) ? CANONICITY_MASK_5LP : CANONICITY_MASK_4LP;
    if (((pCom64->ResumeRip & canonicity_mask) != 0) && ((pCom64->ResumeRip & canonicity_mask) != canonicity_mask)) {
        _ud2();
    }
        
    SeamldrAcm(pCom64, PtCtx);
    ReopenTPMLocality(PtCtx);
    *(UINT16*)pCom64->NewGdtr = 0xFFF;
    *(UINT64*)(pCom64->NewGdtr + 2) = (UINT64)TempGdt;
    *(UINT64*)(TempGdt + pCom64->OriginalES) = GdtBasePtr.AcmDataDescriptor.Raw;
    *(UINT64*)(TempGdt + pCom64->OriginalFS) = GdtBasePtr.AcmDataDescriptor.Raw;
    *(UINT64*)(TempGdt + pCom64->OriginalGS) = GdtBasePtr.AcmDataDescriptor.Raw;
    *(UINT64*)(TempGdt + pCom64->OriginalSS) = GdtBasePtr.AcmDataDescriptor.Raw;
    *(UINT64*)(TempGdt + pCom64->OriginalECX) = GdtBasePtr.AcmCode64Descriptor.Raw;
}
