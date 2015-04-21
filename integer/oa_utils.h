
/*--------------------------------------------------------------------*/
/*--- Cojac-grind numerical problem sniffer.            oa_utils.h ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of Cojac-grind, which watches arithmetic operations to
   detect overflows, cancellation, smearing, and other suspicious phenomena.

   Copyright (C) 2011-2011 Frederic Bapst
      frederic.bapst@gmail.com

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.

   The GNU General Public License is contained in the file COPYING.
 */



#ifndef __OA_UTILS_H
#define __OA_UTILS_H

#include "pub_tool_basics.h"

/* Bitwise type coercion - Useful typedefs */

typedef union {
  UChar u;
  Char  s;
} oa_mix8_t;

typedef union {
  UShort u;
  Short  s;
  struct {
    oa_mix8_t h2;
    oa_mix8_t h1;
  } halves;
} oa_mix16_t;

typedef union {
  UInt  u;
  Int   s;
  Float f;
  struct {
    oa_mix16_t h2;
    oa_mix16_t h1;
  } halves;
} oa_mix32_t;

typedef union {
  ULong   u;
  Long    s;
  Double  f;
  struct {
    oa_mix32_t h2;
    oa_mix32_t h1;
  } halves;
} oa_mix64_t;

/*--------------------------------------------------------------------*/

UInt   OA_(uintFromInt) (Int s);
Float  OA_(floatFromInt)(Int i);
Short  OA_(shortFromInt)(Int i);
double OA_(doubleFromTwoInts)(Int a, Int b);
void   OA_(longToTwoInts)(ULong d, Int *a, Int *b);
double OA_(doubleFromULong)(ULong l);
ULong  OA_(ulongFromTwoInts)(Int a, Int b);

#endif /* ndef __OA_UTILS_H */

/*--------------------------------------------------------------------*/
/*--- end                                                          ---*/
/*--------------------------------------------------------------------*/
