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

#ifndef COMMON64_H
#define COMMON64_H
#pragma pack (1)
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//                          USAGE REQUIREMENTS
// Only general-purpose definitions can be placed in this file,
// free from chipset etc. dependencies.
//
// All definitions must be self-contained, not dependent on information from
// other include files.
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma data_seg("EDATA32")
#pragma bss_seg("EDATA32")
#pragma const_seg("EDATA32")

#define UINT8  unsigned __int8          // char
#define UINT16 unsigned __int16         // short int
#define UINT32 unsigned __int32         // long
#define UINT64 unsigned __int64         // long long


//-----------------------------------------------------------------------------
// Leaf value (in eax) calling GETSEC
//-----------------------------------------------------------------------------
#define CAPABILITIES		0
#define ENTERACCS				2
#define EXITAC				  3
#define SENTER				  4
#define SEXIT				    5
#define PARAMETERS			6
#define SMCTRL					7
#define WAKEUP					8

typedef struct {
  UINT16  Limit;
  UINT64  Base;
} IDTR;        

#define ZMM_REG_QWORDS 8
#define YMM_REG_QWORDS 4

typedef struct {  
  UINT64 OriginalR8;
  UINT64 OriginalR9;
  UINT64 OriginalR10;
  UINT64 OriginalR11;
  UINT64 OriginalR12;
  UINT64 OriginalCR3;
  UINT32 OriginalCR4;
  UINT16 OriginalES;
  UINT16 OriginalFS;
  UINT16 OriginalGS;
  UINT16 OriginalSS;
  UINT32 OriginalECX;
  UINT16 OriginalIDTRLimit;
  IDTR   NewIDTR;
  UINT8  OriginalGdtr[10];
  UINT64 ResumeRip;
  UINT64 PtCtxPtr;
  UINT64 RetVal;
  UINT64 HeaderStart;
  UINT32 PseamldrSize;
  UINT64 PseamldrOffset;
  UINT64 PseamldrConstsOffset;
  UINT8  NewGdtr[10];
} SEAMLDR_COM64_DATA;


//
// 64-bit error codes are displayed with progress code SCHECK - see
// file SinitCom.h
//
enum ENUM_ALIAS_CHECK {
  ERR_INTERRUPT64 = 1,
  ERR_MAPPING64,
  ERR_ALIAS,
  ERR_GTT_CFG,
  ERR_NOT_DRAM,
  ERR_RD_WR_CNT_MISMATCH,
  ERR_CONFIG_DIGEST =0x10,
};

#define LT_SINIT_BASE 0xFED20270
#define LT_SINIT_SIZE 0xFED20278

#define REQ_CSE_WAKE  1

// Return status
#define CSE_ACTIVE    0
#define REQ_SUCCESS   1



#define LT_SINIT_BASE 0xFED20270
#define LT_SINIT_SIZE 0xFED20278


extern  SEAMLDR_COM64_DATA SeamldrCom64Data;

void  Entry64 ();  
void  SeamldrCom64(SEAMLDR_COM64_DATA *);
void Init64bitComArea();

#define TXT_SHUTDOWN()             __ud2();
#define PauseCpu _mm_pause


#pragma pack ()
#endif


