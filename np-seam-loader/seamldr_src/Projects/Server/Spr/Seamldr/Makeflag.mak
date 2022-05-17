#;**********************************************************************;
#;*                                                                    *;
#;* Intel Proprietary                                                  *;
#;*                                                                    *;
#;* Copyright 2021 Intel Corporation All Rights Reserved.              *;
#;*                                                                    *;
#;* Your use of this software is governed by the TDX Source Code       *;
#;* LIMITED USE LICENSE.                                               *;
#;*                                                                    *;
#;* The Materials are provided "as is," without any express or         *;
#;* implied warranty of any kind including warranties of               *;
#;* merchantability, non-infringement, title, or fitness for a         *;
#;* particular purpose.                                                *;
#;*                                                                    *;
#;**********************************************************************;

#*****************************************************************************
#
# Build control flags.
#
#*****************************************************************************
SIMICS_BLD = 0
!if "$(SEAMLDR_SIMICS_BUILD)"!=""
SIMICS_BLD=$(SEAMLDR_SIMICS_BUILD)
!endif

MKF_LTSX_MODE=0

MKF_USB_TRACE=0

MKF_64BIT_CODE_SUPPORT=1

MKF_USE_MAKEFLAG_H=1

TDX_IO_FLAG=
!if "$(NP_SEAMLDR_TDXIO_SUPPORT)"!=""
TDX_IO_FLAG=/D_TDXIO_SUPPORT=1
!endif

MKF_FLAGS =  $(MKF_FLAGS) /DMKF_USE_MAKEFLAG_H=$(MKF_USE_MAKEFLAG_H) 

ACM_PE2BINx_FLAGS = $(ACM_PE2BINx_FLAGS) #-AcmSize:128KB

#----------------------------------------------------------------------------
# Note : Enabling S-ACM Benchmarking for a platform which have TXT enabled
#        will cause failure due to MTRR 8 usage. Early IBB must consume
#        benchmarking info and deallocate MTRR8.
#----------------------------------------------------------------------------
MKF_BENCHMARKING=0

#-----------------------------------------------------------------------------
# TPM selection controls.
# To enable Set selection to 1
# To disable set to 0
#-----------------------------------------------------------------------------

MKF_CAP_TPM_FAM_20=0
MKF_CAP_TPM_FAM_12=0
MKF_CAP_TPM_ITF_CRB=0
MKF_CAP_TPM_ITF_FIFO=0

#-----------------------------------------------------------------------------
# LCP elements selection controls. Must be defined in preprocessing as well
# as in code.
#-----------------------------------------------------------------------------

MKF_LCP_POLELT_TYPE_MLE_SUPPORTED=0
MKF_LCP_POLELT_TYPE_PCONF_SUPPORTED=0
MKF_LCP_POLELT_TYPE_SBIOS_SUPPORTED=0
MKF_LCP_POLELT_TYPE_STM_SUPPORTED=0

LCP_ELT_SELECTION=0
!if "$(MKF_LCP_POLELT_TYPE_MLE_SUPPORTED)"=="1"
LCP_ELT_SELECTION=$(LCP_ELT_SELECTION)+LCP_POLELT_TYPE_MLE_SUPPORTED
!endif
!if "$(MKF_LCP_POLELT_TYPE_PCONF_SUPPORTED)"=="1"
LCP_ELT_SELECTION=$(LCP_ELT_SELECTION)+LCP_POLELT_TYPE_PCONF_SUPPORTED
!endif
!if "$(MKF_LCP_POLELT_TYPE_SBIOS_SUPPORTED)"=="1"
LCP_ELT_SELECTION=$(LCP_ELT_SELECTION)+LCP_POLELT_TYPE_SBIOS_SUPPORTED
!endif
!if "$(MKF_LCP_POLELT_TYPE_STM_SUPPORTED)"=="1"
LCP_ELT_SELECTION=$(LCP_ELT_SELECTION)+LCP_POLELT_TYPE_STM_SUPPORTED
!endif

#-----------------------------------------------------------------------------
# SW supported crypto algorithm selection.
#-----------------------------------------------------------------------------
MF_ALG_SHA1_PRESENT=0
MF_ALG_SHA256_PRESENT=0
MF_ALG_SHA384_PRESENT=0
MF_ALG_SHA512_PRESENT=0
MF_ALG_SM3_256_PRESENT=0
MF_ALG_RSASSA_PRESENT=0
MF_ALG_ECDSA_PRESENT=0
MF_ALG_SM2_PRESENT=0

MKF_USE_INTERNAL_HASH=0

#-----------------------------------------------------------------------------
# Module and platform types.
#
#-----------------------------------------------------------------------------

ACM_TYPE=AC_SINIT

PLATFORM_TYPE=SERVER

#-----------------------------------------------------------------------------
# Use of SCLEAN function
#
#-----------------------------------------------------------------------------
MKF_SCLEAN_SUPPORT=0

