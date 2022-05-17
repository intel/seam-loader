// Intel Proprietary
// 
// Copyright 2021 Intel Corporation All Rights Reserved.
// 
// Your use of this software is governed by the TDX Source Code LIMITED USE LICENSE.
// 
// The Materials are provided “as is,” without any express or implied warranty of any kind including warranties
// of merchantability, non-infringement, title, or fitness for a particular purpose.
/**
 * @file vmcs_defs.h
 * @brief VMCS x86 definitions
 */
#ifndef SRC_COMMON_X86_DEFS_VMCS_DEFS_H_
#define SRC_COMMON_X86_DEFS_VMCS_DEFS_H_


#include "../../../include/pseamldr_basic_defs.h"
#include "../../../include/pseamldr_basic_types.h"

#define VM_EXIT_CTRL_SAVE_DEBUG_CTRL            BIT(2)
#define VM_EXIT_CTRL_HOST_ADDR_SPACE_SIZE       BIT(9)
#define VM_EXIT_CTRL_LOAD_IA32_PERF_GLOBAL_CTL  BIT(12)
#define VM_EXIT_CTRL_SAVE_IA32_PAT              BIT(18)
#define VM_EXIT_CTRL_LOAD_IA32_PAT              BIT(19)
#define VM_EXIT_CTRL_SAVE_IA32_EFER             BIT(20)
#define VM_EXIT_CTRL_LOAD_IA32_EFER             BIT(21)
#define VM_EXIT_CTRL_CLEAR_IA32_BNDCFGS         BIT(23)
#define VM_EXIT_CTRL_CONCEAL_VMX_FROM_PT        BIT(24)
#define VM_EXIT_CTRL_CLEAR_IA32_RTIT_CTL        BIT(25)
#define VM_EXIT_CTRL_CLEAR_LBR_CTL              BIT(26)
#define VM_EXIT_CTRL_CLEAR_UINV                 BIT(27)
#define VM_EXIT_CTRL_LOAD_CET_HOST_STATE        BIT(28)
#define VM_EXIT_CTRL_SAVE_IA32_PERF_GLOBAL_CTL  BIT(30)

#define VM_ENTRY_CTRL_LOAD_DEBUG_CTRL               BIT(2)
#define VM_ENTRY_CTRL_LOAD_IA32_PERF_GLOBAL_CTL     BIT(13)
#define VM_ENTRY_CTRL_LOAD_IA32_PAT                 BIT(14)
#define VM_ENTRY_CTRL_LOAD_IA32_EFER                BIT(15)
#define VM_ENTRY_CTRL_LOAD_IA32_BNDFGS              BIT(16)
#define VM_ENTRY_CTRL_CONCEAL_VMX_FROM_PT           BIT(17)
#define VM_ENTRY_CTRL_LOAD_IA32_RTIT_CTL            BIT(18)
#define VM_ENTRY_CTRL_LOAD_UINV                     BIT(19)
#define VM_ENTRY_CTRL_LOAD_GUEST_CET_STATE          BIT(20)
#define VM_ENTRY_CTRL_LOAD_LBR_CTL                  BIT(21)
#define VM_ENTRY_CTRL_LOAD_IA32_PKRS                BIT(22)

typedef union vmcs_revision_u
{
    struct
    {
        uint32_t
            vmcs_revision_identifier       : 31,             // [30:0]
            shadow_vmcs_indicator          : 1;             //  [31]
    };
    uint32_t raw;
} vmcs_revision_t;
pseamldr_static_assert(sizeof(vmcs_revision_t) == 4, vmcs_revision_t);

typedef struct vmcs_header_s
{
    vmcs_revision_t revision;
    uint32_t        vmx_abort_indicator;
} vmcs_header_t;
pseamldr_static_assert(sizeof(vmcs_header_t) == 8, vmcs_header_t);

typedef union vmx_guest_inter_state_u
{
    struct
    {
        uint32_t blocking_by_sti      : 1;   // 0
        uint32_t blocking_by_mov_ss   : 1;   // 1
        uint32_t blocking_by_smi      : 1;   // 2
        uint32_t blocking_by_nmi      : 1;   // 3
        uint32_t enclave_interruption : 1;   // 4
        uint32_t reserved             : 27;  // 31:5
    };
    uint64_t raw;
} vmx_guest_inter_state_t;
pseamldr_static_assert(sizeof(vmx_guest_inter_state_t) == 8, vmx_guest_inter_state_t);

