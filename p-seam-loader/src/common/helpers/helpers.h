// Intel Proprietary
// 
// Copyright 2021 Intel Corporation All Rights Reserved.
// 
// Your use of this software is governed by the TDX Source Code LIMITED USE LICENSE.
// 
// The Materials are provided “as is,” without any express or implied warranty of any kind including warranties
// of merchantability, non-infringement, title, or fitness for a particular purpose.
/**
 * @file helpers.h
 * @brief Common PSEAMLDR API flow helper functions
 */

#ifndef SRC_COMMON_HELPERS_HELPERS_H_
#define SRC_COMMON_HELPERS_HELPERS_H_

#include "../../../include/pseamldr_api_defs.h"
#include "../../../include/pseamldr_basic_defs.h"
#include "../../../include/pseamldr_basic_types.h"
#include "x86_defs/x86_defs.h"
#include "accessors/data_accessors.h"
#include "accessors/vt_accessors.h"
#include "memory_handlers/keyhole_manager.h"
#include "error_reporting.h"

_STATIC_INLINE_ uint8_t bit_test_and_set(volatile uint8_t* mem, uint32_t bit)
{
    uint8_t bit_value = 0;
    uint32_t byte_position = bit / 8;
    uint32_t bit_position_in_byte = bit % 8;

    bit_value = (mem[byte_position] & BIT(bit_position_in_byte)) >> bit_position_in_byte;
    mem[byte_position] |= BIT(bit_position_in_byte);

    return bit_value;
}

_STATIC_INLINE_ uint64_t page_tables_needed_to_map(uint64_t region_size)
{
    return (((region_size / _4KB) * sizeof(ia32e_pxe_t)) + _4KB - 1) / _4KB  // PT pages
         + (((region_size / _2MB) * sizeof(ia32e_pxe_t)) + _4KB - 1) / _4KB  // PD pages
         + (((region_size / _1GB) * sizeof(ia32e_pxe_t)) + _4KB - 1) / _4KB; // PDPT pages
}

_STATIC_INLINE_ uint64_t get_addr_from_pa(pa_t pa)
{
    return pa.full_pa & ~(get_pseamldr_data()->system_info.hkid_mask);
}

_STATIC_INLINE_ uint16_t get_hkid_from_pa(pa_t pa)
{
    uint8_t lsb_idx = (uint8_t)bit_scan_forward64(get_pseamldr_data()->system_info.hkid_mask);

    return (uint16_t)((pa.full_pa & get_pseamldr_data()->system_info.hkid_mask) >> lsb_idx);
}


_STATIC_INLINE_ bool_t is_addr_aligned_pwr_of_2(uint64_t addr, uint64_t size)
{
    pseamldr_debug_assert((size > 1) && ((size & (size - 1)) == 0));
    return ((addr & (size - 1)) == 0);
}

_STATIC_INLINE_ bool_t is_addr_aligned_any(uint64_t addr, uint64_t size)
{
    pseamldr_debug_assert(size > 0);
    return ((addr % size) == 0);
}

_STATIC_INLINE_ bool_t is_pa_smaller_than_max_pa(uint64_t pa)
{
    return (pa < BIT(get_pseamldr_data()->system_info.max_pa));
}

_STATIC_INLINE_ bool_t is_overlap(uint64_t base, uint64_t size, uint64_t base2, uint64_t size2)
{
    if ((base >= (base2 + size2)) || (base2 >= (base + size)))
    {
        return false;
    }
    return true;
}

_STATIC_INLINE_ uint64_t mask_to_size(uint64_t mask)
{
    return BIT(bit_scan_forward64(mask));
}

/**
 * @brief Memset functions that use basic memory access (not direct access with MOVDIR64)
 *
 */
void basic_memset(uint64_t dst, uint64_t dst_bytes, uint8_t val, uint64_t nbytes);
void basic_memset_to_zero(void * dst, uint64_t nbytes);

/**
 * @brief Fill cache lines with movdir64 using cache aligned & sized chunks
 *
 * @note dst address must be aligned to cache line
 * @note cacheline_with_value must be aligned to cache line
 *
 * @param dst destination address
 * @param cacheline_with_value a cache line aligned and sized array with a value
 * @param num_of_cachelines number of cache lines to write to in dst
 */