#-----------------------------------------------------------------------------
# Error handler defined in core or assigned by project via reference variable
#
#-----------------------------------------------------------------------------
MF_PROJECT_DEFINED_ERROR_HANDLER=1


#*****************************************************************************
#
# Module control parameters - most frequently changing.
#
#*****************************************************************************

# Sighting #3864887  (0KB .text alignment)
TEXT_ALIGNMENT = 0KB

MODULE_FLAGS=MF_PROD_WORTHY

ACM_VERSION=0x00
!ifndef ACM_REVISION
ACM_REVISION=<1,7,4>
!endif

!if "$(BLD)"=="ENG_NT"
!if "$(SFX)"=="OD"
STACK_SIZE=0x10000
!else
STACK_SIZE=0x10000
!endif
!elseif "$(BLD)"=="ENG_TR"
!if "$(SFX)"=="OD"
STACK_SIZE=0x10000
!else
STACK_SIZE=0x10000
!endif
!elseif "$(BLD)"=="REL_NT"
STACK_SIZE=0x10000
!endif


#*****************************************************************************
#
# makeflah.h header file is automatically generated.
# Edit text below to change header values and other infrequently changing
# parameters. It is expected to be set only once when project is started.
#
#*****************************************************************************
generate_makefile_header_file:
  @echo > nul <<$(BLD_DIR)\Makeflag.auto
//**********************************************************************;
//*                                                                    *;
//* INTEL CONFIDENTIAL                                                 *;
//* Copyright 2006-2018 Intel Corporation All Rights Reserved.         *;
//*                                                                    *;
//* The source code contained or described herein and all documents    *;
//* related to the source code ("Material") are owned by Intel         *;
//* Corporation or its suppliers or licensors. Title to the Material   *;
//* remains with Intel Corporation or its suppliers and licensors.     *;
//* The Material contains trade secrets and proprietary and            *;
//* confidential information of Intel or its suppliers and licensors.  *;
//* The Material is protected by worldwide copyright and trade secret  *;
//* laws and treaty provisions. No part of the Material may be used,   *;
//* copied, reproduced, modified, published, uploaded, posted,         *;
//* transmitted, distributed, or disclosed in any way without          *;
//* Intel's prior express written permission.                          *;
//*                                                                    *;
//* No license under any patent, copyright, trade secret or other      *;
//* intellectual property right is granted to or conferred upon you    *;
//* by disclosure or delivery of the Materials, either expressly,      *;
//* by implication, inducement, estoppel or otherwise. Any license     *;
//* under such intellectual property rights must be express and        *;
//* approved by Intel in writing.                                      *;
//*                                                                    *;
//**********************************************************************;
//**********************************************************************;
//              !!!AUTO GENERATED FILE - DON'T EDIT!!!
//              !!!   EDIT MAKEFLAG.MAK INSTEAD    !!!
//**********************************************************************;
#ifndef MAKEFLAG_H
#define MAKEFLAG_H
#define MKF_ENTRY_DEAD_LOOP         0
//
// Platform type
//
#define MCP_PLATFORM_TYPE           $(PLATFORM_TYPE)
//
// Module flags control production/debug and PW/NPW module
//
#define MCP_MODULE_FLAGS            $(MODULE_FLAGS)
//
// Module version
//
#define MCP_ACM_VER                 $(ACM_VERSION)
#define MCP_ACM_REVISION            $(ACM_REVISION)
//
// Module type
//
#define MCP_MODULE_TYPE             CS_MODULE_TYPE
//
// Module sub-type
//
#define MCP_MODULE_SUBTYPE          CLIENT_MODULE_SUBTYPE   // 0
//-----------------------------------------------------------------------------
// Header parameters - established once in the beginning of project
//-----------------------------------------------------------------------------
// Module ID
//
#define MCP_MODULE_ID                   0       // no chipset
//
// Module type - SINIT or BIOS AC.
//
#define MCP_CHIPSET_ACM_TYPE        $(ACM_TYPE)
//
// AC Info table version
//
#define ACM_INFO_TAB_VER            0x7
//
// OS SINIT table version
//
#define OS_SINIT_TAB_VER            0
//
// Module capabilities.
//
#define MCP_ACM_CAPABILITIES        0   // no SINIT capabilities

//
// Security Version Number
//
#define MCP_ACM_SVN                 0
#define MCP_SE_SVN                  3    // Update to >=1 for PV and post PV releases

#define LCP_ELT_SELECTION           $(LCP_ELT_SELECTION)
#define MKF_LCP_POLELT_TYPE_MLE_SUPPORTED       $(MKF_LCP_POLELT_TYPE_MLE_SUPPORTED)
#define MKF_LCP_POLELT_TYPE_PCONF_SUPPORTED     $(MKF_LCP_POLELT_TYPE_PCONF_SUPPORTED)
#define MKF_LCP_POLELT_TYPE_SBIOS_SUPPORTED     $(MKF_LCP_POLELT_TYPE_SBIOS_SUPPORTED)
#define MKF_LCP_POLELT_TYPE_STM_SUPPORTED       $(MKF_LCP_POLELT_TYPE_STM_SUPPORTED)

