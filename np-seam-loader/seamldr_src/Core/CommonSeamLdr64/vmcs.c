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
#include "Header.h"
#include "NpSeamldr.h"
#include "msr.h"
#ifndef _SEAMLDR_VALIDATION_
#define uint8_t  UINT8
#define uint16_t UINT16
#define uint32_t UINT32
#define uint64_t UINT64
#endif
#include "vmx_vmcs.h"
#include "vmx_vmcs_accessors.h"
#include "vmcs.h"

void SetupVmcs(UINT64 SeamPtBaseAddr) {
    
    UINT64 VmcsBaseVa = SeamldrData.SeamrrVa + (SeamldrData.PSysInfoTable->PSeamldrRange.Base - SeamldrData.SeamrrBase) + _4KB;
    VmxBasicMsr_u VmxBasic;
    UINT32 PinbasedCtls;
    UINT32 ProcbasedCtls;
    UINT32 ExitCtls;
    UINT32 EntryCtls;
    UINT64 Cr0Fixed0, Cr0Fixed1, Cr0MustBe1;
    UINT64 Cr4Fixed0, Cr4Fixed1, Cr4MustBe1;
    
    VmxBasic.raw = readMsr64(IA32_VMX_BASIC_MSR_INDEX);
    
    PinbasedCtls = (UINT32)(readMsr64(IA32_VMX_TRUE_PINBASED_CTLS_MSR_INDEX) & MAX_DWORD);
    ProcbasedCtls = (UINT32)(readMsr64(IA32_VMX_TRUE_PROCBASED_CTLS_MSR_INDEX) & MAX_DWORD);
    ExitCtls = (UINT32)(readMsr64(IA32_VMX_TRUE_EXIT_CTLS_MSR_ADDR) & MAX_DWORD);
    EntryCtls = (UINT32)(readMsr64(IA32_VMX_TRUE_ENTRY_CTLS_MSR_ADDR) & MAX_DWORD);

    Cr0Fixed0 = readMsr64(IA32_VMX_CR0_FIXED0_MSR_INDEX);
    Cr0Fixed1 = readMsr64(IA32_VMX_CR0_FIXED1_MSR_INDEX);
    Cr0MustBe1 = Cr0Fixed1 & Cr0Fixed0;
    Cr4Fixed0 = readMsr64(IA32_VMX_CR4_FIXED0_MSR_INDEX);
    Cr4Fixed1 = readMsr64(IA32_VMX_CR4_FIXED1_MSR_INDEX);
    Cr4MustBe1 = Cr4Fixed1 & Cr4Fixed0;
            
    Wr_Guest_RIP(VmcsBaseVa, NON_CANONICAL_RIP);
    Wr_Host_CR0(VmcsBaseVa, CR0_PE | CR0_ET | CR0_NE | CR0_WP | CR0_PG | Cr0MustBe1);
    Wr_Host_CR3(VmcsBaseVa, SeamPtBaseAddr);
    Wr_Host_CR4(VmcsBaseVa, CR4_DE | CR4_PAE | CR4_PGE | CR4_OSFXSR | CR4_OSXMMEXCPT | CR4_VMXE | CR4_FSGSBASE | CR4_OSXSAVE | CR4_SMEP | CR4_SMAP | CR4_CET | Cr4MustBe1);
    Wr_Host_CS_Selector(VmcsBaseVa, 8U);
    Wr_Host_SS_Selector(VmcsBaseVa, 0x10U);
    Wr_Host_FS_Selector(VmcsBaseVa, 0x18U);
    Wr_Host_GS_Selector(VmcsBaseVa, 0x18U);
    Wr_Host_TR_Selector(VmcsBaseVa, 0x20U);
    Wr_Host_IA32_PAT(VmcsBaseVa, 0x0006060606060606ULL);
    Wr_Host_IA32_S_Cet(VmcsBaseVa, IA32_CR_S_CET_SH_STK_EN_MASK | IA32_CR_S_CET_ENDBR_EN_MASK | IA32_CR_S_CET_NO_TRACK_EN_MASK);
    Wr_Host_IA32_EFER(VmcsBaseVa, N_IA32_EFER_LMA | LME | N_IA32_EFER_NXE);

    ExitCtls |= (VM_EXIT_CTRL_SAVE_DEBUG_CTRL | VM_EXIT_CTRL_HOST_ADDR_SPACE_SIZE | VM_EXIT_CTRL_SAVE_IA32_PAT | VM_EXIT_CTRL_LOAD_IA32_PAT | \
                 VM_EXIT_CTRL_SAVE_IA32_EFER | VM_EXIT_CTRL_LOAD_IA32_EFER | VM_EXIT_CTRL_CONCEAL_VMX_FROM_PT | VM_EXIT_CTRL_CLEAR_IA32_RTIT_CTL | \
                 VM_EXIT_CTRL_CLEAR_LBR_CTL | VM_EXIT_CTRL_LOAD_CET_HOST_STATE | VM_EXIT_SAVE_IA32_PERF_GLOBAL_CTRL | VM_EXIT_LOAD_IA32_PERF_GLOBAL_CTRL);

    ExitCtls &= ((readMsr64(IA32_VMX_TRUE_EXIT_CTLS_MSR_ADDR) >> 32) & MAX_DWORD);

    Wr_VM_Exit_Control(VmcsBaseVa, ExitCtls);

    EntryCtls |= (VM_ENTRY_CTRL_LOAD_DEBUG_CTRL | VM_ENTRY_CTRL_LOAD_IA32_PERF_GLOBAL_CTRL | VM_ENTRY_CTRL_LOAD_IA32_PAT | VM_ENTRY_CTRL_LOAD_IA32_EFER | \
                  VM_ENTRY_CTRL_CONCEAL_VMX_FROM_PT | VM_ENTRY_CTRL_LOAD_UINV | VM_ENTRY_CTRL_LOAD_IA32_PKRS | \
                  VM_ENTRY_CTRL_LOAD_IA32_RTIT_CTL | VM_ENTRY_CTRL_LOAD_GUEST_CET_STATE | VM_ENTRY_CTRL_LOAD_LBR_CTL);

    EntryCtls &= ((readMsr64(IA32_VMX_TRUE_ENTRY_CTLS_MSR_ADDR) >> 32) & MAX_DWORD);

    Wr_VM_Entry_Control(VmcsBaseVa, EntryCtls);

    Wr_VM_Execution_Control_Pin_Based(VmcsBaseVa, PinbasedCtls);
    Wr_VM_Execution_Control_Proc_Based(VmcsBaseVa, ProcbasedCtls);
    
    Wr_Host_RIP(VmcsBaseVa, (C_CODE_RGN_BASE | SeamldrData.AslrRand) + SeamldrData.PSeamldrConsts->CEntryPointOffset);
    Wr_Host_FS_Base(VmcsBaseVa, C_SYS_INFO_TABLE_BASE | SeamldrData.AslrRand);
    
    UINT64 HostRSP = (C_STACK_RGN_BASE | SeamldrData.AslrRand) + SeamldrData.PSeamldrConsts->CDataStackSize - 8;
    UINT64 HostSSP = (C_STACK_RGN_BASE | SeamldrData.AslrRand) + SeamldrData.PSeamldrConsts->CDataStackSize + P_SEAMLDR_SHADOW_STACK_SIZE - 8;
    UINT64 HostGSBase = (C_DATA_RGN_BASE | SeamldrData.AslrRand);

    Wr_Host_RSP(VmcsBaseVa, HostRSP);
    Wr_Host_SSP(VmcsBaseVa, HostSSP);
    Wr_Host_GS_Base(VmcsBaseVa, HostGSBase);
    Wr_VMCS_Revision_ID(VmcsBaseVa, (UINT32)VmxBasic.RevisionIdentifier & 0x7FFFFFFF);
}