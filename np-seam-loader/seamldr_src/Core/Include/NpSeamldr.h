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

#ifndef NPSEAMLDR_H
#define NPSEAMLDR_H

#include "NpSeamldrApi.h"
#include "vmcs.h"
#include "msr.h"

#define SEAMLDR_FLAG_BOOT               0
#define SEAMLDR_FLAG_UPDATE             1
#define SEAMLDR_FLAG_NOT_LOADED         0
#define SEAMLDR_FLAG_LOADED             1
#define SEAMLDR_FLAG_LOAD_IN_PRORGRESS  2

#define SP_SEAMLDR_PLAG_READY           1
#define SEAM_EXTEND_SEAM_READY_VAL      0

#define NP_SEAMLDR_MUTEX_CLEAR          0
#define NP_SEAMLDR_MUTEX_ACQUIRED       1

#define P_SYS_INFO_TABLE_VERSION        0
#define IO_SYS_INFO_TABLE_MAX_VERSION   1

// linear address defs
#define C_KEYHOLE_EDIT_REGION_BASE      0x0000000100000000
#define C_CODE_RGN_BASE                 0xFFFF800000000000
#define C_STACK_RGN_BASE                0xFFFF800100000000
#define C_KEYHOLE_RGN_BASE              0xFFFF800200000000
#define C_DATA_RGN_BASE                 0xFFFF800300000000
#define C_SYS_INFO_TABLE_BASE           0xFFFF8003FFFF0000
#define C_IO_SYS_INFO_TABLE_BASE        0xFFFF8003FFFF1000
#define C_MODULE_RGN_BASE               0x0000000200000000
#define CODE_REGION_ALLOCATED_SIZE      _2MB
#define P_SEAMLDR_SHADOW_STACK_SIZE     _4KB
#define C_VMCS_REGION_SIZE              _8KB        // including the reserved unmapped page

#ifdef _TDXIO_SUPPORT
#define C_P_SYS_INFO_TABLE_SIZE         _12KB
#define C_P_IO_SYS_INF_TABLE_SIZE       _8KB
#else
#define C_P_SYS_INFO_TABLE_SIZE         _4KB
#endif

#define PAGING_STRUCTURE_SIZE(rgnSize) (((((((rgnSize)/_4KB) * 8) + _4KB - 1) / _4KB) + \
                                        ((((rgnSize) / _2MB) * 8) + _4KB - 1) / _4KB + \
                                        ((((rgnSize) / _1GB) * 8) + _4KB - 1) / _4KB) * _4KB )

#define NON_CANONICAL_RIP               0x1000000000000000ULL

#define CANONICITY_MASK_4LP (~(BIT47 - 1))
#define CANONICITY_MASK_5LP (~(BIT56 - 1))

#define SEAM_EXTEND_VALID_INTEL         0x1FF
#define SEAM_EXTEND_VALID_OEM           0xFFFF  

#define MOVDIR64B_BLOCK_SIZE            64

#define CPUID_FM_MASK                   0xFFFFFFF0

#define SEAMRR_PAGE_SIZE     _4KB

#define ALIGN(x,a)              __ALIGN_MASK(x,(a)-1)
#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))

typedef struct {
    UINT64           SeamrrVa;
    UINT64           SeamrrVaLimit;
    UINT64           AslrRand;    
    UINT64           MaximumSupportMemAddress;
    UINT64           TdxPrivateKidMask;
    UINT64           CStackRgnSize;
    UINT64           CKeyholeRgnSize;
    UINT64           CKeyholeEditRgnSize;
    UINT64           SeamrrBase;
    UINT64           SeamrrSize;
    P_SYS_INFO_TABLE_t* PSysInfoTable;
    __declspec(align(256)) SEAM_EXTEND_t SeamExtend;

    P_SEAMLDR_CONSTS_t* PSeamldrConsts;
} SeamldrData_t;

BOOL SeamldrParamsAddressValid(UINTN SeamldrParamsPa);
BOOL SeamldrParamsValid();
BOOL SafeCompare(UINT8* a, UINT8* b, UINT64 size);
void MemZeroWithMovdir64B(UINT8* dst, UINT64 size);
UINT64 VerifyManifest(P_SYS_INFO_TABLE_t* SysInfoTable, UINT32 Scenario, BOOL IsAcmProductionSigned);

extern SeamldrData_t SeamldrData;
extern UINT8  VmcsBuffer[VMCS_SIZE];
extern UINT8  TempGdt[PAGE4K];

UINT64 memcpy_s(void* dest, UINTN destSize, const void* src, UINTN count);

#endif //NPSEAMLDR_H