_STATIC_INLINE_ void fill_cachelines_no_serialize(void* dst, uint8_t* cacheline_with_value,
                                                  uint64_t num_of_cachelines)
{
    pseamldr_debug_assert(((uint64_t)dst % MOVDIR64_CHUNK_SIZE) == 0);
    pseamldr_debug_assert(((uint64_t)cacheline_with_value % MOVDIR64_CHUNK_SIZE) == 0);
    uint64_t tmp_dst = (uint64_t)dst;

    for (uint64_t i = 0; i < num_of_cachelines; i++, tmp_dst+=MOVDIR64_CHUNK_SIZE)
    {
        movdir64b(cacheline_with_value, tmp_dst);
    }
}

/**
 * @brief Full core/memory serialization after movdir64b, to prevent fetches from
 *        uninitialized memory ahead of movdir64b completion
 */
_STATIC_INLINE_ void movdir64b_serialize(void)
{
    mfence();
    serialize();
}

/**
 * @brief Fill cache lines with movdir64 using cache aligned & sized chunks
 *
 * @note dst address must be aligned to cache line
 * @note cacheline_with_value must be aligned to cache line
 *
 * @param dst destination address
 * @param cacheline_with_value a cache line aligned and sized array with a value
 * @param num_of_cachelines number of cache lines to write to in dst
 */
_STATIC_INLINE_ void fill_cachelines(void* dst, uint8_t* cacheline_with_value, uint64_t num_of_cachelines)
{
    fill_cachelines_no_serialize(dst, cacheline_with_value, num_of_cachelines);

    movdir64b_serialize();
}

/**
 * @brief Fill memory area using movdir64
 *
 * @note dst address must be aligned to cache line
 * @note nBytes size must be in complete cache lines
 * @note val64 value to be filled
 *
 * @param dst
 * @param nBytes
 */
_STATIC_INLINE_ void fill_area_cacheline(void* dst, uint64_t nBytes, uint64_t val64)
{
    pseamldr_debug_assert((nBytes % MOVDIR64_CHUNK_SIZE == 0));
    pseamldr_debug_assert((uint64_t)dst % MOVDIR64_CHUNK_SIZE == 0);

    // create a cache aligned, cache sized chunk and fill it with 'val'
    ALIGN(MOVDIR64_CHUNK_SIZE) uint64_t chunk[MOVDIR64_CHUNK_SIZE / sizeof(uint64_t)];

    for (uint32_t i = 0; i < MOVDIR64_CHUNK_SIZE / sizeof(uint64_t); i++)
    {
        chunk[i] = val64;
    }

    fill_cachelines(dst, (uint8_t*)chunk, nBytes/MOVDIR64_CHUNK_SIZE);
}

/**
 * @brief Zero memory area using movdir64
 *
 * @note nBytes size must be in complete cache lines
 * @note dst address must be aligned to cache line
 *
 * @param dst
 * @param nBytes
 */
_STATIC_INLINE_ void zero_area_cacheline(void* dst, uint64_t nBytes)
{
    fill_area_cacheline(dst, nBytes, 0);
}

_STATIC_INLINE_ void zero_cacheline(void* dst)
{
    fill_area_cacheline(dst, MOVDIR64_CHUNK_SIZE, 0);
}

_STATIC_INLINE_ void pseamldr_memcpy(void * dst, uint64_t dst_bytes, void * src, uint64_t nbytes)
{
    volatile uint64_t junk_a, junk_b;

    pseamldr_sanity_check (dst_bytes >= nbytes, SCEC_HELPERS_SOURCE, 1);

    _ASM_VOLATILE_ ("rep; movsb;"
                    :"=S"(junk_a), "=D"(junk_b)
                    :"c"(nbytes), "S"(src), "D"(dst)
                    :"memory");
}

_STATIC_INLINE_ bool_t pseamldr_memcmp(void * a, void * b, uint64_t nbytes)
{
    ia32_rflags_t rflags;
    uint64_t junk_a, junk_b;
    _ASM_VOLATILE_ ("repe; cmpsb;"
                    "pushfq\n"
                    "popq %0"
                    : "=r"(rflags.raw), "=S"(junk_a), "=D"(junk_b)
                    :"c"(nbytes), "S"(b), "D"(a)
                    :"memory");
    if (rflags.zf == 0)
    {
        return false;
    }
    return true;
}

