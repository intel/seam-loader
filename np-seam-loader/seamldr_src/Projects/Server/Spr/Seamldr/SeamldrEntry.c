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

#include "common.h"
#include "paging.h"
#include "Header.h"
#include "msr.h"

UINT32 ChipsetAcmType = MCP_CHIPSET_ACM_TYPE;

#define PAGE_SIZE       _4KB

static void MapPagingStructure(IA32E_PXE_T* ParentPXE, IA32E_PAGING_TABLE_T* ChildPXBase)
{
    ParentPXE->Raw = (UINT64)ChildPXBase;
    ParentPXE->FieldsPS.P = 1;
    ParentPXE->FieldsPS.RW = 1;
    ParentPXE->FieldsPS.A = 1;
}

static void Map4KPage(IA32E_PXE_T* LeafPXE, UINT64 PhysBase, BOOL IsWritable, BOOL IsWBMemtype, BOOL IsExecutable)
{
    LeafPXE->Raw = PhysBase;
    LeafPXE->Fields4K.P = 1;
    LeafPXE->Fields4K.RW = (IsWritable) ? 1 : 0;
    LeafPXE->Fields4K.A = 1;
    LeafPXE->Fields4K.D = 1;

    if (!IsExecutable)
    {
        LeafPXE->Raw |= BIT63;
    }

    if (IsWBMemtype) {
        // PAT0 is supposed to be WB memtype
        LeafPXE->Fields4K.PAT = 0;
        LeafPXE->Fields4K.PWT = 0;
        LeafPXE->Fields4K.PCD = 0;
    }
    else {
        // PAT7 is supposed to be UC memtype
        LeafPXE->Fields4K.PAT = 1;
        LeafPXE->Fields4K.PWT = 1;
        LeafPXE->Fields4K.PCD = 1;
    }
}

static BOOL IsCodeAcmPage(COM_DATA* PE2BIN_Com_Data, UINT32 Page)
{
    // The structure of the ACM is as follows (from low addresses to high):
    // 32-bit data and stack
    // 32-bit code
    // 64-bit data
    // 64-bit code

    if (((Page >= PE2BIN_Com_Data->Code32Start) && (Page < PE2BIN_Com_Data->Data64Start)) ||
        ((Page >= PE2BIN_Com_Data->Code64Start) && (Page < PE2BIN_Com_Data->Code64End)))
    {
        return TRUE;
    }

    return FALSE;
}

#define MAX_ACM_SIZE        (256 * 1024)

__declspec(align(4096)) SEAMLDR_PAGING_TABLE_T SeamldrPagingTable;

