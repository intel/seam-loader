// Intel Proprietary
// 
// Copyright 2021 Intel Corporation All Rights Reserved.
// 
// Your use of this software is governed by the TDX Source Code LIMITED USE LICENSE.
// 
// The Materials are provided “as is,” without any express or implied warranty of any kind including warranties
// of merchantability, non-infringement, title, or fitness for a particular purpose.
/**
 * @file pseamldr_dispatcher.c
 * @brief VMM dispatcher and return sequence
 */
#include "pseamldr_dispatcher.h"

#include "../../include/pseamldr_api_handlers.h"
#include "data_structures/pseamldr_data.h"
#include "accessors/ia32_accessors.h"
#include "accessors/vt_accessors.h"
#include "accessors/data_accessors.h"
#include "x86_defs/vmcs_defs.h"
#include "debug/tdx_debug.h"
#include "helpers/helpers.h"
#include "x86_defs/msr_defs.h"
#include "x86_defs/x86_defs.h"

static void pseamldr_initialize_system_values(pseamldr_data_t* pseamldr_data)
{
    uint32_t eax, ebx, ecx, edx;

    ia32_safe_cpuid(CPUID_GET_MAX_PA_LEAF, 0, &eax, &ebx, &ecx, &edx);

    pseamldr_data->system_info.max_pa = eax & CPUID_MAX_PA_BITS;

    uint64_t seamrr_base = ia32_rdmsr(IA32_SEAMRR_BASE_MSR_ADDR) & IA32_SEAMRR_BASE_AND_MASK_MASK;
    uint64_t seamrr_mask = ia32_rdmsr(IA32_SEAMRR_MASK_MSR_ADDR) & IA32_SEAMRR_BASE_AND_MASK_MASK;
    uint64_t seamrr_size = mask_to_size(seamrr_mask);

    pseamldr_data->system_info.seamrr_base = seamrr_base;
    pseamldr_data->system_info.seamrr_size = seamrr_size;

    ia32_tme_capability_t ia32_tme_capability;
    ia32_tme_activate_t   ia32_tme_activate;

    ia32_tme_capability.raw = ia32_rdmsr(IA32_TME_CAPABILITY_MSR_ADDR);
    ia32_tme_activate.raw   = ia32_rdmsr(IA32_TME_ACTIVATE_MSR_ADDR);

    uint64_t max_mktme_hkids = MIN((uint64_t)ia32_tme_capability.mk_tme_max_keys,
            BIT(ia32_tme_activate.mk_tme_keyid_bits - ia32_tme_activate.tdx_reserved_keyid_bits));

    pseamldr_data->system_info.max_mktme_hkids = max_mktme_hkids;

    uint64_t hkid_start_bit = pseamldr_data->system_info.max_pa - ia32_tme_activate.mk_tme_keyid_bits;
    uint64_t hkid_mask = BITS(pseamldr_data->system_info.max_pa - 1, hkid_start_bit);

    pseamldr_data->system_info.hkid_mask = hkid_mask;

    init_keyhole_state();

    pseamldr_data->system_info.initialized = true;
}

_STATIC_INLINE_ void sys_control_msr_handling(pseamldr_data_t* pseamldr_data)
{
    // Set bit 13 of IA32_DEBUGCTL (Enable Uncore PMI) according to bit 13 of GUEST_IA32_DEBUGCTL field of P-SEAMLDR VMCS.
    ia32_debugctl_t current_debugctl, guest_debugctl;
    current_debugctl.raw = ia32_rdmsr(IA32_DEBUGCTL_MSR_ADDR);
    ia32_vmread(VMX_GUEST_IA32_DEBUGCTLMSR_FULL_ENCODE, &guest_debugctl.raw);
    current_debugctl.en_uncore_pmi = guest_debugctl.en_uncore_pmi;
    ia32_wrmsr(IA32_DEBUGCTL_MSR_ADDR, current_debugctl.raw);

    // Write IA32_PRED_CMD = 0x1 (IBPB).
    ia32_pred_cmd_t pred_cmd = { .raw = 0 };
    pred_cmd.ibpb = 1;
    ia32_wrmsr(IA32_PRED_CMD_MSR_ADDR, pred_cmd.raw);

    // Save IA32_SPEC_CTRL and write IA32_SPEC_CTRL = 0x4 (SSBD).
    pseamldr_data->vmm_spec_ctrl.raw = ia32_rdmsr(IA32_SPEC_CTRL_MSR_ADDR);
    ia32_spec_ctrl_t spec_ctrl = { .raw = 0 };
    spec_ctrl.ssbd = 1;
    ia32_wrmsr(IA32_SPEC_CTRL_MSR_ADDR, spec_ctrl.raw);
}