//
// TXT Error register
//
#define TXT_ERROR_REGISTER          LT_PRV_BASE + offsetof (TXT, LT_BIOSACMCode)

//
// TXT Error handler control
//
#define NO_CORE_ERRORHANDLE        $(MF_PROJECT_DEFINED_ERROR_HANDLER)

//
// LTSX / Client TXT Flag
//
#define LTSX_SERVER_TXT             SERVER_TXT_FUSED

//
// PCH LT device ID equates
//
#define   ICL_PCH_DID               0xB00A
//
// Module chipset ID structure fields
//
// ICL PCH
#define   MF_CS_PRESENT_1           1
#define   MF_REVID_MASK_1           BIT0
#define   MF_DEVICE_ID_1            ICL_PCH_DID
#define   MF_REVISION_ID_1          0x1

//
// Processor Family/Model/Stepping and Platform ID Equates
//
#define   SPR_FMS                   0x806F0
#define   FMS_MASK                  0xFFF3FF0
#define   PLATFORM_ID               0               // IA32_PLATFORM_ID definition changed in BWG,
#define   PLATFORM_ID_MASK          0x001C000000000000




//
// Processor Family/Model/Stepping and Platform ID structure fields
//
#define     MF_PROC_PRESENT_1       1
#define     MF_FMS_1                SPR_FMS
#define     MF_FMS_MASK_1           FMS_MASK
#define     PLATFORM_ID_1           0x0000000000000000
#define     PLATFORM_ID_MASK_1      0x0000000000000000

#define     MF_PROC_PRESENT_2       0
#define     MF_FMS_2                0
#define     MF_FMS_MASK_2           0
#define     PLATFORM_ID_2           0
#define     PLATFORM_ID_MASK_2      0

#define     MF_PROC_PRESENT_3       0
#define     MF_FMS_3                0
#define     MF_FMS_MASK_3           0
#define     PLATFORM_ID_3           0
#define     PLATFORM_ID_MASK_3      0

#define     MF_PROC_PRESENT_4       0
#define     MF_FMS_4                0
#define     MF_FMS_MASK_4           0
#define     PLATFORM_ID_4           0
#define     PLATFORM_ID_MASK_4      0

//
// TPM family and interface support
//
#define MKF_CAP_TPM_FAM_20          $(MKF_CAP_TPM_FAM_20)
#define MKF_CAP_TPM_FAM_12          $(MKF_CAP_TPM_FAM_12)
#define MKF_CAP_TPM_ITF_CRB         $(MKF_CAP_TPM_ITF_CRB)
#define MKF_CAP_TPM_ITF_FIFO        $(MKF_CAP_TPM_ITF_FIFO)

//
// TPM index numbers
//
//#define TPM20_INDEX_AUX             0x1C10102
//#define TPM20_INDEX_LCP_SUP         0x1C10103
//#define TPM20_INDEX_LCP_OWN         0x1C10106

//
// TPM_CAPABILITIES
//
#define    MCP_ACM_TPM_CAPABILITIES 0
//
// TPM crypto support
//
#define MF_ALG_SHA1_PRESENT         $(MF_ALG_SHA1_PRESENT)
#define MF_ALG_SHA256_PRESENT       $(MF_ALG_SHA256_PRESENT)
#define MF_ALG_SHA384_PRESENT       $(MF_ALG_SHA384_PRESENT)
#define MF_ALG_SHA512_PRESENT       $(MF_ALG_SHA512_PRESENT)
#define MF_ALG_SM3_256_PRESENT      $(MF_ALG_SM3_256_PRESENT)
#define MF_ALG_RSASSA_PRESENT       $(MF_ALG_RSASSA_PRESENT)
#define MF_ALG_ECDSA_PRESENT        $(MF_ALG_ECDSA_PRESENT)
#define MF_ALG_SM2_PRESENT          $(MF_ALG_SM2_PRESENT)
#define MKF_USE_INTERNAL_HASH       $(MKF_USE_INTERNAL_HASH)

//
// Project specific sizes
//
#define HI_SMRR_MASK                0
#define HI_SMRR2_MASK               0

//
// Misc controls
//
#define MKF_LTSX_MODE               $(MKF_LTSX_MODE)

#define MKF_USB_TRACE               $(MKF_USB_TRACE)

#define SIMICS_BLD                  $(SIMICS_BLD)

#if $(MKF_SCLEAN_SUPPORT)
#define MF_SCLEAN_SUPPORT
#endif

#if !defined(MKF_BENCHMARKING)
#define MKF_BENCHMARKING            $(MKF_BENCHMARKING)
#endif

//-----------------------------------------------------------------------------
// Stack size
//-----------------------------------------------------------------------------
#define STACK_SIZE                  $(STACK_SIZE)
#endif
<<KEEP