typedef union
{
    struct
    {
        uint64_t b0          : 1; // Bit 0
        uint64_t b1          : 1; // Bit 1
        uint64_t b2          : 1; // Bit 2
        uint64_t b3          : 1; // Bit 3
        uint64_t rsvd_0      : 8; // Bits 4-11
        uint64_t enable      : 1; // Bit 12
        uint64_t rsvd_1      : 1; // Bits 13
        uint64_t bs          : 1; // Bits 14
        uint64_t rsvd_2      : 1; // Bits 15
        uint64_t rtm         : 1; // Bits 16
        uint64_t rsvd_3      : 47;// Bits 17-63
    };
    uint64_t raw;
} pending_debug_exception_t;
pseamldr_static_assert(sizeof(pending_debug_exception_t) == 8, pending_debug_exception_t);

#define IA32_VMX_BASIC_MSR_ADDR               0x480
#define IA32_VMX_PINBASED_CTLS_MSR_ADDR       0x481
#define IA32_VMX_PROCBASED_CTLS_MSR_ADDR      0x482
#define IA32_VMX_EXIT_CTLS_MSR_ADDR           0x483
#define IA32_VMX_ENTRY_CTLS_MSR_ADDR          0x484
#define IA32_VMX_EPT_VPID_CAP_MSR_ADDR        0x48C
#define IA32_VMX_CR0_FIXED0_MSR_ADDR          0x486
#define IA32_VMX_CR0_FIXED1_MSR_ADDR          0x487
#define IA32_VMX_CR4_FIXED0_MSR_ADDR          0x488
#define IA32_VMX_CR4_FIXED1_MSR_ADDR          0x489
#define IA32_VMX_PROCBASED_CTLS2_MSR_ADDR     0x48B
#define IA32_VMX_TRUE_PINBASED_CTLS_MSR_ADDR  0x48D
#define IA32_VMX_TRUE_PROCBASED_CTLS_MSR_ADDR 0x48E
#define IA32_VMX_TRUE_EXIT_CTLS_MSR_ADDR      0x48F
#define IA32_VMX_TRUE_ENTRY_CTLS_MSR_ADDR     0x490
#define IA32_VMX_PROCBASED_CTLS3_MSR_ADDR     0x492

#define VMX_GUEST_RIP_OFFSET  0x01d8 //8
#define VMX_VMCS_REVISION_ID_OFFSET  0x0000 //4
#define VMX_VM_EXECUTION_CONTROL_PROC_BASED_OFFSET  0x0120 //4
#define VMX_VM_EXECUTION_CONTROL_PIN_BASED_OFFSET  0x0128 //4
#define VMX_VM_EXIT_CONTROL_OFFSET  0x015c //4
#define VMX_VM_ENTRY_CONTROL_OFFSET  0x02b8 //4
#define VMX_HOST_CS_SELECTOR_OFFSET  0x00c2 //2
#define VMX_HOST_SS_SELECTOR_OFFSET  0x00c4 //2
#define VMX_HOST_FS_SELECTOR_OFFSET  0x00c8 //2
#define VMX_HOST_GS_SELECTOR_OFFSET  0x00ca //2
#define VMX_HOST_TR_SELECTOR_OFFSET  0x00cc //2
#define VMX_HOST_RSP_OFFSET  0x0300 //8
#define VMX_HOST_RIP_OFFSET  0x0308 //8
#define VMX_HOST_IA32_PAT_FULL_OFFSET  0x0310 //8
#define VMX_HOST_IA32_EFER_FULL_OFFSET  0x0318 //8
#define VMX_HOST_CR0_OFFSET  0x0328 //8
#define VMX_HOST_CR3_OFFSET  0x0330 //8
#define VMX_HOST_CR4_OFFSET  0x0338 //8
#define VMX_HOST_FS_BASE_OFFSET  0x0350 //8
#define VMX_HOST_GS_BASE_OFFSET  0x0358 //8
#define VMX_HOST_IA32_S_CET_OFFSET  0x0458 //8
#define VMX_HOST_SSP_OFFSET  0x0460 //8

#define Wr_16B_at_VMCS_Offset(VMCS_P, VAL, OFFSET) (*((uint16_t*) (((uint8_t *) (VMCS_P)) + (OFFSET))) = (VAL))
#define Wr_32B_at_VMCS_Offset(VMCS_P, VAL, OFFSET) (*((uint32_t*) (((uint8_t *) (VMCS_P)) + (OFFSET))) = (VAL))
#define Wr_64B_at_VMCS_Offset(VMCS_P, VAL, OFFSET) (*((uint64_t*) (((uint8_t *) (VMCS_P)) + (OFFSET))) = (VAL))