PT_CTX* EstablishSeamldrPaging(SEAMLDR_COM64_DATA *pCom64, IN OUT PT_CTX * PtCtx)
{
    COM_DATA* PE2BIN_Com_Data = (COM_DATA*)((UINT32)AcmEntryPoint - sizeof(COM_DATA));
    // We will use index 0 for PML5 and PML4 because we aren't going to map linear addresses above 4GB
    MapPagingStructure(&SeamldrPagingTable.Pml5.PT[0], &SeamldrPagingTable.Pml4);
    MapPagingStructure(&SeamldrPagingTable.Pml4.PT[0], &SeamldrPagingTable.Pdpt);

    // First indexes 0-3 in PDPT covers the whole lower 4 GB (each PD table cover 512 entries of 2MB, there are 4 PD tables)
    MapPagingStructure(&SeamldrPagingTable.Pdpt.PT[0], &SeamldrPagingTable.Pd[0]);
    MapPagingStructure(&SeamldrPagingTable.Pdpt.PT[1], &SeamldrPagingTable.Pd[1]);
    MapPagingStructure(&SeamldrPagingTable.Pdpt.PT[2], &SeamldrPagingTable.Pd[2]);
    MapPagingStructure(&SeamldrPagingTable.Pdpt.PT[3], &SeamldrPagingTable.Pd[3]);

    // We will fill the entries in PD tables later, at this point they are empty and don't map anything

    // Now we need to establish 1-to-1 paging for the SEAMLDR ACM address space
    // We need to map data/stack pages with XD (execute disabled) bit, and code pages as read-only
    // The structure of the ACM is as follows (from low addresses to high):
    // 32-bit data and stack
    // 32-bit code
    // 64-bit data
    // 64-bit code
    // These offsets are stored in COM_DATA

    // Only bits 30:31 are relevant for low 4GB
    UINT32 PdptIdx = ((AcmBase >> 30) & 0x3);
    // Mask the 9 index bits
    UINT32 PdIdx = ((AcmBase >> 21) & 0x1FF);

    IA32E_PAGING_TABLE_T* Pd = &SeamldrPagingTable.Pd[PdptIdx];

    MapPagingStructure(&Pd->PT[PdIdx], &SeamldrPagingTable.Pt[0]);

    UINT32 PtIdx = ((AcmBase >> 12) & 0x1FF);
    UINT32 LastPtIdx = PtIdx + (rounded(AcmSize, PAGE_SIZE) / PAGE_SIZE);
    UINT64 CurrentAcmPageToMap = AcmBase;

    IA32E_PAGING_TABLE_T* Pt = &SeamldrPagingTable.Pt[0];

    // Map the ACM 4K pages until the last index in the PT table (must be 1:1 mapping)
    for (UINT32 i = PtIdx; i < ((LastPtIdx < 512) ? LastPtIdx : 512); i++) {
        if (IsCodeAcmPage(PE2BIN_Com_Data, CurrentAcmPageToMap))
        {
            Map4KPage(&Pt->PT[i], CurrentAcmPageToMap, FALSE, TRUE, TRUE); // Not-writable, WB memtype, Executable
        }
        else
        {
            Map4KPage(&Pt->PT[i], CurrentAcmPageToMap, TRUE, TRUE, FALSE); // Writable, WB memtype, Non-executable
        }
        CurrentAcmPageToMap += PAGE_SIZE;
    }

    if (LastPtIdx > 512) {
        // In case when the Acm mapping spans over two page tables, we need to map the PtTable1 in the PD
        // However there's also a possibility that we were on our last slot in the current PD, so we need to switch to the next one
        if (PdIdx == 511) {
            // We won't span over the 4GB boundary in Acm, so it's ok to just +1 the PDPT index
            PdptIdx = PdptIdx + 1;
            Pd = &SeamldrPagingTable.Pd[PdptIdx];
            PdIdx = 0;
        }
        else {
            PdIdx = PdIdx + 1;
        }

        MapPagingStructure(&Pd->PT[PdIdx], &SeamldrPagingTable.Pt[1]);

        Pt = &SeamldrPagingTable.Pt[1];

        PtIdx = 0;
        LastPtIdx = LastPtIdx - 512;

        // Map the rest of the ACM 4K pages
        for (UINT32 i = PtIdx; i < LastPtIdx; i++) {
            if (IsCodeAcmPage(PE2BIN_Com_Data, CurrentAcmPageToMap))
            {
                Map4KPage(&Pt->PT[i], CurrentAcmPageToMap, FALSE, TRUE, TRUE); // Not-writable, WB memtype, Executable
            }
            else
            {
                Map4KPage(&Pt->PT[i], CurrentAcmPageToMap, TRUE, TRUE, FALSE); // Writable, WB memtype, Non-executable
            }
            CurrentAcmPageToMap += PAGE_SIZE;
        }
    }

    // One of the unused PD table will be used for rest of the 4K mappings in the system (not necessary 1 to 1)
    PdptIdx = (PdptIdx + 1) % 4;
    Pd = &SeamldrPagingTable.Pd[PdptIdx];

    // We will use only the first index in the unused PD table for the PT Table 2
    // And we won't touch the rest of the indexes, to prevent confusion
    MapPagingStructure(&Pd->PT[0], &SeamldrPagingTable.Pt[2]);

    // Virtual base is calculated as follows:
    // PML5 and PML4 index is 0. PDPT index as chosen. PD index 0, PT index is 0
    PtCtx->VirtualBaseFor4KMappings = (PdptIdx << 30);
    PtCtx->PtBaseFor4KMappings = (UINT64)&SeamldrPagingTable.Pt[2];
    PtCtx->NextFreePtIdx = 0;

    // The last (third or fourth) unused PD table will be used for 2MB mapping in the system (not necessary 1 to 1)
    // This table will cover 512 * 2MB space, which 1GB, which should be enough to map the entire SEAMRR
    PdptIdx = (PdptIdx + 1) % 4;

    // Virtual base is calculated as follows:
    // PML5 and PML4 index is 0. PDPT index as chosen. PD index 0
    PtCtx->VirtualBaseFor2MBMappings = (PdptIdx << 30);
    PtCtx->PdBaseFor2MBMappings = (UINT64)&SeamldrPagingTable.Pd[PdptIdx];
    PtCtx->NextFreePdIdx = 0;

    // Prior to enabling paging, the SEAMLDR should configure the IA32_PAT MSR with its reset default value 0x0007040600070406 (i.e.PAT0 = WB, PAT7 = UC).
    writeMsr(MSR_IA32_PAT, 0x00070406UL, 0x00070406UL);

    // Load CR3 with the PML4/5 base - the SEAMLDR will run with either 4-level or 5-level paging, depending on the original level of the OS
    if (SeamldrCom64Data.OriginalCR4 & CR4_LA57) {
        __writecr3(&SeamldrPagingTable.Pml5);
    }
    else {
        __writecr3(&SeamldrPagingTable.Pml4);
    }

    // Set EFER.LME to re-enable ia32-e
    UINT32 RDX, RAX;
    readMsr(IA32_EFER_MSR, &RDX, &RAX);
    RAX |= (LME | N_IA32_EFER_NXE);
    writeMsr(IA32_EFER_MSR, RDX, RAX);

    // Enable paging
    __writecr0(__readcr0() | CR0_PG | CR0_WP);
    
    return PtCtx;
}



void ProjectAcmEntryPoint()
{
    PT_CTX PtCtx;

    Init64bitComArea();

    SeamldrCom64Data.OriginalCR4 = OriginalCR4;
    SeamldrCom64Data.HeaderStart = (UINT64)&HeaderStart;
    SeamldrCom64Data.PseamldrOffset = (UINT64)&PSeamldrAsm;
    SeamldrCom64Data.PseamldrSize = PSeamldrSizeAsm;
    SeamldrCom64Data.PseamldrConstsOffset = (UINT64)&PSeamldrConstAsm;
    SeamldrCom64Data.OriginalES = OriginalES & 0x0FF8;
    SeamldrCom64Data.OriginalFS = OriginalFS & 0x0FF8;
    SeamldrCom64Data.OriginalGS = OriginalGS & 0x0FF8;
    SeamldrCom64Data.OriginalSS = OriginalSS & 0x0FF8;
    SeamldrCom64Data.OriginalECX = OriginalECX & 0x0FF8;

    MemFill((UINT8*)&SeamldrPagingTable, sizeof(SEAMLDR_PAGING_TABLE_T), 0);

    EstablishSeamldrPaging(&SeamldrCom64Data, &PtCtx);

    SeamldrCom64Data.PtCtxPtr = (UINT64)&PtCtx;

    SeamldrThunk64();

    // No return to here
}