_STATIC_INLINE_ bool_t generate_canary(pseamldr_data_t* pseamldr_data)
{
    if (!ia32_rdrand64(&pseamldr_data->canary))
    {
        return false;
    }

    if (pseamldr_data->canary == 0)
    {
        return false;
    }

    ia32_vmwrite(VMX_HOST_FS_BASE_ENCODE, (uint64_t)pseamldr_data);

    return true;
}

void pseamldr_dispatcher(void)
{
    // Must be first thing to do before accessing data or sysinfo table
    pseamldr_data_t* pseamldr_data = init_data_fast_ref_ptrs();

    TDX_LOG("PSEAMLDR Module entry start\n");

    sys_control_msr_handling(pseamldr_data);

    // Must be the next thing to do after initializing the data pointers above
    // and the SPEC_CTRL MSR
    IF_RARE (!pseamldr_data->canary)
    {
        if (!generate_canary(pseamldr_data))
        {
            TDX_ERROR("Not enough entropy to generate stack canary - PSEAMLDR_EUNSPECERR\n");
            pseamldr_data->vmm_regs.rax = PSEAMLDR_EUNSPECERR;
            goto EXIT;
        }
    }

    IF_RARE (!pseamldr_data->system_info.initialized)
    {
        pseamldr_initialize_system_values(pseamldr_data);
    }

    // Get leaf code from RAX in local data (saved on entry)
    uint64_t leaf_opcode = pseamldr_data->vmm_regs.rax;

    TDX_LOG("Leaf_opcode = 0x%llx\n", leaf_opcode);

    // switch over leaf opcodes
    switch (leaf_opcode)
    {
    case SEAMLDR_INFO_LEAF:
    {
        pseamldr_data->vmm_regs.rax = seamldr_info(pseamldr_data->vmm_regs.rcx);
        break;
    }
    case SEAMLDR_INSTALL_LEAF:
    {
        pseamldr_data->vmm_regs.rax = seamldr_install(pseamldr_data->vmm_regs.rcx);
        break;
    }
    case SEAMLDR_SHUTDOWN_LEAF:
    {
        pseamldr_data->vmm_regs.rax = seamldr_shutdown();
        break;
    }
    case SEAMLDR_SEAMINFO_LEAF:
    {
        pseamldr_data->vmm_regs.rax = seamldr_seaminfo(pseamldr_data->vmm_regs.rcx);
        break;
    }
#ifdef DEBUGFEATURE_TDX_DBG_TRACE
    case TDDEBUGCONFIG_LEAF:
    {
        pseamldr_data->vmm_regs.rax = td_debug_config(pseamldr_data->vmm_regs.rcx, pseamldr_data->vmm_regs.rdx,
                                                   pseamldr_data->vmm_regs.r8);
        break;
    }
#endif

    default:
    {
        TDX_ERROR("Unknown leaf - PSEAMLDR_EBADPARAM\n");
        pseamldr_data->vmm_regs.rax = PSEAMLDR_EBADPARAM;
        goto EXIT;
    }
    }

EXIT:

    // No return after calling the post dispatching operations
    // Eventually call SEAMRET
    pseamldr_post_dispatching();
}


void pseamldr_post_dispatching(void)
{
    advance_guest_rip();

    // Restore IA32_SPEC_CTRL
    ia32_wrmsr(IA32_SPEC_CTRL_MSR_ADDR, get_pseamldr_data()->vmm_spec_ctrl.raw);

    TDX_LOG("pseamldr_post_dispatching - preparing to do SEAMRET\n");

    pseamldr_exit_point(); // Restore GPRs and SEAMRET

    // Shouldn't reach here:

    pseamldr_sanity_check(0, SCEC_VMM_DISPATCHER_SOURCE, 0);
}