#define wr_guest_rip( VMCS_P, VAL ) Wr_64B_at_VMCS_Offset(VMCS_P, VAL, VMX_GUEST_RIP_OFFSET)
#define wr_host_cr0( VMCS_P, VAL )  Wr_64B_at_VMCS_Offset(VMCS_P, VAL, VMX_HOST_CR0_OFFSET)
#define wr_gost_cr3( VMCS_P, VAL )  Wr_64B_at_VMCS_Offset(VMCS_P, VAL, VMX_HOST_CR3_OFFSET)
#define wr_host_cr4( VMCS_P, VAL )  Wr_64B_at_VMCS_Offset(VMCS_P, VAL, VMX_HOST_CR4_OFFSET)
#define wr_host_cs_selector( VMCS_P, VAL )  Wr_16B_at_VMCS_Offset(VMCS_P, VAL, VMX_HOST_CS_SELECTOR_OFFSET)
#define wr_host_ss_selector( VMCS_P, VAL )  Wr_16B_at_VMCS_Offset(VMCS_P, VAL, VMX_HOST_SS_SELECTOR_OFFSET)
#define wr_host_fs_selector( VMCS_P, VAL )  Wr_16B_at_VMCS_Offset(VMCS_P, VAL, VMX_HOST_FS_SELECTOR_OFFSET)
#define wr_host_gs_selector( VMCS_P, VAL )  Wr_16B_at_VMCS_Offset(VMCS_P, VAL, VMX_HOST_GS_SELECTOR_OFFSET)
#define wr_host_tr_selector( VMCS_P, VAL )  Wr_16B_at_VMCS_Offset(VMCS_P, VAL, VMX_HOST_TR_SELECTOR_OFFSET)
#define wr_host_ia32_pat( VMCS_P, VAL )  Wr_64B_at_VMCS_Offset(VMCS_P, VAL, VMX_HOST_IA32_PAT_FULL_OFFSET)
#define wr_host_ia32_s_cet( VMCS_P, VAL )  Wr_64B_at_VMCS_Offset(VMCS_P, VAL, VMX_HOST_IA32_S_CET_OFFSET)
#define wr_host_ia32_efer( VMCS_P, VAL )  Wr_64B_at_VMCS_Offset(VMCS_P, VAL, VMX_HOST_IA32_EFER_FULL_OFFSET)
#define wr_vm_exit_control( VMCS_P, VAL )  Wr_32B_at_VMCS_Offset(VMCS_P, VAL, VMX_VM_EXIT_CONTROL_OFFSET)
#define wr_vm_entry_control( VMCS_P, VAL )  Wr_32B_at_VMCS_Offset(VMCS_P, VAL, VMX_VM_ENTRY_CONTROL_OFFSET)
#define wr_vm_execution_control_pin_based( VMCS_P, VAL )  Wr_32B_at_VMCS_Offset(VMCS_P, VAL, VMX_VM_EXECUTION_CONTROL_PIN_BASED_OFFSET)
#define wr_vm_execution_control_proc_based( VMCS_P, VAL )  Wr_32B_at_VMCS_Offset(VMCS_P, VAL, VMX_VM_EXECUTION_CONTROL_PROC_BASED_OFFSET)
#define wr_host_rip( VMCS_P, VAL )  Wr_64B_at_VMCS_Offset(VMCS_P, VAL, VMX_HOST_RIP_OFFSET)
#define wr_host_fs_base( VMCS_P, VAL )  Wr_64B_at_VMCS_Offset(VMCS_P, VAL, VMX_HOST_FS_BASE_OFFSET)
#define wr_host_rsp( VMCS_P, VAL )  Wr_64B_at_VMCS_Offset(VMCS_P, VAL, VMX_HOST_RSP_OFFSET)
#define wr_host_ssp( VMCS_P, VAL )  Wr_64B_at_VMCS_Offset(VMCS_P, VAL, VMX_HOST_SSP_OFFSET)
#define wr_host_gs_base( VMCS_P, VAL )  Wr_64B_at_VMCS_Offset(VMCS_P, VAL, VMX_HOST_GS_BASE_OFFSET)
#define wr_vmcs_revision_id( VMCS_P, VAL )  Wr_32B_at_VMCS_Offset(VMCS_P, VAL, VMX_VMCS_REVISION_ID_OFFSET)

#define VMX_GUEST_RIP_ENCODE                    0x681EULL
#define VMX_GUEST_INTERRUPTIBILITY_ENCODE  0x4824ULL
#define VMX_VM_EXIT_INSTRUCTION_LENGTH_ENCODE   0x440CULL
#define VMX_GUEST_RFLAGS_ENCODE  0x6820ULL
#define VMX_GUEST_PND_DEBUG_EXCEPTION_ENCODE  0x6822ULL
#define VMX_GUEST_IA32_DEBUGCTLMSR_FULL_ENCODE  0x2802ULL
#define VMX_HOST_FS_BASE_ENCODE                 0x6C06ULL

#endif /* SRC_COMMON_X86_DEFS_VMCS_DEFS_H_ */
