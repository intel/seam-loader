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
#include <header.h>

GDT GdtBasePtr = {
  {0,0,0,0,0,0,0},
  {0xFFFF,0,0,0,0x9B,0xCF,0},
  {0xFFFF,0,0,0,0x93,0xCF,0},
  {0xFFFF,0,0,0,0x9B,0xAF,0}
};