_STATIC_INLINE_ bool_t pseamldr_memcmp_to_zero(void *a, uint64_t nBytes)
{
    uint64_t i = 0;
    uint8_t* a_8 = (uint8_t*)a;

    for ( ; (i + (uint64_t)sizeof(uint64_t)) <= nBytes; i += (uint64_t)sizeof(uint64_t))
    {
        uint64_t* a_64 = (uint64_t*)(&a_8[i]);

        if (*a_64 != 0)
        {
            return false;
        }
    }

    for ( ; i < nBytes; i++)
    {
        if (a_8[i] != 0)
        {
            return false;
        }

    }

    return true;
}

/**
 * @brief Adds alignment check with power of 2 size to shared_hpa_check
 *
 * @param hpa HPA to be checked
 * @param size Size to be aligned with (MUST be power of 2)
 *
 * @return Error code that states the reason of failure
 */
bool_t shared_hpa_check_with_pwr_2_alignment(pa_t hpa, uint64_t size);

/**
 * @brief Check the HPA for shared access semantics, and maps it to LA in TDX-SEAM module address space.
 *
 * @param hpa HPA to be checked
 * @param size Size to be aligned with (MUST be power of 2)
 * @param mapping_type - If write access is required
 * @param la Returns mapped linear address of the HPA. Should be freed after use.
 *
 * @return Error code that states the reason of failure
 */
api_error_type check_and_map_aligned_shared_hpa(pa_t hpa, uint64_t size, mapping_type_t mapping_type, void** la);

/**
 * @brief Adds an instruction_len offset to Guest RIP of current active VMCS
 *
 * @param instruction_len - If 0 - instruction length will be take from VM-Exit Instruction Length VMCS field
 */
_STATIC_INLINE_ void current_vmcs_guest_rip_advance(uint64_t instruction_len)
{
    uint64_t current_vmm_rip;
    ia32_vmread(VMX_GUEST_RIP_ENCODE, &current_vmm_rip);

    if (instruction_len == 0)
    {
        ia32_vmread(VMX_VM_EXIT_INSTRUCTION_LENGTH_ENCODE, &instruction_len);
    }

    ia32_vmwrite(VMX_GUEST_RIP_ENCODE, current_vmm_rip + instruction_len);
}

/**
 * @brief Only this function should be used when guest RIP is advanced to the next instruction.
 *        Clears STI blocking and MOVSS blocking interruptibility state, and sets PDE.BS
 */
void advance_guest_rip(void);

/**
 * @brief Helper function that determines if range1 is contained inside range2
 * @param addr - Address to be check
 * @param base - Base of range
 * @param size - Size of range
 *
 * @return True or false
 */
bool_t is_addr_in_range(uint64_t addr, uint64_t base, uint64_t size);


/**
 * @brief Helper functions to handle SEAMEXTEND internal structure reads/writes
 */
void seamextend_read(seamextend_t* seamextend_dst);
void seamextend_write(seamextend_t* seamextend_src);

/**
 * @brief Helper functions to get the current LPID number
 */
uint32_t get_current_lpid(void);

/**
 * @brief Safe memory compare function for sensitive data (cryptography)
 */
bool_t safe_memcmp(const void * a, const void * b, uint64_t nbytes);

/**
 * @brief Computes hash and compares it to the expected value
 *
 * @param message - pointer to the message buffer on which the hash will be calculated
 * @param message_len - message size in bytes
 * @param expected_hash - calculated hash will be safely compared to that value
 *
 * @return True or false
 */
bool_t compute_and_verify_hash(const uint8_t* message, uint32_t message_len, const uint8_t* expected_hash);

/**
 * @brief Computes and returns the number of addressable LP's on a single socket in the system
 *
 * @return Number of addressable LP's in a single socket
 */
uint64_t get_num_addressable_lps_on_socket(void);

/**
 * @brief Clear MISC_ENABLE_MSR[22] bit if it is set and then calls CPUID. Sets back MISC_ENABLE_MSR[22] bit before returning 
 */
void ia32_safe_cpuid(uint32_t leaf, uint32_t subleaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx);

#endif /* SRC_COMMON_HELPERS_HELPERS_H_ */
